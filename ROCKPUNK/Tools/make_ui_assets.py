#!/usr/bin/env python3
"""Builds the editor artwork of ROCKPUNK from the master mockup.

The mockup (Synthesizer_plugin_user_interface_mockup_2K_*.jpeg, 2400 x 1792) is
a finished picture: its faders already sit at some position, its wheels carry a
block at the middle, its drop-downs read "[WAVEFORM]" and its preset display
reads "ROCKPUNK - Init Patch". A plug-in cannot use that picture as it is, since
every one of those things has to move or change. This script produces what the
editor needs:

  Assets/faceplate.jpg   the mockup with the moving parts removed: fader thumbs
                         and value-dependent rails, wheel blocks, the text of
                         the drop-downs and of the preset display
  Assets/sprites.png     the pieces cut out of the mockup that the editor draws
                         back at run time: knob cap, fader thumbs, rail ends,
                         wheel block
  Assets/layout.json     every position, measured on the mockup (image pixels)

All measurements are done on the picture itself, by looking for edges and runs of
pixels, and are printed so that they can be checked. Nothing is invented: every
pixel that ends up on screen comes from the mockup.

    python3 make_ui_assets.py [path to the mockup]

Needs numpy and Pillow.
"""

import glob
import json
import os
import sys

import numpy as np
from PIL import Image, ImageDraw

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
ASSETS = os.environ.get("ROCKPUNK_ASSETS_OUT") or os.path.join(ROOT, "Assets")


def find_mockup():
    if len(sys.argv) > 1:
        return sys.argv[1]
    hits = sorted(glob.glob(os.path.join(ROOT, "Synthesizer_plugin_user_interfac*")))
    if not hits:
        raise SystemExit("mockup not found: pass its path as an argument")
    return hits[0]


def runs(mask, offset=0, minlen=1):
    """[(start, end)] of the True runs of a 1-D boolean array."""
    out, start = [], None
    for i, v in enumerate(mask):
        if v and start is None:
            start = i
        elif not v and start is not None:
            if i - start >= minlen:
                out.append((start + offset, i - 1 + offset))
            start = None
    if start is not None and len(mask) - start >= minlen:
        out.append((start + offset, len(mask) - 1 + offset))
    return out


class Art:
    def __init__(self, path):
        self.src = Image.open(path).convert("RGB")
        self.w, self.h = self.src.size
        self.orig = np.asarray(self.src).astype(np.float32)
        self.lum = self.orig.mean(axis=2)
        self.plate = self.orig.copy()

    def crop_subpixel(self, cx, cy, size, source=None):
        """size x size crop centred on (cx, cy), pixel-index coordinates."""
        img = self.src if source is None else source
        c = (cx + 0.5) - size / 2.0
        d = (cy + 0.5) - size / 2.0
        return img.transform((size, size), Image.AFFINE, (1, 0, c, 0, 1, d), resample=Image.BICUBIC)


# --------------------------------------------------------------------------
# Measurements (image pixels). Each is either found by the code below or, where
# a person-sized judgement is needed, written down here with how it was read.
# --------------------------------------------------------------------------

# Knobs: centres of the outer pink ring, fitted on the ring's pixels. GLIDE's
# ring is half grey in the mockup, so its centre was read from the cap instead.
KNOBS = {
    "lfoRate": (266.9, 730.0), "lfoAmount": (440.5, 730.2), "lfoPitch": (613.7, 730.2),
    "glide": (1009.0, 1024.5), "filter": (1392.1, 1025.7),
    "vol": (1779.0, 810.5), "reverb": (1955.9, 810.2), "chorus": (2133.2, 811.4),
    "dist": (1778.8, 1026.0), "time": (1956.2, 1026.1), "fb": (2132.9, 1025.3),
}
KNOB_CAP_RADIUS = 46        # knurled ring included
KNOB_SOURCE = "lfoRate"     # its pointer is vertical in the mockup: the sprite is "pointing up"

