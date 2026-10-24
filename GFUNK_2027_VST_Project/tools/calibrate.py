"""Reads rms.csv written by GFunkRenderTest and equalises preset loudness via tools/volume_trim.json."""
import csv
import json
import os
import sys

TARGET = 0.16
here = os.path.dirname(os.path.abspath(__file__))
trim_path = os.path.join(here, "volume_trim.json")
rows = list(csv.reader(open(sys.argv[1])))
trim = json.load(open(trim_path)) if os.path.exists(trim_path) else [1.0] * 1000
for idx, rms, peak in rows:
    i = int(idx)
    r = max(float(rms), 1e-4)
    trim[i] = trim[i] * min(4.0, max(0.25, (TARGET / r) ** 0.85))
json.dump([round(t, 4) for t in trim], open(trim_path, "w"))
print("calibrated; trim min %.2f max %.2f" % (min(trim), max(trim)))
