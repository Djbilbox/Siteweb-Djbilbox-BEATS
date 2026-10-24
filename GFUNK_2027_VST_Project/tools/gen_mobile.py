"""Writes mobile/www/presets.json (parameter table + the 100 GFUNK 2027 Free presets) for the Android app."""
import json
import os

import gfunk_spec as S
import gen_presets as P

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(ROOT, "mobile", "www", "presets.json")
FREE_PER_CAT = 10


def main():
    presets = P.make_presets()
    step = P.PER_CAT // FREE_PER_CAT
    free = [p for k, p in enumerate(presets) if (k % P.PER_CAT) % step == 0 and (k % P.PER_CAT) // step < FREE_PER_CAT]
    data = {
        "params": [{"id": p[0], "type": p[2], "min": p[3], "max": p[4], "def": p[5], "choices": p[8].split("|") if p[8] else []}
                   for p in S.PARAMS],
        "categories": list(P.CATEGORIES),
        "presets": [{"name": n, "cat": c, "v": [round(float(x), 5) for x in v]} for n, c, v in free],
    }
    os.makedirs(os.path.dirname(OUT), exist_ok=True)
    with open(OUT, "w", encoding="utf-8") as fh:
        json.dump(data, fh, separators=(",", ":"))
    print("mobile presets", len(free), "->", OUT)


if __name__ == "__main__":
    main()