PARAM_OF_KNOB = {
    "lfoRate": "lfoRate", "lfoAmount": "lfoAmount", "lfoPitch": "lfoPitch", "glide": "glideMs",
    "filter": "fltCutoff", "vol": "masterVol", "reverb": "revMix", "chorus": "chorAmt",
    "dist": "driveAmt", "time": "delayTime", "fb": "delayFb",
}

FADER_GROUPS = [
    dict(group="r", probe_row=370, x_range=(1690, 2260), names=["fA", "fS", "fD", "fR", "fVel"],
         params=["fenvA", "fenvS", "fenvD", "fenvR", "velAmt"], search=(300, 700), period=57, synth=dict(texture=(292, 332), tick_band=(359, 363), ticks=(361, 418, 475, 532, 589))),
    dict(group="l", probe_row=900, x_range=(180, 760), names=["attack", "decay", "sustain", "release"],
         params=["envA", "envD", "envS", "envR"], search=(870, 1210), period=57),
]

WHEELS = {
    # body = the cylinder, ridges = the part carrying the ridges, block = the finger notch
    "pitchWheel": dict(param="pbend", body=(285, 347), block=(288, 342, 1468, 1518), ridges_y=(1356, 1637)),
    "modWheel":   dict(param="modDepth", body=(470, 532), block=(473, 527, 1468, 1518), ridges_y=(1356, 1637)),
}

COMBOS = [("wf", "pickShape"), ("lfoSrc", "lfoWave"), ("dest", "lfoDest"),
          ("filterType", "fltType"), ("monoPoly", "monoMode")]
COMBO_Y = (1183, 1242)       # outer rim of the boxes
COMBO_FACE_Y = (1188, 1239)
COMBO_FACE_ROW = 1192        # a row crossing the faces above the text, used to find their x extent


# A thumb throws its shadow down and to the right (the light comes from the top
# left): this much of the plate around it has to be rebuilt, and is part of its sprite.
SHADOW = dict(up=12, down=30, left=40, right=70)


def measure_faders(art):
    lum = art.lum
    out = {}
    for g in FADER_GROUPS:
        cores = runs(lum[g["probe_row"], g["x_range"][0]:g["x_range"][1]] < 8, g["x_range"][0], 6)
        assert len(cores) == len(g["names"]), (g["group"], cores)
        group = []
        for name, param, (a, b) in zip(g["names"], g["params"], cores):
            centre = (a + b) / 2.0
            cx = int(np.floor(centre + 0.5))
            col = runs(lum[g["search"][0]:g["search"][1], cx] < 8, g["search"][0], 3)
            core_top = col[0][0]
            thumb_top = col[0][1]            # the thumb's own dark outline row
            thumb_bottom = col[1][0] - 1
            core_bottom = col[1][1]
            group.append(dict(name=name, param=param, cx=cx, core_top=core_top, core_bottom=core_bottom,
                              thumb_top=thumb_top, thumb_bottom=thumb_bottom))
            print("fader %-8s cx=%d core %d..%d thumb %d..%d" % (name, cx, core_top, core_bottom, thumb_top, thumb_bottom))
        out[g["group"]] = group
    return out



