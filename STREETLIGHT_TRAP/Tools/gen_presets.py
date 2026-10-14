#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets STREETLIGHT TRAP (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.003, d=.5, s=.8, r=.35, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.3, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
SUB, BEL, PLK, KEY_, PAD, CHO, BRS, FLT = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("808 & Sub Bass", [
 p("Streetlight 808", eng=SUB, t1=.4, t2=.3, t3=.4, t5=.4, d=1.5, s=.8, r=.5, mono=1, vol=.85, rev=.05),
 p("Long Slide 808", eng=SUB, t1=.3, t2=.3, t3=.3, t5=.3, d=2, s=.9, r=.8, mono=1, gl=.35, vol=.85, rev=.05),
 p("Dirty 808", eng=SUB, t1=.5, t2=.5, t3=.5, t5=.9, d=1.5, s=.8, r=.4, mono=1, dis=.25, vol=.7, rev=.05),
 p("Boom Bap Sub", eng=SUB, t1=.2, t2=.2, t3=.2, t5=.2, d=1, s=.7, r=.3, mono=1, vol=.9, rev=.05),
 p("Hard Knock", eng=SUB, t1=.9, t2=.5, t3=.7, t5=.5, d=.8, s=.5, r=.25, mono=1, vol=.8, rev=.05),
 p("Deep Cellar", eng=SUB, t1=.2, t2=.2, t3=.1, t5=.1, d=2.5, s=.9, r=.8, mono=1, pit=-12, vol=.9, rev=.1),
 p("Short 808", eng=SUB, t1=.6, t2=.4, t3=.5, t5=.4, d=.35, s=0, r=.15, mono=1, vol=.85, rev=.05),
 p("Distorted Rumble", eng=SUB, t1=.4, t2=.6, t3=.4, t5=1, d=1.6, s=.8, r=.5, mono=1, dis=.4, vol=.6, rev=.1),
 p("Smooth Sub", eng=SUB, t1=.1, t2=.1, t3=.1, t5=0, d=1.5, s=.95, r=.4, mono=1, vol=.9, rev=.05),
 p("Pitch Drop", eng=SUB, t1=1, t2=.4, t3=.6, t5=.3, d=1.2, s=.6, r=.4, mono=1, gl=.1, vol=.8, rev=.05)])
bank("Trap Bells", [
 p("Trap Bell", eng=BEL, t1=.3, t2=.5, t3=.5, t4=.3, t5=.1, d=1.2, s=0, r=.8, rev=.4, dly=.25, tm=.375, fb=.4),
 p("Music Box Trap", eng=BEL, t1=.6, t2=.3, t3=.5, t4=.3, t5=.1, d=.8, s=0, r=.6, pit=12, rev=.45),
 p("Glass Lead", eng=BEL, t1=.5, t2=.6, t3=.7, t4=.5, t5=.1, d=1.4, s=0, r=1, pit=12, rev=.5, vol=.6),
 p("Dark Chimes", eng=BEL, t1=.2, t2=.7, t3=.4, t4=.3, t5=.3, d=2, s=0, r=1.5, rev=.6),
 p("Night Bells", eng=BEL, t1=.4, t2=.5, t3=.6, t4=.6, t5=.1, d=1.5, s=0, r=1, dly=.3, tm=.1875, fb=.55, rev=.5),
 p("Kalimba Trap", eng=BEL, t1=.55, t2=.2, t3=.4, t4=.3, t5=.4, d=.5, s=0, r=.4, rev=.35),
 p("Icy Pluck", eng=BEL, t1=.8, t2=.3, t3=.8, t4=.5, t5=.2, d=.6, s=0, r=.4, pit=12, rev=.4, vol=.6),
 p("Haunted Box", eng=BEL, t1=.4, t2=.7, t3=.6, t4=.3, t5=.2, d=1.5, s=0, r=1.2, src=0, dst=0, rate=.8, amt=.15, rev=.65, dly=.3, tm=.5, fb=.5),
 p("Low Bell", eng=BEL, t1=.35, t2=.6, t3=.4, t4=.3, t5=.2, d=1.5, s=0, r=1, pit=-12, rev=.5),
 p("Echo Bells", eng=BEL, t1=.5, t2=.5, t3=.6, t4=.5, t5=.1, d=1, s=0, r=.8, dly=.4, tm=.5, fb=.6, rev=.55, vol=.55)])
