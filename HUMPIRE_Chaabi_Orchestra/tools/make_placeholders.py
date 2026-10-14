#!/usr/bin/env python3
"""
HUMPIRE CHAABI ORCHESTRA - generateur de SFZ + samples PLACEHOLDER (synthetises).

  python3 tools/make_placeholders.py --out Samples            # SFZ + WAV
  python3 tools/make_placeholders.py --out Samples --no-wav   # SFZ seulement

- Les .sfz generes sont les VRAIS fichiers de mapping (cles, velocites, round-robins, keyswitch).
- Les .wav sont des sons de test (Karplus-Strong / synthese percussive) pour valider le moteur
  sans attendre tes enregistrements. Remplace-les par tes samples (memes noms) : le mapping ne change pas.
- Stdlib uniquement (pas de numpy).
"""
import argparse, math, os, random, struct, wave

SR = 44100

# ----------------------------------------------------------------- utilitaires audio
def write_wav(path, data):
    os.makedirs(os.path.dirname(path), exist_ok=True)
    peak = max(1e-9, max(abs(x) for x in data))
    scale = min(1.0, 0.89 / peak) if peak > 0.89 else 1.0
    with wave.open(path, "wb") as w:
        w.setnchannels(1); w.setsampwidth(2); w.setframerate(SR)
        w.writeframes(b"".join(struct.pack("<h", int(max(-1, min(1, x * scale)) * 32767)) for x in data))

def midi_hz(m): return 440.0 * 2 ** ((m - 69) / 12.0)

def karplus(freq, dur, amp, bright, rng):
    n = int(SR * dur)
    period = max(2, int(round(SR / freq)))
    buf = [rng.uniform(-1, 1) * bright + rng.uniform(-1, 1) * (1 - bright) * 0.3 for _ in range(period)]
    out = []
    damp = 0.9965 if freq < 400 else 0.995
    for i in range(n):
        j = i % period
        nxt = buf[(j + 1) % period]
        v = damp * 0.5 * (buf[j] + nxt)
        buf[j] = v
        out.append(v * amp * min(1.0, i / 60.0) * math.exp(-i / (SR * dur * 0.55)))
    return out

def tremolo_loop(freq, amp):
    """Corps de note tenue : periode entiere => boucle sans clic. Retourne (data, loop_start, loop_end)."""
    period = max(2, int(round(SR / freq)))
    cycles = max(1, int(math.ceil(0.8 * SR / period)))
    body = period * cycles
    intro = int(0.05 * SR)
    f = SR / period
    data = []
    for n in range(intro + body):
        t = n / SR
        s = sum((1.0 / h) * math.sin(2 * math.pi * f * h * t) for h in range(1, 7))
        env = min(1.0, n / intro) if n < intro else 1.0
        data.append(s * amp * 0.35 * env)
    return data, intro, intro + body - 1

def noise_burst(dur, amp, ring_hz, ring_amt, decay, rng):
    n = int(SR * dur)
    out, prev = [], 0.0
    for i in range(n):
        x = rng.uniform(-1, 1)
        y = x - 0.6 * prev            # filtre passe-haut simple
        prev = x
        e = math.exp(-i / (SR * decay))
        out.append((y * (1 - ring_amt) + math.sin(2 * math.pi * ring_hz * i / SR) * ring_amt) * e * amp)
    return out

def dum(amp, rng, v):
    n = int(SR * 0.55); out = []; ph = 0.0
    for i in range(n):
        t = i / SR
        f = 70 + 80 * math.exp(-t * 28)
        ph += 2 * math.pi * f / SR
        out.append((math.sin(ph) * math.exp(-t * 7) + (rng.uniform(-1, 1) * 0.25 * math.exp(-t * 90) * v)) * amp)
    return out

# ----------------------------------------------------------------- generation SFZ
def sfz_header(name, extra_global):
    return (f"// HUMPIRE CHAABI ORCHESTRA - {name}\n// Genere par tools/make_placeholders.py - format SFZ (ouvert, libre de droits)\n\n"
            "<control>\ndefault_path=samples/\n\n<global>\n" + extra_global + "\n")

