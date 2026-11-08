"""Step 2 of the premium skin: from the clean faceplate rendered with Google Flow (same layout as the synth's own
interface, no pointers, no fader caps, no screen texts), measure where things are and write:
  - <synth>/Assets/faceplate.jpg (2000 px wide; ticks left on knob caps erased, wrong small texts rewritten),
  - <synth>/Source/FaceplateLayout.h (knob caps, fader slots, mascot, title: panel pixels, 1000 px wide).
The old layout comes from the synth's resized() / paint() (mock-up units, half = panel pixels).
usage: python -E -P skin_layout.py <synth dir> <clean render> [fixes.json] [title text]
fixes.json: [{"box": [x, y, w, h], "text": "Pitch Bend", "size": 9.5, "colour": "#d8dde6"}, ...]  (panel pixels)"""
import json, os, re, sys
import numpy as np
from PIL import Image, ImageDraw, ImageFont
from scipy import ndimage as nd

D = os.path.abspath(sys.argv[1]); name = os.path.basename(D)
img = Image.open(sys.argv[2]).convert('RGB')
fixes = json.load(open(sys.argv[3])) if len(sys.argv) > 3 else []
ed = open(os.path.join(D, 'Source', 'PluginEditor.cpp'), encoding='utf-8').read()
res = ed[ed.index('::resized()'):]
im = np.asarray(img).astype(np.float32)
S = im.shape[1] / 1000.0
lum = im.mean(axis=2)
mag = np.hypot(nd.sobel(nd.gaussian_filter(lum, 1.5), 1), nd.sobel(nd.gaussian_filter(lum, 1.5), 0))
th = np.linspace(0, 2 * np.pi, 72, endpoint=False)

def circle(x, y, rmin, rmax, search=20):
    rs = np.arange(int(rmin * S), int(rmax * S))
    best = (-1, x * S, y * S, rs[0])
    for dy in range(-search, search + 1, 2):
        for dx in range(-search, search + 1, 2):
            cx, cy = x * S + dx, y * S + dy
            xs = np.clip((cx + np.outer(rs, np.cos(th))).astype(int), 0, im.shape[1] - 1)
            ys = np.clip((cy + np.outer(rs, np.sin(th))).astype(int), 0, im.shape[0] - 1)
            sc = mag[ys, xs].mean(axis=1); i = int(np.argmax(sc))
            if sc[i] > best[0]: best = (sc[i], cx, cy, rs[i])
    return best[1] / S, best[2] / S, best[3] / S

# ---- knobs: place (name, cx, cy, size) in resized()
caps = []
for kname, mx, my, sz in re.findall(r'place \((\w+), (\d+), (\d+), (\d+)\)', res):
    mx, my, sz = int(mx), int(my), int(sz)
    r0 = sz * 0.25                                   # the old knob's radius (panel pixels)
    cx, cy, r = circle(mx * 0.5, my * 0.5, r0 * 0.45, r0 * 1.05)
    caps.append((kname, cx, cy, r))
    # a tick left by the render on the cap: a bright vertical band at the top -> filled from both sides
    X, Y, RR = cx * S, cy * S, r * S
    x0, x1 = int(X - 0.16 * RR), int(X + 0.16 * RR)
    band = lum[int(Y - 0.85 * RR):int(Y - 0.2 * RR), x0:x1 + 1]
    sides = np.concatenate([lum[int(Y - 0.85 * RR):int(Y - 0.2 * RR), x0 - 4:x0 - 1], lum[int(Y - 0.85 * RR):int(Y - 0.2 * RR), x1 + 2:x1 + 5]], axis=1)
    if band.size and sides.size and band.max() - np.median(sides) > 40:
        for yy in range(int(Y - 0.92 * RR), int(Y - 0.12 * RR)):
            a, b = im[yy, x0 - 2].copy(), im[yy, x1 + 2].copy()
            for xx in range(x0 - 1, x1 + 2):
                t = (xx - (x0 - 2)) / ((x1 + 2) - (x0 - 2))
                im[yy, xx] = a * (1 - t) + b * t

# ---- faders: ex / tx arrays, env[i] / rsl[i] bounds R (ex[i] - 28, Y, 56, H)
def arr(n): return [int(v) for v in re.search(r'\b%s\[\d\] = \{ ([\d, ]+) \}' % n, res).group(1).split(',')]
ex, tx = arr('ex'), arr('tx')
ey, eh = [int(v) for v in re.search(r'env\[i\]\.s\.setBounds \(R \(ex\[i\] - \d+, (\d+), \d+, (\d+)\)\)', res).groups()]
ty, thh = [int(v) for v in re.search(r'rsl\[i\]\.s\.setBounds \(R \(tx\[i\] - \d+, (\d+), \d+, (\d+)\)\)', res).groups()]
def slot(mx, my, mh):
    x0, y0, y1 = mx * 0.5, my * 0.5, (my + mh) * 0.5
    cols = lum[int(y0 * S):int(y1 * S), int((x0 - 9) * S):int((x0 + 9) * S)]
    prof = cols.mean(axis=0); xc = x0 - 9 + (np.argmin(nd.uniform_filter1d(prof, 3)) + 0.5) / S
    return round(float(xc), 1), round(y0 + 4, 1), round(y1 - 4, 1)   # the ends: the old slider's travel
