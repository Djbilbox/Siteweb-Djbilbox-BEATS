"""GFUNK 2027 artwork, modern dark-glass look.

Outputs to Resources/: bg.jpg (interface), knob.png, led_mask.png, logo.png, hero.jpg (splash).
If Resources/hero_user.jpg (e.g. generated in Google Flow) exists it replaces the procedural
splash and is also used, darkened and blurred, as the interface backdrop.
"""
import math
import os
import sys

import numpy as np
from PIL import Image, ImageDraw, ImageFont

import gfunk_spec as S

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "Resources")
os.makedirs(RES, exist_ok=True)

SC = 2
W, H = S.W * SC, S.H * SC
F = np.float32
rng = np.random.default_rng(2027)

FONT_IMPACT = "C:/Windows/Fonts/impact.ttf"
FONT_UI = "C:/Windows/Fonts/bahnschrift.ttf"
if not os.path.exists(FONT_UI):
    FONT_UI = "C:/Windows/Fonts/ariblk.ttf"
FONT_BLACK = "C:/Windows/Fonts/ariblk.ttf"

CYAN = np.array([0.0, 0.92, 1.0], F)
MAGENTA = np.array([1.0, 0.16, 0.82], F)
GOLD = np.array([1.0, 0.78, 0.25], F)


# ----------------------------------------------------------------------------- helpers
def box_blur(a, r, axis):
    if r < 1:
        return a
    pad = [(0, 0)] * a.ndim
    pad[axis] = (r + 1, r)
    p = np.pad(a, pad, mode="edge")
    c = np.cumsum(p, axis=axis, dtype=np.float64)
    n = a.shape[axis]
    hi = np.take(c, range(2 * r + 1, 2 * r + 1 + n), axis=axis)
    lo = np.take(c, range(0, n), axis=axis)
    return ((hi - lo) / (2 * r + 1)).astype(F)


def gblur(a, sigma):
    r = max(1, int(round(sigma * 1.1)))
    for _ in range(3):
        a = box_blur(box_blur(a, r, 0), r, 1)
    return a


def gblur3(a, sigma):
    return np.stack([gblur(a[..., c], sigma) for c in range(3)], -1)


def smooth(a, e0, e1):
    t = np.clip((a - e0) / (e1 - e0), 0, 1)
    return t * t * (3 - 2 * t)


def rr_sdf(X, Y, x, y, w, h, r):
    cx, cy = x + w / 2, y + h / 2
    qx = np.abs(X - cx) - (w / 2 - r)
    qy = np.abs(Y - cy) - (h / 2 - r)
    out = np.hypot(np.maximum(qx, 0), np.maximum(qy, 0)) + np.minimum(np.maximum(qx, qy), 0) - r
    return -out


def bez(p0, p1, p2, p3, n=48):
    out = []
    for i in range(n + 1):
        t = i / n
        u = 1 - t
        out.append((u**3 * p0[0] + 3 * u * u * t * p1[0] + 3 * u * t * t * p2[0] + t**3 * p3[0],
                    u**3 * p0[1] + 3 * u * u * t * p1[1] + 3 * u * t * t * p2[1] + t**3 * p3[1]))
    return out


def lighting(alb, h, spec, emis=None, hs=10.0, shin=36, L=(-0.42, -0.62, 0.66), ambient=0.30):
    gy, gx = np.gradient(h * hs)
    nz = 1 / np.sqrt(1 + gx * gx + gy * gy)
    nx, ny = -gx * nz, -gy * nz
    Lv = np.array(L, F); Lv /= np.linalg.norm(Lv)
    diff = np.clip(nx * Lv[0] + ny * Lv[1] + nz * Lv[2], 0, 1)
    Hv = Lv + np.array([0, 0, 1], F); Hv /= np.linalg.norm(Hv)
    ndh = np.clip(nx * Hv[0] + ny * Hv[1] + nz * Hv[2], 0, 1)
    col = alb * (ambient + 0.95 * diff)[..., None] + ((ndh ** shin) * spec)[..., None]
    if emis is not None:
        col = col + emis
    return col


def to_u8(col):
    return (np.clip(col, 0, 1) ** (1 / 1.08) * 255 + 0.5).astype(np.uint8)


