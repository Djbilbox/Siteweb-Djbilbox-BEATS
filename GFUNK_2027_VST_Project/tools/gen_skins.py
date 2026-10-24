"""Renders the six interface skins: Resources/bg_<n>.jpg and Resources/knob_<n>.png.

usage: py tools/gen_skins.py [all | <skin index>] [bg | knob]
"""
import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import gfunk_skins as K
import gfunk_spec as S
from gen_art_modern import (F, H, RES, SC, W, FONT_BLACK, FONT_UI, box_blur, gblur, lighting, rr_sdf, smooth,
                            to_u8)

FONT_TITLE = "C:/Windows/Fonts/arialbi.ttf"
if not os.path.exists(FONT_TITLE):
    FONT_TITLE = FONT_BLACK

HEADER_TEXT = [(0.78, 0.82, 0.90), (0.95, 0.85, 0.55), (0.35, 0.95, 1.0), (0.92, 0.97, 1.0), (1.0, 0.92, 0.92), (0.96, 0.90, 0.74)]
BRAND_TEXT = [(0.92, 0.74, 0.32), (0.95, 0.80, 0.40), (1.0, 0.45, 0.85), (0.80, 0.93, 1.0), (1.0, 0.80, 0.80), (0.90, 0.78, 0.55)]

ALB = HT = SPEC = EMIS = None
rng = np.random.default_rng(7)