def harmonic_paste(plate, patch, y0, x0, free_sides=()):
    """Pastes `patch` at (y0, x0) so that it blends into what surrounds it.

    The mismatch between the patch and the plate along the outline of the
    rectangle is spread inside it by solving Laplace's equation (a few hundred
    Jacobi steps), then added to the patch. A side listed in `free_sides` (any of
    "top", "bottom", "left", "right") carries no constraint: the correction runs
    flat towards it, which is what is wanted next to a part that will be painted
    over anyway.
    """
    h, w = patch.shape[:2]
    ring = np.zeros((h + 2, w + 2, 3), np.float32)
    known = np.zeros((h + 2, w + 2), bool)
    surround = plate[y0 - 1:y0 + h + 1, x0 - 1:x0 + w + 1]
    pad = np.pad(patch, ((1, 1), (1, 1), (0, 0)), mode="edge")
    diff = surround - pad
    known[0, :] = "top" not in free_sides
    known[-1, :] = "bottom" not in free_sides
    known[:, 0] = "left" not in free_sides
    known[:, -1] = "right" not in free_sides
    ring[known] = diff[known]
    field = ring.copy()
    free = ~known
    free[1:-1, 1:-1] = True
    free[0, :] = free[-1, :] = free[:, 0] = free[:, -1] = False
    # free sides (outline cells that are unconstrained) copy their inner neighbour
    for it in range(400):
        avg = 0.25 * (np.roll(field, 1, 0) + np.roll(field, -1, 0) + np.roll(field, 1, 1) + np.roll(field, -1, 1))
        field[free] = avg[free]
        if "top" in free_sides:
            field[0, :] = field[1, :]
        if "bottom" in free_sides:
            field[-1, :] = field[-2, :]
        if "left" in free_sides:
            field[:, 0] = field[:, 1]
        if "right" in free_sides:
            field[:, -1] = field[:, -2]
    plate[y0:y0 + h, x0:x0 + w] = patch + field[1:-1, 1:-1]


def synth_strip(orig, spec, cover, x0, x1):
    """Rows `cover` of a fader strip rebuilt from thumb-free pixels of the same
    columns: panel grain from above the slot (mirrored to the height needed) and
    the tick lines copied from a tick that no thumb ever covers."""
    t0, t1 = spec["texture"]
    tex = orig[t0:t1 + 1, x0:x1]
    n = cover[1] - cover[0] + 1
    period = 2 * tex.shape[0]
    idx = np.arange(n) % period
    idx = np.where(idx < tex.shape[0], idx, period - 1 - idx)
    patch = tex[idx].copy()
    b0, b1 = spec["tick_band"]
    band = orig[b0:b1 + 1, x0:x1]
    centre = (b0 + b1) // 2
    for t in spec["ticks"]:
        for dy in range(b0 - centre, b1 - centre + 1):
            r = t + dy - cover[0]
            if 0 <= r < n:
                patch[r] = band[dy - (b0 - centre)]
    return patch


def clean_faders(art, faders):
    """Removes thumbs and the value-dependent pink rails from the plate.

    The rail's outline is pink below the thumb and grey above it, so a fader
    cannot be left as the mockup has it. The rows a thumb covered (with its
    shadow) are rebuilt from the neighbouring fader, which has the same ticks and
    texture, or, for the first of a group, from its own strip one tick period up
    (the ticks repeat every 57 px). The slot is repainted black from rows that no
    thumb ever covered.
    """
    plate = art.plate
    orig = art.orig
    half_slot = 18
    S = SHADOW

    for g in FADER_GROUPS:
        group = faders[g["group"]]
        top_free = min(f["thumb_top"] for f in group) - 8
        black_rows = range(group[0]["core_top"] + 14, top_free)
        samples = []
        for f in group:
            for y in black_rows:
                samples.append(orig[y, f["cx"] - half_slot:f["cx"] + half_slot])
        black_row = np.median(np.stack(samples), axis=0)          # (28, 3)

        previous = None
        for f in group:
            cx = f["cx"]
            cover = (f["thumb_top"] - S["up"], f["thumb_bottom"] + S["down"])
            x0, x1 = cx - S["left"], cx + S["right"]
            if g.get("synth"):
                patch = synth_strip(orig, g["synth"], cover, x0, x1)
                shift = 0
            elif previous is not None:
                patch = plate[cover[0]:cover[1] + 1, previous["cx"] - S["left"]:previous["cx"] + S["right"]].copy()
                shift = 0
            else:
                for shift in (-g["period"], g["period"], -2 * g["period"], 2 * g["period"]):
                    lo, hi = cover[0] + shift, cover[1] + shift
                    if lo >= f["core_top"] - 25 and hi <= f["core_bottom"] + 25 and (hi < cover[0] or lo > cover[1]):
                        break
                else:
                    raise SystemExit("no donor rows for " + f["name"])
                patch = orig[cover[0] + shift:cover[1] + 1 + shift, x0:x1].copy()
            # left and right of the slot separately: the slot itself is repainted below
            harmonic_paste(plate, patch[:, :S["left"] - half_slot], cover[0], x0, free_sides=("right",))
            harmonic_paste(plate, patch[:, S["left"] + half_slot:], cover[0], cx + half_slot, free_sides=("left",))
            previous = f
            # slot: black from just under the top cap down to the bottom of the cap
            bottom = f["core_bottom"] + 5
            plate[f["core_top"] + 10:bottom + 1, cx - half_slot:cx + half_slot] = black_row[None, :, :]
            f["slot_top"] = f["core_top"] - 5
            f["slot_bottom"] = bottom
            f["donor_shift"] = shift
            print("  %-8s thumb rows rebuilt (shift %+d), slot %d..%d" % (f["name"], shift, f["slot_top"], f["slot_bottom"]))


