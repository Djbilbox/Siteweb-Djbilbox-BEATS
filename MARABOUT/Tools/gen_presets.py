#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets MARABOUT (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.005, d=.5, s=.7, r=.4, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.25, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
KOR, BAL, OUD, NEY, ZUR, CHO, PAD, SUB = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Kora & Harps", [
 p("Kora Dawn", eng=KOR, t2=.6, t3=.6, t5=.4, d=1.2, s=0, r=.8, rev=.35, vel=.7),
 p("Griot Kora", eng=KOR, t2=.7, t3=.55, t5=.6, d=1.5, s=0, r=1, cho=.2, rev=.4),
 p("Night Harp", eng=KOR, t2=.8, t3=.35, t5=.3, d=2, s=0, r=1.4, rev=.5, dly=.2, tm=.4, fb=.4),
 p("Bright Kora", eng=KOR, t2=.5, t3=.9, t5=.5, d=.9, s=0, r=.6, cho=.25, rev=.3, vel=.8),
 p("Soft Pluck", eng=KOR, t2=.4, t3=.3, t5=.2, d=.7, s=0, r=.5, cut=5000, rev=.3),
 p("Desert Harp", eng=KOR, t2=.75, t3=.45, t5=.7, d=2.2, s=0, r=1.5, pit=12, dly=.3, tm=.45, fb=.5, rev=.5),
 p("Sahel Arp", eng=KOR, t2=.5, t3=.7, t5=.4, d=.6, s=0, r=.4, dly=.25, tm=.25, fb=.45, rev=.3),
 p("Moonlit Kora", eng=KOR, t2=.85, t3=.4, t5=.5, d=2.4, s=0, r=1.8, cho=.4, rev=.6, vol=.65),
 p("Low Kora", eng=KOR, t2=.65, t3=.35, t5=.3, d=1.4, s=0, r=.9, pit=-12, rev=.35),
 p("Glass Strings", eng=KOR, t2=.9, t3=.95, t5=.8, d=2.8, s=0, r=2, pit=12, cho=.3, rev=.55, vol=.6)])
bank("Balafon & Mallets", [
 p("Balafon Classic", eng=BAL, t2=.5, t3=.5, t5=.4, d=.8, s=0, r=.4, rev=.3, vel=.8),
 p("Gourd Balafon", eng=BAL, t2=.6, t3=.45, t5=.8, d=1, s=0, r=.5, rev=.35),
 p("Wood Bells", eng=BAL, t2=.7, t3=.8, t5=.2, d=1.4, s=0, r=.8, pit=12, rev=.4),
 p("Marimba Moon", eng=BAL, t2=.65, t3=.3, t5=.1, d=1.2, s=0, r=.7, pit=-12, rev=.4, cho=.2),
 p("Dry Mallet", eng=BAL, t2=.2, t3=.5, t5=.3, d=.3, s=0, r=.2, rev=.15, vel=.9),
 p("Rain Balafon", eng=BAL, t2=.55, t3=.6, t5=.5, d=.9, s=0, r=.5, dly=.3, tm=.18, fb=.5, rev=.45),
 p("Kalimba Dream", eng=BAL, t2=.75, t3=.7, t5=.1, d=1.6, s=0, r=1, pit=12, cho=.2, rev=.5),
 p("Buzzy Balafon", eng=BAL, t2=.5, t3=.55, t5=1, d=.8, s=0, r=.4, dis=.15, rev=.25),
 p("Ritual Mallets", eng=BAL, t2=.8, t3=.4, t5=.6, d=1.8, s=0, r=1.2, pit=-12, rev=.5, dly=.2, tm=.35),
 p("Ice Xylo", eng=BAL, t2=.45, t3=.95, t5=0, d=.7, s=0, r=.5, pit=12, rev=.4, vol=.65)])
bank("Oud & Plucks", [
 p("Oud Taksim", eng=OUD, t1=.5, t2=.55, t3=.55, t5=.5, d=1, s=0, r=.5, rev=.3),
 p("Warm Oud", eng=OUD, t1=.3, t2=.7, t3=.35, t5=.6, d=1.3, s=0, r=.6, rev=.35),
 p("Bright Oud", eng=OUD, t1=.8, t2=.5, t3=.8, t5=.4, d=.8, s=0, r=.4, rev=.25, vel=.8),
 p("Saz Night", eng=OUD, t1=.6, t2=.6, t3=.6, t5=.7, d=1.1, s=0, r=.5, cho=.2, rev=.4),
 p("Low Oud", eng=OUD, t1=.4, t2=.65, t3=.4, t5=.6, d=1.2, s=0, r=.6, pit=-12, rev=.3),
 p("Desert Guitar", eng=OUD, t1=.55, t2=.45, t3=.65, t5=.3, d=.9, s=0, r=.5, dly=.25, tm=.35, fb=.4, rev=.35),
 p("Gnawa Guembri", eng=OUD, t1=.35, t2=.4, t3=.3, t5=.8, d=.6, s=0, r=.25, pit=-12, mono=1, dis=.1, rev=.2, vol=.8),
 p("Tape Oud", eng=OUD, t1=.5, t2=.6, t3=.4, t5=.5, d=1, s=0, r=.5, cut=3500, cho=.35, rev=.3),
 p("Muted Pick", eng=OUD, t1=.7, t2=.2, t3=.5, t5=.3, d=.25, s=0, r=.15, rev=.15, vel=.9),
 p("Oud Dub", eng=OUD, t1=.5, t2=.55, t3=.5, t5=.5, d=1, s=0, r=.5, dly=.4, tm=.42, fb=.6, rev=.45)])
