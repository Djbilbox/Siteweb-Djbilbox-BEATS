#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets GOLDHAWK (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.01, d=.5, s=.8, r=.4, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, rso=.15, vel=.5,
         rev=.35, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
HWK, BRS, PAD, PLK, BEL, STR, BAS, EAG = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Hawk Leads", [
 p("Goldhawk Lead", eng=HWK, t1=.5, t2=.4, t3=.6, t4=.4, t5=.2, mono=1, gl=.06, dly=.25, tm=.375, fb=.4, rev=.35),
 p("Soaring", eng=HWK, t1=.7, t2=.5, t3=.7, t4=.3, t5=.3, a=.08, mono=1, gl=.15, cho=.3, rev=.5),
 p("Royal Pulse", eng=HWK, t1=.4, t2=.6, t3=.5, t4=.6, t5=.2, mono=1, rev=.3),
 p("Golden Wire", eng=HWK, t1=.3, t2=.2, t3=.5, t4=.3, t5=.1, mono=1, gl=.08, cut=3500, rev=.3),
 p("Sky Solo", eng=HWK, t1=.6, t2=.3, t3=.8, t4=.4, t5=.3, mono=1, gl=.12, src=0, dst=0, rate=5.5, amt=.07, dly=.3, tm=.3, rev=.4),
 p("Hawk Wide", eng=HWK, t1=.5, t2=.9, t3=.6, t4=.5, t5=.5, cho=.45, rev=.4, vol=.6),
 p("Thermal", eng=HWK, t1=.8, t2=.4, t3=.4, t4=.9, t5=.2, a=.15, mono=1, gl=.2, rev=.5),
 p("Eagle Eye", eng=HWK, t1=.2, t2=.3, t3=.9, t4=.3, t5=.4, mono=1, dis=.2, rev=.25),
 p("Golden Stab", eng=HWK, t1=.5, t2=.4, t3=.7, t4=.4, t5=.2, a=.002, d=.2, s=0, r=.15, fen=.7, fdc=.15, cut=1500, rev=.3, vel=.8),
 p("Altitude", eng=HWK, t1=.6, t2=.5, t3=.5, t4=.5, t5=.2, a=.05, mono=1, pit=12, dly=.35, tm=.45, fb=.5, rev=.55)])
bank("Golden Brass", [
 p("Golden Brass", eng=BRS, t1=.3, t2=.7, t3=.6, t4=.5, t5=.2, a=.03, d=.4, s=.8, r=.3, cho=.2, rev=.4),
 p("Royal Fanfare", eng=BRS, t1=.2, t2=.8, t3=.8, t4=.6, t5=.1, a=.015, d=.3, s=.7, r=.25, cho=.25, rev=.5),
 p("Hawk Stab", eng=BRS, t1=.2, t2=.8, t3=.7, t4=.5, t5=.1, a=.005, d=.25, s=.1, r=.2, rev=.3, vel=.8),
 p("Imperial Horns", eng=BRS, t1=.3, t2=.9, t3=.9, t4=.8, t5=.2, a=.02, d=.4, s=.7, r=.3, cho=.3, rev=.55, vol=.6),
 p("Soft Flugel", eng=BRS, t1=.6, t2=.5, t3=.3, t4=.4, t5=.1, a=.1, d=.6, s=.8, r=.5, rev=.45),
 p("Slow Swell", eng=BRS, t1=.8, t2=.6, t3=.5, t4=.4, t5=.1, a=.4, d=.8, s=.85, r=.7, rev=.5),
 p("Low Brass", eng=BRS, t1=.4, t2=.7, t3=.4, t4=.5, t5=.2, a=.04, d=.5, s=.8, r=.3, pit=-12, rev=.4),
 p("Gold Rush Hit", eng=BRS, t1=.3, t2=.8, t3=.8, t4=.5, t5=.1, a=.003, d=.2, s=0, r=.15, rev=.35),
 p("Trumpet Call", eng=BRS, t1=.4, t2=.6, t3=.8, t4=.2, t5=.2, a=.015, d=.2, s=.6, r=.2, pit=12, mono=1, rev=.4),
 p("Cinematic Brass", eng=BRS, t1=.6, t2=.8, t3=.6, t4=.7, t5=.2, a=.25, d=.8, s=.9, r=.8, cho=.3, rev=.65, vol=.6)])