def thumb_alpha_sprite(art, f):
    """The thumb with its drop shadow, as RGBA pixels that composite exactly back."""
    S = SHADOW
    cx = f["cx"]
    top, bottom = f["thumb_top"], f["thumb_bottom"]
    x0, x1 = cx - S["left"], cx + S["right"]
    y0, y1 = top - S["up"], bottom + S["down"]
    w, h = x1 - x0, y1 - y0 + 1
    orig = art.orig[y0:y1 + 1, x0:x1]
    clean = art.plate[y0:y1 + 1, x0:x1]

    # inner mask: the thumb itself (rounded rectangle, outline included)
    mask = Image.new("L", (w * 4, h * 4), 0)
    d = ImageDraw.Draw(mask)
    tx0, tx1 = (cx - 33 - x0), (cx + 31 - x0)
    ty0, ty1 = (top - y0), (bottom - y0)
    d.rounded_rectangle([tx0 * 4, ty0 * 4, (tx1 + 1) * 4 - 1, (ty1 + 1) * 4 - 1], radius=6 * 4, fill=255)
    inner = np.asarray(mask.resize((w, h), Image.LANCZOS)).astype(np.float32) / 255.0

    # shadow: wherever the original is darker than the clean plate, a black veil
    lo = orig.mean(axis=2)
    lc = np.maximum(clean.mean(axis=2), 1.0)
    veil = np.clip(1.0 - lo / lc, 0.0, 0.85)
    # 3 x 3 blur and a small floor: panel grain must not turn into a veil
    k = np.pad(veil, 1, mode="edge")
    veil = sum(k[dy:dy + h, dx:dx + w] for dy in range(3) for dx in range(3)) / 9.0
    veil = np.clip((veil - 0.05) / 0.95, 0.0, 0.85)
    shadow = veil * (1.0 - inner)
    # the neighbouring fader's thumb starts 59 px to the right: not ours
    shadow[:, :cx - 36 - x0] = 0.0
    shadow[:, cx + 56 - x0:] = 0.0
    # the slot above and below the thumb is repainted by the editor: no veil there
    slot_cols = slice(cx - 16 - x0, cx + 16 - x0)
    shadow[:ty0, slot_cols] = 0.0
    shadow[ty1 + 1:, slot_cols] = 0.0

    rgba = np.zeros((h, w, 4), np.float32)
    rgba[..., :3] = orig * inner[..., None]
    rgba[..., 3] = np.maximum(inner, shadow) * 255.0
    anchor = (cx - x0, (top + bottom) / 2.0 - y0 + 0.5)
    return rgba, anchor


