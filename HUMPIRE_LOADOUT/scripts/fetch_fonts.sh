#!/usr/bin/env bash
# Telecharge Press Start 2P (SIL OFL 1.1) dans assets/fonts/
set -euo pipefail
cd "$(dirname "$0")/../assets/fonts"
curl -fsSL -o PressStart2P-Regular.ttf \
  https://github.com/google/fonts/raw/main/ofl/pressstart2p/PressStart2P-Regular.ttf
curl -fsSL -o OFL-PressStart2P.txt \
  https://github.com/google/fonts/raw/main/ofl/pressstart2p/OFL.txt
echo "OK: $(ls -la PressStart2P-Regular.ttf)"
