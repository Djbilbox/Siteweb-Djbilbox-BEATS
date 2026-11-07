"""Matches the level of the real instruments of a synth to its synthesised engines: plays the first factory preset of
every engine (the same chord and riff, through the synth's UiFrames tool), measures it, and writes the gain of each
sampled engine into the voice (realGain) so that it sounds as loud as the synthesised ones. Rebuild afterwards.
usage: python -E -P set_levels.py <synth dir> <UiFrames exe> <sampled engines, e.g. 0,1,2,3,4>"""
import os, re, subprocess, sys, tempfile
import numpy as np
import soundfile as sf

D = os.path.abspath(sys.argv[1]); exe = sys.argv[2]; sampled = [int(e) for e in sys.argv[3].split(',')]
vf = [f for f in os.listdir(os.path.join(D, 'Source')) if f.endswith('Voice.h')][0]
voice = open(os.path.join(D, 'Source', vf), encoding='utf-8').read()
gains = [float(g) for g in re.search(r'float realGain\[8\] \{ ([^}]*) \}', voice).group(1).replace('f', '').split(',')]
presets = [l for l in open(os.path.join(D, 'Source', 'Presets.h'), encoding='utf-8').read().split('\n') if l.strip().startswith('{"')]
first = {}
for i, l in enumerate(presets):
    w = int(float(re.search(r'"waveform", ([\d.]+)f', l).group(1)))
    first.setdefault(w, i)
tmp = tempfile.mkdtemp()
level = {}
for eng, idx in sorted(first.items()):
    out = os.path.join(tmp, 'e%d' % eng)
    subprocess.run([exe, out, '5', '7', str(idx)], cwd=tmp, capture_output=True)
    x, sr = sf.read(os.path.join(out, 'audio.wav'))
    m = x.mean(axis=1)[int(1.6 * sr):int(6.5 * sr)]
    level[eng] = 20 * np.log10(np.sqrt(np.mean(m ** 2)) + 1e-9)
synth = [level[e] for e in level if e not in sampled]
target = float(np.median(synth)) if synth else -19.0
for e in sampled:
    if e in level:
        gains[e] = float(np.clip(gains[e] * 10 ** ((target - level[e]) / 20), 0.25, 8.0))
    print('engine %d: %5.1f dB -> gain %.2f%s' % (e, level.get(e, float('nan')), gains[e], ''))
print('synthesised engines: %s dB, target %.1f dB' % (', '.join('%.1f' % level[e] for e in level if e not in sampled), target))
new = 'float realGain[8] { %s }' % ', '.join('%.2ff' % g for g in gains)
voice = re.sub(r'float realGain\[8\] \{ [^}]* \}', new, voice, count=1)
open(os.path.join(D, 'Source', vf), 'w', encoding='utf-8', newline='').write(voice)