bank("Dark Plucks", [
 p("Dark Pluck", eng=PLK, t1=.3, t2=.4, t3=.5, t4=.4, t5=.2, d=.4, s=0, r=.25, rev=.3, dly=.25, tm=.25, fb=.4),
 p("Trap Arp", eng=PLK, t1=.2, t2=.3, t3=.6, t4=.3, t5=.1, d=.25, s=0, r=.15, dly=.3, tm=.1875, fb=.5, rev=.25),
 p("Zap Down", eng=PLK, t1=.9, t2=.3, t3=.8, t4=.2, t5=.3, d=.25, s=0, r=.15, rev=.2),
 p("Muted Pluck", eng=PLK, t1=.1, t2=.2, t3=.4, t4=.4, t5=.1, d=.15, s=0, r=.1, rev=.15, vel=.8),
 p("Soft Pluck", eng=PLK, t1=.2, t2=.6, t3=.3, t4=.4, t5=0, d=.6, s=0, r=.4, rev=.4),
 p("Pluck Wall", eng=PLK, t1=.2, t2=.4, t3=.6, t4=.9, t5=.2, d=.5, s=0, r=.3, cho=.5, rev=.4, vol=.6),
 p("Street Harp", eng=PLK, t1=.1, t2=.7, t3=.5, t4=.6, t5=.1, d=.9, s=0, r=.6, pit=12, dly=.2, tm=.3, rev=.45),
 p("Dark Koto", eng=PLK, t1=.3, t2=.5, t3=.6, t4=.1, t5=.5, d=.7, s=0, r=.4, rev=.35),
 p("Drill Pluck", eng=PLK, t1=.5, t2=.3, t3=.7, t4=.5, t5=.3, d=.3, s=0, r=.2, dly=.35, tm=.125, fb=.55, rev=.25),
 p("Sad Pluck", eng=PLK, t1=.2, t2=.6, t3=.4, t4=.5, t5=.1, d=.8, s=0, r=.5, cut=4000, rev=.5)])
bank("Street Keys", [
 p("Street EP", eng=KEY_, t2=.5, t3=.5, d=1.4, s=.3, r=.6, cho=.3, rev=.3, vel=.7),
 p("Lofi Keys", eng=KEY_, t2=.6, t3=.3, d=1.6, s=.3, r=.7, cut=3000, cho=.4, rev=.35),
 p("Bark EP", eng=KEY_, t2=.4, t3=.85, d=1.2, s=.25, r=.5, dis=.15, rev=.2, vel=.9),
 p("Trap Soul Keys", eng=KEY_, t2=.7, t3=.4, d=2, s=.4, r=1, cho=.35, rev=.45),
 p("Tremolo Keys", eng=KEY_, t2=.5, t3=.5, d=1.5, s=.3, src=0, dst=2, rate=5, amt=.45, cho=.2, rev=.3),
 p("Midnight Rhodes", eng=KEY_, t2=.8, t3=.3, d=2.6, s=.5, r=1.4, cho=.35, rev=.55, pit=-12),
 p("Wurly Bite", eng=KEY_, t2=.35, t3=.9, d=.9, s=.15, r=.3, dis=.2, rev=.15),
 p("Bell Keys", eng=KEY_, t2=.6, t3=.95, d=1.4, s=.2, r=.8, pit=12, rev=.4),
 p("Rooftop Chords", eng=KEY_, t2=.65, t3=.45, d=1.5, s=.3, r=.8, dly=.2, tm=.3, fb=.3, cho=.3, rev=.45),
 p("Dusty Piano", eng=KEY_, t2=.55, t3=.35, d=1.2, s=.2, r=.5, cut=2500, rev=.3)])
bank("Neon Pads & Brass", [
 p("Streetlight Pad", eng=PAD, t2=.4, t3=.4, t4=.5, t5=.2, a=.8, d=1, s=.9, r=1.5, cho=.4, rev=.55),
 p("Smoke Pad", eng=PAD, t2=.6, t3=.2, t4=.3, t5=.4, a=1.2, d=1, s=.95, r=2, rev=.65, pit=-12),
 p("Alley Strings", eng=PAD, t2=.35, t3=.6, t4=.6, t5=.1, a=.4, d=.8, s=.9, r=1, cho=.4, rev=.45),
 p("Slow Rise", eng=PAD, t2=.5, t3=.4, t4=.5, t5=.2, a=2, d=1.5, s=.9, r=2.5, fen=.6, fa=1.8, fdc=1.5, cut=700, rev=.6),
 p("Dream Pad", eng=PAD, t2=.4, t3=.5, t4=.8, t5=.3, a=1, d=1, s=.9, r=1.8, cho=.6, rev=.65, vol=.6),
 p("Dark Drone", eng=PAD, t2=.8, t3=.2, t4=.4, t5=.3, a=1.5, d=1.2, s=.95, r=2.5, pit=-12, cut=1200, rev=.7, vol=.6),
 p("Trap Brass", eng=BRS, t1=.3, t2=.7, t3=.6, t4=.4, t5=.2, a=.02, d=.4, s=.7, r=.3, cho=.2, rev=.3),
 p("Horn Stab", eng=BRS, t1=.2, t2=.8, t3=.7, t4=.5, t5=.1, a=.005, d=.25, s=.1, r=.2, rev=.3, vel=.8),
 p("Soft Brass", eng=BRS, t1=.6, t2=.5, t3=.3, t4=.5, t5=.1, a=.12, d=.6, s=.8, r=.5, rev=.4),
 p("Heist Horns", eng=BRS, t1=.4, t2=.8, t3=.8, t4=.8, t5=.2, a=.03, d=.4, s=.6, r=.3, cho=.3, rev=.35, vol=.6)])
