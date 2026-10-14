#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets DZPOWER (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.005, d=.7, s=.8, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.3, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
MAN, QAN, GAS, GHA, ORG, STR, DRB, BEN = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Mandole", [
 p("Mandole Alger", eng=MAN, t1=.5, t2=.6, t3=.6, t4=.4, t5=.4, d=1, s=0, r=.5, rev=.35),
 p("Mandole Chaabi", eng=MAN, t1=.6, t2=.5, t3=.7, t4=.4, t5=.5, d=.8, s=0, r=.4, dly=.2, tm=.25, fb=.3, rev=.3),
 p("Casbah Pick", eng=MAN, t1=.7, t2=.4, t3=.6, t4=.3, t5=.3, d=.6, s=0, r=.3, rev=.25, vel=.8),
 p("Warm Mandole", eng=MAN, t1=.3, t2=.7, t3=.4, t4=.4, t5=.6, d=1.2, s=0, r=.6, cho=.25, rev=.4),
 p("Tremolo Pick", eng=MAN, t1=.5, t2=.5, t3=.6, t4=.4, t5=.4, d=1, s=0, r=.4, src=0, dst=2, rate=9, amt=.35, rev=.3),
 p("Low Mandole", eng=MAN, t1=.4, t2=.6, t3=.4, t4=.4, t5=.5, d=1.1, s=0, r=.5, pit=-12, rev=.35),
 p("Bright Strum", eng=MAN, t1=.8, t2=.4, t3=.9, t4=.4, t5=.3, d=.5, s=0, r=.25, cho=.3, rev=.3),
 p("Rai Pluck", eng=MAN, t1=.5, t2=.5, t3=.6, t4=.4, t5=.4, d=.7, s=0, r=.3, dis=.1, dly=.3, tm=.375, fb=.4, rev=.3),
 p("Muted Mandole", eng=MAN, t1=.6, t2=.2, t3=.4, t4=.3, t5=.3, d=.3, s=0, r=.12, rev=.2, vel=.8),
 p("Desert Night Pluck", eng=MAN, t1=.3, t2=.7, t3=.5, t4=.4, t5=.5, d=1.3, s=0, r=.8, dly=.4, tm=.45, fb=.5, rev=.55)])
bank("Qanun & Plucks", [
 p("Qanun Dar", eng=QAN, t1=.5, t2=.6, t3=.6, t4=.5, t5=.3, d=1.2, s=0, r=.8, rev=.4),
 p("Zither Rain", eng=QAN, t1=.5, t2=.5, t3=.7, t4=.6, t5=.3, d=.9, s=0, r=.6, dly=.3, tm=.1875, fb=.5, rev=.4),
 p("Harp Of Tlemcen", eng=QAN, t1=.4, t2=.7, t3=.5, t4=.5, t5=.2, d=1.6, s=0, r=1.2, cho=.3, rev=.55),
 p("Bright Qanun", eng=QAN, t1=.6, t2=.5, t3=.9, t4=.5, t5=.3, d=.8, s=0, r=.5, pit=12, rev=.35),
 p("Soft Qanun", eng=QAN, t1=.3, t2=.4, t3=.3, t4=.4, t5=.1, d=.7, s=0, r=.4, cut=5000, rev=.4),
 p("Oriental Arp", eng=QAN, t1=.5, t2=.4, t3=.6, t4=.4, t5=.3, d=.4, s=0, r=.2, dly=.35, tm=.125, fb=.55, rev=.3),
 p("Low Qanun", eng=QAN, t1=.4, t2=.7, t3=.4, t4=.5, t5=.2, d=1.4, s=0, r=.8, pit=-12, rev=.4),
 p("Glass Qanun", eng=QAN, t1=.7, t2=.6, t3=.9, t4=.8, t5=.2, d=1.5, s=0, r=1.2, pit=12, cho=.3, rev=.6, vol=.6),
 p("Sahara Strings", eng=QAN, t1=.5, t2=.6, t3=.5, t4=.6, t5=.3, d=1.1, s=0, r=.7, dly=.4, tm=.5, fb=.55, rev=.5),
 p("Zellij Pluck", eng=QAN, t1=.5, t2=.3, t3=.6, t4=.3, t5=.2, d=.5, s=0, r=.3, rev=.3, vel=.8)])
