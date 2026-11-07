"""Real instruments for a DJBILBOX BEATS synth: converts sampled instruments into FLAC notes embedded in the
plug-in and a table for hfx::SampleSet (SampleSet.h). Two kinds of sources:
  - an SFZ instrument (e.g. FENNEC's Library): its tuning, loops and drum key ranges are kept;
  - a folder of note recordings (e.g. VSCO 2 CE, VCSL): each file's pitch is measured, its velocity layer read from
    its name; held instruments get a crossfaded sustain loop baked in their steady middle.
Mono, 44.1 kHz, 16-bit; one-shots cut to `maxlen` seconds with a fade.
usage: python -E -P make_samples.py <synth dir> <spec.json>
spec.json: {"namespace": "Mb", "instruments": [
   {"name": "oud", "sfz": "C:/work/repo/ORIENTAL_INSTRUMENT_2/Library/Oud/Oud.sfz", "maxlen": 2.5, "spacing": 2},
   {"name": "darbuka", "sfz": ".../Darbuka.sfz", "drums": true, "velocities": {"_vl1_": 1, "_vl2_": 2}},
   {"name": "balafon", "folder": "C:/.../VCSL/.../Balafon/Traditional Mallet", "velocity": "_vl(\\\\d)_", "maxlen": 2.0},
   {"name": "flute", "folder": "...", "match": "susvib", "sustain": true}]}
spacing: at most one recording every N semitones per velocity layer; fills: keep transposed '_fill' notes of an SFZ."""
import json, os, re, sys
import numpy as np
import soundfile as sf
import librosa

D = os.path.abspath(sys.argv[1]); spec = json.load(open(sys.argv[2], encoding='utf-8'))
OUT = os.path.join(D, 'Assets', 'samples'); os.makedirs(OUT, exist_ok=True)
SR = 44100

def load(path, sr_in=None):
    x, sr = sf.read(path, dtype='float32', always_2d=True)
    x = x.mean(axis=1)
    if sr != SR: x = librosa.resample(x, orig_sr=sr, target_sr=SR)
    return x, sr

def bake_loop(x, f0):
    """crossfaded loop in the steady middle of a held note: (audio cut at the loop end, loop start, loop end)"""
    n = len(x); period = SR / f0
    a0 = min(int(0.9 * SR), int(0.35 * n)); a1 = min(int(0.85 * n), a0 + int(1.4 * SR))
    def zc(i):
        for k in range(i, min(n - 1, i + int(2 * period) + 2)):
            if x[k] <= 0 < x[k + 1]: return k + 1
        return i
    ls = zc(a0); le = zc(min(n - 2, ls + int(round((a1 - ls) / period)) * int(round(period))))
    xf = int(min(0.15 * SR, (le - ls) * 0.4))
    if le - ls < 2 * xf or xf < 256: return x, -1, -1
    y = x[:le].copy(); w = np.sin(np.linspace(0, np.pi / 2, xf)) ** 2
    y[le - xf:le] = x[le - xf:le] * (1 - w) + x[ls - xf:ls] * w
    return y, ls, le

def one_shot(x, maxlen):
    n = int(maxlen * SR)
    if len(x) > n:
        x = x[:n].copy(); fo = int(0.25 * SR); x[-fo:] *= np.linspace(1, 0, fo) ** 2
    return x

def regions(sfz, group=None):
    txt = open(sfz, encoding='utf-8').read()
    out, gl, gr = [], {}, {}
    for tag, body in re.findall(r'<(\w+)>([^<]*)', txt):
        ops = dict(re.findall(r'(\w+)=(\S+)', body))
        if tag == 'global': gl = ops
        elif tag == 'group': gr = ops
        elif tag == 'region':
            if group:
                k, v = group.split('=')
                if gr.get(k) != v: continue
            out.append({**gl, **gr, **ops})
    return out

