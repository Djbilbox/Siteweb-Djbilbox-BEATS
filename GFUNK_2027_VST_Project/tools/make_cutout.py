"""Chroma-keys a green-screen illustration (Google Flow) and bakes a thick 3D relief:
white sticker outline + extruded side + drop shadow. Output: Resources/hero_cutout.png

usage: py tools/make_cutout.py <green_screen.jpg> [out.png] [width_px]
"""
import os
import sys

import numpy as np
from PIL import Image

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from gen_art_modern import gblur  # noqa: E402

F = np.float32
src = sys.argv[1]
root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
dst = sys.argv[2] if len(sys.argv) > 2 else os.path.join(root, "Resources", "hero_cutout.png")
target_w = int(sys.argv[3]) if len(sys.argv) > 3 else 900

im = np.asarray(Image.open(src).convert("RGB"), F) / 255.0
r, g, b = im[..., 0], im[..., 1], im[..., 2]
green_dom = g - np.maximum(r, b)                       # how much greener than the other channels
alpha = 1.0 - np.clip((green_dom - 0.10) / 0.28, 0, 1)
alpha = np.where((g > 0.55) & (r < 0.35) & (b < 0.35), 0.0, alpha)   # pure screen
alpha = np.clip(gblur(alpha, 0.6) * 1.15 - 0.05, 0, 1)

# remove green spill on the edges
spill = np.clip(g - np.maximum(r, b), 0, None)
g2 = np.where(alpha < 0.999, g - spill * 0.9, g - np.clip(spill - 0.02, 0, None) * 0.6)
rgb = np.stack([r, np.minimum(g, np.maximum(g2, 0)), b], -1)

rgba = np.concatenate([rgb, alpha[..., None]], -1)
img = Image.fromarray((np.clip(rgba, 0, 1) * 255 + 0.5).astype(np.uint8), "RGBA")
bbox = img.getchannel("A").point(lambda v: 255 if v > 30 else 0).getbbox()
img = img.crop(bbox)
scale = target_w / img.width
img = img.resize((target_w, int(img.height * scale)), Image.LANCZOS)
face = np.asarray(img, F) / 255.0
fa = face[..., 3]
fc = face[..., :3]

pad = 70
H0, W0 = fa.shape
H1, W1 = H0 + 2 * pad, W0 + 2 * pad
A = np.zeros((H1, W1), F)
C = np.zeros((H1, W1, 3), F)
A[pad:pad + H0, pad:pad + W0] = fa
C[pad:pad + H0, pad:pad + W0] = fc


def dilate(a, px):
    out = a.copy()
    for k in range(1, px + 1):
        for dx, dy in ((k, 0), (-k, 0), (0, k), (0, -k), (k * 0.7, k * 0.7), (-k * 0.7, k * 0.7), (k * 0.7, -k * 0.7), (-k * 0.7, -k * 0.7)):
            out = np.maximum(out, np.roll(np.roll(a, int(round(dy)), 0), int(round(dx)), 1))
    return gblur(out, 0.8)


outline = dilate(A, 9)                                  # white sticker border
rim = np.clip(outline - A, 0, 1)
depth = 22
ext = outline.copy()
for k in range(1, depth + 1):
    ext = np.maximum(ext, np.roll(np.roll(outline, k, 0), int(k * 0.6), 1))
side = np.clip(ext - outline, 0, 1)
shadow = gblur(np.roll(np.roll(ext, 26, 0), 16, 1), 14) * 0.65

out_rgb = np.zeros((H1, W1, 3), F)
out_a = np.zeros((H1, W1), F)

# 1) shadow
out_a = shadow.copy()
# 2) extruded side: dark rose to black going down
yy = np.linspace(0, 1, H1, dtype=F)[:, None, None]
side_col = np.array([0.30, 0.04, 0.16], F) * (1 - 0.6 * yy)
a2 = np.clip(side + out_a * 0, 0, 1)
out_rgb = out_rgb * (1 - a2[..., None]) + side_col * a2[..., None]
out_a = np.maximum(out_a, a2)
# 3) white outline
a3 = np.clip(rim * 1.2, 0, 1)
out_rgb = out_rgb * (1 - a3[..., None]) + np.array([0.98, 0.96, 0.94], F) * a3[..., None]
out_a = np.maximum(out_a, a3)
# 4) the artwork itself, with a soft top-left edge light for volume
edge = np.clip(A - gblur(A, 5), 0, 1)
gy, gx = np.gradient(gblur(A, 4))
light = np.clip(-(gx * 0.6 + gy * 0.8) * 9, 0, 1) * A
face_rgb = np.clip(C + light[..., None] * 0.22, 0, 1)
out_rgb = out_rgb * (1 - A[..., None]) + face_rgb * A[..., None]
out_a = np.maximum(out_a, A)

# premultiplied shadow colour: black where only the shadow exists
only_shadow = (out_a > 0) & (np.maximum(a2, np.maximum(a3, A)) < 0.01)
out_rgb = np.where(only_shadow[..., None], 0.0, out_rgb)

res = np.concatenate([np.clip(out_rgb, 0, 1), np.clip(out_a, 0, 1)[..., None]], -1)
Image.fromarray((res * 255 + 0.5).astype(np.uint8), "RGBA").save(dst)
print("saved", dst, (W1, H1))
