#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets K2000 (17 d'usine du projet fourni, issus de Reference/web/presets.js, + 53 nouveaux)."""
import json, sys, os
D = {"lfo_rate":4.5,"lfo_amount":.35,"lfo_pitch":0,"lfo_attack":.1,"lfo_decay":.5,"lfo_sustain":1,"lfo_release":.4,
     "amp_attack":.02,"amp_decay":.8,"amp_sustain":.7,"velocity_bank":.75,"velocity_adsr":.4,"master_volume":.8,
     "fx_reverb":.35,"fx_chorus":.45,"fx_distortion":.25,"glide_time":.05,"filter_cutoff":3200,"filter_drive":.15,
     "fx_delay_mix":.3,"fx_delay_time":.375,"fx_delay_fb":.45,"osc_waveform":0,"lfo_source":1,"lfo_dest":0,
     "filter_type":0,"voice_mode":0,"filter_res":.2,"filter_env":0,"amp_release":.4,"osc_detune":.4}
SH = dict(w="osc_waveform", c="filter_cutoff", ft="filter_type", res="filter_res", fe="filter_env", dr="filter_drive",
          a="amp_attack", d="amp_decay", s="amp_sustain", rl="amp_release", vb="velocity_bank", va="velocity_adsr",
          la="lfo_attack", ld="lfo_decay", ls="lfo_sustain", lrl="lfo_release", rate="lfo_rate", amt="lfo_amount", lp="lfo_pitch",
          src="lfo_source", dst="lfo_dest", gl="glide_time", dis="fx_distortion", cho="fx_chorus", dly="fx_delay_mix",
          tm="fx_delay_time", fb="fx_delay_fb", rev="fx_reverb", vol="master_volume", mode="voice_mode", det="osc_detune")
SAW, SQR, TRI, SIN, SSAW, BRS = range(6)
LP24, LP12, BP, HP, ACID = range(5)
POLY8, POLY16, RETRIG, LEGATO = range(4)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, {SH[x]: v for x, v in k.items()})

# --- 17 presets d'usine fournis (params complets dans presets.js) ---
J = json.load(open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'factory.json')))
bank("VOL.1 Factory", [(x['name'], dict(x['params'])) for x in J])