bank("Gasba & Ghaita", [
 p("Gasba Reve", eng=GAS, t1=.4, t2=.5, t3=.3, t4=.5, t5=.4, a=.08, d=.4, s=.85, r=.5, rev=.5),
 p("Desert Gasba", eng=GAS, t1=.5, t2=.6, t3=.4, t4=.6, t5=.35, a=.06, d=.4, s=.9, r=.6, rev=.5, dly=.2, tm=.45),
 p("Breathy Gasba", eng=GAS, t1=.6, t2=.4, t3=.35, t4=.4, t5=.9, a=.1, d=.5, s=.8, r=.5, mono=1, rev=.45),
 p("Sufi Gasba", eng=GAS, t1=.45, t2=.5, t3=.5, t4=.8, t5=.4, a=.1, d=.5, s=.9, r=.7, rev=.55, cho=.2),
 p("High Gasba", eng=GAS, t1=.5, t2=.5, t3=.3, t4=.5, t5=.4, a=.07, d=.4, s=.85, r=.5, pit=12, rev=.5),
 p("Ghaita Call", eng=GHA, t1=.5, t2=.5, t3=.6, t4=.5, t5=.3, a=.02, d=.3, s=.85, r=.25, rev=.3),
 p("Zorna Fete", eng=GHA, t1=.65, t2=.6, t3=.7, t4=.7, t5=.4, a=.015, d=.3, s=.9, r=.2, mono=1, gl=.05, rev=.3),
 p("Nasal Reed", eng=GHA, t1=.3, t2=.7, t3=.5, t4=.4, t5=.2, a=.03, d=.35, s=.8, r=.3, rev=.35),
 p("Mizmar Dz", eng=GHA, t1=.55, t2=.55, t3=.65, t4=.9, t5=.3, a=.02, d=.3, s=.85, r=.3, src=0, dst=0, rate=5.5, amt=.06, rev=.35),
 p("Snake Zorna", eng=GHA, t1=.4, t2=.5, t3=.45, t4=.6, t5=.35, a=.04, d=.4, s=.8, r=.4, mono=1, gl=.12, src=0, dst=0, rate=4.5, amt=.1, rev=.4, dly=.2, tm=.3)])
bank("Rai Organ", [
 p("Rai Organ", eng=ORG, t1=.6, t2=.7, t3=.4, t4=.6, t5=.5, a=.005, d=.2, s=.9, r=.12, cho=.3, rev=.3),
 p("Oran Night", eng=ORG, t1=.7, t2=.5, t3=.2, t4=.4, t5=.2, a=.01, d=.3, s=.9, r=.2, cho=.4, rev=.45),
 p("Full Rai", eng=ORG, t1=.9, t2=.9, t3=.9, t4=.5, t5=.4, d=.2, s=.9, r=.12, cho=.3, rev=.3, vol=.55),
 p("Rai Lead Organ", eng=ORG, t1=.5, t2=.6, t3=.7, t4=.6, t5=.5, d=.2, s=.9, r=.1, mono=1, gl=.04, cho=.3, rev=.3),
 p("Accordeon Dz", eng=ORG, t1=.4, t2=.8, t3=.6, t4=.9, t5=.3, a=.03, d=.3, s=.9, r=.15, cho=.45, rev=.35),
 p("Vintage Rai Keys", eng=ORG, t1=.5, t2=.6, t3=.5, t4=.5, t5=.4, d=.25, s=.8, r=.15, cut=4000, cho=.35, rev=.3),
 p("Soft Flute Stop", eng=ORG, t1=.2, t2=.8, t3=.1, t4=.2, t5=.1, a=.02, d=.3, s=.9, r=.25, rev=.4),
 p("Rotary Casbah", eng=ORG, t1=.6, t2=.6, t3=.5, t4=.5, t5=.3, d=.3, s=.9, r=.2, src=0, dst=3, rate=6, amt=.3, cho=.6, rev=.4),
 p("Tape Rai", eng=ORG, t1=.6, t2=.5, t3=.3, t4=.4, t5=.3, d=.3, s=.9, r=.2, cut=2500, cho=.5, rev=.4),
 p("Golden Swell", eng=ORG, t1=.7, t2=.6, t3=.3, t4=.3, t5=.1, a=.8, d=.5, s=.95, r=1.2, cho=.3, rev=.6)])
bank("Chaabi Strings", [
 p("Chaabi Strings", eng=STR, t1=.4, t2=.5, t3=.4, t4=.6, t5=.3, a=.3, d=.7, s=.9, r=1, cho=.35, rev=.5),
 p("Violon Alger", eng=STR, t1=.5, t2=.4, t3=.5, t4=.8, t5=.2, a=.15, d=.6, s=.9, r=.7, mono=1, gl=.05, rev=.45),
 p("Slow Swell", eng=STR, t1=.4, t2=.6, t3=.35, t4=.5, t5=.3, a=1.4, d=1, s=.95, r=1.8, rev=.6),
 p("Dark Cellos", eng=STR, t1=.3, t2=.8, t3=.25, t4=.4, t5=.2, a=.3, d=.7, s=.9, r=.9, pit=-12, rev=.5),
 p("High Violins", eng=STR, t1=.6, t2=.3, t3=.6, t4=.6, t5=.3, a=.35, d=.7, s=.9, r=1, pit=12, cho=.3, rev=.55),
 p("Pizzicato Dz", eng=STR, t1=.5, t2=.4, t3=.5, t4=.2, t5=.1, a=.002, d=.25, s=0, r=.15, rev=.3, vel=.8, vol=.95),
 p("Andalous Pad", eng=STR, t1=.45, t2=.6, t3=.4, t4=.7, t5=.4, a=.8, d=1, s=.9, r=1.6, cho=.4, rev=.65, vol=.65),
 p("Wobble Violin", eng=STR, t1=.4, t2=.5, t3=.35, t4=1, t5=.5, a=.4, d=.8, s=.9, r=1.2, rev=.5),
 p("Suspense Casbah", eng=STR, t1=.3, t2=.7, t3=.2, t4=.7, t5=.5, a=1.8, d=1, s=.95, r=2.5, fen=.4, fa=1.5, fdc=2, cut=900, rev=.7, vol=.6),
 p("Rooftop Strings", eng=STR, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, a=.5, d=.8, s=.9, r=1.4, dly=.2, tm=.45, fb=.4, cho=.3, rev=.55)])