def build_wheel_clean(art, name, w):
    """Fills the wheel block's place with ridges continued from above and below.

    The ridges are lit from above, so their brightness falls along the wheel; a
    copy of ridges from elsewhere is rescaled to the brightness the place should
    have, interpolated between the two ends of the gap. The copy is chosen (shift
    and a few pixels of stretch) to continue the ridges at both ends.
    """
    plate, orig = art.plate, art.orig
    x0, x1 = w["body"][0] - 3, w["body"][1] + 5
    bx0, bx1, by0, by1 = w["block"]
    a, b = by0 - 4, by1 + 4           # rows to rebuild
    ctx = 6
    rows = orig[:, x0:x1]
    ridge_cols = slice(w["body"][0] + 10 - x0, w["body"][1] - 10 - x0)
    lum_row = rows[:, ridge_cols].mean(axis=(1, 2))
    env = np.convolve(lum_row, np.ones(10) / 10.0, mode="same")

    ya, yb = a - 8, b + 8
    la, lb = max(env[ya], 1.0), max(env[yb], 1.0)
    n = b - a + 1

    def env_target(y):
        t = np.clip((y - ya) / float(yb - ya), 0.0, 1.0)
        return np.exp(np.log(la) * (1 - t) + np.log(lb) * t)

    def sample(rowpos):
        r0 = np.floor(rowpos).astype(int)
        f = (rowpos - r0)[:, None, None]
        return rows[r0] * (1 - f) + rows[r0 + 1] * f

    def env_at(rowpos):
        r0 = np.floor(rowpos).astype(int)
        f = rowpos - r0
        return env[r0] * (1 - f) + env[r0 + 1] * f

    ys_all = np.arange(a - ctx, b + ctx + 1).astype(np.float64)
    real_top = rows[a - ctx:a]
    real_bot = rows[b + 1:b + 1 + ctx]
    lo_lim, hi_lim = w["ridges_y"]
    best = None
    for k in list(range(-170, -(b - a) - 6)) + list(range(b - a + 6, 170)):
        for c in np.arange(-6.0, 6.01, 0.25):
            t = np.clip((ys_all - a) / float(b - a), 0.0, 1.0)
            src = ys_all - k + c * t
            fill_src = src[ctx:ctx + n]
            if src.min() < lo_lim + 2 or src.max() > hi_lim - 2:
                continue
            if ((fill_src > a - 3) & (fill_src < b + 3)).any():
                continue
            gain_ctx_top = env[a - ctx:a] / np.maximum(env_at(src[:ctx]), 1.0)
            gain_ctx_bot = env[b + 1:b + 1 + ctx] / np.maximum(env_at(src[ctx + n:]), 1.0)
            fa = sample(src[:ctx]) * gain_ctx_top[:, None, None]
            fb = sample(src[ctx + n:]) * gain_ctx_bot[:, None, None]
            err = np.abs(fa - real_top).mean() + np.abs(fb - real_bot).mean()
            if best is None or err < best[0]:
                best = (err, k, c)
    err, k, c = best
    yy = np.arange(a, b + 1).astype(np.float64)
    t = np.clip((yy - a) / float(b - a), 0.0, 1.0)
    src = yy - k + c * t
    gain = env_target(yy) / np.maximum(env_at(src), 1.0)
    fill = sample(src) * gain[:, None, None]
    harmonic_paste(plate, fill.astype(np.float32), a, x0)
    print("wheel %-10s ridge fill: shift %d, stretch %.2f px, seam error %.2f" % (name, k, c, err))
    return (a, b)


def wheel_block_sprite(art, w, margin=6):
    bx0, bx1, by0, by1 = w["block"]
    x0, x1 = bx0 - margin, bx1 + margin
    y0, y1 = by0 - margin, by1 + margin
    wd, ht = x1 - x0 + 1, y1 - y0 + 1
    orig = art.orig[y0:y1 + 1, x0:x1 + 1]
    clean = art.plate[y0:y1 + 1, x0:x1 + 1]
    mask = Image.new("L", (wd * 4, ht * 4), 0)
    d = ImageDraw.Draw(mask)
    d.rounded_rectangle([(bx0 - x0) * 4, (by0 - y0) * 4, (bx1 - x0 + 1) * 4 - 1, (by1 - y0 + 1) * 4 - 1],
                        radius=7 * 4, fill=255)
    inner = np.asarray(mask.resize((wd, ht), Image.LANCZOS)).astype(np.float32) / 255.0
    veil = np.clip(1.0 - orig.mean(axis=2) / np.maximum(clean.mean(axis=2), 1.0), 0.0, 0.8) * (1.0 - inner)
    rgba = np.zeros((ht, wd, 4), np.float32)
    rgba[..., :3] = orig * inner[..., None]
    rgba[..., 3] = np.maximum(inner, veil) * 255.0
    anchor = ((bx0 + bx1) / 2.0 - x0 + 0.5, (by0 + by1) / 2.0 - y0 + 0.5)
    return rgba, anchor


