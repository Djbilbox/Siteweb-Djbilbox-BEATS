#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets HBX-84 TALKBOX (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.003, d=.4, s=.8, r=.3, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.25, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
OOH, YEA, AY, CLS, TBS, VOC, ROB, WHI = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)
L = dict(mono=1, gl=.08, a=.005, d=.5, s=.9, r=.15, rev=.22, vol=.75)   # a talkbox lead
def lead(**k): d = dict(L); d.update(k); return d

bank("Ooh-Wah Talk", [
 p("Ooh-Wah Lead", **lead(eng=OOH, t1=.5, t2=.5, t3=.5, t4=.3, t5=.1, dly=.2, tm=.375, fb=.3)),
 p("Boulevard Wah", **lead(eng=OOH, gl=.14, t1=.65, t2=.6, t3=.45, t4=.35, t5=.15, cho=.2, rev=.3)),
 p("Slow Mouth Wah", **lead(eng=OOH, t1=.5, t2=.5, t3=.5, t4=.8, t5=.1, rev=.3)),
 p("Quick Wah Funk", **lead(eng=OOH, gl=.04, t1=.7, t2=.4, t3=.6, t4=.05, t5=.1, d=.3, s=.6, r=.1, rev=.15)),
 p("Sunset Ooh", **lead(eng=OOH, t1=.35, t2=.7, t3=.35, t4=.5, t5=.2, cho=.3, rev=.4, dly=.25, tm=.5, fb=.35)),
 p("Wah Chords", eng=OOH, t1=.6, t2=.5, t3=.5, t4=.3, t5=.1, a=.01, d=.6, s=.8, r=.3, cho=.35, rev=.35, vol=.6),
 p("Hollow Wah", **lead(eng=OOH, t1=.1, t2=.5, t3=.45, t4=.3, t5=.1)),
 p("Breathy Wah", **lead(eng=OOH, t1=.5, t2=.5, t3=.5, t4=.35, t5=.8, rev=.3)),
 p("Octave Wah", **lead(eng=OOH, pit=12, t1=.6, t2=.4, t3=.6, t4=.3, t5=.1, dly=.2, tm=.375, fb=.3)),
 p("Echo Wah", **lead(eng=OOH, t1=.55, t2=.55, t3=.5, t4=.3, t5=.1, dly=.4, tm=.375, fb=.5, rev=.35))])
bank("Yeah Talk", [
 p("Yeah Lead", **lead(eng=YEA, t1=.55, t2=.5, t3=.5, t4=.3, t5=.1, dly=.2, tm=.375, fb=.3)),
 p("Yeah Hook", **lead(eng=YEA, gl=.06, t1=.7, t2=.5, t3=.55, t4=.2, t5=.1)),
 p("Slow Yeah", **lead(eng=YEA, t1=.5, t2=.6, t3=.45, t4=.75, t5=.15, rev=.35)),
 p("Yeah Stab", eng=YEA, t1=.7, t2=.4, t3=.6, t4=.08, t5=.1, a=.003, d=.25, s=0, r=.12, rev=.2, vol=.7),
 p("Bright Yeah", **lead(eng=YEA, t1=.8, t2=.4, t3=.8, t4=.3, t5=.1)),
 p("Dark Yeah", **lead(eng=YEA, t1=.3, t2=.7, t3=.2, t4=.35, t5=.1, cut=3500)),
 p("Yeah Chords", eng=YEA, t1=.6, t2=.5, t3=.5, t4=.3, t5=.1, a=.01, d=.6, s=.8, r=.3, cho=.35, rev=.35, vol=.6),
 p("Yeah Echo", **lead(eng=YEA, t1=.55, t2=.5, t3=.5, t4=.3, t5=.1, dly=.4, tm=.375, fb=.5, rev=.35)),
 p("Wide Yeah", **lead(eng=YEA, t1=.6, t2=.5, t3=.5, t4=.4, t5=.15, cho=.5, rev=.35)),
 p("Yeah Glide", **lead(eng=YEA, gl=.25, t1=.6, t2=.5, t3=.5, t4=.35, t5=.1, rev=.3))])