bank("Leads", [
 p("17 - Laser Grid Lead", w=SSAW, c=5200, res=.3, a=.005, d=.6, s=.8, gl=.1, mode=LEGATO, cho=.45, dly=.35, tm=.375, rev=.4, det=.5),
 p("18 - Sunset Strip Lead", w=SQR, c=3600, res=.25, a=.01, d=.5, s=.8, gl=.08, mode=LEGATO, lp=1, amt=.25, src=0, cho=.4, dly=.3, rev=.45),
 p("19 - Neon Sirens", w=SAW, c=4200, res=.35, a=.05, d=1, s=.9, gl=.3, mode=LEGATO, dst=1, src=0, amt=.6, rate=.5, cho=.4, rev=.5),
 p("20 - Turbo Brass Lead", w=BRS, c=3200, res=.3, fe=.5, a=.02, d=.4, s=.7, gl=.06, mode=LEGATO, cho=.3, dly=.25, rev=.4),
 p("21 - Pulse Rider", w=SQR, c=2800, res=.3, a=.005, d=.4, s=.8, gl=.05, mode=RETRIG, dst=2, src=1, amt=.6, rate=.8, cho=.5, dly=.3),
 p("22 - Crystal Hero", w=SSAW, c=7000, res=.2, a=.01, d=.8, s=.85, gl=.12, mode=LEGATO, cho=.55, dly=.4, tm=.5, fb=.5, rev=.55, det=.6),
 p("23 - Acid Rain Lead", w=SAW, c=1400, ft=ACID, res=.6, fe=.7, d=.3, s=.4, gl=.08, mode=LEGATO, dis=.3, dly=.3, rev=.35),
 p("24 - Lost Highway", w=TRI, c=3000, res=.2, a=.02, d=.8, s=.9, gl=.15, mode=LEGATO, lp=1, amt=.3, src=0, rate=5.5, cho=.5, rev=.6, dly=.4),
 p("25 - Cyber Whistle", w=SIN, c=9000, res=.1, a=.04, d=.5, s=.9, gl=.2, mode=LEGATO, lp=1, amt=.35, src=0, rate=5, rev=.55, dly=.4, tm=.45),
])
bank("Bass", [
 p("26 - Night Pulse Bass", w=SAW, c=700, ft=LP24, res=.4, fe=.6, a=.003, d=.25, s=.3, rl=.12, mode=RETRIG, dis=.1, cho=.1, dly=0, rev=.1, amt=0),
 p("27 - Sub Highway", w=SIN, c=1200, res=.1, a=.005, d=.8, s=.9, rl=.2, mode=LEGATO, gl=.05, cho=0, rev=.05, dly=0, dis=.05, amt=0),
 p("28 - Darksynth Growl", w=SSAW, c=500, ft=ACID, res=.55, fe=.7, a=.005, d=.3, s=.5, rl=.15, mode=RETRIG, dis=.45, cho=.2, rev=.1, dst=0, src=0, amt=.35, rate=3),
 p("29 - Octave Chase", w=SQR, c=900, res=.35, fe=.5, a=.003, d=.2, s=.35, rl=.1, mode=RETRIG, cho=.15, dly=.2, tm=.1875, rev=.15, amt=0),
 p("30 - Moog Miami", w=SAW, c=600, res=.45, fe=.65, a=.005, d=.3, s=.4, rl=.15, mode=RETRIG, gl=.04, cho=.1, rev=.1, amt=0),
 p("31 - Rubber Grid", w=SQR, c=650, res=.5, fe=.7, a=.003, d=.18, s=.2, rl=.1, mode=RETRIG, dis=.2, cho=.1, rev=.1, amt=0),
 p("32 - Terminator Sub", w=SIN, c=800, res=.2, a=.01, d=1.2, s=.85, rl=.3, mode=LEGATO, gl=.1, dis=.2, cho=0, rev=.1, amt=0),
 p("33 - Wobble Chrome", w=SAW, c=550, res=.4, a=.005, d=.4, s=.7, rl=.2, mode=RETRIG, dst=0, src=0, amt=.7, rate=2, dis=.2, cho=.15, rev=.1),
 p("34 - Hi-NRG Bass", w=SAW, c=1100, res=.3, fe=.5, a=.002, d=.15, s=.5, rl=.08, mode=RETRIG, cho=.1, rev=.1, dly=.2, tm=.125, amt=0),
])
bank("Pads", [
 p("35 - Sunset Strip Pad", w=SSAW, c=2400, res=.15, a=.8, d=2, s=.9, rl=1.4, cho=.8, rev=.6, dly=.25, det=.6, amt=.2, rate=.4),
 p("36 - Blade Fog", w=SAW, c=1500, res=.2, a=1.2, d=2.5, s=.9, rl=2, cho=.7, rev=.75, amt=.4, rate=.3, src=0),
 p("37 - Outrun Strings", w=SSAW, c=3000, res=.1, a=.5, d=1.5, s=.9, rl=1.2, cho=.85, rev=.5, det=.7, amt=.15, rate=.5),
 p("38 - Neon Choir", w=TRI, c=2500, res=.15, a=.7, d=2, s=.9, rl=1.5, cho=.7, rev=.7, amt=.25, lp=1, src=0, rate=5),
 p("39 - Dark Cinema", w=SAW, c=900, res=.3, fe=.4, a=1.5, d=3, s=.95, rl=2.5, cho=.5, rev=.85, dst=0, src=1, amt=.5, rate=.2),
 p("40 - Aurora Slow Pad", w=SSAW, c=3500, res=.1, a=2, d=3, s=.95, rl=3, cho=.8, rev=.8, det=.8, amt=.2, rate=.15),
 p("41 - Cassette Warmth", w=SQR, c=1800, res=.1, a=.6, d=1.5, s=.85, rl=1.2, cho=.8, rev=.5, dis=.15, dst=2, amt=.3, rate=.6),
 p("42 - Retro Glass Pad", w=SIN, c=6000, res=.1, a=.5, d=2, s=.9, rl=1.5, cho=.6, rev=.75, dly=.3, tm=.5, amt=.1),
 p("43 - Cyborg Breath", w=BRS, c=2000, res=.25, fe=.5, a=1, d=2, s=.85, rl=1.6, cho=.6, rev=.7, amt=.3, rate=.35),
])
bank("Plucks & Arps", [
 p("44 - Arp Machine", w=SAW, c=2200, res=.4, fe=.7, a=.002, d=.2, s=0, rl=.15, mode=POLY16, dly=.5, tm=.1875, fb=.55, cho=.3, rev=.3, amt=0),
 p("45 - Neon Pluck", w=SQR, c=2800, res=.3, fe=.6, a=.002, d=.25, s=0, rl=.2, dly=.4, tm=.25, fb=.5, cho=.4, rev=.4, amt=0),
 p("46 - Glass Arp", w=SIN, c=8000, res=.1, a=.002, d=.35, s=0, rl=.3, dly=.5, tm=.1875, fb=.6, cho=.3, rev=.5, amt=0),
 p("47 - Sequence Chrome", w=SSAW, c=3200, res=.35, fe=.6, a=.002, d=.18, s=0, rl=.1, dly=.35, tm=.125, fb=.5, cho=.4, rev=.25, amt=0),
 p("48 - Dream Bell", w=TRI, c=6000, res=.1, a=.002, d=1, s=0, rl=.8, cho=.5, rev=.7, dly=.4, tm=.375, amt=0),
 p("49 - Laser Tag", w=SAW, c=5000, res=.5, fe=.9, a=.001, d=.15, s=0, rl=.1, dly=.3, tm=.125, cho=.2, rev=.2, amt=0),
 p("50 - Night Marimba", w=TRI, c=2500, res=.15, fe=.4, a=.002, d=.3, s=0, rl=.25, cho=.3, rev=.5, dly=.3, tm=.375, amt=0),
 p("51 - Filter Gate", w=SQR, c=1500, res=.45, fe=.8, a=.002, d=.12, s=0, rl=.08, dly=.4, tm=.1875, cho=.2, rev=.25, amt=0),
 p("52 - Echo Pluck", w=SAW, c=2400, res=.3, fe=.5, a=.002, d=.3, s=0, rl=.2, dly=.6, tm=.375, fb=.65, cho=.3, rev=.45, amt=0),
])
bank("Keys & Brass", [
 p("53 - Retro Electric Keys", w=TRI, c=2200, res=.1, fe=.3, a=.005, d=1.2, s=.35, rl=.5, cho=.7, rev=.4, amt=.2, lp=1, src=0, rate=5),
 p("54 - Synthpop Keys", w=SQR, c=2800, res=.15, fe=.4, a=.003, d=.6, s=.4, rl=.3, cho=.6, rev=.35, dly=.2, amt=0),
 p("55 - Power Brass", w=BRS, c=2600, res=.25, fe=.6, a=.04, d=.5, s=.6, rl=.25, cho=.4, rev=.4, amt=0),
 p("56 - 80s Horns", w=BRS, c=3400, res=.2, fe=.5, a=.03, d=.6, s=.7, rl=.3, cho=.5, rev=.5, dly=.2, amt=0, det=.6),
 p("57 - Stab Section", w=SSAW, c=3000, res=.3, fe=.7, a=.005, d=.3, s=.1, rl=.2, cho=.4, rev=.4, amt=0),
 p("58 - Soft Strings Keys", w=SAW, c=2400, res=.1, a=.15, d=1.2, s=.8, rl=.8, cho=.8, rev=.5, amt=.15, rate=.5),
 p("59 - Chip Organ", w=SQR, c=4500, res=.1, a=.003, d=.3, s=.9, rl=.1, cho=.3, rev=.25, dst=2, amt=.25, rate=6),
 p("60 - Polysynth Chord", w=SSAW, c=3300, res=.2, fe=.4, a=.02, d=.8, s=.7, rl=.5, cho=.65, rev=.45, amt=0, det=.5),
 p("61 - Jazz Fusion Keys", w=TRI, c=3500, res=.2, a=.004, d=1, s=.5, rl=.5, cho=.5, rev=.4, dly=.2, amt=.15),
])
bank("FX & Atmos", [
 p("62 - Warp Drive", w=SAW, c=800, ft=ACID, res=.6, fe=.8, a=.05, d=3, s=.9, rl=2, dst=0, src=3, amt=.8, rate=.3, rev=.6, dis=.3),
 p("63 - Nuclear Dawn", w=SSAW, c=1200, res=.3, fe=.5, a=2.5, d=4, s=.9, rl=3, rev=.9, cho=.6, amt=.3, rate=.12),
 p("64 - Sonar Ping", w=SIN, c=7000, res=.1, a=.002, d=1.5, s=0, rl=1.2, rev=.8, dly=.6, tm=.5, fb=.7, amt=0),
 p("65 - Static Rain", w=SQR, c=5000, ft=HP, res=.2, a=.5, d=1, s=.8, rl=1.5, dst=5, src=5, amt=.5, rate=8, rev=.7, dly=.4),
 p("66 - Tape Stop Dream", w=TRI, c=1800, res=.2, a=.4, d=2, s=.8, rl=2, dst=1, src=4, amt=.5, rate=.2, cho=.7, rev=.7),
 p("67 - Hologram", w=SSAW, c=4500, ft=BP, res=.4, a=.8, d=2, s=.8, rl=1.5, dst=5, src=0, amt=.6, rate=.4, rev=.75, cho=.6),
 p("68 - Overheating", w=SAW, c=1800, res=.4, a=.1, d=1, s=.8, rl=.8, dis=.7, dr=.7, dst=0, src=2, amt=.5, rate=9, rev=.4),
 p("69 - Last Exit", w=SSAW, c=2600, res=.2, a=1, d=3, s=.9, rl=3.5, cho=.8, rev=.9, dly=.5, tm=.5, fb=.6, amt=.2, rate=.2),
])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace K2k {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
           'inline const std::vector<PresetDef>& presets() { static const std::vector<PresetDef> P = {']
    n = 0
    for b, rows in BANKS:
        for name, k in rows:
            kv = dict(D); kv.update(k)
            assert all(x in D for x in kv), (name, [x for x in kv if x not in D])
            items = ', '.join('{"%s", %sf}' % (x, repr(float(kv[x]))) for x in D)
            safe = name.encode('ascii', 'replace').decode().replace('?', '-')
            out.append('  {"%s", "%s", {%s}},' % (b, safe, items)); n += 1
    out.append('}; return P; } }')
    assert n == 70, n
    return '\n'.join(out) + '\n'

if __name__ == '__main__':
    open(sys.argv[1], 'w').write(emit())
