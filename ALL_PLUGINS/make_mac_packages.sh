#!/usr/bin/env bash
# =============================================================================
#  DJBILBOX BEATS - macOS: one .pkg installer + one ZIP per product (+ bundles)
#  Input : <stage>/{synths,loadout,orbit}/<Name>.vst3 and <Name>.component  (+ <stage>/docs/*)
#  Output: <out>/packages/<slug>-macOS.zip   (setup .pkg + README + EULA + notices + manual)
#          <out>/DJBILBOX_BEATS_Complete_Collection_macOS.dmg
#          <out>/packages_report.txt
#  Unsigned / not notarized: the installer shows the Gatekeeper warning on first open
#  (see README: right-click > Open, or System Settings > Privacy & Security > Open Anyway).
#  Usage : make_mac_packages.sh <stage> <out> [version]
# =============================================================================
set -euo pipefail

STAGE="$(cd "$1" && pwd)"
mkdir -p "$2"
OUT="$(cd "$2" && pwd)"
VERSION="${3:-1.0.0}"
DOCS="$STAGE/docs"
EULA="$DOCS/EULA_DJBILBOX_BEATS.txt"
NOTICES="$DOCS/THIRD_PARTY_NOTICES.txt"
[ -f "$EULA" ] || { echo "EULA missing: $EULA"; exit 1; }

mkdir -p "$OUT/packages" "$OUT/work"
REPORT="$OUT/packages_report.txt"
: > "$REPORT"

slug() { echo "$1" | tr '[:upper:]' '[:lower:]' | sed -E 's/[^a-z0-9]+/-/g; s/^-+//; s/-+$//'; }