bank("Ay Talk", [
 p("Ay Lead", **lead(eng=AY, t1=.55, t2=.5, t3=.5, t4=.3, t5=.1, dly=.2, tm=.375, fb=.3)),
 p("Sweet Ay", **lead(eng=AY, t1=.4, t2=.6, t3=.45, t4=.45, t5=.15, cho=.25, rev=.35)),
 p("Long Ay", **lead(eng=AY, t1=.5, t2=.5, t3=.5, t4=.85, t5=.1, rev=.35)),
 p("Ay Hook", **lead(eng=AY, gl=.05, t1=.7, t2=.45, t3=.6, t4=.2, t5=.1)),
 p("Ay Glide", **lead(eng=AY, gl=.25, t1=.55, t2=.5, t3=.5, t4=.35, t5=.1, rev=.3)),
 p("Breathy Ay", **lead(eng=AY, t1=.5, t2=.5, t3=.5, t4=.35, t5=.75, rev=.3)),
 p("Ay Chords", eng=AY, t1=.6, t2=.5, t3=.5, t4=.35, t5=.1, a=.01, d=.6, s=.8, r=.3, cho=.35, rev=.35, vol=.6),
 p("Bright Ay", **lead(eng=AY, t1=.8, t2=.4, t3=.8, t4=.3, t5=.1)),
 p("Ay Echo", **lead(eng=AY, t1=.55, t2=.5, t3=.5, t4=.35, t5=.1, dly=.4, tm=.375, fb=.5, rev=.35)),
 p("Low Ay", **lead(eng=AY, pit=-12, t1=.5, t2=.7, t3=.4, t4=.35, t5=.1))])
bank("Classic Talkbox", [
 p("Talkbox Oo", **lead(eng=CLS, t1=.05, t2=.5, t3=.5, t4=.1, t5=.1)),
 p("Talkbox Oh", **lead(eng=CLS, t1=.27, t2=.5, t3=.5, t4=.1, t5=.1)),
 p("Talkbox Ah", **lead(eng=CLS, t1=.5, t2=.5, t3=.5, t4=.1, t5=.1)),
 p("Talkbox Eh", **lead(eng=CLS, t1=.72, t2=.5, t3=.5, t4=.1, t5=.1)),
 p("Talkbox Ee", **lead(eng=CLS, t1=.95, t2=.5, t3=.5, t4=.1, t5=.1)),
 p("Wobble Talk", **lead(eng=CLS, t1=.4, t2=.5, t3=.5, t4=.7, t5=.1, rev=.3)),
 p("Slow Wobble", **lead(eng=CLS, t1=.35, t2=.6, t3=.45, t4=.3, t5=.1, rev=.35)),
 p("Talk Pad", eng=CLS, t1=.4, t2=.6, t3=.4, t4=.5, t5=.2, a=.4, d=.8, s=.9, r=1, cho=.4, rev=.5, vol=.55),
 p("Mod Wheel Talk", **lead(eng=CLS, t1=.0, t2=.5, t3=.5, t4=.05, t5=.1, rev=.25)),
 p("Classic Echo Talk", **lead(eng=CLS, t1=.45, t2=.5, t3=.5, t4=.25, t5=.1, dly=.4, tm=.375, fb=.5, rev=.35))])
bank("Talk Bass", [
 p("Talk Bass", eng=TBS, mono=1, gl=.06, t1=.5, t2=.6, t3=.5, t4=.2, t5=.1, d=.6, s=.7, r=.12, rev=.05, vol=.85),
 p("Oh-Ah Bass", eng=TBS, mono=1, gl=.06, t1=.5, t2=.6, t3=.5, t4=.45, t5=.1, d=.8, s=.8, r=.15, rev=.05, vol=.85),
 p("Slide Talk Bass", eng=TBS, mono=1, gl=.3, t1=.5, t2=.7, t3=.45, t4=.3, t5=.1, d=1, s=.8, r=.2, rev=.05, vol=.85),
 p("Short Talk Bass", eng=TBS, mono=1, gl=.03, t1=.6, t2=.5, t3=.55, t4=.05, t5=.1, d=.25, s=.2, r=.08, rev=.05, vol=.85),
 p("Deep Talk Bass", eng=TBS, mono=1, gl=.06, pit=-12, t1=.4, t2=.9, t3=.35, t4=.25, t5=.05, d=.8, s=.8, r=.15, rev=.05, vol=.9),
 p("Funky Talk Bass", eng=TBS, mono=1, gl=.04, t1=.7, t2=.5, t3=.6, t4=.1, t5=.2, d=.4, s=.5, r=.1, dis=.1, rev=.05, vol=.8),
 p("Breathy Bass", eng=TBS, mono=1, gl=.06, t1=.5, t2=.6, t3=.5, t4=.25, t5=.7, d=.6, s=.7, r=.12, rev=.08, vol=.85),
 p("Bright Talk Bass", eng=TBS, mono=1, gl=.06, t1=.8, t2=.5, t3=.8, t4=.2, t5=.1, d=.6, s=.7, r=.12, rev=.05, vol=.8),
 p("Talk Sub", eng=TBS, mono=1, gl=.08, t1=.2, t2=1, t3=.3, t4=.3, t5=0, d=1, s=.9, r=.2, rev=.03, vol=.9),
 p("Rubber Talk Bass", eng=TBS, mono=1, gl=.12, t1=.6, t2=.6, t3=.5, t4=.15, t5=.1, d=.5, s=.6, r=.1, cho=.2, rev=.05, vol=.85)])