bank("Ney & Zurna", [
 p("Ney Whisper", eng=NEY, t1=.4, t2=.5, t3=.3, t4=.5, t5=.5, a=.12, d=.4, s=.85, r=.5, rev=.45),
 p("Desert Ney", eng=NEY, t1=.5, t2=.6, t3=.4, t4=.6, t5=.35, a=.08, d=.4, s=.9, r=.6, rev=.5, dly=.2, tm=.45),
 p("Breathy Reed", eng=NEY, t1=.6, t2=.4, t3=.35, t4=.4, t5=.85, a=.15, d=.5, s=.8, r=.5, rev=.4),
 p("Sufi Flute", eng=NEY, t1=.45, t2=.5, t3=.5, t4=.8, t5=.4, a=.1, d=.5, s=.9, r=.7, rev=.55, cho=.2),
 p("High Ney", eng=NEY, t1=.5, t2=.5, t3=.3, t4=.5, t5=.4, a=.07, d=.4, s=.85, r=.5, pit=12, rev=.5),
 p("Zurna Call", eng=ZUR, t1=.5, t2=.5, t3=.6, t4=.5, t5=.3, a=.02, d=.3, s=.85, r=.25, rev=.3),
 p("Wedding Zurna", eng=ZUR, t1=.65, t2=.6, t3=.7, t4=.7, t5=.4, a=.015, d=.3, s=.9, r=.2, mono=1, gl=.05, rev=.3),
 p("Nasal Reed", eng=ZUR, t1=.3, t2=.7, t3=.5, t4=.4, t5=.2, a=.03, d=.35, s=.8, r=.3, rev=.35),
 p("Mizmar", eng=ZUR, t1=.55, t2=.55, t3=.65, t4=.9, t5=.3, a=.02, d=.3, s=.85, r=.3, src=0, dst=0, rate=5.5, amt=.06, rev=.35),
 p("Snake Charmer", eng=ZUR, t1=.4, t2=.5, t3=.45, t4=.6, t5=.35, a=.04, d=.4, s=.8, r=.4, mono=1, gl=.12, src=0, dst=0, rate=4.5, amt=.1, rev=.4, dly=.2, tm=.3)])
bank("Choir & Voices", [
 p("Marabout Choir", eng=CHO, t1=.15, t2=.5, t3=.5, t4=.5, t5=.3, a=.5, d=.8, s=.9, r=1.2, cho=.3, rev=.55),
 p("Ooo Ancestors", eng=CHO, t1=.7, t2=.55, t3=.35, t4=.6, t5=.2, a=.7, d=.9, s=.9, r=1.5, rev=.6),
 p("Ah Elders", eng=CHO, t1=0, t2=.5, t3=.5, t4=.4, t5=.3, a=.4, d=.7, s=.9, r=1, cho=.25, rev=.5),
 p("Ee Spirits", eng=CHO, t1=.95, t2=.5, t3=.6, t4=.5, t5=.4, a=.6, d=.8, s=.85, r=1.2, rev=.55),
 p("Vowel Sweep", eng=CHO, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, a=.4, d=.8, s=.9, r=1, src=0, dst=3, rate=.3, amt=.7, rev=.5),
 p("Whisper Choir", eng=CHO, t1=.3, t2=.4, t3=.3, t4=.7, t5=.9, a=.8, d=1, s=.85, r=1.6, rev=.65, vol=.6),
 p("Low Voices", eng=CHO, t1=.25, t2=.6, t3=.4, t4=.5, t5=.3, a=.5, d=.8, s=.9, r=1.2, pit=-12, rev=.5),
 p("Muezzin Pad", eng=CHO, t1=.1, t2=.55, t3=.55, t4=.3, t5=.2, a=.3, d=.6, s=.9, r=.9, gl=.2, mono=1, src=0, dst=0, rate=5.2, amt=.12, rev=.5),
 p("Gospel Dust", eng=CHO, t1=.6, t2=.5, t3=.6, t4=.8, t5=.3, a=.35, d=.7, s=.9, r=1.1, cho=.5, rev=.55),
 p("Hollow Chant", eng=CHO, t1=.4, t2=.7, t3=.2, t4=.4, t5=.5, a=.6, d=.9, s=.85, r=1.4, cut=2500, rev=.6, dly=.25, tm=.5)])