bank("Royal Pads", [
 p("Royal Pad", eng=PAD, t2=.4, t3=.4, t4=.5, t5=.2, a=1, d=1, s=.9, r=2, cho=.4, rev=.6),
 p("Golden Hour", eng=PAD, t2=.3, t3=.7, t4=.7, t5=.2, a=.8, d=.8, s=.85, r=1.5, pit=12, cho=.4, rev=.55),
 p("Palace Air", eng=PAD, t2=.5, t3=.5, t4=.6, t5=.3, a=1.2, d=1, s=.9, r=2, cho=.5, rev=.65),
 p("Rising Hawk", eng=PAD, t2=.5, t3=.4, t4=.5, t5=.2, a=2.5, d=1.5, s=.9, r=3, fen=.6, fa=2.2, fdc=1.5, cut=700, rso=.3, rev=.7),
 p("Cloud Throne", eng=PAD, t2=.6, t3=.3, t4=.4, t5=.4, a=1.8, d=1, s=.95, r=2.5, rev=.75, pit=-12),
 p("Warm Gold", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.1, a=.6, d=.9, s=.9, r=1.2, cho=.35, rev=.5),
 p("Dream Flight", eng=PAD, t2=.35, t3=.6, t4=.6, t5=.3, a=.8, d=.8, s=.9, r=1.5, src=0, dst=1, rate=.2, amt=.4, cho=.4, rev=.6),
 p("Dusk Strings", eng=PAD, t2=.35, t3=.65, t4=.5, t5=.1, a=.4, d=.8, s=.9, r=1, cho=.4, rev=.45),
 p("Whisper Wind", eng=PAD, t2=.2, t3=.5, t4=.6, t5=.9, a=1.5, d=1, s=.9, r=2.5, cut=2200, src=0, dst=1, rate=.15, amt=.5, rev=.65, vol=.6),
 p("Pulse Of Gold", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.2, a=.8, d=.9, s=.9, r=1.5, src=0, dst=2, rate=1.1, amt=.5, rev=.6)])
bank("Falcon Plucks", [
 p("Falcon Pluck", eng=PLK, t1=.3, t2=.4, t3=.6, t4=.4, t5=.2, d=.4, s=0, r=.25, rev=.35, dly=.25, tm=.25, fb=.4),
 p("Gold Arp", eng=PLK, t1=.2, t2=.3, t3=.6, t4=.3, t5=.1, d=.25, s=0, r=.15, dly=.3, tm=.1875, fb=.5, rev=.3),
 p("Dive Zap", eng=PLK, t1=.9, t2=.3, t3=.8, t4=.2, t5=.3, d=.25, s=0, r=.15, rev=.25),
 p("Muted Talon", eng=PLK, t1=.1, t2=.2, t3=.4, t4=.4, t5=.1, d=.15, s=0, r=.1, rev=.2, vel=.8),
 p("Soft Feather", eng=PLK, t1=.2, t2=.6, t3=.3, t4=.4, t5=0, d=.6, s=0, r=.4, rev=.45),
 p("Pluck Choir", eng=PLK, t1=.2, t2=.4, t3=.6, t4=.9, t5=.2, d=.5, s=0, r=.3, cho=.5, rev=.45, vol=.6),
 p("Golden Harp", eng=PLK, t1=.1, t2=.7, t3=.5, t4=.6, t5=.1, d=.9, s=0, r=.6, pit=12, dly=.2, tm=.3, rev=.5),
 p("Royal Koto", eng=PLK, t1=.3, t2=.5, t3=.6, t4=.1, t5=.5, d=.7, s=0, r=.4, rev=.4),
 p("Rhythm Gate", eng=PLK, t1=.5, t2=.3, t3=.7, t4=.5, t5=.3, d=.3, s=0, r=.2, dly=.35, tm=.125, fb=.55, rev=.3),
 p("Sad Hawk", eng=PLK, t1=.2, t2=.6, t3=.4, t4=.5, t5=.1, d=.8, s=0, r=.5, cut=4000, rev=.55)])
bank("Sky Bells & Strings", [
 p("Sky Bell", eng=BEL, t1=.4, t2=.5, t3=.5, t4=.3, t5=.1, d=1.3, s=0, r=.9, rev=.5, dly=.25, tm=.375, fb=.4),
 p("Golden Chimes", eng=BEL, t1=.6, t2=.5, t3=.7, t4=.5, t5=.2, d=1.2, s=0, r=.8, pit=12, rev=.55, vol=.6),
 p("Crystal Crown", eng=BEL, t1=.5, t2=.8, t3=.7, t4=.7, t5=.1, d=2.2, s=0, r=1.8, pit=12, cho=.3, rev=.65, vol=.55),
 p("Temple Bell", eng=BEL, t1=.3, t2=.9, t3=.4, t4=.3, t5=.3, d=3, s=0, r=2.5, pit=-12, rev=.7),
 p("Music Box Gold", eng=BEL, t1=.6, t2=.3, t3=.5, t4=.3, t5=.1, d=.8, s=0, r=.6, pit=12, rev=.5),
 p("Feather Strings", eng=STR, t1=.4, t2=.5, t3=.4, t4=.6, t5=.3, a=.3, d=.7, s=.9, r=1, cho=.35, rev=.5),
 p("Violin Flight", eng=STR, t1=.5, t2=.4, t3=.5, t4=.8, t5=.2, a=.15, d=.6, s=.9, r=.7, mono=1, gl=.05, rev=.45),
 p("Slow Rise Strings", eng=STR, t1=.4, t2=.6, t3=.35, t4=.5, t5=.3, a=1.4, d=1, s=.95, r=1.8, rev=.6),
 p("Dark Cellos", eng=STR, t1=.3, t2=.8, t3=.25, t4=.4, t5=.2, a=.3, d=.7, s=.9, r=.9, pit=-12, rev=.5),
 p("Pizz Gold", eng=STR, t1=.5, t2=.4, t3=.5, t4=.2, t5=.1, a=.002, d=.25, s=0, r=.15, rev=.3, vel=.8, vol=.95)])
