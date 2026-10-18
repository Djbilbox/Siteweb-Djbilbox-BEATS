#!/usr/bin/env python3
"""
analyze_library.py - objective quality gate for the HUMPIRE CHAABI ORCHESTRA built-in modelled library.

Reads the SFZ + WAV files written by ChaabiRenderLibrary (or any SFZ library with the same layout) and prints,
for every zone: peak dBFS, loudness (max RMS over any 50 ms window), DC offset, pitch error in cents against the
zone's pitch_keycenter (pitched zones), decay time to -40 dB, attack time, click detection (start / end / loop seam /
internal glitches) and spectral centroid.  Then it applies a PASS / FAIL gate.

    ChaabiRenderLibrary --out /tmp/lib --bits 24
    python3 tools/analyze_library.py /tmp/lib                       # all instruments, all zones
    python3 tools/analyze_library.py /tmp/lib --instrument Mandole --failures-only
    python3 tools/analyze_library.py /tmp/lib --csv metrics.csv --spectrograms /tmp/specs

Gate (per zone)                                                    default        option
  peak                                       -24 dBFS <= peak <= -1 dBFS
  |DC offset|                                < 0.002
  pitch error (pitched zones, vs keycenter)  <= 8 cents            --max-cents
  clicks                                     first frame ~0, end faded, no loop-seam jump, no internal glitch
  loop seam                                  jump <= 3 x local step, slope step <= 5 x local step, level match <= 1.5 dB
Gate (per instrument)
  loudness inside one (articulation, velocity layer) group:  every zone within +-4 dB of the group median
  loudness of the hard layers across articulations:          within +-4 dB of the median (ghost / roll / zil lanes excluded)
  velocity layers:                                           the soft layer must be quieter than the hard layer

Dependencies: numpy only (spectrogram PNGs are written with zlib, no matplotlib needed).
Exit code: 0 = every zone and instrument PASS, 1 = at least one FAIL, 2 = usage / read error.
"""
import argparse
import csv
import math
import os
import re
import struct
import sys
import zlib

import numpy as np

# ------------------------------------------------------------------------------ thresholds
THRESH = dict(
    peak_max_db=-1.0, peak_min_db=-24.0, dc_max=0.002, pitch_cents=8.0,
    start_max=0.01, end_max=0.002, seam_jump=3.0, seam_slope=5.0, seam_level_db=1.5,
    group_loudness_db=4.0, across_loudness_db=4.0, glitch_ratio=20.0,
)
UNPITCHED_LANES_EXCLUDED_FROM_ACROSS = {"ghost", "roll", "zil", "air", "bowlift"}