# ---- collect "group:Name" items (a product = a .vst3 and/or .component with the same name)
ITEMS_FILE="$OUT/work/items.txt"
: > "$ITEMS_FILE"
for g in synths loadout orbit chaabi; do
    [ -d "$STAGE/$g" ] || continue
    for b in "$STAGE/$g"/*.vst3 "$STAGE/$g"/*.component; do
        [ -e "$b" ] || continue
        n="$(basename "$b")"; n="${n%.vst3}"; n="${n%.component}"
        echo "$g:$n"
    done | sort -u >> "$ITEMS_FILE"
done
sort -u "$ITEMS_FILE" -o "$ITEMS_FILE"
[ -s "$ITEMS_FILE" ] || { echo "No plugin found in $STAGE"; exit 1; }

build_pkg() {  # <AppName> <slug> <blurb> <group:Name>...
    local APP="$1" SLUG="$2" BLURB="$3"; shift 3
    local W="$OUT/work/$SLUG"
    local ROOT="$W/root"
    local VST3="$ROOT/Library/Audio/Plug-Ins/VST3"
    local COMP="$ROOT/Library/Audio/Plug-Ins/Components"
    local DOCDIR="$ROOT/Library/Application Support/DJBILBOX BEATS/Documentation"
    rm -rf "$W"
    mkdir -p "$VST3" "$COMP" "$DOCDIR" "$W/res" "$W/zip"

    local it g n
    for it in "$@"; do
        g="${it%%:*}"; n="${it#*:}"
        [ -d "$STAGE/$g/$n.vst3" ] && ditto "$STAGE/$g/$n.vst3" "$VST3/$n.vst3"
        [ -d "$STAGE/$g/$n.component" ] && ditto "$STAGE/$g/$n.component" "$COMP/$n.component"
    done
    rmdir "$VST3" "$COMP" 2>/dev/null || true   # drop empty format folders

    cp "$EULA" "$DOCDIR/"
    [ -f "$NOTICES" ] && cp "$NOTICES" "$DOCDIR/"
    [ -f "$DOCS/manuals/$SLUG.pdf" ] && cp "$DOCS/manuals/$SLUG.pdf" "$DOCDIR/"

    # components must never be "relocated" by the Installer to a stale build folder
    pkgbuild --analyze --root "$ROOT" "$W/components.plist" >/dev/null
    python3 - "$W/components.plist" <<'PY'
import plistlib, sys
p = sys.argv[1]
d = plistlib.load(open(p, 'rb'))
for e in d:
    e['BundleIsRelocatable'] = False
    e['BundleOverwriteAction'] = 'upgrade'
plistlib.dump(d, open(p, 'wb'))
PY
    pkgbuild --root "$ROOT" --component-plist "$W/components.plist" \
             --identifier "com.djbilbox.$SLUG" --version "$VERSION" \
             --install-location / "$W/component.pkg" >/dev/null

    cp "$EULA" "$W/res/EULA.txt"
    cat > "$W/dist.xml" <<XML
<?xml version="1.0" encoding="utf-8"?>
<installer-gui-script minSpecVersion="2">
    <title>$APP</title>
    <license file="EULA.txt" mime-type="text/plain"/>
    <options customize="never" require-scripts="false" hostArchitectures="arm64,x86_64"/>
    <domains enable_anywhere="false" enable_currentUserHome="false" enable_localSystem="true"/>
    <choices-outline><line choice="default"><line choice="com.djbilbox.$SLUG"/></line></choices-outline>
    <choice id="default"/>
    <choice id="com.djbilbox.$SLUG" visible="false"><pkg-ref id="com.djbilbox.$SLUG"/></choice>
    <pkg-ref id="com.djbilbox.$SLUG" version="$VERSION" onConclusion="none">component.pkg</pkg-ref>
</installer-gui-script>
XML
    productbuild --distribution "$W/dist.xml" --resources "$W/res" --package-path "$W" \
                 "$W/zip/$SLUG-setup-macOS.pkg" >/dev/null

    cp "$EULA" "$W/zip/"
    [ -f "$NOTICES" ] && cp "$NOTICES" "$W/zip/"
    [ -f "$DOCS/manuals/$SLUG.pdf" ] && cp "$DOCS/manuals/$SLUG.pdf" "$W/zip/"
    cat > "$W/zip/README.txt" <<TXT
$APP  -  version $VERSION  (macOS)
$BLURB

INSTALL
1. Close your DAW (Logic Pro, Ableton Live, FL Studio, Reaper...).
2. Open $SLUG-setup-macOS.pkg and follow the steps (administrator password required).
   This installer is not notarized by Apple. If macOS says it "cannot be opened":
     - right-click the .pkg > Open > Open, or
     - System Settings > Privacy & Security > scroll down > "Open Anyway".
3. Plugins are installed in:
     /Library/Audio/Plug-Ins/VST3         (VST3)
     /Library/Audio/Plug-Ins/Components   (Audio Units - Logic Pro / GarageBand)
   Manual and license: /Library/Application Support/DJBILBOX BEATS/Documentation
4. Rescan plugins in your DAW. Logic Pro: a restart may be needed for new Audio Units.
5. Supported: macOS 11 or later, Apple Silicon and Intel (universal binaries).

LICENSE
Everything you create with this plugin is yours and royalty-free.
Read EULA_DJBILBOX_BEATS.txt for the full terms.

SUPPORT: djbilboxbeats.com
TXT
    rm -f "$OUT/packages/$SLUG-macOS.zip"
    ( cd "$W/zip" && zip -qr "$OUT/packages/$SLUG-macOS.zip" . )
    echo "$SLUG-macOS.zip  ($APP, $# plugin(s))" >> "$REPORT"
    echo "OK  $SLUG"
}

# ---- single products
while IFS= read -r line; do
    n="${line#*:}"
    build_pkg "$n" "$(slug "$n")" "Audio plugin by DJBILBOX BEATS." "$line" || echo "FAILED $n" | tee -a "$REPORT"
done < "$ITEMS_FILE"

# ---- bundles
group_items() { grep "^$1:" "$ITEMS_FILE" || true; }
bundle() {  # <AppName> <slug> <group|all> <blurb>
    local list=()
    local line
    if [ "$3" = "all" ]; then
        while IFS= read -r line; do list+=("$line"); done < "$ITEMS_FILE"
    else
        while IFS= read -r line; do [ -n "$line" ] && list+=("$line"); done < <(group_items "$3")
    fi
    if [ "${#list[@]}" -gt 1 ]; then
        build_pkg "$1" "$2" "$4" "${list[@]}" || echo "FAILED $2" | tee -a "$REPORT"
    fi
}
bundle "HUMPIRE LOADOUT" humpire-loadout loadout "Eight studio effects: EQ, compressor, enhancer, limiter, reverb, delay, lo-fi and performance FX."
bundle "HUMPIRE ORBIT" humpire-orbit orbit "Vocal effects bundle: pitch correction, harmonies, doubling and more."
bundle "DJBILBOX BEATS Synth Collection" djbilbox-beats-synth-collection synths "The complete DJBILBOX BEATS synthesizer collection."
bundle "DJBILBOX BEATS Complete Collection" djbilbox-beats-complete-collection all "Every DJBILBOX BEATS and HUMPIRE plugin of this release."

# ---- all-in-one disk image (the Complete Collection installer + docs)
ALL="$OUT/work/djbilbox-beats-complete-collection/zip"
if [ -d "$ALL" ]; then
    hdiutil create -quiet -volname "DJBILBOX BEATS Complete Collection" -srcfolder "$ALL" -ov -format UDZO \
        "$OUT/DJBILBOX_BEATS_Complete_Collection_macOS.dmg" || echo "dmg FAILED" | tee -a "$REPORT"
fi
cat "$REPORT"
