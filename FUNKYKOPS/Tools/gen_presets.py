#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets VOODOO (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.005, d=.4, s=.7, r=.3, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fdc=.3, rso=.15, vel=.5,
         rev=.2, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fdc='fdecay', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
CLV, RHO, BAS, BRA, STR, LEA, GTR, TLK = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Clavinet & Funk Keys", [
 p("Chicken Scratch", eng=CLV, t2=.4, t3=.75, t5=.5, d=.25, s=.1, r=.15, rev=.12, vel=.8),
 p("Disco Clav", eng=CLV, t2=.5, t3=.65, t5=.4, d=.4, s=.2, r=.2, cho=.2, rev=.18),
 p("Wah Patrol", eng=CLV, t2=.45, t3=.6, t5=.5, d=.5, s=.3, fen=.5, fdc=.35, rso=.45, cut=600, src=0, dst=1, rate=2.5, amt=.3, vel=.8),
 p("Afro Clav", eng=CLV, t2=.55, t3=.7, t5=.7, d=.35, s=.15, dis=.1, rev=.15),
 p("Stevie Stab", eng=CLV, t2=.4, t3=.8, t5=.6, d=.2, s=0, r=.12, vel=.9, rev=.1),
 p("Funk Pickup", eng=CLV, t2=.6, t3=.5, t5=.3, d=.6, s=.3, r=.3, cho=.15, rev=.2),
 p("Bootsy Clav", eng=CLV, t2=.5, t3=.55, t5=.6, d=.3, s=.1, pit=-12, mono=1, dis=.2, rev=.1),
 p("Siren Clav", eng=CLV, t2=.5, t3=.7, t5=.5, d=.4, s=.2, src=0, dst=0, rate=6, amt=.12, rev=.2),
 p("Dusty Clav", eng=CLV, t2=.65, t3=.35, t5=.3, d=.7, s=.3, r=.4, cut=3500, rev=.25),
 p("Badge Clav", eng=CLV, t2=.45, t3=.85, t5=.8, d=.25, s=.1, fen=.4, fdc=.2, rso=.35, cut=1200, rev=.12)])
bank("Rhodes & EP", [
 p("Rhodes Suitcase", eng=RHO, t2=.5, t3=.5, d=1.4, s=.3, r=.6, cho=.35, rev=.3, vel=.7),
 p("Silk Rhodes", eng=RHO, t2=.6, t3=.35, d=1.8, s=.35, r=.8, cho=.3, rev=.4),
 p("Bark Rhodes", eng=RHO, t2=.4, t3=.85, d=1.2, s=.25, r=.5, dis=.15, rev=.2, vel=.9),
 p("Midnight EP", eng=RHO, t2=.7, t3=.3, d=2.2, s=.4, r=1, cho=.4, rev=.5),
 p("Disco EP", eng=RHO, t2=.45, t3=.6, d=1, s=.2, r=.4, cho=.5, dly=.15, tm=.3, rev=.25),
 p("Tremolo EP", eng=RHO, t2=.5, t3=.5, d=1.5, s=.3, src=0, dst=2, rate=5, amt=.45, cho=.2, rev=.3),
 p("Slow Jam Keys", eng=RHO, t2=.8, t3=.3, d=2.6, s=.5, r=1.4, cho=.35, rev=.5),
 p("Wurly Bite", eng=RHO, t2=.35, t3=.9, d=.9, s=.15, r=.3, dis=.2, rev=.15),
 p("Bell EP", eng=RHO, t2=.6, t3=.95, d=1.4, s=.2, r=.8, pit=12, rev=.4),
 p("Lowrider Keys", eng=RHO, t2=.7, t3=.4, d=1.8, s=.4, r=.8, pit=-12, cho=.3, rev=.35)])