# ------------------------------------------------------------------------------ WAV
def read_wav(path):
    """Returns (data[frames, channels] float64, sample_rate, loop (start, end) or None)."""
    with open(path, "rb") as f:
        raw = f.read()
    if raw[:4] != b"RIFF" or raw[8:12] != b"WAVE":
        raise ValueError("not a RIFF/WAVE file")
    pos, fmt, data, loop = 12, None, None, None
    while pos + 8 <= len(raw):
        cid, size = raw[pos:pos + 4], struct.unpack("<I", raw[pos + 4:pos + 8])[0]
        body = raw[pos + 8:pos + 8 + size]
        if cid == b"fmt ":
            tag, ch, sr, _, _, bits = struct.unpack("<HHIIHH", body[:16])
            if tag == 0xFFFE and len(body) >= 26:
                tag = struct.unpack("<H", body[24:26])[0]
            fmt = (tag, ch, sr, bits)
        elif cid == b"data":
            data = body
        elif cid == b"smpl" and len(body) >= 60 and struct.unpack("<I", body[28:32])[0] >= 1:
            loop = struct.unpack("<II", body[44:52])
        pos += 8 + size + (size & 1)
    if fmt is None or data is None:
        raise ValueError("missing fmt/data chunk")
    tag, ch, sr, bits = fmt
    if tag == 3 and bits == 32:
        a = np.frombuffer(data, dtype="<f4").astype(np.float64)
    elif tag == 1 and bits == 16:
        a = np.frombuffer(data, dtype="<i2").astype(np.float64) / 32768.0
    elif tag == 1 and bits == 24:
        b = np.frombuffer(data[:len(data) // 3 * 3], dtype=np.uint8).reshape(-1, 3).astype(np.int32)
        v = b[:, 0] | (b[:, 1] << 8) | (b[:, 2] << 16)
        v = np.where(v >= 1 << 23, v - (1 << 24), v)
        a = v.astype(np.float64) / 8388608.0
    elif tag == 1 and bits == 32:
        a = np.frombuffer(data, dtype="<i4").astype(np.float64) / 2147483648.0
    else:
        raise ValueError("unsupported WAV format tag=%d bits=%d" % (tag, bits))
    a = a[:len(a) // ch * ch].reshape(-1, ch)
    return a, sr, loop


# ------------------------------------------------------------------------------ SFZ
NOTE_BASE = {"c": 0, "d": 2, "e": 4, "f": 5, "g": 7, "a": 9, "b": 11}


def note_or_int(v, default=0):
    v = v.strip().lower()
    if re.fullmatch(r"-?\d+", v):
        return int(v)
    m = re.fullmatch(r"([a-g])([#b]?)(-?\d+)", v)
    if m:
        s = NOTE_BASE[m.group(1)] + (1 if m.group(2) == "#" else -1 if m.group(2) == "b" else 0)
        return max(0, min(127, (int(m.group(3)) + 1) * 12 + s))
    return default


def parse_sfz(path):
    text = open(path, encoding="utf-8", errors="replace").read()
    text = re.sub(r"/\*.*?\*/", "", text, flags=re.S)
    text = re.sub(r"//[^\n]*", "", text)
    toks = list(re.finditer(r"<(\w+)>|([A-Za-z_][A-Za-z0-9_$]*)=", text))
    scopes = {"control": {}, "global": {}, "master": {}, "group": {}, "region": {}}
    cur, regions, sw = None, [], {}
    base = os.path.dirname(os.path.abspath(path))

    def commit():
        if cur != "region":
            return
        o = {}
        for k in ("global", "master", "group", "region"):
            o.update(scopes[k])
        if "sample" not in o:
            return
        rel = o["sample"].replace("\\", "/")
        dp = scopes["control"].get("default_path", "").replace("\\", "/")
        z = dict(
            sample=os.path.normpath(os.path.join(base, dp + rel)),
            lokey=note_or_int(o.get("lokey", "0")), hikey=note_or_int(o.get("hikey", "127")),
            keycenter=note_or_int(o.get("pitch_keycenter", "60")),
            lovel=int(float(o.get("lovel", 0))), hivel=int(float(o.get("hivel", 127))),
            seq_length=int(float(o.get("seq_length", 1))), seq_position=int(float(o.get("seq_position", 1))),
            sw_last=note_or_int(o["sw_last"]) if "sw_last" in o else -1,
            tune=float(o.get("tune", 0)), keytrack=float(o.get("pitch_keytrack", 100)),
            transpose=int(float(o.get("transpose", 0))),
            loop_mode=o.get("loop_mode", o.get("loopmode", "no_loop")).strip().lower(),
            loop_start=int(float(o["loop_start"])) if "loop_start" in o else -1,
            loop_end=int(float(o["loop_end"])) if "loop_end" in o else -1,
            release=o.get("trigger", "attack").strip().lower() == "release",
            group=int(float(o.get("group", 0))), off_by=int(float(o.get("off_by", 0))),
            volume=float(o.get("volume", 0)),
        )
        if "key" in o:
            z["lokey"] = z["hikey"] = z["keycenter"] = note_or_int(o["key"])
            z["lokey"] = note_or_int(o.get("lokey", o["key"]))
            z["hikey"] = note_or_int(o.get("hikey", o["key"]))
        regions.append(z)
        scopes["region"] = {}

    for i, m in enumerate(toks):
        if m.group(1):
            commit()
            cur = m.group(1).lower()
            if cur == "global":
                scopes["global"], scopes["master"], scopes["group"], scopes["region"] = {}, {}, {}, {}
            elif cur == "master":
                scopes["master"], scopes["group"], scopes["region"] = {}, {}, {}
            elif cur == "group":
                scopes["group"], scopes["region"] = {}, {}
            elif cur == "region":
                scopes["region"] = {}
        else:
            end = toks[i + 1].start() if i + 1 < len(toks) else len(text)
            val = text[m.end():end].strip()
            if cur in scopes:
                scopes[cur][m.group(2).lower()] = val
            if m.group(2).lower() in ("sw_lokey", "sw_hikey", "sw_default"):
                sw[m.group(2).lower()] = note_or_int(val)
    commit()
    return regions, sw


# ------------------------------------------------------------------------------ signal measurements
def db(x):
    return 20.0 * math.log10(max(float(x), 1e-12))


def mono(a):
    return a.mean(axis=1) if a.ndim == 2 else a


def loudness_db(x, sr, window=0.05):
    w = max(16, int(window * sr))
    if len(x) <= w:
        e = float(np.sum(x * x)) / w
        return 10 * math.log10(max(e, 1e-20))
    c = np.concatenate(([0.0], np.cumsum(x * x)))
    starts = np.arange(0, len(x) - w + 1, max(1, w // 2))
    e = (c[starts + w] - c[starts]) / w
    return 10 * math.log10(max(float(e.max()), 1e-20))


def envelope(x, sr, block_ms=2.0):
    b = max(1, int(sr * block_ms / 1000.0))
    n = len(x) // b
    if n == 0:
        return np.array([float(np.max(np.abs(x))) if len(x) else 0.0]), b
    return np.max(np.abs(x[:n * b]).reshape(n, b), axis=1), b


def attack_and_decay(x, sr, looped):
    env, b = envelope(x, sr)
    pk = float(env.max())
    if pk <= 0:
        return 0.0, None, 0.0, 0.0
    ipk = int(np.argmax(env))
    above = np.nonzero(env >= 0.02 * pk)[0]
    t_start = above[0] if len(above) else 0
    reach = np.nonzero(env >= 0.9 * pk)[0]
    attack_ms = (reach[0] - t_start) * b / sr * 1000.0 if len(reach) else 0.0
    decay = None
    if not looped:
        thr = pk * 10 ** (-40 / 20.0)
        below = env[ipk:] < thr
        idx = None
        for k in range(len(below)):          # first time below -40 dB that is never exceeded again
            if below[k] and not np.any(env[ipk + k:] >= thr):
                idx = k
                break
        decay = idx * b / sr if idx is not None else None
    end_db = db(env[-1] / pk) if pk > 0 else -200.0
    return attack_ms, decay, end_db, ipk * b / sr


def spectral_centroid(x, sr, t0, dur=0.5):
    a = int(t0 * sr)
    seg = x[a:a + int(dur * sr)]
    if len(seg) < 256:
        seg = x[:max(256, len(x))]
    n = 1 << int(math.ceil(math.log2(max(len(seg), 256))))
    s = np.abs(np.fft.rfft(seg * np.hanning(len(seg)), n))
    f = np.fft.rfftfreq(n, 1.0 / sr)
    m = (f >= 20) & (f <= 20000)
    tot = float(s[m].sum())
    return float((f[m] * s[m]).sum() / tot) if tot > 0 else 0.0


def parabolic(y, i):
    if i <= 0 or i >= len(y) - 1:
        return float(i)
    a, b, c = y[i - 1], y[i], y[i + 1]
    d = a - 2 * b + c
    return float(i) + (0.5 * (a - c) / d if abs(d) > 1e-18 else 0.0)


def pitch_phase(seg, sr, f_exp, cents=300.0):
    """Mean frequency of the fundamental by weighted phase regression of its analytic signal."""
    n = len(seg)
    if n < 1024:
        return None
    npad = 1 << int(math.ceil(math.log2(n))) + 2
    spec = np.abs(np.fft.rfft(seg * np.hanning(n), npad))
    freqs = np.fft.rfftfreq(npad, 1.0 / sr)
    lo, hi = f_exp * 2 ** (-cents / 1200), f_exp * 2 ** (cents / 1200)
    m = np.nonzero((freqs >= lo) & (freqs <= hi))[0]
    if len(m) < 3:
        return None
    k = m[int(np.argmax(spec[m]))]
    # reject a missing fundamental: the peak must stand out from the local floor
    if spec[k] < 4.0 * float(np.median(spec[m])) + 1e-12:
        return None
    ls = np.log(spec + 1e-20)
    f_est = parabolic(ls, k) * sr / npad
    full = np.fft.fft(seg)
    fr = np.fft.fftfreq(n, 1.0 / sr)
    band = (fr > 0) & (np.abs(fr - f_est) <= max(0.03 * f_est, 4.0 * sr / n))
    z = np.fft.ifft(2.0 * full * band)
    t = int(0.12 * n)
    z = z[t:n - t]
    if len(z) < 256:
        return None
    ph = np.unwrap(np.angle(z))
    w = np.abs(z) ** 2
    tt = np.arange(len(z)) / sr
    if w.sum() <= 0:
        return None
    wm = w / w.sum()
    tm, pm = (wm * tt).sum(), (wm * ph).sum()
    slope = (wm * (tt - tm) * (ph - pm)).sum() / max((wm * (tt - tm) ** 2).sum(), 1e-30)
    return slope / (2 * math.pi)


def pitch_acf(seg, sr, f_exp, cents=300.0):
    """Normalised autocorrelation pitch with multi-period refinement."""
    n = len(seg)
    if n < 2048:
        return None, 0.0
    t_exp = sr / f_exp
    lo, hi = int(t_exp * 2 ** (-cents / 1200)) - 1, int(t_exp * 2 ** (cents / 1200)) + 2
    if hi * 3 > n:
        return None, 0.0
    nfft = 1 << int(math.ceil(math.log2(2 * n)))
    sp = np.fft.rfft(seg - seg.mean(), nfft)
    ac = np.fft.irfft(sp * np.conj(sp), nfft)[:n]
    sq = np.concatenate(([0.0], np.cumsum(seg * seg)))
    lags = np.arange(n)
    e0 = sq[n - lags]                      # energy of x[0 : n-lag]
    e1 = sq[n] - sq[lags]                  # energy of x[lag : n]
    r = ac / np.sqrt(np.maximum(e0 * e1, 1e-30))
    k = lo + int(np.argmax(r[lo:hi]))
    if r[k] < 0.4:
        return None, float(r[k])
    t0 = parabolic(r, k)
    m = max(1, int(min(0.4 * n, 0.25 * sr) / t0))
    if m > 1:
        c = int(round(m * t0))
        a, b = max(1, c - 2), min(n - 2, c + 3)
        kk = a + int(np.argmax(r[a:b]))
        t0 = parabolic(r, kk) / m
    return sr / t0, float(r[k])


def analysis_window(x, sr, ipk_t, looped, loop):
    """Samples used for pitch measurement."""
    n = len(x)
    if looped and loop is not None and loop[1] > loop[0] + 2048:
        a, b = loop[0], min(loop[1], loop[0] + int(1.5 * sr))
        return a, b
    a = int(ipk_t * sr + 0.04 * sr)
    # stop where the signal has decayed 35 dB below its peak
    env, bl = envelope(x, sr, 5.0)
    pk = float(env.max())
    ia = a // bl
    dead = np.nonzero(env[ia:] < pk * 10 ** (-35 / 20.0))[0] if ia < len(env) else []
    b = a + int(0.6 * sr)
    if len(dead):
        b = min(b, (ia + int(dead[0])) * bl)
    b = min(b, n)
    if b - a < 2048:
        a, b = max(0, min(a, n - 2048)), n
    return a, b


def glitch_count(x, sr, ratio):
    d2 = np.abs(x[2:] - 2 * x[1:-1] + x[:-2])
    if len(d2) < 600:
        return 0
    k = 129
    ker = np.ones(k) / k
    loc = np.convolve(d2, ker, mode="same")
    skip = int(0.005 * sr)
    cand = np.nonzero((d2 > ratio * loc + 1e-9) & (d2 > 0.02 * max(float(np.max(np.abs(x))), 1e-9)))[0]
    cand = cand[(cand > skip) & (cand < len(d2) - 130)]
    # a glitch must exceed its neighbourhood measured WITHOUT the sample itself
    bad = 0
    for i in cand:
        nb = np.concatenate((d2[max(0, i - 64):i], d2[i + 1:i + 65]))
        if d2[i] > ratio * (nb.mean() + 1e-9):
            bad += 1
    return bad


# ------------------------------------------------------------------------------ per zone
def analyse_zone(z, a, sr, loop_wav):
    x = mono(a)
    n = len(x)
    pk = float(np.max(np.abs(a))) if n else 0.0
    looped = z["loop_mode"] in ("loop_continuous", "loop_sustain")
    ls, le = (z["loop_start"], z["loop_end"]) if looped else (-1, -1)
    if looped and (ls < 0 or le <= ls) and loop_wav:
        ls, le = loop_wav
    m = dict(frames=n, sr=sr, dur=n / sr if sr else 0.0, peak_db=db(pk), rms_db=db(math.sqrt(float(np.mean(x * x))) if n else 0),
             loud_db=loudness_db(x, sr), dc=float(np.mean(x)) if n else 0.0, looped=looped)
    att, dec, end_db, tpk = attack_and_decay(x, sr, looped)
    m.update(attack_ms=att, decay40=dec, end_db=end_db, peak_t=tpk, centroid=spectral_centroid(x, sr, tpk))

    # clicks
    m["start_click"] = bool(n and float(np.max(np.abs(a[0]))) > THRESH["start_max"])
    m["end_click"] = False if (looped or not n) else bool(float(np.max(np.abs(a[-1]))) > THRESH["end_max"])
    m["glitches"] = glitch_count(x, sr, THRESH["glitch_ratio"]) if n else 0

    # loop seam
    m["seam_jump"] = m["seam_slope"] = m["seam_level_db"] = None
    m["seam_ok"] = True
    if looped and 0 <= ls < le < n:
        body = x[ls:le + 1]
        step = float(np.sqrt(np.mean(np.diff(body) ** 2))) + 1e-9
        jump = abs(x[ls] - x[le])
        slope = abs((x[ls] - x[le]) - (x[le] - x[le - 1]))
        w = max(8, int(0.02 * sr))
        l_end, l_start = float(np.sqrt(np.mean(x[le - w:le] ** 2))) + 1e-12, float(np.sqrt(np.mean(x[ls:ls + w] ** 2))) + 1e-12
        m["seam_jump"], m["seam_slope"], m["seam_level_db"] = jump / step, slope / step, abs(db(l_end / l_start))
        m["seam_ok"] = (m["seam_jump"] <= THRESH["seam_jump"] and m["seam_slope"] <= THRESH["seam_slope"]
                        and m["seam_level_db"] <= THRESH["seam_level_db"])
    elif looped:
        m["seam_ok"] = False

    # pitch
    m["pitch_cents"] = m["acf_cents"] = None
    m["pitched"] = z["keytrack"] > 0 and not z["release"]
    if m["pitched"]:
        f_exp = 440.0 * 2 ** ((z["keycenter"] - 69) / 12.0 + z["tune"] / 1200.0 + z["transpose"] / 12.0)
        if f_exp < 0.45 * sr:
            wa, wb = analysis_window(x, sr, tpk, looped, (ls, le))
            seg = x[wa:wb]
            fp = pitch_phase(seg, sr, f_exp)
            fa, conf = pitch_acf(seg, sr, f_exp)
            if fp:
                m["pitch_cents"] = 1200 * math.log2(fp / f_exp)
            if fa:
                m["acf_cents"] = 1200 * math.log2(fa / f_exp)
            m["f_exp"] = f_exp
    return m


def zone_status(z, m):
    fails = []
    if m["peak_db"] > THRESH["peak_max_db"]:
        fails.append("peak>%.0fdB" % THRESH["peak_max_db"])
    if m["peak_db"] < THRESH["peak_min_db"]:
        fails.append("peak<%.0fdB" % THRESH["peak_min_db"])
    if abs(m["dc"]) >= THRESH["dc_max"]:
        fails.append("DC")
    if m["pitched"]:
        pc = m["pitch_cents"] if m["pitch_cents"] is not None else m["acf_cents"]
        if pc is None:
            fails.append("pitch?")
        elif abs(pc) > THRESH["pitch_cents"]:
            fails.append("pitch%+.0fc" % pc)
    if m["start_click"]:
        fails.append("click@start")
    if m["end_click"]:
        fails.append("click@end")
    if m["glitches"]:
        fails.append("glitch x%d" % m["glitches"])
    if not m["seam_ok"]:
        fails.append("loop-seam")
    return fails


# ------------------------------------------------------------------------------ spectrogram PNG (numpy + zlib)
def write_png(path, rgb):
    h, w, _ = rgb.shape
    raw = b"".join(b"\x00" + rgb[y].tobytes() for y in range(h))

    def chunk(t, d):
        c = struct.pack(">I", len(d)) + t + d
        return c + struct.pack(">I", zlib.crc32(t + d) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw, 6)) + chunk(b"IEND", b""))


MAGMA = np.array([[0, 0, 4], [40, 17, 89], [101, 21, 110], [159, 42, 99], [212, 72, 66], [245, 125, 21], [250, 193, 39], [252, 255, 164]], float)


def spectrogram_image(x, sr, width=640, height=160, tmax=3.0, fmin=40.0, fmax=16000.0, range_db=80.0):
    x = x[:int(tmax * sr)]
    nfft = 2048
    if len(x) < nfft:
        x = np.concatenate((x, np.zeros(nfft - len(x))))
    hop = max(1, (len(x) - nfft) // (width - 1))
    cols = []
    win = np.hanning(nfft)
    for i in range(width):
        s = min(i * hop, len(x) - nfft)
        cols.append(np.abs(np.fft.rfft(x[s:s + nfft] * win)))
    S = np.array(cols).T                                         # [freq, time]
    f = np.fft.rfftfreq(nfft, 1.0 / sr)
    rows = np.geomspace(fmin, min(fmax, sr / 2 * 0.99), height)[::-1]
    idx = np.clip(np.searchsorted(f, rows), 0, len(f) - 1)
    img = 20 * np.log10(S[idx, :] + 1e-12)
    img = np.clip((img - (img.max() - range_db)) / range_db, 0, 1)
    pos = img * (len(MAGMA) - 1)
    i0 = np.floor(pos).astype(int)
    i1 = np.minimum(i0 + 1, len(MAGMA) - 1)
    fr = (pos - i0)[..., None]
    return (MAGMA[i0] * (1 - fr) + MAGMA[i1] * fr).astype(np.uint8)


# ------------------------------------------------------------------------------ main
def find_sfz(root):
    if os.path.isfile(root) and root.lower().endswith(".sfz"):
        return [root]
    out = []
    for d, _, files in os.walk(root):
        for f in files:
            if f.lower().endswith(".sfz"):
                out.append(os.path.join(d, f))
    return sorted(out)


def layer_name(z, layers):
    k = sorted(layers).index((z["lovel"], z["hivel"]))
    if len(layers) == 1:
        return "all"
    return "soft" if k == 0 else ("hard" if k == len(layers) - 1 else "mid%d" % k)


def main():
    ap = argparse.ArgumentParser(description="Objective quality gate for the HUMPIRE built-in modelled library.")
    ap.add_argument("root", help="folder written by ChaabiRenderLibrary (or a single instrument folder / .sfz)")
    ap.add_argument("--instrument", action="append", help="only this instrument folder name (repeatable), e.g. Mandole")
    ap.add_argument("--failures-only", action="store_true", help="print only failing zones")
    ap.add_argument("--quiet", action="store_true", help="print only the per-instrument summary")
    ap.add_argument("--csv", help="write all metrics to this CSV file")
    ap.add_argument("--spectrograms", metavar="DIR", help="write spectrogram PNG sheets (lowest / middle / highest key per articulation)")
    ap.add_argument("--max-cents", type=float, help="override the pitch error threshold (default %.1f)" % THRESH["pitch_cents"])
    args = ap.parse_args()
    if args.max_cents is not None:
        THRESH["pitch_cents"] = args.max_cents

    sfzs = find_sfz(args.root)
    if args.instrument:
        keep = {s.lower() for s in args.instrument}
        sfzs = [s for s in sfzs if os.path.splitext(os.path.basename(s))[0].lower() in keep]
    if not sfzs:
        print("no .sfz file found under", args.root)
        return 2

    all_pass = True
    rows = []
    wav_cache = {}
    for sfz in sfzs:
        inst = os.path.splitext(os.path.basename(sfz))[0]
        regions, sw = parse_sfz(sfz)
        if not regions:
            print("[%s] no regions in %s" % (inst, sfz))
            all_pass = False
            continue
        print("=" * 150)
        print("%s  (%d zones)  %s" % (inst, len(regions), sfz))
        print("=" * 150)
        hdr = "%-34s %-9s %3s %-7s %6s %6s %6s %8s %7s %8s %7s %6s  %s"
        if not args.quiet:
            print(hdr % ("zone", "keys", "kc", "vel", "peak", "loud", "dc", "pitch_c", "att_ms", "dec-40s", "cent_Hz", "dur", "status"))

        zres = []
        layer_sets = {}
        for z in regions:
            art = os.path.basename(os.path.dirname(z["sample"]))
            z["art"] = art
            layer_sets.setdefault((art, z["release"]), set()).add((z["lovel"], z["hivel"]))
        for z in regions:
            p = z["sample"]
            if p not in wav_cache:
                try:
                    wav_cache[p] = read_wav(p)
                except Exception as e:                      # noqa: BLE001
                    wav_cache[p] = e
            w = wav_cache[p]
            name = os.path.relpath(p, os.path.dirname(sfz))
            if isinstance(w, Exception):
                print("%-34s READ ERROR: %s" % (name[-34:], w))
                all_pass = False
                continue
            a, sr, loop_wav = w
            m = analyse_zone(z, a, sr, loop_wav)
            fails = zone_status(z, m)
            z["layer"] = layer_name(z, layer_sets[(z["art"], z["release"])])
            zres.append((z, m, fails, name))
            if not args.quiet and (fails or not args.failures_only):
                pc = m["pitch_cents"] if m["pitch_cents"] is not None else m["acf_cents"]
                flags = []
                if m["decay40"] is None and not m["looped"] and m["end_db"] > -50:
                    flags.append("tail-cut(%.0fdB)" % m["end_db"])
                if m["pitched"] and m["pitch_cents"] is not None and m["acf_cents"] is not None and abs(m["pitch_cents"] - m["acf_cents"]) > 25:
                    flags.append("est-disagree(acf%+.0f)" % m["acf_cents"])
                print(hdr % (name[-34:], "%d-%d" % (z["lokey"], z["hikey"]), z["keycenter"], "%d-%d" % (z["lovel"], z["hivel"]),
                             "%.1f" % m["peak_db"], "%.1f" % m["loud_db"], "%+.4f" % m["dc"],
                             ("%+.1f" % pc) if pc is not None else ("-" if not m["pitched"] else "n/a"),
                             "%.1f" % m["attack_ms"],
                             "loop" if m["looped"] else (("%.2f" % m["decay40"]) if m["decay40"] is not None else ">dur"),
                             "%.0f" % m["centroid"], "%.2f" % m["dur"],
                             ("FAIL " + ",".join(fails) if fails else "ok") + (" [" + " ".join(flags) + "]" if flags else "")))
                if m["looped"] and m["seam_jump"] is not None and not args.failures_only:
                    print("%-34s    loop seam: jump %.2f x step, slope %.2f x step, level step %.2f dB" % ("", m["seam_jump"], m["seam_slope"], m["seam_level_db"]))
            rows.append((inst, z, m, fails, name))
            if fails:
                all_pass = False

        # ------- instrument-level gate
        inst_fail = []
        groups = {}
        for z, m, _, _ in zres:
            if z["release"]:
                continue
            groups.setdefault((z["art"], z["layer"]), []).append(m["loud_db"])
        for (art, layer), v in sorted(groups.items()):
            med = float(np.median(v))
            spread_hi, spread_lo = max(v) - med, med - min(v)
            if spread_hi > THRESH["group_loudness_db"] or spread_lo > THRESH["group_loudness_db"]:
                inst_fail.append("loudness spread in %s/%s: median %.1f dB, range %.1f..%.1f dB (limit +-%.0f)"
                                 % (art, layer, med, min(v), max(v), THRESH["group_loudness_db"]))
        hard = {}
        for (art, layer), v in groups.items():
            if layer in ("hard", "all") and art not in UNPITCHED_LANES_EXCLUDED_FROM_ACROSS:
                hard.setdefault(art, []).extend(v)
        if len(hard) > 1:
            meds = {k: float(np.median(v)) for k, v in hard.items()}
            allmed = float(np.median(list(meds.values())))
            for k, v in meds.items():
                if abs(v - allmed) > THRESH["across_loudness_db"]:
                    inst_fail.append("articulation '%s' hard layer %.1f dB is %+.1f dB from the instrument median %.1f dB (limit +-%.0f)"
                                     % (k, v, v - allmed, allmed, THRESH["across_loudness_db"]))
        for (art, layer), v in groups.items():
            if layer == "soft" and (art, "hard") in groups and np.median(v) >= np.median(groups[(art, "hard")]) - 1.0:
                inst_fail.append("%s: soft layer (%.1f dB) is not quieter than the hard layer (%.1f dB)" % (art, np.median(v), np.median(groups[(art, "hard")])))

        nfail = sum(1 for _, _, f, _ in zres if f)
        print("-" * 150)
        print("%s: %d zones, %d zone FAIL, %d instrument-level FAIL  ->  %s" % (inst, len(zres), nfail, len(inst_fail), "PASS" if not nfail and not inst_fail else "FAIL"))
        for msg in inst_fail:
            print("   FAIL  " + msg)
        if inst_fail:
            all_pass = False

        # ------- compact statistics
        if zres:
            pk = [m["peak_db"] for _, m, _, _ in zres]
            pc = [abs(m["pitch_cents"] if m["pitch_cents"] is not None else m["acf_cents"]) for _, m, _, _ in zres
                  if m["pitched"] and (m["pitch_cents"] is not None or m["acf_cents"] is not None)]
            print("   peak %.1f..%.1f dBFS | loudness %.1f..%.1f dB | |pitch err| max %s cents | clicks start/end/seam/glitch: %d/%d/%d/%d"
                  % (min(pk), max(pk), min(m["loud_db"] for _, m, _, _ in zres), max(m["loud_db"] for _, m, _, _ in zres),
                     ("%.1f" % max(pc)) if pc else "-",
                     sum(m["start_click"] for _, m, _, _ in zres), sum(m["end_click"] for _, m, _, _ in zres),
                     sum(not m["seam_ok"] for _, m, _, _ in zres), sum(m["glitches"] > 0 for _, m, _, _ in zres)))

        # ------- spectrograms
        if args.spectrograms:
            os.makedirs(args.spectrograms, exist_ok=True)
            by_art = {}
            for z, m, _, _ in zres:
                by_art.setdefault((z["art"], z["release"]), []).append(z)
            for (art, rel), zs in sorted(by_art.items()):
                pick = [q for q in zs if q["layer"] in ("hard", "all") and q["seq_position"] == 1] or zs
                pick = sorted(pick, key=lambda q: q["keycenter"])
                sel = [pick[0], pick[len(pick) // 2], pick[-1]] if len(pick) > 2 else pick
                panels = []
                for q in sel:
                    aa, ssr, _ = wav_cache[q["sample"]]
                    panels.append(spectrogram_image(mono(aa), ssr))
                    panels.append(np.full((3, panels[-1].shape[1], 3), 255, np.uint8))
                write_png(os.path.join(args.spectrograms, "%s_%s.png" % (inst, art)), np.vstack(panels))
            print("   spectrograms written to", args.spectrograms)

    if args.csv:
        with open(args.csv, "w", newline="") as f:
            wr = csv.writer(f)
            wr.writerow(["instrument", "zone", "lokey", "hikey", "keycenter", "lovel", "hivel", "seq_position", "sw_last", "loop", "release",
                         "peak_db", "rms_db", "loudness_db", "dc", "pitch_cents", "acf_cents", "attack_ms", "decay_to_minus40_s", "end_db_rel_peak",
                         "centroid_hz", "duration_s", "start_click", "end_click", "glitches", "seam_jump", "seam_slope", "seam_level_db", "status"])
            for inst, z, m, fails, name in rows:
                wr.writerow([inst, name, z["lokey"], z["hikey"], z["keycenter"], z["lovel"], z["hivel"], z["seq_position"], z["sw_last"],
                             z["loop_mode"], int(z["release"]), "%.2f" % m["peak_db"], "%.2f" % m["rms_db"], "%.2f" % m["loud_db"], "%.5f" % m["dc"],
                             "" if m["pitch_cents"] is None else "%.2f" % m["pitch_cents"], "" if m["acf_cents"] is None else "%.2f" % m["acf_cents"],
                             "%.2f" % m["attack_ms"], "" if m["decay40"] is None else "%.3f" % m["decay40"], "%.1f" % m["end_db"],
                             "%.0f" % m["centroid"], "%.3f" % m["dur"], int(m["start_click"]), int(m["end_click"]), m["glitches"],
                             "" if m["seam_jump"] is None else "%.2f" % m["seam_jump"], "" if m["seam_slope"] is None else "%.2f" % m["seam_slope"],
                             "" if m["seam_level_db"] is None else "%.2f" % m["seam_level_db"], "FAIL:" + ";".join(fails) if fails else "PASS"])
        print("metrics written to", args.csv)

    print()
    print("OVERALL GATE: %s" % ("PASS" if all_pass else "FAIL"))
    return 0 if all_pass else 1


if __name__ == "__main__":
    sys.exit(main())