bank("Talon Bass", [
 p("Talon Bass", eng=BAS, t1=.5, t2=.5, t3=.4, t4=.2, mono=1, pit=-12, vol=.8, rev=.1),
 p("Gold Sub", eng=BAS, t1=.8, t2=.3, t3=.3, t4=.1, mono=1, pit=-12, vol=.85, rev=.05),
 p("Dirty Talon", eng=BAS, t1=.4, t2=.6, t3=.5, t4=.7, mono=1, pit=-12, dis=.2, vol=.75, rev=.1),
 p("Wobble Wing", eng=BAS, t1=.5, t2=.5, t3=.4, t4=.4, mono=1, pit=-12, src=0, dst=1, rate=3, amt=.7, rso=.4, cut=500, vol=.8, rev=.1),
 p("Industrial Hawk", eng=BAS, t1=.3, t2=.7, t3=.6, t4=.8, mono=1, pit=-12, dis=.3, vol=.7, rev=.05),
 p("Slide Bass", eng=BAS, t1=.5, t2=.4, t3=.4, t4=.3, mono=1, gl=.25, pit=-12, vol=.8, rev=.1),
 p("Acid Talon", eng=BAS, t1=.4, t2=.4, t3=.5, t4=.5, mono=1, pit=-12, fen=.9, fdc=.2, rso=.7, cut=400, vol=.75, rev=.1),
 p("Deep Nest", eng=BAS, t1=.7, t2=.2, t3=.2, t4=.1, mono=1, pit=-12, vol=.85, rev=.15),
 p("Bass Stab", eng=BAS, t1=.4, t2=.5, t3=.6, t4=.4, d=.2, s=0, r=.1, fen=.8, fdc=.12, cut=600, pit=-12, mono=1, vol=.8),
 p("Meltdown Gold", eng=BAS, t1=.5, t2=.9, t3=.5, t4=1, mono=1, pit=-12, dis=.4, src=0, dst=0, rate=.3, amt=.2, vol=.65)])
bank("Eagle Cry & FX", [
 p("Eagle Cry", eng=EAG, t1=.5, t2=.4, t3=.5, t4=.5, t5=.4, a=.005, d=.8, s=.3, r=.8, mono=1, rev=.5),
 p("Hawk Screech", eng=EAG, t1=.8, t2=.3, t3=.7, t4=.7, t5=.6, a=.003, d=.5, s=.2, r=.5, mono=1, pit=12, rev=.4),
 p("Distant Call", eng=EAG, t1=.4, t2=.5, t3=.4, t4=.4, t5=.3, a=.01, d=1.2, s=.3, r=1.5, mono=1, dly=.4, tm=.5, fb=.6, rev=.7),
 p("Falcon Dive", eng=EAG, t1=1, t2=.3, t3=.6, t4=1, t5=.3, a=.002, d=.6, s=.1, r=.4, mono=1, rev=.4),
 p("Wind Rush", eng=EAG, t1=.3, t2=.2, t3=.3, t4=.3, t5=1, a=.6, d=1, s=.9, r=1.2, rev=.6, vol=.35),
 p("Mountain Echo", eng=EAG, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, a=.02, d=1, s=.3, r=1.5, mono=1, dly=.45, tm=.5, fb=.65, rev=.7, vol=.7),
 p("Golden Whistle", eng=EAG, t1=.6, t2=.2, t3=.3, t4=.2, t5=.1, a=.05, d=.5, s=.7, r=.5, mono=1, gl=.1, pit=12, rev=.5),
 p("Raptor Siren", eng=EAG, t1=.7, t2=.4, t3=.6, t4=.9, t5=.3, a=.2, s=1, r=1, mono=1, gl=.3, src=2, dst=0, rate=.3, amt=.8, rev=.5),
 p("Night Hunter", eng=EAG, t1=.3, t2=.6, t3=.3, t4=.5, t5=.5, a=.3, d=1, s=.6, r=1.5, pit=-12, cut=2500, rev=.7, vol=.6),
 p("Thunder Wing", eng=EAG, t1=.2, t2=.7, t3=.4, t4=.8, t5=.8, a=.01, d=1, s=.2, r=1.5, pit=-12, dis=.2, rev=.6, vol=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Gh {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