bank("Synth Bass", [
 p("Funk Moog", eng=BAS, t3=.5, t4=.5, t5=.2, cut=500, fen=.7, fdc=.25, rso=.4, d=.3, s=.6, r=.15, mono=1, vol=.8),
 p("Dre Bass", eng=BAS, t3=.4, t4=.7, cut=400, fen=.5, fdc=.35, rso=.3, d=.5, s=.7, r=.2, mono=1, gl=.05, vol=.85),
 p("Rubber Bass", eng=BAS, t3=.6, t4=.4, t5=.3, cut=700, fen=.9, fdc=.12, rso=.55, d=.2, s=.4, r=.1, mono=1, vol=.8),
 p("Sub Cop", eng=BAS, t3=.3, t4=.9, cut=300, fen=.3, fdc=.4, rso=.2, d=.6, s=.8, r=.25, mono=1, vol=.9),
 p("Slap Synth", eng=BAS, t3=.7, t4=.3, t5=.5, cut=900, fen=.8, fdc=.08, rso=.5, d=.15, s=.3, r=.08, mono=1, vol=.8),
 p("G-Funk Whine Bass", eng=BAS, t3=.5, t4=.6, cut=600, fen=.4, fdc=.3, rso=.35, d=.4, s=.7, gl=.12, mono=1, src=0, dst=0, rate=5.5, amt=.08),
 p("Disco Octave", eng=BAS, t3=.55, t4=.5, cut=800, fen=.5, fdc=.15, rso=.3, d=.2, s=.5, r=.1, mono=1, vol=.8),
 p("Acid Funk", eng=BAS, t3=.5, t4=.3, t5=.7, cut=400, fen=.9, fdc=.2, rso=.8, d=.25, s=.4, mono=1, dis=.15, vol=.75),
 p("Bus Driver", eng=BAS, t3=.4, t4=.8, cut=350, fen=.35, fdc=.5, rso=.25, d=.7, s=.8, r=.3, mono=1, pit=-12, vol=.9),
 p("Mean Streets", eng=BAS, t3=.45, t4=.6, t5=.5, cut=500, fen=.6, fdc=.28, rso=.5, d=.35, s=.6, mono=1, dis=.25, vol=.75)])
bank("Brass & Stabs", [
 p("Horn Section", eng=BRA, t3=.6, t4=.5, cut=1200, fen=.7, fdc=.22, rso=.2, a=.03, d=.3, s=.8, r=.25, cho=.2, rev=.25),
 p("Funk Stab", eng=BRA, t3=.7, t4=.4, cut=1000, fen=.9, fdc=.12, rso=.25, a=.01, d=.18, s=0, r=.12, vel=.9, rev=.15),
 p("Police Horns", eng=BRA, t3=.65, t4=.6, cut=1500, fen=.6, fdc=.3, rso=.2, a=.02, d=.35, s=.7, r=.3, rev=.3, dly=.12, tm=.28),
 p("Disco Brass", eng=BRA, t3=.75, t4=.5, cut=1400, fen=.8, fdc=.18, rso=.15, a=.01, d=.25, s=.5, r=.2, cho=.3, rev=.3),
 p("Swell Brass", eng=BRA, t3=.55, t4=.5, cut=900, fen=.8, fdc=1.2, rso=.2, a=.25, d=.8, s=.8, r=.5, rev=.35),
 p("Soft Flugel", eng=BRA, t3=.35, t4=.4, cut=1000, fen=.4, fdc=.4, rso=.1, a=.06, d=.4, s=.7, r=.35, rev=.4),
 p("Big Band Hit", eng=BRA, t3=.7, t4=.7, cut=1300, fen=.8, fdc=.15, rso=.2, a=.01, d=.3, s=.2, r=.25, rev=.35, vol=.65),
 p("Synth Brass 80", eng=BRA, t3=.6, t4=.8, cut=1100, fen=.7, fdc=.25, rso=.3, a=.04, d=.4, s=.8, r=.3, cho=.4, rev=.3),
 p("Sax Pad", eng=BRA, t3=.4, t4=.3, t5=.5, cut=1500, fen=.3, fdc=.5, rso=.15, a=.08, d=.5, s=.8, r=.4, rev=.4),
 p("Trumpet Call", eng=BRA, t3=.8, t4=.2, cut=2000, fen=.9, fdc=.1, rso=.2, a=.015, d=.2, s=.6, r=.2, pit=12, rev=.3)])
bank("Strings & Disco", [
 p("Disco Strings", eng=STR, t3=.55, t4=.7, cut=3500, a=.25, d=.6, s=.9, r=.9, cho=.5, rev=.4),
 p("Philly Strings", eng=STR, t3=.6, t4=.8, cut=4500, a=.3, d=.7, s=.9, r=1, cho=.55, rev=.45),
 p("Cop Show Strings", eng=STR, t3=.5, t4=.6, cut=3000, a=.15, d=.5, s=.85, r=.7, cho=.4, rev=.35, src=0, dst=0, rate=5, amt=.08),
 p("Mirror Ball Pad", eng=STR, t3=.7, t4=.9, cut=5500, a=.6, d=.8, s=1, r=1.6, cho=.6, rev=.6, dly=.2, tm=.4),
 p("Slow Jam Pad", eng=STR, t3=.4, t4=.6, cut=2200, a=.7, s=1, r=1.8, cho=.4, rev=.55),
 p("Staccato Strings", eng=STR, t3=.6, t4=.5, cut=3500, a=.005, d=.2, s=0, r=.15, cho=.2, rev=.2, vel=.8),
 p("Lush Ensemble", eng=STR, t3=.5, t4=1, cut=4000, a=.4, s=1, r=1.4, cho=.7, rev=.5),
 p("String Machine", eng=STR, t3=.45, t4=.7, cut=2800, a=.08, d=.4, s=.9, r=.5, cho=.55, rev=.3),
 p("Velvet Pad", eng=STR, t3=.3, t4=.7, cut=2000, a=.9, s=1, r=2.2, cho=.4, rev=.65),
 p("Closing Credits", eng=STR, t3=.55, t4=.8, cut=3200, a=1.1, s=1, r=2.5, cho=.5, rev=.7, pit=12)])