# ----------------------------------------------------------------------------- surfaces
def surface(kind, p, shape, xx, yy, ty):
    """Returns (albedo[h,w,3], spec[h,w], height[h,w])."""
    r = np.random.default_rng(abs(hash((kind, shape))) % (2**32))
    h, w = shape
    if kind == "metal":
        n = box_blur(r.random(shape).astype(F), 60, 1)
        n = (n - n.mean()) / (n.std() + 1e-6)
        n2 = box_blur(r.random(shape).astype(F), 5, 1)
        n2 = (n2 - n2.mean()) / (n2.std() + 1e-6)
        g = p["g"] + 0.04 * (1 - ty[..., 0]) + 0.016 * p["brush"] * n + 0.010 * p["brush"] * n2
        return (g[..., None] * np.array(p["tint"], F)).astype(F), np.full(shape, 0.55, F), 0.03 * n2
    if kind == "brass":
        n = box_blur(r.random(shape).astype(F), 70, 1)
        n = (n - n.mean()) / (n.std() + 1e-6)
        n2 = box_blur(r.random(shape).astype(F), 6, 1)
        n2 = (n2 - n2.mean()) / (n2.std() + 1e-6)
        sheen = 0.5 + 0.5 * np.sin((xx * 0.0016 + yy * 0.0007) * 2 * math.pi)
        k = 0.80 + 0.07 * n + 0.05 * n2 + 0.16 * sheen - 0.18 * ty[..., 0]
        return (k[..., None] * np.array(p["col"], F)).astype(F), np.full(shape, 0.9, F), 0.03 * n2
    if kind == "glass":
        n = r.normal(0, 1, shape).astype(F)
        grid = (((xx // (14 * SC)) % 2) == 0) ^ (((yy // (14 * SC)) % 2) == 0)
        g = 0.030 + 0.020 * (1 - ty[..., 0]) + 0.0015 * gblur(n, 0.7) + 0.004 * grid
        return (g[..., None] * np.array([0.80, 1.0, 1.30], F)).astype(F), np.full(shape, 0.75, F), np.zeros(shape, F)
    if kind == "flake":
        top, bot = np.array(p["top"], F), np.array(p["bot"], F)
        base = top * (1 - ty) + bot * ty
        flake = np.kron(r.random((h // 3 + 1, w // 3 + 1)).astype(F), np.ones((3, 3), F))[:h, :w]
        flake = gblur(flake, 0.6)
        spark = gblur((r.random(shape) > 0.9985).astype(F), 0.7) * 9
        sheen = 0.5 + 0.5 * np.sin((xx * 0.0011 - yy * 0.0017) * 2 * math.pi)
        alb = base * (0.86 + 0.26 * flake + 0.22 * sheen)[..., None] + spark[..., None] * 0.35
        return alb.astype(F), np.full(shape, 0.95, F), (flake - 0.5) * 0.012
    if kind == "carbon":
        weave = 0.5 + 0.5 * np.sin(xx / (1.7 * SC)) * np.sin(yy / (1.7 * SC))
        band = 0.5 + 0.5 * np.sin((xx * 0.9 + yy) * 0.0026 * 2 * math.pi)
        g = 0.018 + 0.040 * weave + 0.020 * band * weave
        return (g[..., None] * np.array([1.0, 1.0, 1.06], F)).astype(F), np.full(shape, 0.8, F), 0.028 * weave
    if kind == "wood":
        low = box_blur(box_blur(r.random(shape).astype(F), 200, 1), 20, 0)
        low = (low - low.mean()) / (low.std() + 1e-6)
        fine = box_blur(r.random(shape).astype(F), 120, 1)
        fine = (fine - fine.mean()) / (fine.std() + 1e-6)
        grain = 0.5 + 0.5 * np.sin(yy / (2.2 * SC) + 7.0 * low + 1.5 * fine)
        dark = np.array([0.16, 0.075, 0.035], F)
        light = np.array([0.34, 0.185, 0.085], F)
        alb = dark * (1 - grain[..., None]) + light * grain[..., None]
        alb = alb * (0.88 + 0.12 * fine[..., None] * 0.6 + 0.04 * (1 - ty))
        return alb.astype(F), np.full(shape, 0.40, F), 0.02 * grain + 0.01 * fine
    # plastic
    top, bot = np.array(p["top"], F), np.array(p["bot"], F)
    base = top * (1 - ty) + bot * ty
    grain = r.normal(0, 1, shape).astype(F)
    brush = box_blur(r.random(shape).astype(F), 36, 1)
    brush = (brush - brush.mean()) / (brush.std() + 1e-6)
    alb = base * (1 + 0.020 * gblur(grain, 0.6)[..., None] + 0.010 * brush[..., None])
    alb = alb + ((r.random(shape) > 0.9985).astype(F) * 0.25)[..., None]
    return alb.astype(F), np.full(shape, 0.40, F), 0.012 * gblur(grain, 0.8) + 0.010 * brush


def region(x, y, w, h, m):
    x0, y0 = max(0, int((x - m) * SC)), max(0, int((y - m) * SC))
    x1, y1 = min(W, int((x + w + m) * SC)), min(H, int((y + h + m) * SC))
    yy, xx = np.mgrid[y0:y1, x0:x1].astype(F)
    return (slice(y0, y1), slice(x0, x1)), xx, yy


def init_maps(kind, p):
    global ALB, HT, SPEC, EMIS
    yy, xx = np.mgrid[0:H, 0:W].astype(F)
    ty = (yy / H)[..., None]
    a, s, t = surface(kind, p, (H, W), xx, yy, ty)
    ALB, SPEC, HT = a, s, t.astype(F)
    EMIS = np.zeros((H, W, 3), F)


def raised(x, y, w, h, radius, kind, p, glass_edge=False):
    sl, X, Y = region(x, y, w, h, 34)
    d = rr_sdf(X, Y, x * SC, y * SC, w * SC, h * SC, radius * SC)
    mask = d > 0
    inside = smooth(d, -0.7, 0.7)
    sh = gblur(np.roll((d > 0).astype(F), 7 * SC, axis=0), 8 * SC)
    ALB[sl] *= (1 - 0.6 * sh * (1 - inside))[..., None]
    ty = np.clip((Y - y * SC) / (h * SC), 0, 1)[..., None]
    alb, spec, tex = surface(kind, p, d.shape, X, Y, ty)
    bev = (2.8 if kind == "glass" else 4.5) * SC
    hh = smooth(d, 0, bev) + tex
    ALB[sl] = np.where(mask[..., None], alb, ALB[sl])
    HT[sl] = np.where(d > -3, hh, HT[sl])
    SPEC[sl] = np.where(mask, spec, SPEC[sl])
    if glass_edge:
        tx = np.clip((X - x * SC) / (w * SC), 0, 1)[..., None]
        col = np.array([0.0, 0.92, 1.0], F) * (1 - tx) + np.array([1.0, 0.16, 0.82], F) * tx
        edge = smooth(d, 3 * SC, 16 * SC)
        line = np.exp(-((Y - (y * SC + 1.4 * SC)) ** 2) / (2 * (0.9 * SC) ** 2)) * (d > 0) * edge
        glow = np.exp(-np.clip(Y - y * SC, 0, None) / (8 * SC)) * (d > 0) * edge * 0.10
        ring = np.exp(-(d ** 2) / (2 * (0.8 * SC) ** 2)) * (d > -2 * SC) * 0.25
        EMIS[sl] += (line[..., None] * 1.6 + glow[..., None] * 2.2 + ring[..., None] * 1.8) * col


def sunken(x, y, w, h, radius, color, depth=0.9):
    sl, X, Y = region(x, y, w, h, 10)
    d = rr_sdf(X, Y, x * SC, y * SC, w * SC, h * SC, radius * SC)
    mask = d > 0
    ALB[sl] = np.where(mask[..., None], np.array(color, F), ALB[sl])
    HT[sl] = np.where(d > -3, -smooth(d, 0, 4.5 * SC) * depth, HT[sl])
    SPEC[sl] = np.where(mask, 0.7, SPEC[sl])


def sunken_gradient(x, y, w, h, radius, top, bot):
    sl, X, Y = region(x, y, w, h, 10)
    d = rr_sdf(X, Y, x * SC, y * SC, w * SC, h * SC, radius * SC)
    mask = d > 0
    ty = np.clip((Y - y * SC) / (h * SC), 0, 1)[..., None]
    cx = np.clip(np.hypot((X - (x + w * 0.5) * SC) / (w * SC * 0.7), (Y - (y + h * 0.55) * SC) / (h * SC * 0.8)), 0, 1)[..., None]
    col = (np.array(top, F) * (1 - ty) + np.array(bot, F) * ty) * (1.15 - 0.55 * cx)
    ALB[sl] = np.where(mask[..., None], col, ALB[sl])
    HT[sl] = np.where(d > -3, -smooth(d, 0, 4.5 * SC) * 0.8, HT[sl])
    SPEC[sl] = np.where(mask, 0.5, SPEC[sl])


def print_text(text, x, y, size, color, spacing=1.0, font=None, anchor="l", engrave=False):
    font = ImageFont.truetype(font or FONT_TITLE, int(size * SC))
    probe = ImageDraw.Draw(Image.new("L", (4, 4)))
    widths = [probe.textlength(c, font=font) for c in text]
    tw = int(sum(widths) + spacing * SC * (len(text) - 1))
    th = int(size * SC * 1.7)
    layer = Image.new("L", (tw + 8, th), 0)
    dr = ImageDraw.Draw(layer)
    cx = 4
    for c, wd in zip(text, widths):
        dr.text((cx, th // 2), c, font=font, fill=255, anchor="lm")
        cx += wd + spacing * SC
    m = np.asarray(layer, F) / 255.0
    ox = int(x * SC) - (0 if anchor == "l" else tw // 2)
    oy = int(y * SC - th / 2)
    h0, w0 = m.shape
    ys, xs = max(0, oy), max(0, ox)
    ye, xe = min(H, oy + h0), min(W, ox + w0)
    m = m[ys - oy:ye - oy, xs - ox:xe - ox]
    ALB[ys:ye, xs:xe] = ALB[ys:ye, xs:xe] * (1 - m[..., None]) + m[..., None] * np.array(color, F)
    HT[ys:ye, xs:xe] += gblur(m, 0.5 * SC) * (-0.10 if engrave else 0.10)
    return tw / SC


def hline(x0, x1, y, color, thick=1.2, alpha=0.8):
    ys = slice(int((y - thick / 2) * SC), int((y + thick / 2) * SC) + 1)
    xs = slice(int(x0 * SC), int(x1 * SC))
    ALB[ys, xs] = ALB[ys, xs] * (1 - alpha) + np.array(color, F) * alpha


# ----------------------------------------------------------------------------- background per skin
def render_bg(idx):
    sk = K.SKINS[idx]
    ck, cp = sk["chassis"]
    hk, hp = sk["header"]
    pk, pp = sk["panel"]
    glass = pk == "glass"
    init_maps(ck, cp)
    title_col = sk["title"]
    engrave = sk["title_style"] == "engrave"

    hx, hy, hw, hh = S.HEADER_RECT
    raised(hx, hy, hw, hh, 12, hk, hp, glass_edge=(hk == "glass"))
    for name, title, (x, y, w, h), cols, rows in S.PANELS:
        if name in ("screen", "keys"):
            raised(x, y, w, h, 16 if name == "screen" else 14, hk, hp, glass_edge=(hk == "glass"))
        elif name == "hero":
            raised(x, y, w, h, 16, hk, hp, glass_edge=(hk == "glass"))
        else:
            raised(x, y, w, h, 14, pk, pp, glass_edge=glass)
            if title:
                print_text(title, x + 18, y + 20, 12.5, title_col, spacing=1.5, engrave=engrave)
                hline(x + 18, x + w - 18, y + 31, sk["rule"], 1.4, 0.55)

    hx2, hy2, hw2, hh2 = S.HERO_RECT
    sunken_gradient(hx2 + 12, hy2 + 12, hw2 - 24, hh2 - 24, 10, sk["hero_back"][0], sk["hero_back"][1])

    sx, sy, sw, sh = S.SCREEN_INNER
    sunken(sx - 7, sy - 7, sw + 14, sh + 14, 10, (0.025, 0.027, 0.034), 0.7)
    sunken(sx, sy, sw, sh, 8, (0.002, 0.003, 0.005), 0.5)
    tx, ty, tw_, th_ = S.TABS_RECT
    sunken(tx - 2, ty - 3, tw_ + 4, th_ + 6, 8, (0.010, 0.011, 0.015), 0.5)
    lx, ly, lw, lh = S.LCD_RECT
    sunken(lx - 4, ly - 4, lw + 8, lh + 8, 10, (0.025, 0.027, 0.034), 0.6)
    sunken(lx, ly, lw, lh, 7, (0.002, 0.004, 0.007), 0.4)
    for bx in S.BTN_X:
        sunken(bx - 2, S.BTN_Y - 2, 44, S.BTN_H + 4, 11, (0.012, 0.013, 0.018), 0.5)
    mx, my, mw, mh = S.METER_RECT
    sunken(mx, my, mw, mh, 7, (0.003, 0.005, 0.008), 0.5)
    kx, ky, kw, kh = S.KEYBOARD_RECT
    sunken(kx - 7, ky - 7, kw + 14, kh + 14, 9, (0.004, 0.005, 0.007), 0.8)
    wx, wy, ww, wh = S.WHEELS_RECT
    sunken(wx, wy, ww, wh, 10, (0.010, 0.011, 0.015), 0.7)

    kx0, ky0, kw0, kh0 = S.KEYS_RECT
    print_text("WEST COAST  G-FUNK  EDITION", kx0 + kw0 / 2, ky0 + 22, 12, BRAND_TEXT[idx], spacing=3.2, anchor="m", engrave=False)
    print_text("HUMPIRE by Djbilbox BEATS", hx + 26, hy + hh - 10, 10.5, HEADER_TEXT[idx], spacing=2.4, font=FONT_UI)

    col = lighting(ALB, HT, SPEC, EMIS, hs=11.0, shin=20, ambient=0.62)
    yy, xx = np.mgrid[0:H, 0:W].astype(F)
    v = 1 - 0.30 * (((xx / W - 0.5) * 1.5) ** 2 + ((yy / H - 0.5) * 1.5) ** 2)
    col *= v[..., None]
    path = os.path.join(RES, f"bg_{idx}.jpg")
    Image.fromarray(to_u8(col)).save(path, quality=93, subsampling=0)
    print("bg", idx, sk["name"], os.path.getsize(path) // 1024, "KB")


# ----------------------------------------------------------------------------- knob per skin
def render_knob(idx, size_logical=64, ss=4):
    kn = K.SKINS[idx]["knob"]
    n = size_logical * 2 * ss
    c = n / 2
    yy, xx = np.mgrid[0:n, 0:n].astype(F)
    dx, dy = xx - c, yy - c
    Rb = n * 0.5 * 0.70
    r = np.hypot(dx, dy) / Rb
    th = np.arctan2(dy, dx)
    body = r <= 1.0

    skirt = body & (r > 0.90)
    grip = body & (r > 0.70) & (r <= 0.90)
    top = body & (r <= 0.70)
    h = np.zeros((n, n), F)
    h = np.where(skirt, 0.25 + 0.2 * np.sin(np.pi * np.clip((r - 0.90) / 0.10, 0, 1)), h)
    ridges = 0.5 + 0.5 * np.cos(44 * th)
    h = np.where(grip, 0.62 + kn["ridge"] * ridges, h)
    h = np.where(top, 0.72 + 0.06 * np.sqrt(np.clip(1 - (r / 0.70) ** 2, 0, 1)), h)
    h = np.where(body & (np.abs(r - 0.70) < 0.02), h - 0.08, h)

    cap = np.array(kn["cap"], F)
    shade = (1.0 + 0.5 * (1 - np.clip(yy / n, 0, 1)))[..., None]
    alb = cap * shade
    alb = np.where(skirt[..., None], np.array(kn["skirt"], F), alb)
    lobe = (0.5 + 0.5 * np.cos(2 * (th - math.radians(-125)))) ** 5
    sheen = np.where(top, lobe * 0.10 * (0.4 + 0.6 * np.clip(r / 0.7, 0, 1)), 0)
    fine = np.interp(np.clip(r, 0, 1) * 220, np.arange(221), rng.random(221)).astype(F)
    alb = alb + sheen[..., None] * (0.7 + 0.6 * fine)[..., None]
    spec = np.where(skirt, kn["skirt_spec"], np.where(top, kn["cap_spec"], 0.18))
    col = lighting(alb, h, spec, hs=16 * (n / 128), shin=14 if kn["cap_spec"] < 0.5 else 26, ambient=0.8)
    gl = np.exp(-(((xx - c) / (Rb * 0.55)) ** 2 + ((yy - (c - Rb * 0.42)) / (Rb * 0.17)) ** 2)) * top
    col = col + gl[..., None] * (0.06 if kn["cap_spec"] < 0.5 else 0.20)
    if "glow" in kn:
        col = col + (skirt[..., None] * np.array(kn["glow"], F) * 0.45)
    col = np.clip(col, 0, 1)

    alpha = np.where(body, np.clip((1.0 - r) * Rb / 1.2, 0, 1), 0).astype(F)
    disc = body.astype(F)
    sh = np.roll(np.roll(disc, int(3 * ss * 2), axis=0), int(1.0 * ss * 2), axis=1)
    sh = gblur(sh, 5.0 * ss * 2 / 2) * 0.85
    out_a = alpha + sh * (1 - alpha)
    out_c = (col * alpha[..., None]) / np.maximum(out_a[..., None], 1e-5)
    rgba = np.concatenate([np.clip(out_c, 0, 1), out_a[..., None]], -1)
    img = Image.fromarray((rgba * 255 + 0.5).astype(np.uint8), "RGBA").resize((size_logical * 2, size_logical * 2), Image.LANCZOS)
    path = os.path.join(RES, f"knob_{idx}.png")
    img.save(path)
    print("knob", idx)


if __name__ == "__main__":
    which = sys.argv[1] if len(sys.argv) > 1 else "all"
    part = sys.argv[2] if len(sys.argv) > 2 else "all"
    idxs = range(len(K.SKINS)) if which == "all" else [int(which)]
    for i in idxs:
        if part in ("all", "knob"):
            render_knob(i)
        if part in ("all", "bg"):
            render_bg(i)
