#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets SAN_ANDREAS_LEAD (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.003, d=.4, s=.8, r=.3, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.25, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
GFK, MOG, SQR, TLK, BAS, RHO, STR, CLV = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("G-Funk Whistles", [
 p("Compton Whistle", eng=GFK, mono=1, gl=.12, t1=.3, t2=.6, t3=.5, t4=.5, t5=.2, a=.01, dly=.2, tm=.375, fb=.35, rev=.25, vol=.75),
 p("Dre Whine", eng=GFK, mono=1, gl=.2, t1=.4, t2=.4, t3=.6, t4=.6, t5=.4, a=.01, src=0, dst=0, rate=5.5, amt=.05, rev=.25, vol=.75),
 p("Long Beach Glide", eng=GFK, mono=1, gl=.3, t1=.3, t2=.7, t3=.4, t4=.5, t5=.1, rev=.3, vol=.75),
 p("Sunset Boulevard", eng=GFK, mono=1, gl=.15, t1=.2, t2=.8, t3=.3, t4=.7, t5=.1, a=.04, cho=.3, rev=.4, vol=.75),
 p("Whistle Pure", eng=GFK, mono=1, gl=.1, t1=0, t2=1, t3=.3, t4=.4, t5=0, rev=.25, vol=.75),
 p("Sharp Whine", eng=GFK, mono=1, gl=.12, t1=.5, t2=.3, t3=.8, t4=.5, t5=.8, dly=.2, tm=.25, fb=.3, rev=.25, vol=.7),
 p("Cruise Control", eng=GFK, mono=1, gl=.25, t1=.3, t2=.5, t3=.4, t4=.9, t5=.2, a=.05, dly=.25, tm=.5, fb=.4, rev=.3, vol=.75),
 p("Hydraulic", eng=GFK, mono=1, gl=.35, pit=-12, t1=.4, t2=.6, t3=.4, t4=.3, t5=.3, rev=.2, vol=.8),
 p("High Rider", eng=GFK, mono=1, gl=.1, pit=12, t1=.3, t2=.5, t3=.5, t4=.5, t5=.2, rev=.3, vol=.65),
 p("Nate Dogg Air", eng=GFK, mono=1, gl=.18, t1=.35, t2=.6, t3=.45, t4=.6, t5=.3, cho=.3, rev=.45, vol=.75)])
bank("Moog Leads", [
 p("Moog Lowrider", eng=MOG, mono=1, gl=.12, t1=.5, t2=.4, t3=.5, t4=.4, t5=.2, fen=.5, fdc=.4, cut=3000, rso=.3, rev=.2, vol=.75),
 p("Fat Funk Lead", eng=MOG, mono=1, gl=.08, t1=.7, t2=.5, t3=.6, t4=.5, t5=.3, rev=.2, vol=.7),
 p("Chronic Lead", eng=MOG, mono=1, gl=.1, t1=.6, t2=.3, t3=.7, t4=.6, t5=.2, dly=.2, tm=.375, fb=.35, rev=.25, vol=.7),
 p("Saw Cruiser", eng=MOG, mono=1, gl=.15, t1=.4, t2=.4, t3=.4, t4=.3, t5=0, cut=2500, rso=.3, rev=.25, vol=.75),
 p("Slide Lead", eng=MOG, mono=1, gl=.3, t1=.5, t2=.5, t3=.5, t4=.5, t5=.1, rev=.25, vol=.75),
 p("Pimp Lead", eng=MOG, mono=1, gl=.06, t1=.8, t2=.5, t3=.7, t4=.7, t5=.4, dis=.15, rev=.2, vol=.65),
 p("Smooth Moog", eng=MOG, mono=1, gl=.1, t1=.3, t2=.6, t3=.3, t4=.4, t5=0, a=.03, cut=2000, cho=.3, rev=.3, vol=.75),
 p("Acid Cali", eng=MOG, mono=1, gl=.05, t1=.5, t2=.3, t3=.6, t4=.3, t5=.2, fen=.8, fdc=.25, cut=900, rso=.6, vol=.65, rev=.15),
 p("Octave Sub Lead", eng=MOG, mono=1, gl=.1, t1=.5, t2=.9, t3=.5, t4=.4, t5=.1, rev=.2, vol=.7),
 p("Wide Moog", eng=MOG, mono=1, gl=.1, t1=.6, t2=.4, t3=.5, t4=1, t5=.2, cho=.4, rev=.3, vol=.65)])
