"""Turns a logo generated on a black background (Google Flow) into a transparent PNG for the header.

usage: py tools/make_logo_from_flow.py <input.jpg> [output.png]
The alpha channel is keyed from the brightness (screen key), then the logo is cropped and fitted into 640 x 120.
"""
import os
import sys

import numpy as np
from PIL import Image

src = sys.argv[1]
dst = sys.argv[2] if len(sys.argv) > 2 else os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Resources", "logo_user.png")

im = np.asarray(Image.open(src).convert("RGB"), np.float32) / 255.0
alpha = np.clip((im.max(axis=2) - 0.05) / 0.30, 0, 1)
alpha = alpha ** 0.8
rgb = np.where(alpha[..., None] > 0.02, im / np.maximum(alpha[..., None], 0.02), 0)
rgba = np.concatenate([np.clip(rgb, 0, 1), alpha[..., None]], -1)
img = Image.fromarray((rgba * 255 + 0.5).astype(np.uint8), "RGBA")
bbox = img.getchannel("A").point(lambda v: 255 if v > 40 else 0).getbbox()
img = img.crop(bbox)
lw, lh = 640, 120
s = min((lw - 8) / img.width, (lh - 4) / img.height)
img = img.resize((max(1, int(img.width * s)), max(1, int(img.height * s))), Image.LANCZOS)
out = Image.new("RGBA", (lw, lh), (0, 0, 0, 0))
out.alpha_composite(img, ((lw - img.width) // 2, (lh - img.height) // 2))
out.save(dst)
print("saved", dst, out.size)
