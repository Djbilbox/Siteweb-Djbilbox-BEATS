"""Extrait les echantillons des instruments Maize Sampler (.mse).

Un .mse est un conteneur Maize : l'habillage graphique en tete (un PNG brut
puis un XML <player> dont les images sont en base64 JUCE), et, apres la balise
</player>, la partie instrument : une entree par echantillon (chemin d'origine
du WAV + canaux / frequence / nombre de trames) suivie du PCM 16 bits brut,
concatene a la fin du fichier.

    python extract_mse.py <dossier .instruments> <dossier de sortie>
"""

import os
import re
import struct
import sys
import wave

# Chaque entree est ".....\<nom>.wav" suivi de 5 entiers 32 bits little-endian :
# canaux, frequence, profondeur (code), inconnu, nombre de trames.
ENTRY = re.compile(rb"[ -~]{4,240}\.wav")


def entries(tail):
    """Renvoie [(nom, canaux, frequence, trames)] dans l'ordre du fichier."""
    out = []
    for m in ENTRY.finditer(tail):
        try:
            ch, sr, _depth, _unk, frames = struct.unpack_from("<5i", tail, m.end() + 1)
        except struct.error:
            continue
        if ch in (1, 2) and 8000 <= sr <= 192000 and 0 < frames < 200_000_000:
            name = m.group().decode("latin-1").replace("\\", "/").rsplit("/", 1)[-1]
            out.append((name[:-4], ch, sr, frames))
    return out


def extract(path, out_dir):
    data = open(path, "rb").read()
    marker = data.find(b"</player>")
    tail = data[marker + len(b"</player>"):] if marker >= 0 else data

    found = entries(tail)
    if not found:
        return []

    # Le PCM des echantillons est colle en fin de fichier, dans l'ordre des
    # entrees : on remonte depuis la fin pour trouver ou commence le premier.
    total = sum(f * c * 2 for _, c, _, f in found)
    if total > len(tail):
        return []
    pos = len(tail) - total

    base = os.path.splitext(os.path.basename(path))[0]
    written = []
    for i, (name, ch, sr, frames) in enumerate(found):
        nbytes = frames * ch * 2
        pcm = tail[pos:pos + nbytes]
        pos += nbytes

        stem = base if len(found) == 1 else "%s %02d" % (base, i + 1)
        target = os.path.join(out_dir, stem + ".wav")
        with wave.open(target, "wb") as w:
            w.setnchannels(ch)
            w.setsampwidth(2)
            w.setframerate(sr)
            w.writeframes(pcm)
        written.append((target, name, frames / float(sr)))
    return written


def main():
    if len(sys.argv) < 3:
        print(__doc__)
        return 1

    src, dst = sys.argv[1], sys.argv[2]
    os.makedirs(dst, exist_ok=True)

    ok = skipped = 0
    for entry in sorted(os.listdir(src)):
        if not entry.lower().endswith(".mse"):
            continue
        results = extract(os.path.join(src, entry), dst)
        if results:
            ok += len(results)
            for target, origin, seconds in results:
                print("  %-42s %5.2fs  <- %s" % (os.path.basename(target), seconds, origin))
        else:
            skipped += 1
            print("  IGNORE %s (aucune entree exploitable)" % entry)

    print("\nextraits : %d\nignores  : %d\nvers     : %s" % (ok, skipped, dst))
    return 0


if __name__ == "__main__":
    sys.exit(main())