# ----------------------------------------------------------------------------- synthwave scene
def palm(mask_draw, x, base_y, ht, lean, seed):
    r = np.random.default_rng(seed)
    pts = bez((x, base_y), (x + lean * 0.1, base_y - ht * 0.4), (x + lean * 0.7, base_y - ht * 0.8), (x + lean, base_y - ht), 60)
    for i, (px, py) in enumerate(pts):
        t = i / (len(pts) - 1)
        rad = (11 - 6 * t) * SC
        mask_draw.ellipse((px - rad, py - rad, px + rad, py + rad), fill=255)
    top = pts[-1]
    for k in range(11):
        ang = math.radians(-175 + k * 17 + r.uniform(-6, 6))
        ln = ht * r.uniform(0.36, 0.5)
        dx, dy = math.cos(ang), math.sin(ang)
        p1 = (top[0] + dx * ln * 0.45, top[1] + dy * ln * 0.45 - ln * 0.22)
        p2 = (top[0] + dx * ln * 0.85, top[1] + dy * ln * 0.55 - ln * 0.05)
        p3 = (top[0] + dx * ln, top[1] + dy * ln * 0.35 + ln * 0.55)
        rib = bez(top, p1, p2, p3, 40)
        mask_draw.line(rib, fill=255, width=int(3.2 * SC), joint="curve")
        for j in range(3, len(rib) - 1, 2):
            t = j / len(rib)
            ax, ay = rib[j]
            bx, by = rib[j + 1]
            tx, ty = bx - ax, by - ay
            nl = math.hypot(tx, ty) + 1e-6
            nxp, nyp = -ty / nl, tx / nl
            leaf = ln * 0.20 * (1 - t) ** 0.6 + 4 * SC
            for sgn in (-1, 1):
                mask_draw.line([(ax, ay), (ax + sgn * nxp * leaf + tx / nl * leaf * 0.5, ay + sgn * nyp * leaf + abs(leaf) * 0.65)],
                               fill=255, width=int(2.2 * SC))


def impala_body(width):
    """Side-view '64 Impala convertible lowrider body mask."""
    s = width / 300.0
    body = Image.new("L", (int(width), int(66 * s)), 0)
    bd = ImageDraw.Draw(body)
    pts = (bez((2, 52), (0, 44), (4, 38), (14, 35), 12) + [(60, 33), (70, 30), (84, 31), (130, 33)]
           + bez((130, 33), (146, 31), (158, 28), (168, 17), 14)[1:] + bez((168, 17), (172, 17), (174, 28), (178, 33), 8)[1:]
           + bez((178, 33), (210, 33), (250, 35), (278, 41), 14)[1:] + bez((278, 41), (292, 43), (298, 48), (296, 54), 8)[1:] + [(2, 54)])
    bd.polygon([(px * s, py * s) for px, py in pts], fill=255)
    return np.asarray(body, F) / 255.0, s