slots_env = [slot(x, ey, eh) for x in ex]
slots_rsl = [slot(x, ty, thh) for x in tx]

# ---- mascot medallion and title (paint)
# (once the skin is applied, paint() no longer holds them: the previous FaceplateLayout.h does)
prev_path = os.path.join(D, 'Source', 'FaceplateLayout.h')
prev = open(prev_path, encoding='utf-8').read() if os.path.exists(prev_path) else ''
num = lambda k: float(re.search(r'\b%s = ([\d.]+)f' % k, prev).group(1))
m = re.search(r'drawImage \(camel, R \((\d+), (\d+), (\d+), (\d+)\)', ed) or re.search(r'\bmed = R \((\d+), (\d+), (\d+), (\d+)\)', ed)   # or clipped in a medallion rect
if m:
    mx, my, mw, mh = [int(v) for v in m.groups()]
    mcx, mcy, mr = circle((mx + mw / 2) * 0.5, (my + mh / 2) * 0.5, min(mw, mh) * 0.5 * 0.5 * 0.85, min(mw, mh) * 0.5 * 0.5 * 1.08, 16)
else:
    mcx, mcy, mr = circle(num('mascotX'), num('mascotY'), num('mascotR') * 0.85, num('mascotR') * 1.08, 10)
t = re.search(r'titleR = R \((\d+), (\d+), (\d+), (\d+)\)', ed)
title = [int(v) * 0.5 for v in t.groups()] if t else ([num('titleX'), num('titleY'), num('titleW'), num('titleH')] if 'titleX' in prev else None)
tsize = re.search(r'setFont \(juce::FontOptions \((\d+(?:\.\d+)?)f?, juce::Font::bold\)\);\s*\n[^\n]*titleR', ed)
tsize = float(tsize.group(1)) if tsize else (num('titleSize') if 'titleSize' in prev else 56.0)
ttext = re.search(r'drawFittedText \("([^"]+)", titleR', ed)
ttext = ttext.group(1) if ttext else (re.search(r'titleText = "([^"]*)"', prev).group(1) if 'titleText' in prev else name.replace('_', ' '))
if len(sys.argv) > 4: ttext = sys.argv[4]   # the product's name when it changed

# ---- small texts the render got wrong: plate colour behind, then the right text
out = Image.fromarray(np.clip(im, 0, 255).astype(np.uint8))
dr = ImageDraw.Draw(out)
for f in fixes:
    x, y, w, h = [v * S for v in f['box']]
    patch = np.asarray(out)[int(y):int(y + h), int(x):int(x + w)].reshape(-1, 3)
    dark = patch[patch.mean(axis=1) <= np.percentile(patch.mean(axis=1), 60)]   # the plate, not the old letters
    bg = tuple(int(v) for v in np.median(dark if len(dark) else patch, axis=0))
    dr.rectangle([x, y, x + w, y + h], fill=bg)
    font = ImageFont.truetype('C:/Windows/Fonts/bahnschrift.ttf', int(f.get('size', 10) * S))
    try: font.set_variation_by_name('SemiBold')
    except Exception: pass
    tw = dr.textlength(f['text'], font=font)
    dr.text((x + (w - tw) / 2, y + h / 2), f['text'], font=font, fill=f.get('colour', '#d8dde6'), anchor='lm')
out.resize((2000, round(2000 * im.shape[0] / im.shape[1])), Image.LANCZOS).save(os.path.join(D, 'Assets', 'faceplate.jpg'), quality=90, optimize=True)

L = ['// Generated by ALL_PLUGINS/HumpireFX/tools/skin_layout.py from the faceplate render: where things are (panel pixels).',
     '#pragma once', '', 'namespace Face', '{',
     'struct Cap { const char* name; float x, y, r; };', 'inline constexpr Cap caps[] = {']
L += ['    { "%s", %.1ff, %.1ff, %.1ff },' % c for c in caps]
L += ['};', 'struct Slot { float x, top, bottom; };   // fader slot: centre line and its ends',
      'inline constexpr Slot envSlots[] = { %s };' % ', '.join('{ %.1ff, %.1ff, %.1ff }' % s for s in slots_env),
      'inline constexpr Slot rslSlots[] = { %s };' % ', '.join('{ %.1ff, %.1ff, %.1ff }' % s for s in slots_rsl),
      'inline constexpr float mascotX = %.1ff, mascotY = %.1ff, mascotR = %.1ff;' % (mcx, mcy, mr)]
if title:
    L += ['inline constexpr float titleX = %.1ff, titleY = %.1ff, titleW = %.1ff, titleH = %.1ff, titleSize = %.1ff;' % (*title, tsize),
          'inline constexpr const char* titleText = "%s";' % ttext]
L += ['}', '']
open(os.path.join(D, 'Source', 'FaceplateLayout.h'), 'w', newline='\n').write('\n'.join(L))
print(name, 'caps', len(caps), 'env', slots_env, 'rsl', slots_rsl, 'mascot', (round(mcx, 1), round(mcy, 1), round(mr, 1)), 'title', title, tsize, ttext)
