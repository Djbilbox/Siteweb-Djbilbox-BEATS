#!/usr/bin/env bash
# render.sh - headless UI snapshot of a camel-synth plugin editor (for product screenshots).
#
#   render.sh <ProjectDir> <out.png> [presetIndex]
#
#   <ProjectDir>   folder of the repo root, e.g. ARENA_BASS, K2000, WAZONE808
#                  (the CMake project name also works: WARZONE808 -> WAZONE808)
#   <out.png>      output file (relative paths are relative to the current directory)
#   [presetIndex]  factory preset to apply before the editor is created (default 0)
#
# What it does: reads the project's CMakeLists.txt + Source/PluginProcessor.h/PluginEditor.h to
# auto-detect the processor/editor class names (and C++ namespace), the preset-apply method, the
# editor default size, the .cpp files, include dirs, compile definitions (JUCE_WEB_BROWSER=0 ...)
# and the juce_add_binary_data() assets, then builds snapshot_main.cpp against the project's real
# PluginProcessor.cpp/PluginEditor.cpp in /tmp/claude-0/snap_build/<ProjectDir> (ninja -j2)
# and runs it. The project's sources are never modified.
#
# Environment (all optional):
#   SNAP_SCALE=1.5        pixel scale of the PNG (editor 1000x746 -> 1500x1119)
#   SNAP_SIZE=WxH         force the editor size instead of the editor's own default
#   SNAP_SETTLE_MS=150    message-loop time before the pixels are grabbed
#   SNAP_KEEP_RESIZER=1   keep the host-resize grip in the bottom-right corner (hidden by default)
#   SNAP_JOBS=2           build parallelism
#   SNAP_BUILD_ROOT=/tmp/claude-0/snap_build
#   JUCE_DIR=/tmp/claude-0/juce      JUCE 8 checkout used for the build
#   SNAP_REBUILD=1        wipe the project's build dir first
#   SNAP_VERBOSE=1        stream the cmake/ninja output
#   SNAP_PROC_CLASS / SNAP_EDITOR_CLASS / SNAP_APPLY_METHOD   override a wrong auto-detection
#                         (class names fully qualified, e.g. Wz::WarzoneProcessor)

set -euo pipefail

usage() { sed -n '2,12p' "$0" | sed 's/^# \{0,1\}//'; exit 1; }
die()   { echo "render.sh: error: $*" >&2; exit 1; }

