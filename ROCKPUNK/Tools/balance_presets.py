"""Cale les 25 presets d'usine au meme niveau percu.

Le banc de rendu mesure chaque preset, ce script en deduit le masterVol a
ecrire dans make_presets.py, regenere les presets et recommence jusqu'a ce que
l'ecart tienne dans la tolerance.

La corde est excitee par un bruit non graine : une mesure isolee varie de
plusieurs dB. Chaque passe moyenne donc plusieurs rendus, et le banc lui-meme
moyenne quatre notes.

    python balance_presets.py [nb_passes]
"""

import math
import os
import re
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
TEST = os.path.join(ROOT, "Build", "ROCKPUNK_RenderTest_artefacts", "Release",
                    "ROCKPUNK_RenderTest.exe")
SCRIPT = os.path.join(ROOT, "Tools", "make_presets.py")
PRESETS = os.path.join(ROOT, "Presets")

TARGET = 0.10          # RMS vise
TOLERANCE_DB = 2.5     # au-dela, on corrige
# Le bruit de mesure vaut environ 1,5 dB : viser plus serre ferait
# osciller la correction d'une passe a l'autre sans rien gagner.
MASTER_MIN, MASTER_MAX = -60.0, 6.0
DEFAULT_MASTER = -3.0   # la valeur de DEFAULTS dans make_presets.py

LINE = re.compile(r"^  OK\s+(\d{2}) .*RMS (\d+\.\d+)", re.M)


def measure(runs):
    """RMS moyen par preset, sur plusieurs rendus."""
    totals = {}
    for _ in range(runs):
        out = subprocess.run([TEST, PRESETS], capture_output=True, text=True).stdout
        for num, rms in LINE.findall(out):
            totals.setdefault(num, []).append(float(rms))
    return {k: sum(v) / len(v) for k, v in totals.items() if v}


MASTER_BLOCK = re.compile(r"MASTER = \{.*?\n\}", re.S)


def read_master(source):
    """Le tableau MASTER tel qu'il est ecrit dans make_presets.py."""
    block = MASTER_BLOCK.search(source).group(0)
    return {k: float(v) for k, v in re.findall(r'"(\d{2})":\s*(-?[\d.]+)', block)}


def write_master(source, table):
    body = "\n".join('    "%s": %g,' % (k, table[k]) for k in sorted(table))
    return MASTER_BLOCK.sub(lambda _: "MASTER = {\n%s\n}" % body, source, count=1)


def apply(levels):
    """Corrige le volume de sortie des presets trop loin de la cible."""
    source = open(SCRIPT, encoding="utf-8").read()
    table = read_master(source)
    changed = 0

    for num, rms in sorted(levels.items()):
        if rms <= 0.0:
            continue
        delta = 20.0 * math.log10(TARGET / rms)
        if abs(delta) < TOLERANCE_DB:
            continue

        current = table.get(num, DEFAULT_MASTER)
        new = round(max(MASTER_MIN, min(MASTER_MAX, current + delta)), 1)
        if new == current:
            continue

        table[num] = new
        changed += 1
        print("   %s  %.4f  %+5.1f dB  ->  masterVol %g" % (num, rms, delta, new))

    if changed:
        open(SCRIPT, "w", encoding="utf-8").write(write_master(source, table))
        subprocess.run([sys.executable, SCRIPT, PRESETS], check=True,
                       stdout=subprocess.DEVNULL)
    return changed


def main():
    passes = int(sys.argv[1]) if len(sys.argv) > 1 else 4

    for i in range(1, passes + 1):
        levels = measure(4)
        if not levels:
            print("aucune mesure : le banc a-t-il ete compile ?")
            return 1

        values = sorted(levels.values())
        spread = 20.0 * math.log10(values[-1] / values[0])
        print("passe %d : %d presets, %.4f a %.4f, ecart %.1f dB"
              % (i, len(levels), values[0], values[-1], spread))

        if not apply(levels):
            print("\nrien a corriger : tout tient dans %.1f dB de la cible." % TOLERANCE_DB)
            break

    return 0


if __name__ == "__main__":
    sys.exit(main())
