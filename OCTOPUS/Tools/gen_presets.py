#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets OCTOPUS (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.01, d=.5, s=.8, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.4, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
SUB, BUB, BEL, PAD, RSE, LED, SIR, WHA = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Kraken Sub", [
 p("Kraken Sub", eng=SUB, t1=.4, t2=.3, t3=.4, t5=.4, a=.004, d=1.5, s=.8, r=.5, mono=1, vol=.85, rev=.1),
 p("Deep Trench", eng=SUB, t1=.2, t2=.2, t3=.1, t5=.1, d=2.5, s=.9, r=.8, mono=1, pit=-12, vol=.9, rev=.15),
 p("Pressure Drop", eng=SUB, t1=1, t2=.4, t3=.6, t5=.3, d=1.2, s=.6, r=.4, mono=1, gl=.1, vol=.8, rev=.1),
 p("Long Slide Sub", eng=SUB, t1=.3, t2=.3, t3=.3, t5=.3, d=2, s=.9, r=.8, mono=1, gl=.35, vol=.85, rev=.1),
 p("Dirty Kraken", eng=SUB, t1=.5, t2=.5, t3=.5, t5=.9, d=1.5, s=.8, r=.4, mono=1, dis=.25, vol=.7, rev=.1),
 p("Smooth Tide", eng=SUB, t1=.1, t2=.1, t3=.1, t5=0, d=1.5, s=.95, r=.4, mono=1, vol=.9, rev=.1),
 p("Hard Knock Reef", eng=SUB, t1=.9, t2=.5, t3=.7, t5=.5, d=.8, s=.5, r=.25, mono=1, vol=.8, rev=.1),
 p("Short Wave", eng=SUB, t1=.6, t2=.4, t3=.5, t5=.4, d=.35, s=0, r=.15, mono=1, vol=.85, rev=.1),
 p("Rumble Below", eng=SUB, t1=.4, t2=.6, t3=.4, t5=1, d=1.6, s=.8, r=.5, mono=1, dis=.4, vol=.6, rev=.15),
 p("Submarine", eng=SUB, t1=.3, t2=.5, t3=.3, t5=.2, d=1.2, s=.85, r=.6, mono=1, src=0, dst=2, rate=.5, amt=.3, vol=.85, rev=.2)])
bank("Bubbles & Drops", [
 p("Bubble Pluck", eng=BUB, t1=.5, t2=.4, t3=.4, t4=.3, t5=.2, d=.4, s=0, r=.3, rev=.45, dly=.25, tm=.25, fb=.4),
 p("Water Drop", eng=BUB, t1=.7, t2=.2, t3=.3, t4=.3, t5=.3, d=.25, s=0, r=.2, rev=.55),
 p("Deep Bloop", eng=BUB, t1=.8, t2=.5, t3=.2, t4=.3, t5=.1, d=.5, s=0, r=.3, pit=-12, rev=.5),
 p("Rising Bubbles", eng=BUB, t1=1, t2=.3, t3=.5, t4=.3, t5=.2, d=.3, s=0, r=.2, dly=.3, tm=.1875, fb=.55, rev=.5),
 p("Reef Marimba", eng=BUB, t1=.2, t2=.6, t3=.6, t4=.3, t5=.1, d=.6, s=0, r=.4, rev=.4),
 p("Aqua Arp", eng=BUB, t1=.4, t2=.2, t3=.5, t4=.3, t5=.2, d=.2, s=0, r=.15, dly=.35, tm=.125, fb=.55, rev=.35),
 p("Pop Pop", eng=BUB, t1=.9, t2=.1, t3=.3, t4=.3, t5=.5, d=.12, s=0, r=.1, rev=.3, vel=.8),
 p("Glass Bubble", eng=BUB, t1=.6, t2=.5, t3=.9, t4=.3, t5=.1, d=.5, s=0, r=.4, pit=12, rev=.55, vol=.6),
 p("Tide Pool", eng=BUB, t1=.3, t2=.7, t3=.4, t4=.3, t5=.2, d=.8, s=0, r=.6, cho=.3, rev=.6),
 p("Foam", eng=BUB, t1=.5, t2=.3, t3=.6, t4=.3, t5=1, d=.3, s=0, r=.2, rev=.4)])