bank("Derbouka & Perc", [
 p("Derbouka Doum", eng=DRB, t1=.5, t2=.5, t3=.5, t4=.3, t5=.3, d=.4, s=0, r=.1, rev=.2, vol=.85),
 p("Tak Tak", eng=DRB, t1=.3, t2=.2, t3=.8, t4=.8, t5=.5, d=.15, s=0, r=.08, rev=.15, vol=.8),
 p("Deep Doum", eng=DRB, t1=.4, t2=.8, t3=.3, t4=.2, t5=.2, d=.6, s=0, r=.2, pit=-12, rev=.25, vol=.9),
 p("Slap Derbouka", eng=DRB, t1=.6, t2=.3, t3=.9, t4=.9, t5=.6, d=.2, s=0, r=.1, rev=.2, vol=.75),
 p("Tabla Dz", eng=DRB, t1=.8, t2=.5, t3=.6, t4=.4, t5=.3, d=.35, s=0, r=.1, pit=12, rev=.3),
 p("Rai Percussion", eng=DRB, t1=.5, t2=.4, t3=.6, t4=.6, t5=.4, d=.3, s=0, r=.1, dly=.3, tm=.1875, fb=.4, rev=.3),
 p("Dub Doum", eng=DRB, t1=.5, t2=.6, t3=.4, t4=.3, t5=.2, d=.5, s=0, r=.2, dly=.4, tm=.375, fb=.55, rev=.4),
 p("Hand Drum Fill", eng=DRB, t1=.4, t2=.5, t3=.5, t4=.3, t5=.3, d=.35, s=0, r=.15, pit=7, rev=.3),
 p("Crowd Clap", eng=DRB, t1=.2, t2=.1, t3=1, t4=1, t5=.9, d=.15, s=0, r=.1, rev=.5, vol=.6),
 p("Drop Doum", eng=DRB, t1=1, t2=.9, t3=.5, t4=.3, t5=.3, d=1.2, s=0, r=.5, pit=-12, rev=.4, vol=.8)])
bank("Bendir Sub & Bass", [
 p("Bendir Sub", eng=BEN, t1=.4, t2=.3, t3=.4, t4=.3, t5=.4, a=.004, d=1.5, s=.8, r=.5, mono=1, vol=.85, rev=.1),
 p("Deep Casbah", eng=BEN, t1=.2, t2=.2, t3=.1, t4=.2, t5=.1, d=2.5, s=.9, r=.8, mono=1, pit=-12, vol=.9, rev=.15),
 p("Rattle Sub", eng=BEN, t1=.4, t2=.4, t3=.5, t4=.3, t5=.9, d=1.2, s=.7, r=.4, mono=1, vol=.8, rev=.1),
 p("Long Slide Sub", eng=BEN, t1=.3, t2=.3, t3=.3, t4=.3, t5=.2, d=2, s=.9, r=.8, mono=1, gl=.35, vol=.85, rev=.1),
 p("Pitch Drop Dz", eng=BEN, t1=1, t2=.4, t3=.6, t4=.3, t5=.3, d=1.2, s=.6, r=.4, mono=1, gl=.1, vol=.8, rev=.1),
 p("Dirty Bendir", eng=BEN, t1=.5, t2=.5, t3=.5, t4=.9, t5=.5, d=1.5, s=.8, r=.4, mono=1, dis=.25, vol=.7, rev=.1),
 p("Smooth Sub", eng=BEN, t1=.1, t2=.1, t3=.1, t4=0, t5=0, d=1.5, s=.95, r=.4, mono=1, vol=.9, rev=.1),
 p("Hard Knock Dz", eng=BEN, t1=.9, t2=.5, t3=.7, t4=.4, t5=.3, d=.8, s=.5, r=.25, mono=1, vol=.8, rev=.1),
 p("Short Boom", eng=BEN, t1=.6, t2=.4, t3=.5, t4=.3, t5=.4, d=.35, s=0, r=.15, mono=1, vol=.85, rev=.1),
 p("Rumble Alger", eng=BEN, t1=.4, t2=.6, t3=.4, t4=1, t5=.6, d=1.6, s=.8, r=.5, mono=1, dis=.4, vol=.6, rev=.15)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Dz {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