bank("Dark Pads & Drones", [
 p("Candle Pad", eng=PAD, t2=.4, t3=.35, t4=.5, t5=.1, a=1, d=1, s=.9, r=1.8, cho=.4, rev=.6),
 p("Smoke Drone", eng=PAD, t2=.6, t3=.2, t4=.3, t5=.3, a=1.5, d=1, s=.95, r=2.5, rev=.7, pit=-12),
 p("Midnight Souk", eng=PAD, t2=.5, t3=.5, t4=.6, t5=.2, a=.8, d=.9, s=.9, r=1.5, cho=.5, rev=.55),
 p("Shell Glow", eng=PAD, t2=.3, t3=.7, t4=.7, t5=.1, a=.6, d=.8, s=.85, r=1.2, pit=12, cho=.3, rev=.5),
 p("Ritual Swell", eng=PAD, t2=.5, t3=.4, t4=.5, t5=.2, a=2.2, d=1.5, s=.9, r=2.8, fen=.6, fa=2, fdc=1.5, fs=.5, fr=2, cut=700, rso=.3, rev=.65),
 p("Voodoo Mist", eng=PAD, t2=.6, t3=.3, t4=.8, t5=.5, a=1.2, d=1, s=.9, r=2, src=0, dst=1, rate=.15, amt=.4, cut=1500, rev=.7),
 p("Dune Wind", eng=PAD, t2=.2, t3=.6, t4=.6, t5=.9, a=1.5, d=1, s=.9, r=2.5, ft=2, cut=1800, rso=.4, src=0, dst=1, rate=.2, amt=.5, rev=.6, vol=.6),
 p("Amber Strings", eng=PAD, t2=.35, t3=.65, t4=.5, t5=.1, a=.4, d=.8, s=.9, r=1, cho=.4, rev=.45),
 p("Tomb Choir Pad", eng=PAD, t2=.7, t3=.25, t4=.4, t5=.3, a=1.8, d=1.2, s=.95, r=3, pit=-12, cut=1200, rev=.75, vol=.65),
 p("Pulse Drone", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.2, a=.8, d=.9, s=.9, r=1.5, src=0, dst=2, rate=1.2, amt=.5, rev=.5)])
bank("Sub Voodoo & FX", [
 p("Voodoo Sub", eng=SUB, t1=.3, t2=.3, t3=.3, t5=.2, a=.005, d=1.2, s=.8, r=.35, mono=1, vol=.85, rev=.1),
 p("808 Marabout", eng=SUB, t1=.6, t2=.4, t3=.5, t5=.3, d=2, s=0, r=.5, vol=.85, rev=.1, vel=.7),
 p("Deep Hex", eng=SUB, t1=.2, t2=.2, t3=.2, t5=.1, d=1.5, s=.9, r=.5, mono=1, pit=-12, vol=.9, rev=.05),
 p("Knock Bass", eng=SUB, t1=.9, t2=.5, t3=.7, t5=.4, d=.6, s=.3, r=.2, mono=1, vol=.8, rev=.1),
 p("Dirty Sub", eng=SUB, t1=.4, t2=.5, t3=.4, t5=.9, d=1, s=.8, r=.3, mono=1, dis=.25, vol=.7, rev=.1),
 p("Glide Sub", eng=SUB, t1=.2, t2=.3, t3=.2, t5=.3, d=1.5, s=.9, r=.4, mono=1, gl=.25, vol=.85, rev=.1),
 p("Ghost Boom", eng=SUB, t1=.7, t2=.3, t3=.6, t5=.2, d=2.5, s=0, r=1.5, pit=-12, rev=.6, dly=.3, tm=.4, fb=.5, vol=.8),
 p("Bell Drop", eng=BAL, t2=.9, t3=.9, t5=.3, d=2.4, s=0, r=2, pit=12, src=2, dst=0, rate=.4, amt=.15, rev=.7, dly=.35, tm=.4, fb=.55, vol=.6),
 p("Spirit Siren", eng=NEY, t1=.5, t2=.5, t3=.6, t4=.9, t5=.3, a=.3, s=1, r=1.2, gl=.4, mono=1, src=2, dst=0, rate=.3, amt=.9, rev=.6, dly=.3, tm=.4),
 p("Hex Zap", eng=ZUR, t1=.9, t2=.4, t3=.8, t4=.6, t5=.6, a=.002, d=.25, s=0, r=.2, fen=.9, fdc=.15, rso=.6, cut=800, gl=.2, mono=1, dis=.2, dly=.2, tm=.2, rev=.3, vol=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Mb {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
           'inline const std::vector<PresetDef>& presets() { static const std::vector<PresetDef> P = {']
    n = 0
    for b, rows in BANKS:
        assert len(rows) == 10, b
        for name, k in rows:
            kv = dict(D); kv.update(k)
            items = ', '.join('{"%s", %sf}' % (KEY[x], repr(float(kv[x]))) for x in KEY)
            out.append('  {"%s", "%s", {%s}},' % (b, name, items)); n += 1
    out.append('}; return P; } }')
    assert n == 70
    return '\n'.join(out) + '\n'

if __name__ == '__main__':
    import sys
    open(sys.argv[1], 'w').write(emit())