bank("Square Leads", [
 p("Square Cruiser", eng=SQR, mono=1, gl=.1, t1=.3, t2=.3, t3=.5, t4=.3, t5=.2, rev=.2, vol=.7),
 p("PWM Lowrider", eng=SQR, mono=1, gl=.12, t1=.7, t2=.3, t3=.5, t4=.4, t5=.3, cho=.3, rev=.25, vol=.7),
 p("Hollow Square", eng=SQR, mono=1, gl=.08, t1=.1, t2=.2, t3=.4, t4=.2, t5=.1, cut=3500, rev=.2, vol=.75),
 p("Chiptune Cali", eng=SQR, mono=1, gl=.03, t1=0, t2=0, t3=.8, t4=.1, t5=0, rev=.15, vol=.65),
 p("Wide Pulse", eng=SQR, mono=1, gl=.1, t1=.6, t2=.4, t3=.6, t4=1, t5=.5, cho=.4, rev=.3, vol=.6),
 p("Pulse Slide", eng=SQR, mono=1, gl=.3, t1=.5, t2=.4, t3=.5, t4=.5, t5=.2, rev=.25, vol=.7),
 p("Funky Square", eng=SQR, mono=1, gl=.05, t1=.4, t2=.3, t3=.7, t4=.4, t5=.4, fen=.6, fdc=.2, cut=2000, rso=.35, dly=.2, tm=.25, fb=.3, rev=.2, vol=.7),
 p("Soft Pulse Pad", eng=SQR, a=.3, r=1, t1=.6, t2=.4, t3=.3, t4=.8, t5=.3, cho=.5, rev=.5, vol=.6),
 p("Buzz Square", eng=SQR, mono=1, gl=.08, t1=.8, t2=.5, t3=.9, t4=.7, t5=.9, dis=.2, rev=.2, vol=.55),
 p("Dirty South Lead", eng=SQR, mono=1, gl=.1, t1=.3, t2=.6, t3=.6, t4=.3, t5=.2, dis=.3, rev=.2, vol=.65)])
bank("Talkbox", [
 p("Roger Talk", eng=TLK, mono=1, gl=.06, t1=.3, t2=.5, t3=.4, t4=.4, t5=.1, rev=.25, vol=.75),
 p("Talk Ah-Oh", eng=TLK, mono=1, gl=.05, t1=.1, t2=.5, t3=.4, t4=.6, t5=.1, rev=.25, vol=.75),
 p("Talk Ee-Oo", eng=TLK, mono=1, gl=.05, t1=.8, t2=.5, t3=.4, t4=.6, t5=.1, rev=.25, vol=.75),
 p("Wah Mouth", eng=TLK, mono=1, gl=.04, t1=.5, t2=.5, t3=.3, t4=.9, t5=.1, rev=.2, vol=.75),
 p("Robot Funk", eng=TLK, mono=1, gl=.03, t1=.4, t2=.7, t3=.7, t4=.2, t5=.4, dis=.15, rev=.2, vol=.65),
 p("Slow Vowel", eng=TLK, mono=1, gl=.1, t1=.5, t2=.5, t3=.4, t4=.3, t5=.1, src=0, dst=3, rate=.8, amt=.6, rev=.35, vol=.75),
 p("Choir Talk", eng=TLK, a=.15, r=.8, t1=.4, t2=.6, t3=.4, t4=.4, t5=.2, cho=.4, rev=.5, vol=.7),
 p("Whisper Box", eng=TLK, mono=1, t1=.6, t2=.4, t3=.5, t4=.5, t5=.9, rev=.3, vol=.7),
 p("Low Talker", eng=TLK, mono=1, pit=-12, t1=.2, t2=.6, t3=.3, t4=.4, t5=.1, rev=.2, vol=.8),
 p("Zapp Voice", eng=TLK, mono=1, gl=.08, t1=.5, t2=.5, t3=.6, t4=.7, t5=.3, dly=.25, tm=.25, fb=.4, rev=.3, vol=.7)])
bank("Funk Bass", [
 p("Funk Slap", eng=BAS, mono=1, pit=-12, d=.25, s=.4, r=.1, t1=.5, t2=.5, t3=.5, t4=.2, t5=.7, vol=.85, rev=.05),
 p("Sub Cruiser", eng=BAS, mono=1, pit=-12, gl=.1, t1=.2, t2=.9, t3=.2, t4=.4, t5=.2, vol=.9, rev=.05),
 p("G-Funk Bass", eng=BAS, mono=1, pit=-12, gl=.08, t1=.4, t2=.7, t3=.4, t4=.3, t5=.5, vol=.85, rev=.05),
 p("Pop Bass", eng=BAS, mono=1, pit=-12, d=.18, s=.2, r=.08, t1=.6, t2=.4, t3=.6, t4=.1, t5=.9, vol=.85, rev=.05),
 p("Wobble Low", eng=BAS, mono=1, pit=-12, src=0, dst=1, rate=3, amt=.6, t1=.5, t2=.6, t3=.3, t4=.5, t5=.3, vol=.85, rev=.05),
 p("808 Cali", eng=BAS, mono=1, pit=-24, d=1, s=.6, r=.3, t1=.1, t2=1, t3=.1, t4=.6, t5=0, vol=.9, rev=.05),
 p("Moog Bass", eng=BAS, mono=1, pit=-12, t1=.8, t2=.5, t3=.5, t4=.5, t5=.4, vol=.8, rev=.05),
 p("Dirty Funk", eng=BAS, mono=1, pit=-12, t1=.7, t2=.5, t3=.6, t4=.3, t5=.6, dis=.3, vol=.75, rev=.05),
 p("Round Bass", eng=BAS, mono=1, pit=-12, t1=.2, t2=.8, t3=.2, t4=.4, t5=.1, d=.5, s=.5, vol=.9, rev=.1),
 p("Slide Bass", eng=BAS, mono=1, pit=-12, gl=.3, t1=.5, t2=.6, t3=.4, t4=.4, t5=.4, vol=.85, rev=.05)])
