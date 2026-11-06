#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets NEON SYNTH 80s 2.0 (6 banques)."""
import os

D = {"lfo_rate": 4.5, "lfo_amount": .35, "lfo_pitch": 0, "lfo_attack": .1, "lfo_decay": .5, "lfo_sustain": 1, "lfo_release": .4,
     "amp_attack": .02, "amp_decay": .8, "amp_sustain": .7, "velocity_bank": .75, "velocity_adsr": .4, "master_volume": .8,
     "fx_reverb": .35, "fx_chorus": .45, "fx_distortion": .25, "glide_time": .05, "filter_cutoff": 3200, "filter_drive": .15,
     "fx_delay_mix": .3, "fx_delay_time": .375, "fx_delay_fb": .45, "osc_waveform": 0, "lfo_source": 1, "lfo_dest": 0,
     "filter_type": 0, "voice_mode": 0, "filter_res": .2, "filter_env": 0, "amp_release": .4, "osc_detune": .4,
     "osc2_waveform": 0, "osc2_semi": 0, "osc2_fine": 7, "osc2_mix": 0}
SH = dict(w="osc_waveform", c="filter_cutoff", ft="filter_type", res="filter_res", fe="filter_env", dr="filter_drive",
          a="amp_attack", d="amp_decay", s="amp_sustain", rl="amp_release", vb="velocity_bank", va="velocity_adsr",
          la="lfo_attack", ld="lfo_decay", ls="lfo_sustain", lrl="lfo_release", rate="lfo_rate", amt="lfo_amount", lp="lfo_pitch",
          src="lfo_source", dst="lfo_dest", gl="glide_time", dis="fx_distortion", cho="fx_chorus", dly="fx_delay_mix",
          tm="fx_delay_time", fb="fx_delay_fb", rev="fx_reverb", vol="master_volume", mode="voice_mode", det="osc_detune",
          w2="osc2_waveform", st="osc2_semi", fi="osc2_fine", m2="osc2_mix")
SAW, SQR, TRI, SIN, SSAW, BRS = range(6)            # oscillateur 1
O2SAW, O2SQR, O2TRI, O2SIN, O2P25 = range(5)          # oscillateur 2
LP24, LP12, BP, HP, ACID = range(5)
POLY8, POLY16, RETRIG, LEGATO = range(4)
L_SIN, L_TRI, L_SQR, L_UP, L_DN, L_SH = range(6)
D_CUT, D_PITCH, D_PW, D_VOL, D_CHO, D_PAN = range(6)

BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, {SH[x]: v for x, v in k.items()})