bank("Trap Choir", [
 p("Trap Choir", eng=CHO, t1=.15, t2=.5, t3=.5, t4=.5, t5=.3, a=.5, d=.9, s=.9, r=1.2, cho=.3, rev=.55),
 p("Ooo Voices", eng=CHO, t1=.7, t2=.55, t3=.35, t4=.6, t5=.3, a=.6, d=1, s=.9, r=1.4, rev=.6),
 p("Ah Gospel", eng=CHO, t1=0, t2=.5, t3=.5, t4=.4, t5=.3, a=.4, d=.8, s=.9, r=1, cho=.25, rev=.5),
 p("Ee Ghost", eng=CHO, t1=.95, t2=.5, t3=.6, t4=.5, t5=.4, a=.5, d=.9, s=.85, r=1.2, rev=.55),
 p("Vowel Drift", eng=CHO, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, a=.4, d=.9, s=.9, r=1.1, src=0, dst=3, rate=.25, amt=.7, rev=.5),
 p("Whisper Crowd", eng=CHO, t1=.3, t2=.4, t3=.3, t4=.7, t5=.9, a=.8, d=1, s=.85, r=1.6, rev=.65, vol=.6),
 p("Low Choir", eng=CHO, t1=.25, t2=.6, t3=.4, t4=.5, t5=.3, a=.5, d=.9, s=.9, r=1.2, pit=-12, rev=.55),
 p("Chopped Vox", eng=CHO, t1=.4, t2=.5, t3=.6, t4=.4, t5=.2, a=.005, d=.25, s=0, r=.2, dly=.3, tm=.1875, fb=.5, rev=.4),
 p("Hymn In The Rain", eng=CHO, t1=.1, t2=.55, t3=.55, t4=.3, t5=.2, a=.4, d=.7, s=.9, r=1, gl=.2, mono=1, rev=.65),
 p("Hollow Chant", eng=CHO, t1=.4, t2=.7, t3=.2, t4=.4, t5=.5, a=.6, d=1, s=.85, r=1.4, cut=2500, rev=.6, dly=.25, tm=.5)])
bank("Flutes & FX", [
 p("Trap Flute", eng=FLT, t1=.4, t2=.5, t3=.3, t4=.5, t5=.4, a=.06, d=.4, s=.85, r=.4, mono=1, gl=.05, rev=.4, dly=.25, tm=.375, fb=.4),
 p("Pan Flute", eng=FLT, t1=.5, t2=.6, t3=.4, t4=.3, t5=.7, a=.05, d=.4, s=.85, r=.4, rev=.45),
 p("Breathy Flute", eng=FLT, t1=.6, t2=.4, t3=.35, t4=.4, t5=.9, a=.1, d=.5, s=.8, r=.5, mono=1, rev=.45),
 p("Sad Flute", eng=FLT, t1=.45, t2=.5, t3=.3, t4=.8, t5=.4, a=.12, d=.5, s=.9, r=.7, mono=1, rev=.55, cho=.2),
 p("High Whistle", eng=FLT, t1=.5, t2=.5, t3=.2, t4=.5, t5=.3, a=.07, mono=1, pit=12, gl=.08, rev=.5),
 p("Flute Echo", eng=FLT, t1=.5, t2=.5, t3=.4, t4=.5, t5=.4, a=.06, mono=1, dly=.45, tm=.375, fb=.6, rev=.5),
 p("Wind Lead", eng=FLT, t1=.7, t2=.7, t3=.5, t4=.6, t5=1, a=.15, mono=1, gl=.15, rev=.5, vol=.6),
 p("Low Flute", eng=FLT, t1=.4, t2=.6, t3=.4, t4=.5, t5=.4, a=.08, d=.5, s=.85, r=.5, pit=-12, rev=.45),
 p("Fog Flute", eng=FLT, t1=.3, t2=.4, t3=.2, t4=.6, t5=.8, a=.4, d=1, s=.9, r=1.2, rev=.7, vol=.6),
 p("Siren Flute", eng=FLT, t1=.5, t2=.5, t3=.5, t4=.9, t5=.3, a=.2, s=1, r=.9, mono=1, gl=.4, src=2, dst=0, rate=.3, amt=.7, rev=.5)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Sl {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