notes, credits = [], []   # dicts: inst, key, cents, vel, audio, ls, le, lo, hi
for inst in spec['instruments']:
    mine = []
    def far_enough(key, vel):
        return all(abs(key - m['key']) >= inst.get('spacing', 1) or m['vel'] != vel for m in mine)
    if 'sfz' in inst:
        base = os.path.dirname(inst['sfz']); drums = inst.get('drums', False); seen = set()
        for r in regions(inst['sfz'], inst.get('group')):
            if '_fill' in r['sample'] and not inst.get('fills', False): continue
            key = int(r.get('pitch_keycenter', r.get('lokey', 60)))
            vel = next((v for p, v in inst.get('velocities', {}).items() if p in r['sample']), 1)
            if drums:
                if (r['sample'], r['lokey']) in seen: continue
                seen.add((r['sample'], r['lokey']))
            elif not far_enough(key, vel): continue
            x, sr = load(os.path.join(base, r['sample']))
            ls, le = int(r.get('loop_start', -1)), int(r.get('loop_end', -1))
            if sr != SR and ls >= 0: ls, le = int(ls * SR / sr), int(le * SR / sr)
            if r.get('loop_mode') == 'loop_sustain' and le > ls > 0: x = x[:le + 1]; le += 1
            else: ls = le = -1; x = one_shot(x, inst.get('maxlen', 2.5))
            mine.append(dict(key=key, cents=-float(r.get('tune', 0)), vel=vel, audio=x, ls=ls, le=le,
                             lo=int(r['lokey']) if drums else -1, hi=int(r['hikey']) if drums else -1))
        credits.append('%s: %s' % (inst['name'], inst['sfz']))
    else:
        folder = inst['folder']
        found = []   # (velocity, audio, measured midi, midi from the file name or None)
        for f in sorted(os.listdir(folder)):
            if not f.lower().endswith(('.wav', '.flac')) or not re.search(inst.get('match', '.'), f): continue
            vm = re.search(inst['velocity'], f) if inst.get('velocity') else None
            vel = int(vm.group(1)) if vm else 1
            for pat, v in inst.get('velocities', {}).items():   # dynamics written as words: {"_pp_": 1, "_ff_": 2}
                if pat in f: vel = v
            x, _ = load(os.path.join(folder, f))
            pk = np.abs(x).max(); i0 = int(np.argmax(np.abs(x) > 0.02 * pk)); x = x[max(0, i0 - int(0.003 * SR)):]
            fi = int(0.003 * SR); x[:fi] *= np.linspace(0, 1, fi)
            seg = x[int(0.05 * SR):int(min(len(x), 0.9 * SR))]
            # pitch in two steps: a coarse search over the whole range, then to the cent around what it found
            f0, _, _ = librosa.pyin(seg, fmin=40, fmax=2500, sr=SR, frame_length=4096, hop_length=256)
            f0 = f0[~np.isnan(f0)]
            det = None
            if len(f0) >= 5:
                hz = float(np.median(f0))
                f0, _, _ = librosa.pyin(seg, fmin=hz / 1.2, fmax=hz * 1.2, sr=SR, frame_length=4096, hop_length=256, resolution=0.01)
                f0 = f0[~np.isnan(f0)]
                det = float(np.median(librosa.hz_to_midi(f0))) if len(f0) >= 5 else float(librosa.hz_to_midi(hz))
            nm = re.search(r'(?<![A-Za-z])([A-G]#?)(-?\d)(?=_|\.|$)', os.path.splitext(f)[0].replace('_', '_'))
            named = int(librosa.note_to_midi(nm.group(1) + nm.group(2))) if nm else None
            if det is None and named is None: print('  no pitch:', f); continue
            found.append((vel, x, det, named))
        # the file names are trusted (libraries name octaves differently: the usual offset is found from the
        # measured pitches); the measurement only gives the fine tuning, and the key when a name has no note
        offs = [12 * round((d - n) / 12) for _, _, d, n in found if d is not None and n is not None]
        offset = max(set(offs), key=offs.count) if offs else 0
        for vel, x, det, named in found:
            key = named + offset if named is not None else int(round(det))
            cents = (det - key) * 100 if det is not None and abs(det - key) < 0.5 else 0.0
            if not far_enough(key, vel): continue
            if inst.get('sustain'): x, ls, le = bake_loop(x, librosa.midi_to_hz(key + cents / 100))
            else: x, ls, le = one_shot(x, inst.get('maxlen', 2.5)), -1, -1
            mine.append(dict(key=key, cents=cents, vel=vel, audio=x, ls=ls, le=le, lo=-1, hi=-1))
        credits.append('%s: %s' % (inst['name'], folder))
    # velocity layers numbered 1..n from the softest; levels evened inside a layer, layers keep their difference
    vels = sorted({m['vel'] for m in mine}); remap = {v: i + 1 for i, v in enumerate(vels)}
    for m in mine:
        m['vel'] = remap[m['vel']]; m['inst'] = inst['name']
        # every note at the same peak (libraries record at very different levels), softer layers a little lower
        layer = 0.55 + 0.45 * m['vel'] / len(vels)
        m['audio'] = m['audio'] * (0.89 * layer / (np.abs(m['audio']).max() + 1e-9))
    notes += mine
    print('%-10s %3d notes, keys %s..%s, %d velocity layers' % (inst['name'], len(mine), min((m['key'] for m in mine), default='-'), max((m['key'] for m in mine), default='-'), len(vels)))