bank("Leads & Talkbox", [
 p("Funk Lead", eng=LEA, t3=.6, t4=.4, cut=3000, fen=.5, fdc=.3, rso=.3, a=.01, d=.3, s=.8, r=.2, mono=1, gl=.06, rev=.2, dly=.1, tm=.3),
 p("Chrome Lead", eng=LEA, t3=.7, t4=.5, cut=4000, fen=.4, fdc=.4, rso=.25, a=.01, s=.85, mono=1, gl=.08, cho=.25, rev=.25),
 p("G-Funk Whistle", eng=LEA, t3=.4, t4=.3, cut=2500, rso=.2, a=.02, s=.9, mono=1, gl=.1, src=0, dst=0, rate=5.5, amt=.1, rev=.25),
 p("Talkbox Wah", eng=TLK, t1=.5, t2=.5, t3=.5, t5=.3, a=.01, s=.9, mono=1, src=0, dst=3, rate=3, amt=.5, rev=.2, vol=.6),
 p("Robot Cop", eng=TLK, t1=.3, t2=.6, t3=.4, t5=.5, a=.01, s=.9, mono=1, src=3, dst=3, rate=4, amt=.6, dis=.1, rev=.15, vol=.6),
 p("Vocoder Cruise", eng=TLK, t1=.7, t2=.5, t3=.6, t4=.5, t5=.3, a=.05, s=.9, cho=.3, rev=.3, vol=.6),
 p("Squeal Lead", eng=LEA, t3=.8, t4=.3, cut=5000, fen=.6, fdc=.2, rso=.5, a=.005, d=.3, s=.7, mono=1, gl=.04, dis=.15),
 p("Pop Lock Lead", eng=LEA, t3=.5, t4=.5, cut=2500, fen=.8, fdc=.15, rso=.4, a=.005, d=.2, s=.5, mono=1, rev=.15, dly=.18, tm=.22),
 p("Midnight Cruiser", eng=LEA, t3=.45, t4=.6, cut=2000, fen=.3, fdc=.5, rso=.3, a=.04, s=.85, mono=1, gl=.12, cho=.3, rev=.4),
 p("Disco Solo", eng=LEA, t3=.65, t4=.4, cut=3500, fen=.5, fdc=.3, rso=.35, a=.01, s=.8, mono=1, gl=.05, src=0, dst=0, rate=5.8, amt=.15, rev=.3)])
bank("Guitar & FX", [
 p("Chank Guitar", eng=GTR, t2=.35, t3=.7, d=.2, s=0, r=.1, rev=.12, vel=.9),
 p("Funk Strum", eng=GTR, t2=.45, t3=.65, d=.3, s=.1, r=.15, rev=.15, cho=.2, vel=.8),
 p("Wah Guitar", eng=GTR, t2=.5, t3=.6, d=.5, s=.2, r=.2, fen=.6, fdc=.3, rso=.5, cut=700, src=0, dst=1, rate=3, amt=.4),
 p("Clean Pluck", eng=GTR, t2=.55, t3=.5, d=.6, s=.2, r=.3, cho=.2, rev=.25),
 p("Jazz Box", eng=GTR, t2=.6, t3=.3, d=.8, s=.3, r=.4, cut=4000, rev=.3),
 p("Muted Pick", eng=GTR, t2=.25, t3=.55, d=.15, s=0, r=.08, rev=.1),
 p("Siren FX", eng=LEA, t3=.7, t4=.3, cut=5000, a=.1, s=1, r=.6, gl=.3, mono=1, src=2, dst=0, rate=.5, amt=.8, rev=.4),
 p("Radio Dispatch", eng=TLK, t1=.6, t2=.4, t3=.3, t5=.8, a=.02, d=.2, s=.5, mono=1, src=4, dst=3, rate=8, amt=.5, dis=.3, cut=3500, rev=.2, vol=.6),
 p("Disco Ball Bell", eng=RHO, t2=.8, t3=.95, d=1.8, s=0, r=1.5, pit=12, dly=.3, tm=.375, fb=.5, rev=.55, cho=.3),
 p("Laser Zap", eng=LEA, t3=.9, t4=.2, cut=7000, fen=.9, fdc=.12, rso=.6, a=.001, d=.2, s=0, r=.15, gl=.25, mono=1, pit=12, dly=.2, tm=.2)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Fk {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