def draw_scene(with_car=True):
    yy, xx = np.mgrid[0:H, 0:W].astype(F)
    horizon = 0.62 * H
    t = np.clip(yy / horizon, 0, 1)[..., None]
    top = np.array([0.03, 0.01, 0.10], F)
    mid = np.array([0.35, 0.03, 0.40], F)
    low = np.array([1.0, 0.35, 0.28], F)
    sky = np.where(t < 0.62, top + (mid - top) * (t / 0.62), mid + (low - mid) * ((t - 0.62) / 0.38)).astype(F)
    cx, cy, R = W * 0.5, horizon - 0.02 * H, 0.26 * H
    d = np.hypot(xx - cx, yy - cy)
    sun = smooth(R - d, -2, 2)
    stripes = np.where(yy > cy - R * 0.05, ((yy - cy) / (R * 0.5)) % 1.0 > np.clip((yy - cy) / R * 0.9, 0.1, 0.8), True)
    sun *= stripes.astype(F)
    k = np.clip((yy - (cy - R)) / (2 * R), 0, 1)[..., None]
    sc = np.array([1.0, 0.95, 0.35], F) * (1 - k) + np.array([1.0, 0.1, 0.55], F) * k
    sky = sky * (1 - sun[..., None]) + sc * sun[..., None]
    glow = np.exp(-np.clip(d - R, 0, None) / (0.18 * H))[..., None] * np.array([1.0, 0.25, 0.6], F) * 0.55
    sky = sky + glow * (yy < horizon + 40)[..., None]
    floor_t = np.clip((yy - horizon) / (H - horizon), 0, 1)
    floor = np.array([0.06, 0.0, 0.14], F) + np.array([0.18, 0.0, 0.25], F) * (1 - floor_t[..., None]) ** 2
    zline = 1.0 / (floor_t * 1.6 + 0.02)
    grid_h = (np.abs(((zline * 1.2) % 1.0) - 0.5) > 0.47) & (yy > horizon)
    vx = (xx - cx) / (floor_t * 1.6 + 0.02) / (W * 0.06)
    grid_v = (np.abs((vx % 1.0) - 0.5) > 0.485) & (yy > horizon)
    grid = np.clip((grid_h | grid_v).astype(F) * (0.25 + 0.75 * floor_t), 0, 1)[..., None] * np.array([1.0, 0.2, 0.85], F) * 0.9
    img = np.where((yy > horizon)[..., None], floor + grid, sky).astype(F)

    pm = Image.new("L", (W, H), 0)
    pd = ImageDraw.Draw(pm)
    palm(pd, W * 0.07, H * 1.02, H * 0.78, W * 0.05, 1)
    palm(pd, W * 0.17, H * 1.02, H * 0.55, -W * 0.03, 2)
    palm(pd, W * 0.94, H * 1.02, H * 0.80, -W * 0.05, 3)
    palm(pd, W * 0.85, H * 1.02, H * 0.52, W * 0.03, 4)
    palm(pd, W * 0.30, horizon + 6, H * 0.22, W * 0.01, 5)
    palm(pd, W * 0.70, horizon + 6, H * 0.24, -W * 0.01, 6)
    pmask = np.asarray(pm, F) / 255.0
    pmask = smooth(gblur(pmask, 0.8), 0.25, 0.75)
    rim = np.clip(gblur(pmask, 3 * SC) - pmask, 0, 1)
    img = img * (1 - pmask[..., None] * 0.97) + (rim * 0.8)[..., None] * np.array([1.0, 0.3, 0.7], F) * 0.5

    if with_car:
        cw = int(W * 0.46)
        body, s = impala_body(cw)
        bh, bw = body.shape
        ox, oy = int(W * 0.27), int(horizon + 0.17 * H)
        sl = (slice(oy, oy + bh), slice(ox, ox + bw))
        ty = np.linspace(0, 1, bh, dtype=F)[:, None, None]
        paint = np.array([1.0, 0.42, 0.74], F) * (1.05 - 0.55 * ty) + np.array([0.3, 0.0, 0.25], F) * ty
        gy_, gx_ = np.gradient(gblur(body, 2 * SC) * 6)
        shade = np.clip(0.55 + 0.9 * (-gy_ * 0.8 - gx_ * 0.3), 0.2, 1.4)[..., None]
        img[sl] = img[sl] * (1 - body[..., None]) + paint * shade * body[..., None]
        yy2, xx2 = np.mgrid[0:bh, 0:bw]
        line = ((np.abs(yy2 - 0.62 * bh) < 2.2 * SC) & (body > 0.5)).astype(F)
        mural = ((np.abs(yy2 - 0.70 * bh) < 5.5 * SC) & (body > 0.5) & (xx2 > 0.10 * bw) & (xx2 < 0.88 * bw)).astype(F)
        flowers = (np.sin(xx2 / (6.0 * SC)) * np.sin(yy2 / (3.0 * SC)) > 0.35).astype(F) * mural
        img[sl] = img[sl] * (1 - line[..., None]) + line[..., None] * GOLD
        img[sl] = img[sl] * (1 - mural[..., None] * 0.75) + mural[..., None] * 0.75 * np.array([0.40, 0.05, 0.28], F)
        img[sl] = img[sl] * (1 - flowers[..., None]) + flowers[..., None] * np.array([1.0, 0.85, 0.4], F)
        inter = Image.new("L", (bw, bh), 0)
        ImageDraw.Draw(inter).polygon([(86 * s, 31 * s), (132 * s, 33 * s), (150 * s, 29.5 * s), (120 * s, 26 * s), (92 * s, 27 * s)], fill=255)
        im = np.asarray(inter, F) / 255.0 * (body > 0.5)
        img[sl] = img[sl] * (1 - im[..., None] * 0.9) + im[..., None] * 0.9 * np.array([0.25, 0.02, 0.2], F)
        wsm = Image.new("L", (bw, bh), 0)
        ImageDraw.Draw(wsm).polygon([(150 * s, 30 * s), (163 * s, 20 * s), (167 * s, 20 * s), (171 * s, 32 * s)], fill=255)
        wm = np.asarray(wsm, F) / 255.0
        img[sl] = img[sl] * (1 - wm[..., None] * 0.8) + wm[..., None] * 0.8 * np.array([0.65, 0.95, 1.0], F)

        # the Latina rising from the car: stylised cut-out with rose and hoop
        hx = 112 * s
        base_y = int(bh * 1.3)
        fig = Image.new("L", (bw, base_y + int(14 * s)), 0)
        fd = ImageDraw.Draw(fig)
        head = (hx, base_y - 36 * s + 8 * s)
        fd.ellipse((head[0] - 7.4 * s, head[1] - 9 * s, head[0] + 7.4 * s, head[1] + 9 * s), fill=255)
        hair = bez((head[0] - 8 * s, head[1] - 4 * s), (head[0] - 22 * s, head[1] + 4 * s), (head[0] - 17 * s, head[1] + 24 * s), (head[0] - 22 * s, head[1] + 33 * s), 20) + \
            bez((head[0] - 22 * s, head[1] + 33 * s), (head[0] - 6 * s, head[1] + 30 * s), (head[0] + 6 * s, head[1] + 24 * s), (head[0] + 9 * s, head[1] + 8 * s), 20)
        fd.polygon(hair + [(head[0] + 6 * s, head[1] - 7 * s), (head[0] - 3 * s, head[1] - 12 * s)], fill=255)
        fd.polygon([(head[0] - 3 * s, head[1] + 8 * s), (head[0] + 3 * s, head[1] + 8 * s), (head[0] + 4 * s, base_y), (head[0] - 4 * s, base_y)], fill=255)
        fd.polygon([(head[0] - 14 * s, base_y - 2 * s), (head[0] - 3 * s, head[1] + 12 * s), (head[0] + 3 * s, head[1] + 12 * s), (head[0] + 14 * s, base_y - 2 * s), (head[0] + 14 * s, base_y + 10 * s), (head[0] - 14 * s, base_y + 10 * s)], fill=255)
        fm = np.asarray(fig, F) / 255.0
        fx0 = ox
        fy0 = oy + int(33 * s) - base_y
        hh = fm.shape[0]
        sl2 = (slice(fy0, fy0 + hh), slice(fx0, fx0 + bw))
        fm_s = smooth(gblur(fm, 0.7), 0.3, 0.7)
        img[sl2] = img[sl2] * (1 - fm_s[..., None] * 0.96) + fm_s[..., None] * 0.96 * np.array([0.04, 0.0, 0.06], F)
        rimf = np.clip(gblur(fm_s, 2.5 * SC) - fm_s, 0, 1)
        img[sl2] += (rimf * 0.9)[..., None] * np.array([1.0, 0.3, 0.7], F)
        rose_c = (fx0 + head[0] + 6.5 * s, fy0 + head[1] - 5 * s)
        rr = 6.2 * s
        y0r, y1r = int(rose_c[1] - rr - 4), int(rose_c[1] + rr + 4)
        x0r, x1r = int(rose_c[0] - rr - 4), int(rose_c[0] + rr + 4)
        ry, rx = np.mgrid[y0r:y1r, x0r:x1r].astype(F)
        dd = np.hypot(rx - rose_c[0], ry - rose_c[1])
        ang = np.arctan2(ry - rose_c[1], rx - rose_c[0])
        petals = 0.55 + 0.45 * np.cos(dd / (rr * 0.18) + 2.2 * ang)
        rose = smooth(rr - dd, -1, 1)
        rc = (np.array([1.0, 0.25, 0.55], F) * petals[..., None]) * (0.6 + 0.6 * (1 - dd / rr))[..., None]
        img[y0r:y1r, x0r:x1r] = img[y0r:y1r, x0r:x1r] * (1 - rose[..., None]) + np.clip(rc, 0, 1) * rose[..., None]
        hoop_c = (fx0 + head[0] - 8.5 * s, fy0 + head[1] + 9 * s)
        y0h, y1h = int(hoop_c[1] - 9 * s), int(hoop_c[1] + 9 * s)
        x0h, x1h = int(hoop_c[0] - 9 * s), int(hoop_c[0] + 9 * s)
        hy2, hx2 = np.mgrid[y0h:y1h, x0h:x1h].astype(F)
        hd = np.hypot(hx2 - hoop_c[0], hy2 - hoop_c[1])
        hm = smooth(1.3 * SC - np.abs(hd - 6.5 * s), -0.5, 0.5)
        img[y0h:y1h, x0h:x1h] = img[y0h:y1h, x0h:x1h] * (1 - hm[..., None]) + hm[..., None] * GOLD

        for wx in (60, 238):
            c = (ox + wx * s, oy + 54 * s)
            rr = 13.5 * s
            y0w, y1w = int(c[1] - rr - 3), int(c[1] + rr + 3)
            x0w, x1w = int(c[0] - rr - 3), int(c[0] + rr + 3)
            wy, wxg = np.mgrid[y0w:y1w, x0w:x1w].astype(F)
            dd = np.hypot(wxg - c[0], wy - c[1])
            ang = np.arctan2(wy - c[1], wxg - c[0])
            tire = smooth(rr - dd, -1, 1)
            rim_m = smooth(rr * 0.74 - dd, -1, 1)
            spokes = (np.abs(np.sin(ang * 36)) > 0.55).astype(F) * rim_m
            wcol = np.array([0.03, 0.03, 0.04], F) * np.ones_like(dd)[..., None]
            wcol = wcol * (1 - rim_m[..., None]) + rim_m[..., None] * np.array([0.78, 0.8, 0.88], F)
            wcol = wcol * (1 - spokes[..., None] * 0.7) + spokes[..., None] * 0.7 * np.array([0.12, 0.12, 0.16], F)
            ring = smooth(1.1 * SC - np.abs(dd - rr * 0.78), -0.5, 0.5)
            wcol = wcol * (1 - ring[..., None]) + ring[..., None] * CYAN
            hub = smooth(rr * 0.16 - dd, -1, 1)
            wcol = wcol * (1 - hub[..., None]) + hub[..., None] * GOLD
            img[y0w:y1w, x0w:x1w] = img[y0w:y1w, x0w:x1w] * (1 - tire[..., None]) + wcol * tire[..., None]
        gl = np.zeros((H, W), F)
        gy0 = oy + int(60 * s)
        gl[gy0 - 6 * SC:gy0 + 6 * SC, ox:ox + bw] = 1.0
        gl = gblur(gl, 14 * SC)
        img += gl[..., None] * np.array([1.0, 0.15, 0.8], F) * 1.2
    return img