bank("Coral Bells", [
 p("Coral Bell", eng=BEL, t1=.4, t2=.5, t3=.5, t4=.3, t5=.1, d=1.3, s=0, r=.9, rev=.5, dly=.25, tm=.375, fb=.4),
 p("Shell Chimes", eng=BEL, t1=.6, t2=.5, t3=.7, t4=.5, t5=.2, d=1.2, s=0, r=.8, pit=12, rev=.55, vol=.6),
 p("Glass Jellyfish", eng=BEL, t1=.5, t2=.8, t3=.7, t4=.7, t5=.1, d=2.2, s=0, r=1.8, pit=12, cho=.3, rev=.65, vol=.55),
 p("Pearl", eng=BEL, t1=.3, t2=.4, t3=.4, t4=.3, t5=.2, d=1, s=0, r=.7, rev=.45),
 p("Sunken Bell", eng=BEL, t1=.3, t2=.9, t3=.4, t4=.3, t5=.3, d=3, s=0, r=2.5, pit=-12, rev=.7),
 p("Music Box Sea", eng=BEL, t1=.6, t2=.3, t3=.5, t4=.3, t5=.1, d=.8, s=0, r=.6, pit=12, rev=.5),
 p("Echo Chamber", eng=BEL, t1=.5, t2=.5, t3=.6, t4=.4, t5=.1, d=1, s=0, r=.8, dly=.45, tm=.5, fb=.6, rev=.5, vol=.55),
 p("Drift Chimes", eng=BEL, t1=.8, t2=.5, t3=.7, t4=.8, t5=.1, d=1.2, s=0, r=1, pit=12, dly=.35, tm=.25, fb=.55, rev=.65, vol=.55),
 p("Hollow Kalimba", eng=BEL, t1=.55, t2=.2, t3=.4, t4=.3, t5=.4, d=.5, s=0, r=.4, rev=.4),
 p("Sonar Ping", eng=BEL, t1=.2, t2=.7, t3=.3, t4=.2, t5=0, d=1.8, s=0, r=1.2, pit=12, dly=.4, tm=.5, fb=.65, rev=.5, vol=.6)])
bank("Abyss Pads", [
 p("Abyss Pad", eng=PAD, t2=.4, t3=.4, t4=.5, t5=.2, a=1.2, d=1, s=.9, r=2, cho=.4, rev=.65),
 p("Ink Cloud", eng=PAD, t2=.6, t3=.2, t4=.3, t5=.5, a=1.8, d=1, s=.95, r=2.5, rev=.75, pit=-12),
 p("Bioluminescence", eng=PAD, t2=.3, t3=.7, t4=.7, t5=.3, a=.8, d=.8, s=.85, r=1.5, pit=12, cho=.4, rev=.6),
 p("Rising Current", eng=PAD, t2=.5, t3=.4, t4=.5, t5=.2, a=2.5, d=1.5, s=.9, r=3, fen=.6, fa=2.2, fdc=1.5, cut=700, rso=.3, rev=.7),
 p("Kelp Forest", eng=PAD, t2=.35, t3=.6, t4=.6, t5=.3, a=.6, d=.8, s=.9, r=1.2, src=0, dst=1, rate=.2, amt=.4, cho=.4, rev=.6),
 p("Dark Water", eng=PAD, t2=.8, t3=.2, t4=.4, t5=.3, a=2, d=1.2, s=.95, r=3, pit=-12, cut=1200, rev=.8, vol=.6),
 p("Surface Light", eng=PAD, t2=.2, t3=.8, t4=.6, t5=.2, a=1, d=1, s=.9, r=1.8, pit=12, cho=.5, rev=.6, vol=.6),
 p("Whisper Current", eng=PAD, t2=.2, t3=.5, t4=.6, t5=.9, a=1.5, d=1, s=.9, r=2.5, cut=2200, src=0, dst=1, rate=.15, amt=.5, rev=.65, vol=.6),
 p("Pulse Of The Deep", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.2, a=.8, d=.9, s=.9, r=1.5, src=0, dst=2, rate=1.1, amt=.5, rev=.6),
 p("Drowned Strings", eng=PAD, t2=.4, t3=.5, t4=.9, t5=.4, a=1.4, d=1, s=.9, r=2.2, cho=.6, rev=.7, vol=.6)])
bank("Ink Reese & Leads", [
 p("Ink Reese", eng=RSE, t1=.5, t2=.5, t3=.4, t4=.2, mono=1, pit=-12, vol=.8, rev=.1),
 p("Eel Bass", eng=RSE, t1=.4, t2=.6, t3=.5, t4=.7, mono=1, pit=-12, dis=.2, vol=.75, rev=.1),
 p("Wobble Reef", eng=RSE, t1=.5, t2=.5, t3=.4, t4=.4, mono=1, pit=-12, src=0, dst=1, rate=3, amt=.7, rso=.4, cut=500, vol=.8, rev=.1),
 p("Octopus Bass", eng=RSE, t1=.4, t2=.4, t3=.5, t4=.5, mono=1, pit=-12, fen=.9, fdc=.2, rso=.7, cut=400, vol=.75, rev=.1),
 p("Tentacle Lead", eng=LED, t1=.5, t2=.4, t3=.5, t4=.4, t5=.2, mono=1, gl=.08, dly=.25, tm=.375, fb=.4, rev=.4),
 p("Chrome Squid", eng=LED, t1=.8, t2=.6, t3=.5, t4=.3, t5=.2, cho=.4, rev=.4),
 p("Pulse Current", eng=LED, t1=.9, t2=.3, t3=.6, t4=.9, t5=.2, mono=1, rev=.3),
 p("Slow Wave Lead", eng=LED, t1=.6, t2=.5, t3=.4, t4=.2, t5=.1, a=.1, mono=1, gl=.15, rev=.5),
 p("Sea Cave Lead", eng=LED, t1=.3, t2=.5, t3=.3, t4=.3, t5=.4, mono=1, cut=2500, dly=.35, tm=.5, fb=.5, rev=.6),
 p("Pulse Stab", eng=LED, t1=.5, t2=.4, t3=.7, t4=.4, t5=.1, a=.002, d=.2, s=0, r=.15, fen=.7, fdc=.15, cut=1500, rev=.3, vel=.8)])