bank("Leads", [
 p("Saw Lead", w=SSAW, c=5200, res=.25, a=.005, d=.6, s=.85, gl=.08, mode=LEGATO, w2=O2SAW, st=12, fi=4, m2=.35, cho=.4, dly=.35, tm=.375, rev=.4, amt=0),
 p("Fifth Lead", w=SAW, c=4200, res=.3, a=.01, d=.5, s=.8, gl=.06, mode=LEGATO, w2=O2SAW, st=7, fi=0, m2=.45, cho=.35, dly=.3, rev=.45, amt=0),
 p("Square Lead", w=SQR, c=3800, res=.2, a=.003, d=.3, s=.7, gl=.03, mode=RETRIG, w2=O2SQR, st=12, fi=-6, m2=.4, dst=D_PW, src=L_TRI, amt=.5, rate=5, dly=.35, tm=.25, rev=.3),
 p("Pop Lead", w=SQR, c=3000, res=.25, fe=.3, a=.005, d=.4, s=.6, mode=POLY8, w2=O2TRI, st=12, m2=.4, cho=.5, dly=.3, tm=.375, rev=.4, amt=0),
 p("Whistle Lead", w=SIN, c=8000, res=.1, a=.03, d=.5, s=.9, gl=.18, mode=LEGATO, w2=O2TRI, st=12, m2=.25, lp=1, src=L_SIN, amt=.3, rate=5.5, rev=.55, dly=.4, tm=.5, dis=.05),
 p("Rock Lead", w=SAW, c=3600, res=.35, dr=.5, a=.005, d=.7, s=.85, gl=.1, mode=LEGATO, w2=O2SAW, st=0, fi=12, m2=.5, dis=.45, cho=.3, dly=.3, rev=.35, amt=0),
 p("Sax Lead", w=BRS, c=2600, res=.3, fe=.45, a=.04, d=.6, s=.75, gl=.07, mode=LEGATO, lp=1, src=L_SIN, amt=.2, rate=5, la=.6, cho=.25, rev=.45, dly=.2),
 p("Super Lead", w=SSAW, c=6500, res=.2, a=.01, d=.8, s=.85, gl=.12, mode=LEGATO, w2=O2P25, st=-12, m2=.3, cho=.55, dly=.4, tm=.5, fb=.5, rev=.5, det=.55, amt=0),
 p("Acid Lead", w=SAW, c=2400, ft=ACID, res=.55, fe=.6, a=.003, d=.25, s=.5, gl=.06, mode=LEGATO, dis=.3, dly=.3, tm=.1875, rev=.3, amt=0),
 p("Theremin", w=SIN, c=9000, res=.05, a=.12, d=1, s=1, gl=.35, mode=LEGATO, w2=O2SIN, st=0, fi=-9, m2=.4, lp=1, src=L_SIN, amt=.45, rate=6, rev=.6, dly=.35),
 p("Glass Lead", w=TRI, c=5000, res=.2, a=.01, d=.6, s=.8, gl=.1, mode=LEGATO, w2=O2SQR, st=19, m2=.3, cho=.45, dly=.4, tm=.375, rev=.5, amt=0),
 p("Anthem Lead", w=SSAW, c=4500, res=.25, fe=.25, a=.02, d=.9, s=.85, mode=POLY8, w2=O2SAW, st=-12, m2=.35, cho=.6, rev=.55, dly=.25, det=.6, amt=0),
])
bank("Basses", [
 p("Synth Bass", w=SAW, c=650, res=.4, fe=.6, a=.003, d=.25, s=.3, rl=.12, mode=RETRIG, w2=O2SQR, st=-12, m2=.4, cho=.1, dly=0, rev=.08, dis=.1, amt=0),
 p("Sub Bass", w=SIN, c=1000, res=.1, a=.005, d=.8, s=.9, rl=.2, mode=LEGATO, gl=.05, w2=O2TRI, st=12, m2=.15, cho=0, rev=.04, dly=0, dis=.05, amt=0),
 p("Octave Bass", w=SQR, c=900, res=.35, fe=.5, a=.003, d=.2, s=.35, rl=.1, mode=RETRIG, w2=O2SAW, st=12, m2=.35, cho=.1, dly=0, rev=.08, amt=0),
 p("Punch Bass", w=SAW, c=500, res=.45, fe=.7, a=.002, d=.18, s=.25, rl=.1, mode=RETRIG, dr=.35, dis=.15, cho=.05, rev=.05, dly=0, amt=0),
 p("Funk Bass", w=SQR, c=1200, ft=LP12, res=.5, fe=.65, a=.002, d=.15, s=.4, rl=.08, mode=RETRIG, w2=O2P25, st=0, fi=-5, m2=.3, cho=.1, rev=.05, dly=0, amt=0),
 p("Dark Bass", w=SSAW, c=480, ft=ACID, res=.55, fe=.6, a=.004, d=.3, s=.5, rl=.15, mode=RETRIG, dis=.45, dst=D_CUT, src=L_SIN, amt=.3, rate=3, cho=.15, rev=.08),
 p("Disco Bass", w=SAW, c=1100, res=.3, fe=.5, a=.002, d=.14, s=.45, rl=.08, mode=RETRIG, w2=O2SAW, st=-12, fi=8, m2=.45, cho=.1, rev=.08, dly=.15, tm=.125, amt=0),
 p("Wobble Bass", w=SAW, c=560, res=.4, a=.004, d=.4, s=.75, rl=.2, mode=RETRIG, dst=D_CUT, src=L_SIN, amt=.65, rate=2, w2=O2SQR, st=-12, m2=.3, dis=.2, cho=.1, rev=.08),
 p("Acid Bass", w=SAW, c=700, ft=ACID, res=.7, fe=.75, a=.002, d=.2, s=.2, rl=.1, mode=LEGATO, gl=.06, dis=.25, dly=.2, tm=.1875, rev=.1, amt=0),
 p("Pulse Bass", w=SQR, c=800, res=.3, fe=.4, a=.003, d=.3, s=.5, rl=.12, mode=RETRIG, dst=D_PW, src=L_TRI, amt=.5, rate=.6, w2=O2SIN, st=-12, m2=.35, cho=.15, rev=.06),
 p("Fretless Bass", w=TRI, c=1500, res=.25, fe=.3, a=.01, d=.6, s=.7, rl=.2, mode=LEGATO, gl=.12, w2=O2SIN, st=0, m2=.4, cho=.3, rev=.12, dly=0, amt=0),
 p("Brass Bass", w=BRS, c=900, res=.35, fe=.5, a=.004, d=.25, s=.45, rl=.12, mode=RETRIG, w2=O2SAW, st=-12, fi=-6, m2=.4, dis=.3, cho=.1, rev=.1, amt=0),
])
bank("Pads", [
 p("Warm Pad", w=SSAW, c=2400, res=.15, a=.8, d=2, s=.9, rl=1.4, w2=O2SAW, st=12, fi=5, m2=.25, cho=.8, rev=.6, dly=.25, det=.6, amt=.2, rate=.4),
 p("Strings", w=SSAW, c=3000, res=.1, a=.5, d=1.5, s=.9, rl=1.2, w2=O2SAW, st=0, fi=-12, m2=.35, cho=.85, rev=.5, det=.7, amt=.15, rate=.5),
 p("Choir", w=TRI, c=2500, res=.15, a=.7, d=2, s=.9, rl=1.5, w2=O2SIN, st=12, m2=.4, cho=.7, rev=.7, amt=.25, lp=1, src=L_SIN, rate=5),
 p("Soft Pad", w=SQR, c=1800, res=.1, a=.6, d=1.5, s=.85, rl=1.3, w2=O2TRI, st=7, m2=.3, cho=.8, rev=.55, dis=.12, dst=D_PW, amt=.3, rate=.6),
 p("Rain Pad", w=SAW, c=1600, res=.2, a=1.1, d=2.5, s=.9, rl=2, w2=O2SQR, st=-12, m2=.3, cho=.7, rev=.75, dly=.3, tm=.5, amt=.4, rate=.3, src=L_SIN),
 p("Dark Pad", w=SAW, c=900, res=.3, fe=.4, a=1.5, d=3, s=.95, rl=2.5, w2=O2SAW, st=0, fi=15, m2=.5, cho=.5, rev=.85, dst=D_CUT, src=L_TRI, amt=.5, rate=.2),
 p("Wide Pad", w=SSAW, c=3500, res=.1, a=2, d=3, s=.95, rl=3, w2=O2SIN, st=19, m2=.2, cho=.8, rev=.8, det=.8, amt=.2, rate=.15),
 p("Tape Pad", w=SAW, c=1400, ft=LP12, res=.1, a=.7, d=2, s=.85, rl=1.6, w2=O2SAW, st=0, fi=-7, m2=.45, cho=.75, rev=.5, dis=.18, amt=.15, rate=.7),
 p("Glass Pad", w=SIN, c=6000, res=.1, a=.5, d=2, s=.9, rl=1.8, w2=O2TRI, st=24, m2=.25, cho=.6, rev=.8, dly=.35, tm=.5, amt=.1),
 p("Slow Pad", w=TRI, c=2200, res=.15, a=2.5, d=3, s=1, rl=3.5, w2=O2SAW, st=12, fi=3, m2=.3, cho=.7, rev=.85, dly=.2, amt=.3, rate=.12, dst=D_PAN, src=L_SIN),
 p("Breath Pad", w=BRS, c=2000, res=.25, fe=.5, a=1, d=2, s=.85, rl=1.6, w2=O2P25, st=12, m2=.25, cho=.6, rev=.7, amt=.3, rate=.35),
 p("Night Pad", w=SSAW, c=1900, res=.2, a=1.2, d=2.5, s=.9, rl=2.2, w2=O2SQR, st=5, m2=.3, cho=.65, rev=.75, dly=.3, tm=.6, fb=.55, det=.5, amt=.25, rate=.25),
])
bank("Keys & Brass", [
 p("Brass Stab", w=BRS, c=3000, res=.3, fe=.55, a=.01, d=.35, s=.5, rl=.25, w2=O2SAW, st=0, fi=10, m2=.35, cho=.35, rev=.4, dly=.15, amt=0),
 p("Ballad Keys", w=SQR, c=2600, res=.15, fe=.35, a=.004, d=1.2, s=.4, rl=.6, w2=O2SIN, st=12, m2=.35, cho=.55, rev=.5, dly=.2, amt=0),
 p("Electric Piano", w=SIN, c=4200, res=.1, fe=.3, a=.003, d=1.4, s=.3, rl=.5, w2=O2TRI, st=12, fi=-4, m2=.45, cho=.6, rev=.35, dly=.15, amt=.15, rate=4, dst=D_VOL, src=L_SIN),
 p("Horn Section", w=BRS, c=2400, res=.25, fe=.5, a=.06, d=.6, s=.75, rl=.35, w2=O2SAW, st=7, m2=.3, cho=.4, rev=.45, amt=.15, lp=1, src=L_SIN, rate=5, la=.5),
 p("Organ", w=SQR, c=5000, res=.1, a=.005, d=.3, s=.95, rl=.1, w2=O2SQR, st=19, m2=.35, cho=.3, rev=.3, amt=.2, lp=1, src=L_SIN, rate=6),
 p("Chords", w=SSAW, c=3200, res=.2, fe=.35, a=.008, d=.9, s=.55, rl=.5, w2=O2SAW, st=12, m2=.25, cho=.65, rev=.5, dly=.2, det=.55, amt=0),
 p("Bells", w=SIN, c=7000, res=.1, fe=.2, a=.002, d=2.2, s=0, rl=1.5, w2=O2SIN, st=24, fi=40, m2=.45, cho=.35, rev=.6, dly=.3, tm=.5, amt=0),
 p("Clav", w=SQR, c=2200, ft=BP, res=.45, fe=.6, a=.002, d=.25, s=.2, rl=.1, w2=O2P25, st=0, m2=.4, cho=.25, rev=.2, amt=0),
 p("Rhodes", w=TRI, c=3600, res=.15, fe=.25, a=.004, d=1.6, s=.35, rl=.6, w2=O2SIN, st=12, m2=.4, cho=.5, rev=.45, amt=.2, rate=4.5, dst=D_PAN, src=L_SIN),
 p("Brass", w=SAW, c=2000, res=.3, fe=.6, a=.03, d=.5, s=.7, rl=.3, w2=O2SAW, st=0, fi=-10, m2=.5, cho=.3, rev=.45, dis=.15, amt=0),
 p("Jazz Organ", w=TRI, c=4000, res=.1, a=.005, d=.2, s=1, rl=.12, w2=O2SQR, st=12, m2=.3, cho=.6, rev=.35, amt=.3, lp=1, src=L_SIN, rate=6.5),
 p("Piano", w=SAW, c=2800, ft=LP12, res=.1, fe=.45, a=.002, d=1.1, s=.25, rl=.5, w2=O2TRI, st=12, m2=.4, cho=.4, rev=.45, dly=.15, amt=0),
])
bank("Plucks", [
 p("Arp", w=SAW, c=2200, res=.4, fe=.7, a=.002, d=.2, s=0, rl=.15, mode=POLY16, w2=O2SQR, st=12, m2=.3, dly=.5, tm=.1875, fb=.55, cho=.3, rev=.3, amt=0),
 p("Pluck", w=SQR, c=2800, res=.3, fe=.6, a=.002, d=.25, s=0, rl=.2, w2=O2TRI, st=7, m2=.3, dly=.4, tm=.25, fb=.5, cho=.4, rev=.4, amt=0),
 p("Harp", w=TRI, c=5000, res=.2, fe=.4, a=.002, d=.6, s=0, rl=.5, mode=POLY16, w2=O2SIN, st=12, m2=.4, cho=.4, rev=.55, dly=.35, tm=.375, amt=0),
 p("Sequencer", w=SAW, c=1500, ft=ACID, res=.6, fe=.7, a=.002, d=.15, s=0, rl=.1, mode=RETRIG, dis=.2, dly=.45, tm=.1875, fb=.4, rev=.2, amt=0),
 p("Pixel Pluck", w=SQR, c=4000, res=.25, fe=.5, a=.001, d=.12, s=0, rl=.1, mode=POLY16, w2=O2P25, st=24, m2=.3, dly=.55, tm=.125, fb=.6, rev=.4, amt=0),
 p("Marimba", w=SIN, c=3000, res=.15, fe=.3, a=.001, d=.4, s=0, rl=.3, w2=O2TRI, st=19, m2=.3, cho=.3, rev=.45, dly=.3, tm=.25, amt=0),
 p("Gated Pluck", w=SSAW, c=3400, res=.2, fe=.55, a=.002, d=.18, s=0, rl=.12, w2=O2SAW, st=12, m2=.25, cho=.5, rev=.5, dly=.25, tm=.125, det=.5, amt=0),
 p("Koto", w=SAW, c=2600, ft=BP, res=.5, fe=.5, a=.001, d=.3, s=0, rl=.25, w2=O2SQR, st=5, m2=.25, rev=.45, dly=.35, tm=.375, amt=0),
 p("Star Arp", w=TRI, c=4500, res=.25, fe=.4, a=.002, d=.22, s=0, rl=.2, mode=POLY16, w2=O2SQR, st=12, fi=-8, m2=.35, dly=.5, tm=.25, fb=.55, cho=.4, rev=.5, amt=0),
 p("Bounce Arp", w=SQR, c=1900, res=.4, fe=.65, a=.002, d=.2, s=.1, rl=.15, mode=RETRIG, dst=D_PW, src=L_TRI, amt=.4, rate=3, dly=.4, tm=.1875, fb=.45, rev=.3),
 p("Music Box", w=SIN, c=8000, res=.05, fe=.2, a=.001, d=.9, s=0, rl=.6, w2=O2SIN, st=24, m2=.4, cho=.25, rev=.6, dly=.25, tm=.5, amt=0),
 p("Echo Pluck", w=SAW, c=2400, res=.3, fe=.55, a=.002, d=.3, s=0, rl=.25, w2=O2SAW, st=0, fi=14, m2=.4, dis=.1, dly=.55, tm=.333, fb=.6, rev=.35, amt=0),
])
bank("FX", [
 p("Riser", w=SSAW, c=1800, res=.5, fe=.8, a=.5, d=2.5, s=.6, rl=2, w2=O2SAW, st=12, m2=.3, dst=D_PITCH, src=L_UP, amt=.8, rate=.4, dly=.5, tm=.5, fb=.6, rev=.8),
 p("Radio", w=SQR, c=3000, ft=BP, res=.6, a=.6, d=2, s=.8, rl=2, dst=D_CUT, src=L_SH, amt=.7, rate=8, w2=O2P25, st=7, m2=.3, cho=.5, rev=.75, dly=.3),
 p("Siren", w=SAW, c=3500, res=.3, a=.02, d=1, s=1, rl=.5, mode=LEGATO, dst=D_PITCH, src=L_TRI, amt=.9, rate=.8, w2=O2SQR, st=0, fi=15, m2=.4, rev=.5, dly=.3),
 p("Thunder", w=SSAW, c=300, ft=LP24, res=.4, a=1.2, d=3, s=.8, rl=3, w2=O2SAW, st=-24, m2=.5, dis=.35, dst=D_CUT, src=L_SH, amt=.5, rate=1.5, rev=.85),
 p("Data", w=SQR, c=4500, ft=HP, res=.4, a=.01, d=.5, s=.6, rl=.4, dst=D_PW, src=L_SH, amt=.9, rate=12, w2=O2P25, st=24, m2=.3, dly=.45, tm=.125, fb=.5, rev=.4),
 p("Waves", w=SSAW, c=1200, ft=LP12, res=.2, a=2, d=3, s=.9, rl=3.5, dst=D_CUT, src=L_SIN, amt=.6, rate=.1, w2=O2TRI, st=-12, m2=.3, cho=.7, rev=.9, det=.9),
 p("Alien", w=SAW, c=2400, ft=BP, res=.5, a=.1, d=1, s=.8, rl=1, dst=D_PITCH, src=L_SQR, amt=.6, rate=7, w2=O2SIN, st=7, fi=30, m2=.45, dly=.5, tm=.375, fb=.6, rev=.6),
 p("Tunnel", w=SAW, c=2000, ft=ACID, res=.6, fe=.4, a=.05, d=1.5, s=.7, rl=1, dst=D_CUT, src=L_UP, amt=.7, rate=2, dis=.3, dly=.4, tm=.25, rev=.5),
 p("Power Up", w=SQR, c=5000, res=.2, a=.002, d=.6, s=.2, rl=.3, dst=D_PITCH, src=L_UP, amt=.85, rate=6, w2=O2SQR, st=12, m2=.3, dly=.35, tm=.125, rev=.35),
 p("Drone", w=TRI, c=900, res=.35, a=2.5, d=4, s=1, rl=4, w2=O2SAW, st=7, fi=-11, m2=.4, dst=D_PAN, src=L_SIN, amt=.6, rate=.08, cho=.6, rev=.9, dly=.3, tm=.75, fb=.6),
])