for inst in spec['instruments']:
    for f in os.listdir(OUT):
        if f.startswith(inst['name'] + '_'): os.remove(os.path.join(OUT, f))
L = ['// Generated by ALL_PLUGINS/HumpireFX/tools/make_samples.py: the real instruments of this synth.', '#pragma once',
     '#include "SampleSet.h"', '', 'namespace %s' % spec['namespace'], '{']
for inst in spec['instruments']:
    L.append('inline constexpr hfx::SampleNote %sNotes[] = {' % inst['name'])
    count = {}
    for m in notes:
        if m['inst'] != inst['name']: continue
        k = (m['key'], m['vel']); count[k] = count.get(k, 0) + 1
        name = '%s_%d_v%d_%d.flac' % (inst['name'], m['key'], m['vel'], count[k])
        sf.write(os.path.join(OUT, name), m['audio'].astype(np.float32), SR, subtype='PCM_16')
        L.append('    { "%s", %d, %.1ff, 0, %d, %d, %d, %d, %d, %d },' % (re.sub(r'[^A-Za-z0-9]', '_', name), m['key'], m['cents'], m['vel'],
                                                                    m['ls'], m['le'], len(m['audio']), m['lo'], m['hi']))
    L.append('};')
L += ['}', '']
open(os.path.join(D, 'Source', 'SampleMap.h'), 'w', newline='\n').write('\n'.join(L))
open(os.path.join(OUT, 'SOURCES.txt'), 'w', newline='\n').write('Where each real instrument comes from (licences: see CREDITS.txt):\n' + '\n'.join(credits) + '\n')

# credits: the FENNEC library's own credit lines for its instruments, the Versilian libraries (CC0) for the others
fennec = os.path.join(os.path.dirname(os.path.abspath(__file__)), '..', '..', '..', 'ORIENTAL_INSTRUMENT_2', 'Library', 'CREDITS.txt')
flines = open(fennec, encoding='utf-8').read().split('\n') if os.path.exists(fennec) else []
vcsl = 'Versilian Community Sample Library, Versilian Studios (CC0)  https://github.com/sgossner/VCSL'
vsco = 'VSCO 2 Community Edition, Versilian Studios (CC0)  https://github.com/sgossner/VSCO-2-CE'
C = ['%s - real instruments: credits' % os.path.basename(D).replace('_', ' '), '',
     'Some engines play recorded notes (cut, tuned, level-matched and looped by HUMPIRE PLUGINS).',
     'Thank you to the musicians and recordists.', '',
     'Licences:  CC0 1.0    https://creativecommons.org/publicdomain/zero/1.0/',
     '           CC BY 3.0  https://creativecommons.org/licenses/by/3.0/',
     '           CC BY 4.0  https://creativecommons.org/licenses/by/4.0/', '']
for inst in spec['instruments']:
    where = inst.get('sfz') or inst.get('folder')
    if 'ORIENTAL_INSTRUMENT_2' in where:
        lib = os.path.basename(os.path.dirname(where))
        if lib == 'Violin': C.append('%-10s violin from %s' % (inst['name'], vsco))
        elif lib in ('Darbuka', 'Bendir', 'Riq'): C.append('%-10s %s' % (inst['name'], vcsl))
        else: C += ['%-10s %s' % (inst['name'], l.split(None, 1)[1]) for l in flines if l.startswith(lib + ' ') and 'freesound.org/s/' in l]
    elif 'VCSL' in where: C.append('%-10s %s' % (inst['name'], vcsl))
    elif 'VSCO' in where: C.append('%-10s %s' % (inst['name'], vsco))
    else: C.append('%-10s %s' % (inst['name'], inst.get('credit', where)))
open(os.path.join(OUT, 'CREDITS.txt'), 'w', newline='\n', encoding='utf-8').write('\n'.join(C) + '\n')
print('total %.1f MB' % (sum(os.path.getsize(os.path.join(OUT, f)) for f in os.listdir(OUT) if f.endswith('.flac')) / 1e6))
