# Animates a studio still into a cinematic HD loop:
# slow push-in, pulsing green/magenta LEDs with bloom, drifting haze, floating dust.
# python tools/studio_video.py in.jpg out.mp4 [--sec 8] [--zoom 0.07] [--fx 0.5 --fy 0.45] [--w 1920]
import sys, argparse, subprocess, numpy as np, cv2

ap = argparse.ArgumentParser()
ap.add_argument('src'); ap.add_argument('out')
ap.add_argument('--sec', type=float, default=8); ap.add_argument('--fps', type=int, default=30)
ap.add_argument('--zoom', type=float, default=0.07)
ap.add_argument('--fx', type=float, default=0.5); ap.add_argument('--fy', type=float, default=0.45)
ap.add_argument('--pan', type=float, default=0.0, help='horizontal drift as a fraction of width')
ap.add_argument('--w', type=int, default=1920); ap.add_argument('--crf', type=int, default=24)
a = ap.parse_args()

W = a.w; H = W * 9 // 16
img = cv2.imread(a.src, cv2.IMREAD_COLOR)
# cover-crop the source to 16:9 at 1.25x the output size (room for the push-in)
S = 1.25; sw, sh = int(W * S), int(H * S)
h0, w0 = img.shape[:2]; r = max(sw / w0, sh / h0)
img = cv2.resize(img, (int(w0 * r + .5), int(h0 * r + .5)), interpolation=cv2.INTER_LANCZOS4)
y0 = (img.shape[0] - sh) // 2; x0 = (img.shape[1] - sw) // 2
base = img[y0:y0 + sh, x0:x0 + sw].astype(np.float32) / 255.0

# LED masks from colour: green and magenta/purple glowing pixels
hsv = cv2.cvtColor((base * 255).astype(np.uint8), cv2.COLOR_BGR2HSV).astype(np.float32)
hh, ss, vv = hsv[..., 0], hsv[..., 1] / 255, hsv[..., 2] / 255
glow = np.clip((ss - .35) / .4, 0, 1) * np.clip((vv - .35) / .4, 0, 1)
mg = glow * ((hh > 35) & (hh < 90))            # green
mm = glow * ((hh > 125) & (hh < 170))          # magenta / purple
def blur(m, k): return cv2.GaussianBlur(m, (0, 0), k)
bloom_g = blur(mg, sh * .02)[..., None] * np.array([.25, 1, .35], np.float32)
bloom_m = blur(mm, sh * .02)[..., None] * np.array([1, .3, .95], np.float32)
mg3, mm3 = blur(mg, 2)[..., None], blur(mm, 2)[..., None]

# haze: two tileable low-frequency noise fields that scroll
rng = np.random.default_rng(7)
def noise(scale):
    n = rng.random((sh // scale + 2, sw // scale + 2)).astype(np.float32)
    n = cv2.resize(n, (sw * 2, sh), interpolation=cv2.INTER_CUBIC)
    return blur(n, scale * .6)
fog1, fog2 = noise(90), noise(160)
fog_tint = (base.mean(axis=(0, 1)) * .6 + np.array([.55, .45, .6], np.float32) * .4)

# dust particles
N = 260
px, py = rng.random(N) * sw, rng.random(N) * sh
pv = 3 + rng.random(N) * 9; pr = .6 + rng.random(N) * 1.6; pa = .25 + rng.random(N) * .6; ph = rng.random(N) * 6.28

T = int(a.sec * a.fps); XF = int(a.fps * 1.0)   # 1 s crossfade for a seamless loop
ff = subprocess.Popen(['ffmpeg', '-y', '-loglevel', 'error', '-f', 'rawvideo', '-pix_fmt', 'bgr24', '-s', f'{W}x{H}',
                       '-r', str(a.fps), '-i', '-', '-c:v', 'libx264', '-preset', 'slow', '-crf', str(a.crf),
                       '-pix_fmt', 'yuv420p', '-movflags', '+faststart', '-an', a.out], stdin=subprocess.PIPE)

def frame(t):
    u = t / T                                     # 0..1 over the clip
    e = .5 - .5 * np.cos(np.pi * u)               # ease in-out
    pg = .5 + .5 * np.sin(2 * np.pi * (2 * u))    # two pulses per loop
    pm = .5 + .5 * np.sin(2 * np.pi * (2 * u) + np.pi)
    f = base.copy()
    f += mg3 * f * (.35 * pg - .1) + mm3 * f * (.35 * pm - .1)
    f += bloom_g * (.18 + .22 * pg) + bloom_m * (.18 + .22 * pm)
    o1 = int(u * sw) % sw; o2 = int(u * sw * .5) % sw
    fog = fog1[:, o1:o1 + sw] * .6 + fog2[:, o2:o2 + sw] * .4
    fog = np.clip((fog - .45) * 2.2, 0, 1)[..., None] * fog_tint * .16
    f = 1 - (1 - f) * (1 - fog)                   # screen blend
    lay = np.zeros((sh, sw), np.float32)
    yy = (py - pv * t / a.fps * 6) % sh; xx = (px + 8 * np.sin(ph + t / a.fps)) % sw
    for x, y, rr, al in zip(xx, yy, pr, pa):
        cv2.circle(lay, (int(x), int(y)), int(rr + .5), float(al), -1, cv2.LINE_AA)
    f += blur(lay, 1.2)[..., None] * np.array([.85, .95, .9], np.float32) * .5
    z = 1 + a.zoom * e
    cw, chh = sw / z, sh / z                      # whole frame at the start, then push in
    cx = sw * a.fx + a.pan * sw * (e - .5); cy = sh * a.fy
    cx = min(max(cx, cw / 2), sw - cw / 2); cy = min(max(cy, chh / 2), sh - chh / 2)
    M = np.float32([[W / cw, 0, -(cx - cw / 2) * W / cw], [0, H / chh, -(cy - chh / 2) * H / chh]])
    out = cv2.warpAffine(f, M, (W, H), flags=cv2.INTER_CUBIC)
    vig = getattr(frame, 'vig', None)
    if vig is None:
        yv, xv = np.mgrid[0:H, 0:W].astype(np.float32)
        d = ((xv / W - .5) ** 2 + (yv / H - .5) ** 2) ** .5
        frame.vig = vig = (1 - np.clip((d - .35) * 1.1, 0, .45))[..., None]
    return np.clip(out * vig, 0, 1)

frames_head = []
for t in range(T):
    fr = frame(t)
    if t < XF: frames_head.append(fr); continue
    if t >= T - XF:                               # blend the tail into the head
        k = (t - (T - XF)) / XF
        fr = fr * (1 - k) + frames_head[t - (T - XF)] * k
    ff.stdin.write((fr * 255).astype(np.uint8).tobytes())
ff.stdin.close(); ff.wait()
print('wrote', a.out)