bank("Vocoder Choir & Robot", [
 p("Vocoder Choir", eng=VOC, t1=.5, t2=.6, t3=.5, t4=.4, t5=.1, a=.15, d=.8, s=.9, r=.8, cho=.4, rev=.45, vol=.6),
 p("Oh Choir", eng=VOC, t1=.15, t2=.6, t3=.45, t4=.4, t5=.1, a=.2, d=.8, s=.9, r=.9, cho=.4, rev=.5, vol=.6),
 p("Ah Choir", eng=VOC, t1=.8, t2=.6, t3=.55, t4=.4, t5=.1, a=.2, d=.8, s=.9, r=.9, cho=.4, rev=.5, vol=.6),
 p("Wide Choir", eng=VOC, t1=.5, t2=.6, t3=.5, t4=.9, t5=.15, a=.25, d=.8, s=.9, r=1, cho=.6, rev=.55, vol=.55),
 p("Choir Pad", eng=VOC, t1=.4, t2=.7, t3=.4, t4=.6, t5=.2, a=.8, d=1, s=.9, r=1.6, cho=.5, rev=.65, vol=.55),
 p("Robot Talk", **lead(eng=ROB, t1=.5, t2=.5, t3=.5, t4=.3, t5=.1)),
 p("Fast Robot", **lead(eng=ROB, t1=.6, t2=.5, t3=.55, t4=.8, t5=.1)),
 p("Slow Robot", **lead(eng=ROB, t1=.3, t2=.6, t3=.45, t4=.05, t5=.1, rev=.3)),
 p("Robot Chords", eng=ROB, t1=.5, t2=.5, t3=.5, t4=.4, t5=.1, a=.01, d=.6, s=.8, r=.3, cho=.3, rev=.35, vol=.6),
 p("Robot Echo", **lead(eng=ROB, t1=.5, t2=.5, t3=.5, t4=.35, t5=.1, dly=.4, tm=.375, fb=.5, rev=.35))])
bank("G-Funk Whistles", [
 p("Boulevard Whistle", eng=WHI, mono=1, gl=.12, t1=.3, t2=.6, t3=.5, t4=.5, t5=.2, a=.01, dly=.2, tm=.375, fb=.35, rev=.25, vol=.75),
 p("Whine Lead", eng=WHI, mono=1, gl=.2, t1=.4, t2=.4, t3=.6, t4=.6, t5=.4, a=.01, src=0, dst=0, rate=5.5, amt=.05, rev=.25, vol=.75),
 p("Long Glide Whistle", eng=WHI, mono=1, gl=.3, t1=.3, t2=.7, t3=.4, t4=.5, t5=.1, rev=.3, vol=.75),
 p("Vibrato Whistle", eng=WHI, mono=1, gl=.12, t1=.3, t2=.6, t3=.5, t4=.9, t5=.2, rev=.25, vol=.75),
 p("Soft Whistle", eng=WHI, mono=1, gl=.12, t1=.1, t2=.7, t3=.3, t4=.4, t5=.1, rev=.3, vol=.75),
 p("Bright Whistle", eng=WHI, mono=1, gl=.12, t1=.6, t2=.4, t3=.8, t4=.5, t5=.3, rev=.25, vol=.7),
 p("Echo Whistle", eng=WHI, mono=1, gl=.12, t1=.3, t2=.6, t3=.5, t4=.5, t5=.2, dly=.4, tm=.375, fb=.5, rev=.35, vol=.75),
 p("Octave Whistle", eng=WHI, mono=1, gl=.12, pit=12, t1=.3, t2=.5, t3=.5, t4=.5, t5=.2, rev=.3, vol=.7),
 p("Breathy Whistle", eng=WHI, mono=1, gl=.12, t1=.3, t2=.6, t3=.5, t4=.5, t5=.8, rev=.3, vol=.75),
 p("Sunset Whistle", eng=WHI, mono=1, gl=.18, t1=.35, t2=.65, t3=.45, t4=.55, t5=.25, cho=.25, dly=.25, tm=.5, fb=.35, rev=.35, vol=.75)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Tk {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