bank("Siren Choir", [
 p("Siren Choir", eng=SIR, t1=.15, t2=.5, t3=.5, t4=.5, t5=.3, a=.5, d=.9, s=.9, r=1.4, cho=.3, rev=.6),
 p("Ooo Sirens", eng=SIR, t1=.7, t2=.55, t3=.35, t4=.6, t5=.3, a=.6, d=1, s=.9, r=1.6, rev=.65),
 p("Ah Mermaids", eng=SIR, t1=0, t2=.5, t3=.5, t4=.4, t5=.3, a=.4, d=.8, s=.9, r=1.2, cho=.25, rev=.55),
 p("Ee Deep", eng=SIR, t1=.95, t2=.5, t3=.6, t4=.5, t5=.4, a=.5, d=.9, s=.85, r=1.4, rev=.6),
 p("Vowel Tide", eng=SIR, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, a=.5, d=.9, s=.9, r=1.2, src=0, dst=3, rate=.25, amt=.7, rev=.6),
 p("Whisper Reef", eng=SIR, t1=.3, t2=.4, t3=.3, t4=.7, t5=.9, a=.8, d=1, s=.85, r=1.8, rev=.7, vol=.6),
 p("Low Sirens", eng=SIR, t1=.25, t2=.6, t3=.4, t4=.5, t5=.3, a=.6, d=.9, s=.9, r=1.3, pit=-12, rev=.6),
 p("Lure Song", eng=SIR, t1=.1, t2=.55, t3=.55, t4=.3, t5=.2, a=.4, d=.7, s=.9, r=1, gl=.2, mono=1, src=0, dst=0, rate=5.2, amt=.1, rev=.65),
 p("Coral Chorale", eng=SIR, t1=.6, t2=.5, t3=.6, t4=.8, t5=.3, a=.45, d=.8, s=.9, r=1.2, cho=.5, rev=.6),
 p("Hollow Chant", eng=SIR, t1=.4, t2=.7, t3=.2, t4=.4, t5=.5, a=.7, d=1, s=.85, r=1.5, cut=2500, rev=.65, dly=.25, tm=.5)])
bank("Whale Song", [
 p("Whale Song", eng=WHA, t1=.4, t2=.3, t3=.5, t4=.7, t5=.2, a=.4, d=1, s=.9, r=1.5, mono=1, gl=.2, rev=.65, dly=.3, tm=.5, fb=.5),
 p("Humpback", eng=WHA, t1=.3, t2=.5, t3=.4, t4=.8, t5=.3, a=.5, d=1, s=.9, r=1.8, mono=1, pit=-12, gl=.25, rev=.7),
 p("Blue Whale Call", eng=WHA, t1=.2, t2=.6, t3=.3, t4=.9, t5=.2, a=.8, d=1.2, s=.9, r=2.5, mono=1, pit=-12, rev=.75, vol=.8),
 p("Sonar Sweep", eng=WHA, t1=.9, t2=.2, t3=.6, t4=.5, t5=.1, a=.05, d=.6, s=.6, r=.8, mono=1, dly=.4, tm=.5, fb=.6, rev=.5),
 p("Dolphin Click", eng=WHA, t1=.8, t2=.1, t3=.8, t4=.2, t5=.8, a=.002, d=.15, s=0, r=.1, pit=12, rev=.4, vol=.6),
 p("Siren Of The Deep", eng=WHA, t1=.6, t2=.4, t3=.5, t4=.9, t5=.2, a=.3, s=1, r=1, mono=1, gl=.4, src=2, dst=0, rate=.3, amt=.7, rev=.6),
 p("Ghost Ship", eng=WHA, t1=.4, t2=.5, t3=.4, t4=.6, t5=.5, a=1, d=1, s=.9, r=2, cut=2500, rev=.75, vol=.6),
 p("Distant Pod", eng=WHA, t1=.35, t2=.4, t3=.5, t4=.7, t5=.3, a=.6, d=1, s=.9, r=1.5, mono=1, pit=12, dly=.45, tm=.45, fb=.65, rev=.7, vol=.55),
 p("Ocean Drone", eng=WHA, t1=.1, t2=.7, t3=.3, t4=.4, t5=.4, a=1.5, d=1, s=.95, r=3, pit=-12, rev=.8, vol=.7),
 p("Sperm Song", eng=WHA, t1=.5, t2=.3, t3=.6, t4=.6, t5=.2, a=.3, d=.8, s=.85, r=1.2, mono=1, gl=.15, cho=.3, rev=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Oc {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