bank("West Coast Keys", [
 p("Rhodes Cali", eng=RHO, t1=.3, t2=.5, t3=.4, t4=.4, t5=.3, d=1.2, s=.3, r=.4, cho=.3, rev=.35, vol=.8),
 p("Mellow Rhodes", eng=RHO, t1=.2, t2=.6, t3=.2, t4=.3, t5=.2, d=1.5, s=.4, r=.5, cho=.3, rev=.4, vol=.8),
 p("Bell Rhodes", eng=RHO, t1=.9, t2=.4, t3=.6, t4=.3, t5=.2, d=1, s=.2, r=.4, cho=.2, rev=.35, vol=.8),
 p("Tremolo Keys", eng=RHO, t1=.3, t2=.5, t3=.4, t4=.7, t5=.7, d=1.2, s=.3, r=.4, rev=.35, vol=.8),
 p("Funk Clav", eng=CLV, t1=.5, t2=.3, t3=.4, t4=.3, t5=.3, d=.35, s=0, r=.12, rev=.15, vol=.75),
 p("Clav Stab", eng=CLV, t1=.8, t2=.2, t3=.6, t4=.2, t5=.4, d=.2, s=0, r=.1, vel=.8, rev=.15, vol=.75),
 p("Chrome Pluck", eng=CLV, t1=.3, t2=.6, t3=.5, t4=.5, t5=.2, d=.5, s=0, r=.2, cho=.3, dly=.2, tm=.375, fb=.35, rev=.25, vol=.75),
 p("Wah Clav", eng=CLV, t1=.7, t2=.4, t3=.5, t4=.4, t5=.6, d=.4, s=.1, r=.15, src=0, dst=1, rate=4, amt=.4, rev=.15, vol=.75),
 p("Piano Funk", eng=RHO, t1=.5, t2=.3, t3=.7, t4=.2, t5=0, d=.8, s=.1, r=.3, rev=.3, vol=.8),
 p("Dre Stab", eng=CLV, t1=.4, t2=.5, t3=.6, t4=.6, t5=.2, a=.002, d=.25, s=0, r=.15, dly=.25, tm=.25, fb=.3, rev=.25, vol=.75)])
bank("Lowrider Strings", [
 p("Lowrider Strings", eng=STR, a=.35, r=1, t1=.7, t2=.3, t3=.4, t4=.5, t5=.2, cho=.4, rev=.45, vol=.65),
 p("Sunset Pad", eng=STR, a=.6, r=1.5, t1=.6, t2=.4, t3=.3, t4=.6, t5=.2, cho=.5, rev=.55, vol=.65),
 p("Velvet Strings", eng=STR, a=.25, r=.8, t1=.8, t2=.2, t3=.5, t4=.4, t5=.1, cho=.3, rev=.4, vol=.65),
 p("Smooth Ensemble", eng=STR, a=.3, r=1, t1=.5, t2=.4, t3=.4, t4=.8, t5=.2, cho=.5, rev=.45, vol=.65),
 p("Palm Tree Pad", eng=STR, a=.8, r=2, t1=.4, t2=.5, t3=.3, t4=.7, t5=.3, cho=.4, rev=.6, vol=.6),
 p("Hot Summer", eng=STR, a=.2, r=.8, t1=.9, t2=.3, t3=.6, t4=.4, t5=.2, cho=.3, rev=.4, vol=.6),
 p("Cali Night", eng=STR, a=.5, r=1.8, t1=.3, t2=.6, t3=.2, t4=.6, t5=.3, cho=.4, rev=.6, dly=.3, tm=.5, fb=.4, vol=.6),
 p("Stab Strings", eng=STR, a=.01, d=.3, s=.2, r=.2, t1=.7, t2=.3, t3=.6, t4=.4, t5=.1, cho=.3, rev=.3, vol=.7),
 p("Low Ride Brass", eng=STR, a=.05, d=.5, s=.8, r=.3, t1=.9, t2=.4, t3=.7, t4=.3, t5=.2, fen=.4, fdc=.4, cut=3000, rev=.3, vol=.65),
 p("Fog Pad", eng=STR, a=1, r=2.5, t1=.2, t2=.5, t3=.2, t4=.9, t5=.5, cho=.5, rev=.7, vol=.55)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Sa {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