# basses (souvent jouees en mono) et plucks (enveloppes courtes) sortent plus bas que les pads a volume egal
LOUDER = {"Basses": 1.0, "Plucks": 1.0}

def fmt(v):
    return f"{float(v)}f"

lines = ["// Genere par Tools/gen_presets.py - ne pas editer a la main", "#pragma once", "#include <vector>", "#include <utility>",
         "namespace Neon {",
         "struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };",
         "inline const std::vector<PresetDef>& presets() { static const std::vector<PresetDef> P = {"]
names = set()
count = 0
for bname, rows in BANKS:
    for name, vals in rows:
        assert name not in names, name
        names.add(name)
        for k in vals: assert k in D, k
        full = dict(D)
        if bname in LOUDER: full["master_volume"] = LOUDER[bname]   # equilibre de niveau entre banques
        full.update(vals)
        body = ", ".join(f'{{"{k}", {fmt(v)}}}' for k, v in full.items())
        lines.append(f'  {{"{bname}", "{name}", {{{body}}}}},')
        count += 1
lines += ["}; return P; }", "}"]
assert count == 70, count
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "Source", "Presets.h")
open(out, "w", encoding="utf-8").write("\n".join(lines) + "\n")
print(f"{count} presets -> {os.path.normpath(out)}")