def gen_mandole(root, with_wav):
    rng = random.Random(1)
    kcs = list(range(52, 77, 3))                     # 52..76 = 2 octaves, une zone toutes les 3 notes
    lines = [sfz_header("Mandole algérien",
        "// Keyswitch : 12 = médiator (défaut), 13 = trémolo (tenue en boucle)\n"
        "sw_lokey=12 sw_hikey=23 sw_default=12\nampeg_release=0.30\n")]
    vlayers = [("soft", 1, 80, 0.55), ("hard", 81, 127, 1.0)]

    lines.append("// ---------- Articulation 12 : médiator (2 vitesses x 3 round-robins)\n")
    for vname, lo, hi, amp in vlayers:
        lines.append(f"<group>\nsw_last=12 lovel={lo} hivel={hi} seq_length=3 amp_veltrack=70\n")
        for kc in kcs:
            lk = 50 if kc == kcs[0] else kc - 1
            hk = 78 if kc == kcs[-1] else kc + 1
            for rr in (1, 2, 3):
                f = f"pick/mandole_pick_{kc:03d}_{vname}_rr{rr}.wav"
                lines.append(f"<region> sample={f} lokey={lk} hikey={hk} pitch_keycenter={kc} seq_position={rr}")
                if with_wav:
                    write_wav(os.path.join(root, "samples", f),
                              karplus(midi_hz(kc), 1.9, amp, 0.55 if vname == "soft" else 0.9, rng))
        lines.append("")

    lines.append("// ---------- Articulation 13 : trémolo (tenue en boucle, loop_start/loop_end)\n")
    lines.append("<group>\nsw_last=13 loop_mode=loop_continuous ampeg_attack=0.02 ampeg_release=0.35\n")
    for kc in kcs:
        lk = 50 if kc == kcs[0] else kc - 1
        hk = 78 if kc == kcs[-1] else kc + 1
        f = f"tremolo/mandole_trem_{kc:03d}.wav"
        data, ls, le = tremolo_loop(midi_hz(kc), 0.8)
        lines.append(f"<region> sample={f} lokey={lk} hikey={hk} pitch_keycenter={kc} loop_start={ls} loop_end={le}")
        if with_wav:
            write_wav(os.path.join(root, "samples", f), data)
    with open(os.path.join(root, "Mandole.sfz"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")

def gen_derbouka(root, with_wav):
    rng = random.Random(2)
    lines = [sfz_header("Derbouka",
        "// Percussion : one_shot, pas de pitch-tracking. DUM 36-37 / TEK 38-39 / KA 40 / SLAP 41-42 / GHOST 43-44\n"
        "loop_mode=one_shot pitch_keytrack=0 ampeg_release=0.08\n")]
    # (nom, lokey, hikey, couches, rr, generateur)
    arts = [
        ("dum",   36, 37, "2v", 3),
        ("tek",   38, 39, "2v", 3),
        ("ka",    40, 40, "2v", 3),
        ("slap",  41, 42, "2v", 2),
        ("ghost", 43, 44, "1v", 3),
    ]
    for name, lk, hk, layers, nrr in arts:
        lines.append(f"// ---------- {name.upper()}\n")
        vl = [("soft", 1, 80, 0.55), ("hard", 81, 127, 1.0)] if layers == "2v" else [("soft", 1, 127, 0.4)]
        for vname, lo, hi, amp in vl:
            lines.append(f"<group>\nlovel={lo} hivel={hi} seq_length={nrr} pitch_keycenter={lk}\n")
            for rr in range(1, nrr + 1):
                f = f"{name}/derbouka_{name}_{vname}_rr{rr}.wav"
                lines.append(f"<region> sample={f} lokey={lk} hikey={hk} seq_position={rr}")
                if with_wav:
                    v = 0.4 if vname == "soft" else 1.0
                    if name == "dum":    d = dum(amp, rng, v)
                    elif name == "tek":  d = noise_burst(0.16, amp * 0.9, 2300 + rng.uniform(-100, 100), 0.35, 0.035, rng)
                    elif name == "ka":   d = noise_burst(0.11, amp * 0.8, 3300 + rng.uniform(-100, 100), 0.30, 0.025, rng)
                    elif name == "slap": d = noise_burst(0.30, amp * 1.0, 1500 + rng.uniform(-80, 80), 0.45, 0.07, rng)
                    else:                d = noise_burst(0.09, amp * 0.5, 2000, 0.25, 0.02, rng)
                    write_wav(os.path.join(root, "samples", f), d)
            lines.append("")
    with open(os.path.join(root, "Derbouka.sfz"), "w", encoding="utf-8") as fh:
        fh.write("\n".join(lines) + "\n")

if __name__ == "__main__":
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", default="Samples")
    ap.add_argument("--no-wav", action="store_true")
    a = ap.parse_args()
    for inst, fn in (("Mandole", gen_mandole), ("Derbouka", gen_derbouka)):
        d = os.path.join(a.out, inst)
        os.makedirs(d, exist_ok=True)
        fn(d, not a.no_wav)
        print("OK", inst)
