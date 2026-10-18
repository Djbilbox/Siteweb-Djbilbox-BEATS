#!/usr/bin/env bash
# render.sh <PluginFolder e.g. Glitch> <out.png> [presetIndex]   (env: SNAP_JOBS=2, SNAP_SCALE=1.5, JUCE_DIR)
set -euo pipefail
[[ $# -ge 2 ]] || { sed -n '2p' "$0"; exit 1; }
HERE=$(cd "$(dirname "$0")" && pwd)
BUILD=${SNAP_BUILD_ROOT:-/tmp/claude-0/snap_build}/loadout
JUCE_DIR=${JUCE_DIR:-/tmp/claude-0/juce}
[[ -f $JUCE_DIR/CMakeLists.txt ]] || { echo "JUCE not found at $JUCE_DIR" >&2; exit 1; }
GEN=(); command -v ninja >/dev/null && GEN=(-G Ninja)
OUT=$2; [[ $OUT = /* ]] || OUT=$PWD/$OUT
mkdir -p "$BUILD"
cmake -S "$HERE" -B "$BUILD" "${GEN[@]}" -DCMAKE_BUILD_TYPE=Release -DJUCE_DIR="$JUCE_DIR" >"$BUILD/configure.log" 2>&1 || { tail -20 "$BUILD/configure.log"; exit 1; }
cmake --build "$BUILD" --target HumpireSnapshot -j"${SNAP_JOBS:-2}" >"$BUILD/build.log" 2>&1 || { grep -m15 -B1 -A3 "error" "$BUILD/build.log"; exit 1; }
BIN=$(find "$BUILD" -name HumpireSnapshot -type f -perm -u+x | head -1)
cd "$BUILD"
if [[ -z ${DISPLAY:-} ]] && command -v xvfb-run >/dev/null; then
  exec xvfb-run -a "$BIN" "$1" "$OUT" "${3:-0}" --scale="${SNAP_SCALE:-1.5}"
fi
exec "$BIN" "$1" "$OUT" "${3:-0}" --scale="${SNAP_SCALE:-1.5}"