[[ $# -ge 2 && $# -le 3 ]] || usage
PROJ_ARG=${1%/}
OUT=$2
PRESET=${3:-0}
[[ $PRESET =~ ^[0-9]+$ ]] || die "presetIndex must be a non-negative integer (got '$PRESET')"

HERE=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
REPO=$(cd "$HERE/../../.." && pwd)
JUCE_DIR=${JUCE_DIR:-/tmp/claude-0/juce}
BUILD_ROOT=${SNAP_BUILD_ROOT:-/tmp/claude-0/snap_build}
JOBS=${SNAP_JOBS:-2}

case $OUT in /*) ;; *) OUT=$PWD/$OUT ;; esac
[[ -f $JUCE_DIR/CMakeLists.txt ]] || die "JUCE not found at $JUCE_DIR (set JUCE_DIR=<JUCE 8 checkout>)"
command -v cmake >/dev/null || die "cmake not found"
command -v python3 >/dev/null || die "python3 not found"

# ---- locate the project directory ------------------------------------------------------------
PROJ_DIR=
if [[ -f $REPO/$PROJ_ARG/CMakeLists.txt ]]; then
    PROJ_DIR=$REPO/$PROJ_ARG
else
    # accept the CMake project name too (folder WAZONE808 builds the target WARZONE808)
    for c in "$REPO"/*/CMakeLists.txt; do
        if grep -qiE "^[[:space:]]*project[[:space:]]*\([[:space:]]*${PROJ_ARG}([[:space:]]|\))" "$c"; then
            PROJ_DIR=$(dirname "$c"); break
        fi
    done
fi
[[ -n $PROJ_DIR ]] || die "no project '$PROJ_ARG' under $REPO"
PROJ_NAME=$(basename "$PROJ_DIR")
[[ -f $PROJ_DIR/Source/PluginProcessor.h && -f $PROJ_DIR/Source/PluginEditor.h ]] \
    || die "$PROJ_NAME does not look like a camel-template synth (Source/PluginProcessor.h / PluginEditor.h missing)"

BUILD=$BUILD_ROOT/$PROJ_NAME
[[ ${SNAP_REBUILD:-0} == 1 ]] && rm -rf "$BUILD"
mkdir -p "$BUILD" "$(dirname "$OUT")"
CFG=$BUILD/snap_config.cmake

# ---- 1. auto-detect everything project specific -> snap_config.cmake ---------------------------
detect() { python3 -I - "$@" <<'PY'
import glob, os, re, sys

proj, build, cfg_path = os.path.abspath(sys.argv[1]), sys.argv[2], sys.argv[3]
warns, env = [], os.environ

def die(msg):
    print("render.sh: error: " + msg, file=sys.stderr); sys.exit(1)
def rel(p):  return os.path.relpath(p, proj)
def path(p): return os.path.normpath(p if os.path.isabs(p) else os.path.join(proj, p))
def read(p):
    with open(p, encoding="utf-8", errors="replace") as f: return f.read()

def blank_cpp(t):
    """Blank out comments and string/char literals but keep every offset and newline."""
    out, i, n = [], 0, len(t)
    while i < n:
        c, d = t[i], (t[i + 1] if i + 1 < n else "")
        if c == "/" and d == "/":
            j = t.find("\n", i); j = n if j < 0 else j
            out.append(" " * (j - i)); i = j
        elif c == "/" and d == "*":
            j = t.find("*/", i + 2); j = n if j < 0 else j + 2
            out.append(re.sub(r"[^\n]", " ", t[i:j])); i = j
        elif c in "\"'":
            j = i + 1
            while j < n and t[j] != c and t[j] != "\n":
                j += 2 if t[j] == "\\" else 1
            j = min(j + 1, n)
            seg = t[i:j]
            out.append(c + re.sub(r"[^\n]", " ", seg[1:-1]) + (c if len(seg) > 1 else "")); i = j
        else:
            out.append(c); i += 1
    return "".join(out)

# ------------------------------------------------------------------ CMake parsing
vars_ = {k: [proj] for k in ("CMAKE_CURRENT_SOURCE_DIR", "PROJECT_SOURCE_DIR", "CMAKE_SOURCE_DIR", "CMAKE_CURRENT_LIST_DIR")}

def commands(text):
    text = re.sub(r"#[^\n]*", "", text)
    pat, i, res = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\("), 0, []
    while True:
        m = pat.search(text, i)
        if not m: break
        j, depth, inq = m.end(), 1, False
        while j < len(text) and depth:
            c = text[j]
            if c == '"': inq = not inq
            elif not inq:
                depth += (c == "(") - (c == ")")
            j += 1
        res.append((m.group(1).lower(), text[m.end():j - 1])); i = j
    return res

def split_args(s):
    return [a[1:-1] if len(a) >= 2 and a[0] == '"' and a[-1] == '"' else a for a in re.findall(r'"[^"]*"|[^\s]+', s)]

def expand(tokens):
    out = []
    for t in tokens:
        m = re.fullmatch(r"\$\{(\w+)\}", t)
        if m and m.group(1) in vars_:
            out += vars_[m.group(1)]; continue
        t = re.sub(r"\$\{(\w+)\}", lambda mm: ";".join(vars_[mm.group(1)]) if mm.group(1) in vars_ else mm.group(0), t)
        out += [t] if t.startswith("$<") else t.split(";")
    return [x for x in out if x != ""]

cm_path = os.path.join(proj, "CMakeLists.txt")
cmds = []
for name, raw in commands(read(cm_path)):
    args = expand(split_args(raw))
    if name == "set" and args:
        vals = []
        for a in args[1:]:
            if a in ("CACHE", "PARENT_SCOPE"): break
            vals.append(a)
        vars_[args[0]] = vals
    elif name == "list" and len(args) >= 2 and args[0].upper() == "APPEND":
        vars_.setdefault(args[1], []).extend(args[2:])
    cmds.append((name, args))

project_name = next((a[0] for n, a in cmds if n == "project" and a), proj.rsplit("/", 1)[-1])
plugin_target = next((a[0] for n, a in cmds if n == "juce_add_plugin" and a), project_name)

KEYWORDS = {"PRIVATE", "PUBLIC", "INTERFACE", "SYSTEM", "BEFORE", "AFTER"}
def target_args(command, target):
    res = []
    for n, a in cmds:
        if n == command and a and a[0] == target:
            res += [x for x in a[1:] if x not in KEYWORDS and "$<" not in x]
    return res

# ------------------------------------------------------------------ sources / includes / defs / modules
sources = []
for s in target_args("target_sources", plugin_target):
    if re.search(r"\.(cpp|cc|cxx|c)$", s, re.I):
        p = path(s)
        if os.path.isfile(p): sources.append(p)
        else: warns.append("source listed in CMakeLists but missing on disk: " + s)
if not sources:
    warns.append("no target_sources() found for '%s' - falling back to Source/**/*.cpp" % plugin_target)
    sources = sorted(glob.glob(os.path.join(proj, "Source", "**", "*.cpp"), recursive=True))
for must in ("PluginProcessor.cpp", "PluginEditor.cpp"):
    p = os.path.join(proj, "Source", must)
    if os.path.isfile(p) and p not in sources: sources.append(p)
sources = list(dict.fromkeys(sources))

includes = [os.path.join(proj, "Source")]
for s in target_args("target_include_directories", plugin_target):
    p = path(s)
    if os.path.isdir(p): includes.append(p)
includes = list(dict.fromkeys(includes))

# baseline: what the real plugin build uses for a headless run, then the project's own definitions win
defs = {"JUCE_WEB_BROWSER": "0", "JUCE_USE_CURL": "0", "JUCE_USE_XRANDR": "0", "JUCE_MODAL_LOOPS_PERMITTED": "1"}
def_tokens = target_args("target_compile_definitions", plugin_target)
for n, a in cmds:
    if n == "add_compile_definitions": def_tokens += [x for x in a if "$<" not in x]
    if n == "add_definitions":         def_tokens += [x[2:] for x in a if x.startswith("-D")]
for d in def_tokens:
    k, _, v = d.partition("=")
    if re.fullmatch(r"[A-Za-z_]\w*", k): defs[k] = v
project_defs = [d for d in def_tokens]

modules = ["juce::juce_audio_utils", "juce::juce_dsp"]
for l in target_args("target_link_libraries", plugin_target):
    if re.fullmatch(r"juce::juce_\w+", l) and not l.startswith("juce::juce_recommended_") and l not in modules:
        modules.append(l)

# ------------------------------------------------------------------ binary data (assets)
def ident(fn):
    s = re.sub(r"[^A-Za-z0-9_]", "_", fn)
    return "_" + s if s[:1].isdigit() else s

bds = []   # dicts: name, files, header, ns, recurse
for n, a in cmds:
    if n != "juce_add_binary_data" or not a: continue
    cur, srcs, header, ns, recurse = None, [], None, None, False
    for x in a[1:]:
        if x in ("SOURCES", "HEADER_NAME", "NAMESPACE"): cur = x; continue
        if x == "RECURSE": recurse = True; cur = None; continue
        if cur == "SOURCES": srcs.append(x)
        elif cur == "HEADER_NAME": header = x
        elif cur == "NAMESPACE": ns = x
    files = []
    for s in srcs:
        p = path(s)
        hits = sorted(glob.glob(p, recursive=True)) if re.search(r"[*?\[]", p) else ([p] if os.path.exists(p) else [])
        if not hits: warns.append("asset listed in CMakeLists but missing on disk: " + s)
        files += hits
    bds.append(dict(name=a[0], files=list(dict.fromkeys(files)), header=header, ns=ns, recurse=recurse))

def bd_names(bd):
    out = {}
    for f in bd["files"]:
        if os.path.isdir(f):
            for r, _, fs in os.walk(f):
                for x in fs: out[ident(x)] = os.path.join(r, x)
        else: out[ident(os.path.basename(f))] = f
    return out

# cross-check with the BinaryData:: symbols the sources really use
src_text = ""
for r, _, fs in os.walk(os.path.join(proj, "Source")):
    for x in fs:
        if re.search(r"\.(h|hpp|cpp|cc|mm|inl)$", x): src_text += read(os.path.join(r, x)) + "\n"
SKIP_SYMS = {"namedResourceListSize", "namedResourceList", "originalFilenames", "getNamedResource", "getNamedResourceOriginalFilename"}
asset_report = []
used_all = set()
for ns in {b["ns"] or "BinaryData" for b in bds} | {"BinaryData"}:
    for sym in set(re.findall(r"\b%s::([A-Za-z_]\w*)" % re.escape(ns), src_text)):
        if sym not in SKIP_SYMS and not sym.endswith("Size"): used_all.add((ns, sym))
known = {}
for b in bds:
    for k, f in bd_names(b).items(): known[((b["ns"] or "BinaryData"), k)] = f
all_assets = {}
for r, ds, fs in os.walk(proj):
    ds[:] = [d for d in ds if d not in ("build", "Build", "Builds", ".git", "node_modules")]
    for x in fs: all_assets.setdefault(ident(x), os.path.join(r, x))
for ns, sym in sorted(used_all):
    if (ns, sym) in known: continue
    cand = all_assets.get(sym)
    if cand:
        warns.append("BinaryData::%s is used but not in juce_add_binary_data - auto-added %s" % (sym, rel(cand)))
        tgt = next((b for b in bds if (b["ns"] or "BinaryData") == ns), None)
        if tgt is None:
            tgt = dict(name="Extra", files=[], header=None, ns=None if ns == "BinaryData" else ns, recurse=False); bds.append(tgt)
        tgt["files"].append(cand); known[(ns, sym)] = cand
    else:
        warns.append("BinaryData::%s is used by the sources but no matching asset file exists (compile will fail)" % sym)
used_files = {known[k] for k in used_all if k in known}
for b in bds:
    for f in b["files"]:
        asset_report.append("%s%s" % (rel(f), "" if f in used_files or os.path.isdir(f) else " (not referenced by Source/)"))

needs_juce_header = any(n == "juce_generate_juce_header" for n, _ in cmds) or bool(re.search(r"#\s*include\s*[<\"]JuceHeader\.h[>\"]", src_text))

# ------------------------------------------------------------------ class names / namespace / preset method / size
def find_class(header_path, base_re):
    text = blank_cpp(read(header_path))
    tok = re.compile(r"namespace\s*([A-Za-z_][\w:]*)?\s*\{|\{|\}|\b(?:class|struct)\s+([A-Za-z_]\w*)\s*(?:final\s*)?:\s*([^{;]*)\{")
    stack, found = [], []
    for m in tok.finditer(text):
        g = m.group(0)
        if g.startswith("namespace"): stack.append(m.group(1) or "")
        elif g == "{": stack.append(None)
        elif g == "}":
            if stack: stack.pop()
        else:
            if re.search(base_re, m.group(3)):
                ns = [s for s in stack if s]
                found.append(("::".join(ns + [m.group(2)]), "::".join(ns), m.start()))
            stack.append(None)
    return found, text

proc_h = os.path.join(proj, "Source", "PluginProcessor.h")
edit_h = os.path.join(proj, "Source", "PluginEditor.h")
procs, proc_text = find_class(proc_h, r"(?<![\w:])(?:juce::)?AudioProcessor(?![\w])")
edits, edit_text = find_class(edit_h, r"(?<![\w:])(?:juce::)?AudioProcessorEditor(?![\w])")
# an editor header may declare the editor class but not the processor (and vice versa): look in the other header too
if not procs:  procs, proc_text = find_class(edit_h, r"(?<![\w:])(?:juce::)?AudioProcessor(?![\w])")
if not edits:  edits, edit_text = find_class(proc_h, r"(?<![\w:])(?:juce::)?AudioProcessorEditor(?![\w])")
proc_cls = env.get("SNAP_PROC_CLASS") or (procs[0][0] if procs else None)
edit_cls = env.get("SNAP_EDITOR_CLASS") or (edits[0][0] if edits else None)
if not proc_cls: die("no class derived from juce::AudioProcessor found in Source/PluginProcessor.h (set SNAP_PROC_CLASS=...)")
if not edit_cls: die("no class derived from juce::AudioProcessorEditor found in Source/PluginEditor.h (set SNAP_EDITOR_CLASS=...)")
if len(procs) > 1: warns.append("several AudioProcessor classes in the header (%s) - using %s" % (", ".join(p[0] for p in procs), proc_cls))
if len(edits) > 1: warns.append("several AudioProcessorEditor classes in the header (%s) - using %s" % (", ".join(p[0] for p in edits), edit_cls))
namespace = proc_cls.rpartition("::")[0] or "(none, global namespace)"

method = env.get("SNAP_APPLY_METHOD")
if not method:
    for cand in ("applyPreset", "loadPreset", "setPreset", "applyProgram", "selectPreset", "setPresetIndex", "loadProgram"):
        if re.search(r"\bvoid\s+%s\s*\(\s*(?:const\s+)?int\b" % cand, proc_text):
            method = cand; break
    method = method or "setCurrentProgram"

# editor default size: setSize(W,H) in the editor constructor (inline in the header, else in PluginEditor.cpp)
size_re = re.compile(r"\bsetSize\s*\(\s*(\d+)\s*,\s*(\d+)\s*\)")
short = edit_cls.rpartition("::")[2]
size, size_src = None, ""
def line_of(text, off): return text.count("\n", 0, off) + 1
m_cls = re.search(r"\b(?:class|struct)\s+%s\b" % re.escape(short), edit_text)
if m_cls:
    m = size_re.search(edit_text, m_cls.start())
    if m: size, size_src = (int(m.group(1)), int(m.group(2))), "Source/PluginEditor.h:%d (editor constructor)" % line_of(edit_text, m.start())
if not size:
    cpp = os.path.join(proj, "Source", "PluginEditor.cpp")
    if os.path.isfile(cpp):
        ct = blank_cpp(read(cpp))
        mc = re.search(r"\b%s\s*::\s*%s\s*\(" % (re.escape(short), re.escape(short)), ct)
        m = size_re.search(ct, mc.start()) if mc else None
        if m: size, size_src = (int(m.group(1)), int(m.group(2))), "Source/PluginEditor.cpp:%d (editor constructor)" % line_of(ct, m.start())
if not size:
    size, size_src = (1000, 746), "not found in sources - template default (the editor's own size is used at run time anyway)"

# ------------------------------------------------------------------ write snap_config.cmake
def q(items): return '"' + ";".join(items) + '"'
defs.update({"SNAP_PROC_CLASS": proc_cls, "SNAP_EDITOR_CLASS": edit_cls, "SNAP_APPLY_METHOD": method,
             "SNAP_DEFAULT_W": str(size[0]), "SNAP_DEFAULT_H": str(size[1])})
defs_list = [k if v == "" else "%s=%s" % (k, v) for k, v in defs.items()]
lines = ["# generated by render.sh - do not edit",
         'set(SNAP_PROJ_DIR "%s")' % proj,
         "set(SNAP_SOURCES %s)" % q(sources),
         "set(SNAP_INCLUDES %s)" % q(includes),
         "set(SNAP_DEFS %s)" % q(defs_list),
         "set(SNAP_MODULES %s)" % q(modules),
         "set(SNAP_JUCE_HEADER %s)" % ("ON" if needs_juce_header else "OFF"),
         "set(SNAP_BD_COUNT %d)" % len(bds)]
for i, b in enumerate(bds):
    lines.append("set(SNAP_BD_%d_SOURCES %s)" % (i, q(b["files"])))
    lines.append('set(SNAP_BD_%d_HEADER "%s")' % (i, b["header"] or ""))
    lines.append('set(SNAP_BD_%d_NAMESPACE "%s")' % (i, b["ns"] or ""))
new = "\n".join(lines) + "\n"
old = read(cfg_path) if os.path.exists(cfg_path) else None
if new != old:                       # untouched file => cmake does not reconfigure
    with open(cfg_path, "w") as f: f.write(new)

w = lambda k, v: print("  %-10s %s" % (k, v))
print("detected for %s:" % os.path.basename(proj))
w("cmake", "project %s, plugin target %s" % (project_name, plugin_target))
w("processor", "%s   (namespace: %s)" % (proc_cls, namespace))
w("editor", edit_cls)
w("preset", "proc.%s(int)" % method)
w("size", "%dx%d  <- %s" % (size[0], size[1], size_src))
w("sources", ", ".join(rel(s) for s in sources))
w("includes", ", ".join(rel(s) for s in includes))
w("assets", ", ".join(asset_report) if asset_report else "(none)")
w("defs", " ".join(project_defs) or "(none in CMakeLists)")
w("JuceHeader", "yes - project includes <JuceHeader.h>, generated for the snapshot target" if needs_juce_header else "no")
for x in warns: print("  warning: " + x)
PY
}

echo "== $PROJ_NAME: detecting project configuration"
detect "$PROJ_DIR" "$BUILD" "$CFG"

# ---- 2. configure + build ---------------------------------------------------------------------
run_logged() {   # run_logged <logfile> <label> cmd...
    local log=$1 label=$2; shift 2
    if [[ ${SNAP_VERBOSE:-0} == 1 ]]; then "$@" 2>&1 | tee "$log"; return "${PIPESTATUS[0]}"; fi
    if ! "$@" >"$log" 2>&1; then
        echo "render.sh: $label failed - last lines of $log:" >&2
        tail -n 40 "$log" >&2
        return 1
    fi
}

GEN=(); FIRST_BUILD=0
if [[ ! -f $BUILD/CMakeCache.txt ]]; then
    FIRST_BUILD=1
    command -v ninja >/dev/null && GEN=(-G Ninja)
fi

echo "== configuring ($BUILD)"
run_logged "$BUILD/configure.log" "cmake configure" \
    cmake -S "$HERE" -B "$BUILD" ${GEN[@]+"${GEN[@]}"} -DCMAKE_BUILD_TYPE=Release \
          -DSNAP_CONFIG="$CFG" -DJUCE_DIR="$JUCE_DIR" || exit 1

echo "== building (-j$JOBS)$([[ $FIRST_BUILD == 1 ]] && echo ' - first build of this project compiles JUCE, ~2-4 min')"
T0=$(date +%s)
run_logged "$BUILD/build.log" "build" \
    cmake --build "$BUILD" --target Snapshot -j"$JOBS" || exit 1
echo "   build done in $(( $(date +%s) - T0 ))s"

EXE=$(find "$BUILD/Snapshot_artefacts" -type f -name Snapshot -perm -u+x 2>/dev/null | head -n1)
[[ -n $EXE ]] || die "Snapshot executable not found under $BUILD/Snapshot_artefacts"

# ---- 3. run -----------------------------------------------------------------------------------
ARGS=("$OUT" "$PRESET")
[[ -n ${SNAP_SCALE:-}     ]] && ARGS+=("--scale=$SNAP_SCALE")
[[ -n ${SNAP_SIZE:-}      ]] && ARGS+=("--size=$SNAP_SIZE")
[[ -n ${SNAP_SETTLE_MS:-} ]] && ARGS+=("--settle=$SNAP_SETTLE_MS")
[[ ${SNAP_KEEP_RESIZER:-0} == 1 ]] && ARGS+=("--keep-resizer")

echo "== rendering preset $PRESET"
rm -f "$OUT"
set +e
timeout 120 "$EXE" "${ARGS[@]}"
RC=$?
if [[ $RC -ne 0 && $RC -gt 4 ]] && command -v xvfb-run >/dev/null; then
    echo "   run failed (exit $RC) - retrying under xvfb-run" >&2
    timeout 120 xvfb-run -a "$EXE" "${ARGS[@]}"
    RC=$?
fi
set -e
[[ $RC -eq 0 ]] || die "snapshot run failed (exit $RC)"
[[ -s $OUT ]] || die "no output written to $OUT"
echo "== done: $OUT"