def erase_text(art, x0, x1, y0, y1, sample_dx=3):
    """Replaces a rectangle by what is left and right of it, row by row."""
    plate = art.plate
    left = art.orig[y0:y1 + 1, x0 - sample_dx - 2:x0 - 1].mean(axis=1)    # (rows, 3)
    right = art.orig[y0:y1 + 1, x1 + 2:x1 + sample_dx + 3].mean(axis=1)
    n = x1 - x0 + 1
    t = np.linspace(0.0, 1.0, n)[None, :, None]
    plate[y0:y1 + 1, x0:x1 + 1] = left[:, None, :] * (1 - t) + right[:, None, :] * t


def knob_cap_sprite(art):
    r = KNOB_CAP_RADIUS
    size = 2 * (r + 2)
    cx, cy = KNOBS[KNOB_SOURCE]
    crop = np.asarray(art.crop_subpixel(cx, cy, size)).astype(np.float32)
    yy, xx = np.mgrid[0:size, 0:size]
    dist = np.hypot(xx + 0.5 - size / 2.0, yy + 0.5 - size / 2.0)
    alpha = np.clip((r + 0.5 - dist) / 1.5, 0.0, 1.0)
    rgba = np.zeros((size, size, 4), np.float32)
    rgba[..., :3] = crop
    rgba[..., 3] = alpha * 255.0
    return rgba


def fix_glide_ring(art):
    """GLIDE's ring is half grey in the mockup, the other ten are pink all round:
    the ring of FILTER is copied over it so that all eleven match."""
    size = 160
    src = art.crop_subpixel(*KNOBS["filter"], size)
    yy, xx = np.mgrid[0:size, 0:size]
    dist = np.hypot(xx + 0.5 - size / 2.0, yy + 0.5 - size / 2.0)
    inner, outer = KNOB_CAP_RADIUS + 1.5, 74.0
    alpha = np.clip(np.minimum(dist - inner, outer - dist) / 2.0, 0.0, 1.0)
    cx, cy = KNOBS["glide"]
    # place the crop with sub-pixel accuracy, centred on GLIDE
    placed = Image.new("RGB", (size, size))
    glide_crop = art.crop_subpixel(cx, cy, size)
    base = np.asarray(glide_crop).astype(np.float32)
    mix = base * (1 - alpha[..., None]) + np.asarray(src).astype(np.float32) * alpha[..., None]
    # write back: the integer-aligned window around GLIDE's centre
    x0 = int(round(cx)) - size // 2
    y0 = int(round(cy)) - size // 2
    fx = (cx + 0.5) - size / 2.0 - x0
    fy = (cy + 0.5) - size / 2.0 - y0
    # `mix` is sampled at the sub-pixel offset (fx, fy); shift it back onto the pixel grid
    shifted = Image.fromarray(np.clip(mix, 0, 255).astype(np.uint8)).transform(
        (size, size), Image.AFFINE, (1, 0, -fx, 0, 1, -fy), resample=Image.BICUBIC)
    a_shift = Image.fromarray((alpha * 255).astype(np.uint8)).transform(
        (size, size), Image.AFFINE, (1, 0, -fx, 0, 1, -fy), resample=Image.BICUBIC)
    a = np.asarray(a_shift).astype(np.float32)[..., None] / 255.0
    region = art.plate[y0:y0 + size, x0:x0 + size]
    art.plate[y0:y0 + size, x0:x0 + size] = region * (1 - a) + np.asarray(shifted).astype(np.float32) * a