# ----------------------------------------------------------------------------- text / logo
def text_mask(text, font_path, size, spacing=0, shear=0.0):
    font = ImageFont.truetype(font_path, int(size))
    probe = ImageDraw.Draw(Image.new("L", (4, 4)))
    widths = [probe.textlength(c, font=font) for c in text]
    tw = int(sum(widths) + spacing * (len(text) - 1))
    th = int(size * 1.5)
    pad = int(size * 0.4)
    img = Image.new("L", (tw + 2 * pad, th + 2 * pad), 0)
    dr = ImageDraw.Draw(img)
    cx = pad
    for c, wd in zip(text, widths):
        dr.text((cx, pad + th // 2), c, font=font, fill=255, anchor="lm")
        cx += wd + spacing
    if shear:
        img = img.transform(img.size, Image.AFFINE, (1, shear, -shear * img.size[1] / 2, 0, 1, 0), Image.BICUBIC)
    return np.asarray(img, F) / 255.0


def render_logo():
    """3D chrome GFUNK + gold 2027 with neon rim light. Transparent PNG (640x120, 2x).
    A logo generated in Google Flow (Resources/logo_user.png) takes priority."""
    user = os.path.join(RES, "logo_user.png")
    if os.path.exists(user):
        import shutil
        shutil.copyfile(user, os.path.join(RES, "logo.png"))
        print("logo: using logo_user.png")
        return
    lw, lh = 640, 120
    a = text_mask("GFUNK", FONT_IMPACT, 84, spacing=3, shear=0.18)
    b = text_mask("2027", FONT_IMPACT, 84, spacing=3, shear=0.18)
    ha, wa = a.shape
    hb, wb = b.shape
    off = wa - 40
    cw_, ch_ = off + wb, max(ha, hb)
    face = np.zeros((ch_, cw_), F)
    face[:ha, :wa] = a
    is2027 = np.zeros_like(face)
    mb = np.zeros_like(face)
    mb[:hb, off:off + wb] = b
    face = np.maximum(face, mb)
    is2027[:, off + 30:] = 1.0
    ext = face.copy()
    for k in range(1, 8):
        sh = np.zeros_like(face)
        sh[k:, k:] = face[:-k, :-k]
        ext = np.maximum(ext, sh)
    h = gblur(ext, 0.7) * 0.9 + gblur(face, 1.1) * 1.3
    t = np.linspace(0, 1, ch_, dtype=F)[:, None]
    chrome = np.interp(t, [0, 0.42, 0.5, 0.58, 1], [1.0, 0.72, 0.10, 0.62, 0.95])
    chrome_col = chrome[..., None] * np.array([0.92, 0.96, 1.0], F)
    gk = np.interp(t[:, 0], [0, 0.3, 0.52, 1], [1.0, 0.95, 0.62, 0.95])
    gold = np.stack([gk * 1.0, gk * 0.78, gk * 0.25], -1)[:, None, :]
    gold = np.broadcast_to(gold, face.shape + (3,))
    alb = chrome_col * (1 - is2027[..., None]) + gold * is2027[..., None]
    alb = alb * face[..., None] + np.array([0.40, 0.04, 0.35], F) * (ext - face)[..., None]
    col = lighting(alb, h, ext, hs=9, shin=24, ambient=0.42) * ext[..., None]
    outline = np.clip(gblur(ext, 2.2) * 1.8 - ext, 0, 1)
    glow = gblur(ext, 9)
    glow_col = CYAN * (1 - t)[..., None] + MAGENTA * t[..., None]
    rgb = col + glow[..., None] * glow_col * 0.5 * (1 - ext[..., None]) + outline[..., None] * glow_col * 0.5
    alpha = np.clip(ext + glow * 0.55, 0, 1)
    rgba = np.concatenate([np.clip(rgb, 0, 1), alpha[..., None]], -1)
    img = Image.fromarray((rgba * 255 + 0.5).astype(np.uint8), "RGBA")
    bbox = img.getchannel("A").point(lambda v: 255 if v > 6 else 0).getbbox()
    if bbox:
        img = img.crop(bbox)
    scale = min((lw - 8) / img.width, (lh - 4) / img.height)
    img = img.resize((int(img.width * scale), int(img.height * scale)), Image.LANCZOS)
    out = Image.new("RGBA", (lw, lh), (0, 0, 0, 0))
    out.alpha_composite(img, (4, (lh - img.height) // 2))
    out.save(os.path.join(RES, "logo.png"))
    print("logo", out.size)


# ----------------------------------------------------------------------------- hero / backdrop
def render_hero():
    user = os.path.join(RES, "hero_user.jpg")
    if os.path.exists(user):
        im = Image.open(user).convert("RGB")
        r = max(W / im.width, H / im.height)
        im = im.resize((int(im.width * r) + 1, int(im.height * r) + 1), Image.LANCZOS)
        left, top = (im.width - W) // 2, (im.height - H) // 2
        im = im.crop((left, top, left + W, top + H))
        arr = np.asarray(im, F) / 255.0
        print("hero: using hero_user.jpg")
    else:
        arr = draw_scene(with_car=False)
        lg = Image.open(os.path.join(RES, "logo.png")).convert("RGBA")
        lg = lg.resize((int(W * 0.5), int(W * 0.5 * lg.height / lg.width)), Image.LANCZOS)
        base = Image.fromarray(to_u8(arr)).convert("RGBA")
        base.alpha_composite(lg, ((W - lg.width) // 2, int(H * 0.05)))
        arr = np.asarray(base.convert("RGB"), F) / 255.0
    yy, xx = np.mgrid[0:H, 0:W].astype(F)
    v = 1 - 0.45 * (((xx / W - 0.5) * 1.5) ** 2 + ((yy / H - 0.5) * 1.5) ** 2)
    arr = arr * v[..., None]
    Image.fromarray(to_u8(arr)).save(os.path.join(RES, "hero.jpg"), quality=92, subsampling=0)
    print("hero saved")
    return arr


def backdrop(hero_arr):
    small = Image.fromarray(to_u8(hero_arr)).resize((W // 4, H // 4), Image.LANCZOS)
    a = gblur3(np.asarray(small, F) / 255.0, 6)
    big = Image.fromarray(to_u8(a)).resize((W, H), Image.BICUBIC)
    return np.asarray(big, F) / 255.0


# ----------------------------------------------------------------------------- interface (satin plastic + brushed metal)
FONT_TITLE = "C:/Windows/Fonts/arialbi.ttf"
if not os.path.exists(FONT_TITLE):
    FONT_TITLE = FONT_BLACK

ALB = HT = SPEC = EMIS = None
PLASTIC_TOP = np.array([0.80, 0.10, 0.40], F)       # candy rose
PLASTIC_BOT = np.array([0.46, 0.03, 0.24], F)


def init_maps():
    global ALB, HT, SPEC, EMIS
    n = box_blur(rng.random((H, W)).astype(F), 70, 1)
    n = (n - n.mean()) / (n.std() + 1e-6)
    n2 = box_blur(rng.random((H, W)).astype(F), 6, 1)
    n2 = (n2 - n2.mean()) / (n2.std() + 1e-6)
    yy = np.linspace(0, 1, H, dtype=F)[:, None]
    g = 0.075 + 0.03 * (1 - yy) + 0.014 * n + 0.008 * n2
    ALB = (g[..., None] * np.array([0.95, 0.98, 1.06], F)).astype(F)
    HT = (n2 * 0.03).astype(F)
    SPEC = np.full((H, W), 0.45, F)
    EMIS = np.zeros((H, W, 3), F)


def region(x, y, w, h, m):
    x0, y0 = max(0, int((x - m) * SC)), max(0, int((y - m) * SC))
    x1, y1 = min(W, int((x + w + m) * SC)), min(H, int((y + h + m) * SC))
    yy, xx = np.mgrid[y0:y1, x0:x1].astype(F)
    return (slice(y0, y1), slice(x0, x1)), xx, yy


def raised(x, y, w, h, radius, kind):
    """Raised panel. kind: 'plastic' (satin rose) or 'metal' (brushed steel)."""
    sl, X, Y = region(x, y, w, h, 34)
    d = rr_sdf(X, Y, x * SC, y * SC, w * SC, h * SC, radius * SC)
    mask = (d > 0)
    inside = smooth(d, -0.7, 0.7)
    sh = gblur(np.roll((d > 0).astype(F), 7 * SC, axis=0), 8 * SC)
    ALB[sl] *= (1 - 0.6 * sh * (1 - inside))[..., None]

    shp = d.shape
    ty = np.clip((Y - y * SC) / (h * SC), 0, 1)[..., None]
    if kind == "plastic":
        base = PLASTIC_TOP * (1 - ty) + PLASTIC_BOT * ty
        grain = rng.normal(0, 1, shp).astype(F)
        brush = box_blur(rng.random(shp).astype(F), 36, 1)
        brush = (brush - brush.mean()) / (brush.std() + 1e-6)
        alb = base * (1 + 0.020 * gblur(grain, 0.6)[..., None] + 0.010 * brush[..., None])
        sparkle = (rng.random(shp) > 0.9985).astype(F) * 0.25
        alb = alb + sparkle[..., None]
        spec = np.full(shp, 0.40, F)
        tex = 0.012 * gblur(grain, 0.8) + 0.010 * brush
    else:
        n = box_blur(rng.random(shp).astype(F), 60, 1)
        n = (n - n.mean()) / (n.std() + 1e-6)
        n2 = box_blur(rng.random(shp).astype(F), 5, 1)
        n2 = (n2 - n2.mean()) / (n2.std() + 1e-6)
        g = 0.095 + 0.045 * (1 - ty[..., 0]) + 0.020 * n + 0.012 * n2
        alb = (g[..., None] * np.array([0.95, 0.98, 1.06], F)).astype(F)
        spec = np.full(shp, 0.55, F)
        tex = 0.035 * n2
    bev = 4.5 * SC
    hh = smooth(d, 0, bev) * 1.0 + tex
    ALB[sl] = np.where(mask[..., None], alb, ALB[sl])
    HT[sl] = np.where(d > -3, hh, HT[sl])
    SPEC[sl] = np.where(mask, spec, SPEC[sl])


def sunken(x, y, w, h, radius, color, depth=0.9):
    sl, X, Y = region(x, y, w, h, 10)
    d = rr_sdf(X, Y, x * SC, y * SC, w * SC, h * SC, radius * SC)
    mask = d > 0
    ALB[sl] = np.where(mask[..., None], np.array(color, F), ALB[sl])
    HT[sl] = np.where(d > -3, -smooth(d, 0, 4.5 * SC) * depth, HT[sl])
    SPEC[sl] = np.where(mask, 0.7, SPEC[sl])


def print_text(text, x, y, size, color, spacing=1.0, font=None, anchor="l"):
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
    HT[ys:ye, xs:xe] += gblur(m, 0.5 * SC) * 0.10
    return tw / SC


def hline(x0, x1, y, color, thick=1.2, alpha=0.8):
    ys = slice(int((y - thick / 2) * SC), int((y + thick / 2) * SC) + 1)
    xs = slice(int(x0 * SC), int(x1 * SC))
    ALB[ys, xs] = ALB[ys, xs] * (1 - alpha) + np.array(color, F) * alpha


def render_background(hero_arr=None):
    init_maps()
    white = (0.96, 0.96, 0.98)

    hx, hy, hw, hh = S.HEADER_RECT
    raised(hx, hy, hw, hh, 12, "metal")
    for name, title, (x, y, w, h), cols, rows in S.PANELS:
        if name == "screen":
            raised(x, y, w, h, 16, "metal")
        elif name == "keys":
            raised(x, y, w, h, 14, "metal")
        else:
            raised(x, y, w, h, 14, "plastic")
            print_text(title, x + 20, y + 20, 13, white, spacing=1.6)
            hline(x + 20, x + w - 20, y + 31, white, 1.4, 0.55)

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
    print_text("WEST COAST  G-FUNK  EDITION", kx0 + kw0 / 2, ky0 + 22, 12, (0.92, 0.74, 0.32), spacing=3.2, anchor="m")
    print_text("HUMPIRE by Djbilbox BEATS", hx + 26, hy + hh - 11, 10.5, (0.78, 0.82, 0.90), spacing=2.4, font=FONT_UI)

    col = lighting(ALB, HT, SPEC, EMIS, hs=11.0, shin=20, ambient=0.62)
    yy, xx = np.mgrid[0:H, 0:W].astype(F)
    v = 1 - 0.30 * (((xx / W - 0.5) * 1.5) ** 2 + ((yy / H - 0.5) * 1.5) ** 2)
    col *= v[..., None]
    Image.fromarray(to_u8(col)).save(os.path.join(RES, "bg.jpg"), quality=94, subsampling=0)
    print("bg saved", os.path.getsize(os.path.join(RES, "bg.jpg")) // 1024, "KB")


# ----------------------------------------------------------------------------- knob + LED mask
def render_knob(size_logical=64, ss=4):
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
    h = np.where(grip, 0.62 + 0.10 * ridges, h)
    h = np.where(top, 0.72 + 0.06 * np.sqrt(np.clip(1 - (r / 0.70) ** 2, 0, 1)), h)
    h = np.where(body & (np.abs(r - 0.70) < 0.02), h - 0.08, h)

    shade = 0.040 + 0.020 * (1 - np.clip(yy / n, 0, 1))
    alb = np.stack([shade, shade, shade * 1.06], -1)
    alb = np.where(skirt[..., None], np.array([0.55, 0.56, 0.60], F), alb)
    lobe = (0.5 + 0.5 * np.cos(2 * (th - math.radians(-125)))) ** 5
    sheen = np.where(top, lobe * 0.10 * (0.4 + 0.6 * np.clip(r / 0.7, 0, 1)), 0)
    fine = np.interp(np.clip(r, 0, 1) * 220, np.arange(221), rng.random(221)).astype(F)
    alb = alb + sheen[..., None] * (0.7 + 0.6 * fine)[..., None]
    spec = np.where(skirt, 0.9, np.where(top, 0.30, 0.18))
    col = lighting(alb, h, spec, hs=16 * (n / 128), shin=14, ambient=0.8)
    gl = np.exp(-(((xx - c) / (Rb * 0.55)) ** 2 + ((yy - (c - Rb * 0.42)) / (Rb * 0.17)) ** 2)) * top
    col = col + gl[..., None] * 0.06
    col = np.clip(col, 0, 1)

    alpha = np.where(body, np.clip((1.0 - r) * Rb / 1.2, 0, 1), 0).astype(F)
    disc = body.astype(F)
    sh = np.roll(np.roll(disc, int(3 * ss * 2), axis=0), int(1.0 * ss * 2), axis=1)
    sh = gblur(sh, 5.0 * ss * 2 / 2) * 0.85
    out_a = alpha + sh * (1 - alpha)
    out_c = (col * alpha[..., None]) / np.maximum(out_a[..., None], 1e-5)
    rgba = np.concatenate([np.clip(out_c, 0, 1), out_a[..., None]], -1)
    img = Image.fromarray((rgba * 255 + 0.5).astype(np.uint8), "RGBA").resize((size_logical * 2, size_logical * 2), Image.LANCZOS)
    img.save(os.path.join(RES, "knob.png"))
    print("knob", img.size)


def render_led_mask(pitch=2, scale=2):
    x, y, w, h = S.SCREEN_INNER
    pw, ph = w * scale, h * scale
    p = pitch * scale
    yy, xx = np.mgrid[0:ph, 0:pw].astype(F)
    fx = (xx % p) - (p - 1) / 2
    fy = (yy % p) - (p - 1) / 2
    dd = np.hypot(fx, fy)
    hole = np.clip((p * 0.40 + 0.6 - dd) / 1.2, 0, 1)
    alpha = (1 - hole) * 0.88
    d = rr_sdf(xx, yy, 0, 0, pw, ph, 8 * scale)
    alpha = np.where(d > 0, alpha, 1.0)
    rgba = np.zeros((ph, pw, 4), np.uint8)
    rgba[..., 3] = (alpha * 255).astype(np.uint8)
    Image.fromarray(rgba, "RGBA").save(os.path.join(RES, "led_mask.png"))
    print("led mask", pw, ph)


if __name__ == "__main__":
    what = sys.argv[1] if len(sys.argv) > 1 else "all"
    if what in ("all", "knob"):
        render_knob()
    if what in ("all", "led"):
        render_led_mask()
    if what in ("all", "logo"):
        render_logo()
    if what in ("all", "bg", "hero"):
        hero = render_hero()
        if what in ("all", "bg"):
            render_background(hero)