def measure_keyboard(art):
    """White key boundaries (the thin dark lines below the black keys) and the
    black keys (pairs of dark outline runs across them)."""
    lum = art.lum
    row = lum[1700, 640:2260]
    seps = [(a + b) / 2.0 for a, b in runs(row < 44, 640, 1) if 700 < a < 2180 and b - a < 12]
    outl = runs(lum[1350, 640:2260] < 22, 640, 2)
    merged = []
    for a, b in outl:
        if merged and a - merged[-1][1] <= 3:
            merged[-1] = (merged[-1][0], b)
        else:
            merged.append((a, b))
    outl = [(a, b) for a, b in merged if 700 < a and b < 2190 and b - a >= 5]
    black = []
    for i in range(0, len(outl) - 1, 2):
        a0 = outl[i][0]
        b1 = outl[i + 1][1]
        assert 38 <= b1 - a0 <= 58, (outl[i], outl[i + 1])
        black.append(((a0 + b1) / 2.0, a0, b1))
    print("keyboard: %d white-key boundaries, %d black keys" % (len(seps), len(black)))
    assert len(seps) == 21 and len(black) == 15
    return dict(separators=seps, black=black)


def main():
    art = Art(find_mockup())
    assert (art.w, art.h) == (2400, 1792), "unexpected mockup size %s" % ((art.w, art.h),)
    layout = dict(imageWidth=art.w, imageHeight=art.h, designWidth=1000, designHeight=746)

    # ---- faders ----
    faders = measure_faders(art)
    clean_faders(art, faders)

    # ---- sprites ----
    sprites = {}
    atlas_items = []   # (name, rgba, anchor)

    atlas_items.append(("knobCap", knob_cap_sprite(art), (KNOB_CAP_RADIUS + 2, KNOB_CAP_RADIUS + 2)))

    for g in FADER_GROUPS:
        grp = faders[g["group"]]
        donor = grp[2] if g["group"] == "r" else grp[0]
        rgba, anchor = thumb_alpha_sprite(art, donor)
        atlas_items.append(("thumb_" + g["group"], rgba, anchor))
        # pink rail: one row (uniform along the rail) and the rounded end
        cx = donor["cx"]
        ytest = donor["thumb_bottom"] + 40
        row = art.orig[ytest:ytest + 1, cx - 14:cx + 14]
        atlas_items.append(("railRow_" + g["group"], np.concatenate([row, np.full((1, 28, 1), 255, np.float32)], axis=2),
                            (14, 0)))
        cap_h = 12
        cap = art.orig[donor["slot_bottom"] - cap_h + 1:donor["slot_bottom"] + 1, cx - 14:cx + 14]
        atlas_items.append(("railCap_" + g["group"], np.concatenate([cap, np.full((cap_h, 28, 1), 255, np.float32)], axis=2),
                            (14, cap_h)))

    # ---- wheels ----
    for name, w in WHEELS.items():
        a, b = build_wheel_clean(art, name, w)
    for name, w in WHEELS.items():
        rgba, anchor = wheel_block_sprite(art, w)
        atlas_items.append(("block_" + name, rgba, anchor))

    # ---- glide ring ----
    fix_glide_ring(art)

    # ---- text of the drop-downs and the preset display ----
    faces = runs(art.lum[COMBO_FACE_ROW, 740:2300] > 38, 740, 20)
    assert len(faces) == 5, faces
    combos = []
    for (cid, param), (x0, x1) in zip(COMBOS, faces):
        erase_text(art, x0 + 5, x1 - 46, COMBO_FACE_Y[0] + 5, COMBO_FACE_Y[1] - 4)
        combos.append(dict(id=cid, param=param, x0=x0, x1=x1, y0=COMBO_Y[0], y1=COMBO_Y[1],
                           faceY0=COMBO_FACE_Y[0], faceY1=COMBO_FACE_Y[1]))
        print("combo %-10s face %d..%d" % (cid, x0, x1))
    erase_text(art, 928, 1468, 121, 181)

    # ---- pack the sprites ----
    pad = 2
    width = max(s[1].shape[1] for s in atlas_items) + 2 * pad
    height = sum(s[1].shape[0] + pad for s in atlas_items) + pad
    atlas = np.zeros((height, width, 4), np.float32)
    y = pad
    for name, rgba, anchor in atlas_items:
        h, w = rgba.shape[:2]
        atlas[y:y + h, pad:pad + w] = rgba
        sprites[name] = dict(x=pad, y=y, w=w, h=h, ax=float(anchor[0]), ay=float(anchor[1]))
        y += h + pad
    Image.fromarray(np.clip(atlas, 0, 255).astype(np.uint8), "RGBA").save(os.path.join(ASSETS, "sprites.png"), optimize=True)

    # ---- faceplate ----
    Image.fromarray(np.clip(art.plate + 0.5, 0, 255).astype(np.uint8)).save(
        os.path.join(ASSETS, "faceplate.jpg"), quality=93, subsampling=0, optimize=True)

    # ---- layout ----
    # every coordinate in layout.json is continuous: pixel i spans [i, i + 1)
    layout["knobs"] = {k: dict(x=v[0] + 0.5, y=v[1] + 0.5, param=PARAM_OF_KNOB[k]) for k, v in KNOBS.items()}
    layout["knobCapRadius"] = KNOB_CAP_RADIUS
    out_faders = {}
    for g in FADER_GROUPS:
        for f in faders[g["group"]]:
            half = (f["thumb_bottom"] - f["thumb_top"] + 1) / 2.0
            out_faders[f["name"]] = dict(
                group=g["group"], param=f["param"], cx=float(f["cx"]),
                slotTop=f["slot_top"], slotBottom=f["slot_bottom"],
                thumbMinY=f["slot_top"] + half, thumbMaxY=f["slot_bottom"] + 1 - half)
    layout["faders"] = out_faders
    out_wheels = {}
    for name, w in WHEELS.items():
        bx0, bx1, by0, by1 = w["block"]
        out_wheels[name] = dict(param=w["param"], x=(bx0 + bx1 + 1) / 2.0,
                                minY=by0 + (by1 - by0 + 1) / 2.0 - 105, maxY=by0 + (by1 - by0 + 1) / 2.0 + 105,
                                x0=w["body"][0] - 8, x1=w["body"][1] + 8)
    layout["wheels"] = out_wheels
    layout["combos"] = combos
    layout["sprites"] = sprites

    # grills, cassette, preset display, keyboard
    layout["grills"] = dict(a=dict(param="laOn", x0=788, y0=285, x1=940, y1=847),
                            b=dict(param="lbOn", x0=1465, y0=285, x1=1617, y1=847))
    layout["cassette"] = dict(x0=205, y0=180, x1=680, y1=530)
    layout["presetBar"] = dict(
        folder=dict(x0=774, y0=115, x1=861, y1=189),
        field=dict(x0=871, y0=115, x1=1529, y1=187),
        prev=dict(x0=873, y0=117, x1=968, y1=185),
        next=dict(x0=1432, y0=117, x1=1527, y1=185),
        save=dict(x0=1542, y0=115, x1=1627, y1=189),
        text=dict(x0=972, y0=121, x1=1428, y1=181))
    kb = measure_keyboard(art)
    layout["keyboard"] = dict(
        lowNote=36, left=681, right=2205, top=1328, blackBottom=1606, whiteBottom=1784,
        separators=kb["separators"],
        black=[dict(cx=c, x0=a, x1=b) for c, a, b in kb["black"]])
    with open(os.path.join(ASSETS, "layout.json"), "w") as fh:
        json.dump(layout, fh, indent=1)
    print("wrote faceplate.jpg, sprites.png, layout.json")


if __name__ == "__main__":
    main()
