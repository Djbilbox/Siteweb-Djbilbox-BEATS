#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets CYBERPUNK LEAD (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.003, d=.4, s=.8, r=.3, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.25, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
NSW, PWM, GFM, RSE, LSR, BRS, CRU, GLS = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Neon Leads", [
 p("Neon Rider", eng=NSW, t1=.3, t2=.5, t3=.6, t4=.4, mono=1, gl=.06, dly=.25, tm=.375, fb=.4, rev=.3),
 p("Chrome Hero", eng=NSW, t1=.4, t2=.6, t3=.7, t4=.5, mono=1, gl=.05, cho=.3, rev=.3, src=0, dst=0, rate=5.5, amt=.06),
 p("Night Drive", eng=NSW, t1=.5, t2=.4, t3=.5, t4=.3, mono=1, gl=.1, dly=.3, tm=.4, fb=.45, rev=.4),
 p("Hacker Lead", eng=NSW, t1=.2, t2=.7, t3=.8, t4=.6, mono=1, dis=.25, rev=.2),
 p("Laser Solo", eng=NSW, t1=.3, t2=.3, t3=.9, t4=.5, mono=1, gl=.15, fen=.5, fdc=.4, cut=3000, src=0, dst=0, rate=6, amt=.1, rev=.3),
 p("Supersaw Wall", eng=NSW, t1=.4, t2=.9, t3=.6, t4=.3, a=.01, cho=.4, rev=.4, vol=.6),
 p("Soft Neon", eng=NSW, t1=.5, t2=.4, t3=.3, t4=.2, mono=1, a=.03, cut=4000, cho=.3, rev=.45),
 p("Blade Lead", eng=NSW, t1=.3, t2=.5, t3=.75, t4=.8, mono=1, gl=.04, dis=.15, dly=.2, tm=.3, rev=.3),
 p("Octave Jump", eng=NSW, t1=.4, t2=.5, t3=.6, t4=1, mono=1, rev=.3),
 p("Wire Lead", eng=NSW, t1=.2, t2=.2, t3=.5, t4=.3, mono=1, gl=.08, src=0, dst=1, rate=4, amt=.3, cut=2500, rso=.4, rev=.25)])
bank("Chrome & Pulse", [
 p("Chrome Pulse", eng=PWM, t1=.5, t2=.4, t3=.5, t4=.4, t5=.2, rev=.3),
 p("Wide PWM", eng=PWM, t1=.8, t2=.6, t3=.5, t4=.3, t5=.2, cho=.4, rev=.3),
 p("Cyber Bass PWM", eng=PWM, t1=.6, t2=.3, t3=.4, t4=.5, t5=.7, mono=1, pit=-12, fen=.5, fdc=.2, cut=800, rso=.3, vol=.8),
 p("Fast Wobble", eng=PWM, t1=.9, t2=.3, t3=.6, t4=.9, t5=.2, mono=1, rev=.2),
 p("Retro Keys", eng=PWM, t1=.4, t2=.3, t3=.4, t4=.2, t5=.1, d=.5, s=.4, r=.3, cho=.3, rev=.3),
 p("Sunset Pad", eng=PWM, t1=.7, t2=.7, t3=.4, t4=.3, t5=.3, a=.5, r=1.2, cho=.5, rev=.55, vol=.6),
 p("Pulse Stab", eng=PWM, t1=.5, t2=.4, t3=.7, t4=.4, t5=.1, a=.002, d=.2, s=0, r=.15, fen=.7, fdc=.15, cut=1500, rev=.2, vel=.8),
 p("Hollow Pulse", eng=PWM, t1=.3, t2=.5, t3=.3, t4=.2, t5=.2, mono=1, cut=2500, rso=.4, rev=.3),
 p("Arcade Lead", eng=PWM, t1=.2, t2=.2, t3=.7, t4=.6, t5=0, mono=1, gl=.03, dly=.2, tm=.2, rev=.15),
 p("Pulse Drone", eng=PWM, t1=.6, t2=.6, t3=.3, t4=.7, t5=.5, a=.8, r=1.5, src=0, dst=1, rate=.3, amt=.4, rev=.5, vol=.6)])
bank("Glitch FM", [
 p("Glitch Bell", eng=GFM, t1=.5, t2=.4, t3=.6, t4=.2, t5=.2, d=1, s=.1, r=.6, rev=.35),
 p("Data Pluck", eng=GFM, t1=.3, t2=.1, t3=.7, t4=.3, t5=.3, a=.002, d=.2, s=0, r=.15, rev=.2),
 p("FM Lead", eng=GFM, t1=.4, t2=.8, t3=.5, t4=.5, t5=.1, mono=1, gl=.05, dly=.25, tm=.3, rev=.3),
 p("Metal Keys", eng=GFM, t1=.7, t2=.3, t3=.8, t4=.1, t5=.4, d=.6, s=.2, r=.4, rev=.3),
 p("Digital Choir", eng=GFM, t1=.25, t2=.9, t3=.3, t4=.2, t5=.1, a=.3, d=1, s=.9, r=1, cho=.4, rev=.5, vol=.6),
 p("Alien Voice", eng=GFM, t1=.85, t2=.7, t3=.6, t4=.8, t5=.2, mono=1, gl=.12, src=0, dst=3, rate=5, amt=.4, rev=.4),
 p("Cyber Marimba", eng=GFM, t1=.2, t2=.1, t3=.4, t4=.1, t5=.5, d=.5, s=0, r=.3, rev=.3),
 p("Circuit Bass", eng=GFM, t1=.1, t2=.2, t3=.6, t4=.4, t5=.2, mono=1, pit=-12, d=.5, s=.4, r=.15, vol=.8),
 p("Scan Lines", eng=GFM, t1=.6, t2=.5, t3=.9, t4=.9, t5=.1, a=.05, rev=.4, dly=.3, tm=.25, fb=.5, vol=.6),
 p("Neural Net", eng=GFM, t1=.45, t2=.6, t3=.7, t4=.6, t5=.3, a=.2, s=.7, r=.8, src=2, dst=3, rate=.5, amt=.6, rev=.5, vol=.6)])
bank("Reese & Bass", [
 p("Reese Core", eng=RSE, t1=.5, t2=.5, t3=.4, t4=.2, mono=1, pit=-12, vol=.8, rev=.1),
 p("Neon Sub", eng=RSE, t1=.8, t2=.3, t3=.3, t4=.1, mono=1, pit=-12, vol=.85, rev=.05),
 p("Dirty Reese", eng=RSE, t1=.4, t2=.6, t3=.5, t4=.7, mono=1, pit=-12, dis=.2, vol=.75, rev=.1),
 p("Wobble Grid", eng=RSE, t1=.5, t2=.5, t3=.4, t4=.4, mono=1, pit=-12, src=0, dst=1, rate=3, amt=.7, rso=.4, cut=500, vol=.8, rev=.1),
 p("Industrial Bass", eng=RSE, t1=.3, t2=.7, t3=.6, t4=.8, mono=1, pit=-12, dis=.3, vol=.7, rev=.05),
 p("Cyber Slide", eng=RSE, t1=.5, t2=.4, t3=.4, t4=.3, mono=1, gl=.25, pit=-12, vol=.8, rev=.1),
 p("Acid Reese", eng=RSE, t1=.4, t2=.4, t3=.5, t4=.5, mono=1, pit=-12, fen=.9, fdc=.2, rso=.7, cut=400, vol=.75, rev=.1),
 p("Subway Bass", eng=RSE, t1=.7, t2=.2, t3=.2, t4=.1, mono=1, pit=-12, vol=.85, rev=.15),
 p("Bass Stab", eng=RSE, t1=.4, t2=.5, t3=.6, t4=.4, d=.2, s=0, r=.1, fen=.8, fdc=.12, cut=600, pit=-12, mono=1, vol=.8),
 p("Meltdown", eng=RSE, t1=.5, t2=.9, t3=.5, t4=1, mono=1, pit=-12, dis=.4, src=0, dst=0, rate=.3, amt=.2, vol=.65)])
bank("Laser Plucks", [
 p("Laser Pluck", eng=LSR, t1=.4, t2=.4, t3=.7, t4=.4, t5=.2, d=.4, s=0, r=.25, dly=.25, tm=.25, fb=.4, rev=.25),
 p("Neon Arp", eng=LSR, t1=.3, t2=.3, t3=.6, t4=.3, t5=.1, d=.25, s=0, r=.15, dly=.3, tm=.1875, fb=.5, rev=.2),
 p("Zap Down", eng=LSR, t1=.9, t2=.3, t3=.8, t4=.2, t5=.3, d=.25, s=0, r=.15, rev=.2),
 p("Chrome Pluck", eng=LSR, t1=.2, t2=.5, t3=.5, t4=.5, t5=.1, d=.5, s=0, r=.3, cho=.3, rev=.3),
 p("Techno Stab", eng=LSR, t1=.1, t2=.2, t3=.8, t4=.3, t5=.2, d=.15, s=0, r=.1, vel=.8, rev=.15),
 p("Soft Pluck", eng=LSR, t1=.2, t2=.6, t3=.3, t4=.4, t5=0, d=.6, s=0, r=.4, rev=.4),
 p("Synth Harp", eng=LSR, t1=.1, t2=.7, t3=.5, t4=.6, t5=.1, d=.9, s=0, r=.6, pit=12, dly=.2, tm=.3, rev=.45, vol=.55),
 p("Cyber Koto", eng=LSR, t1=.3, t2=.5, t3=.6, t4=.1, t5=.5, d=.7, s=0, r=.4, rev=.35),
 p("Sci-Fi Drop", eng=LSR, t1=1, t2=.6, t3=.7, t4=.5, t5=.3, d=1.2, s=0, r=.5, dly=.35, tm=.4, fb=.5, rev=.4),
 p("Pluck Wall", eng=LSR, t1=.2, t2=.4, t3=.6, t4=.9, t5=.2, d=.5, s=0, r=.3, cho=.5, rev=.4, vol=.65)])
bank("Neon Brass & Pads", [
 p("Neon Brass", eng=BRS, t1=.3, t2=.7, t3=.6, t4=.4, t5=.2, a=.02, d=.4, s=.8, r=.3, cho=.3, rev=.3),
 p("Blade Runner Pad", eng=BRS, t1=.8, t2=.6, t3=.4, t4=.6, t5=.2, a=.6, d=1, s=.9, r=1.5, cho=.4, rev=.6, vol=.65),
 p("Synthwave Stab", eng=BRS, t1=.2, t2=.8, t3=.7, t4=.5, t5=.1, a=.005, d=.25, s=.1, r=.2, cho=.3, rev=.3),
 p("Chrome Horns", eng=BRS, t1=.4, t2=.8, t3=.7, t4=.7, t5=.2, a=.03, d=.4, s=.7, r=.3, cho=.2, rev=.3),
 p("Slow Open", eng=BRS, t1=.9, t2=.6, t3=.5, t4=.4, t5=.2, a=.4, d=1, s=.9, r=1.2, rev=.5, vol=.65),
 p("Warm Neon", eng=BRS, t1=.5, t2=.5, t3=.3, t4=.5, t5=.1, a=.15, d=.6, s=.85, r=.8, cho=.4, rev=.45),
 p("Hollow Glass", eng=GLS, t1=.3, t2=.5, t3=.5, t4=.3, t5=.1, d=1.4, s=0, r=1, rev=.5),
 p("Ice Bell", eng=GLS, t1=.6, t2=.6, t3=.7, t4=.5, t5=.2, d=1.6, s=0, r=1.2, pit=12, dly=.3, tm=.4, fb=.5, rev=.55, vol=.6),
 p("Crystal Rain", eng=GLS, t1=.5, t2=.4, t3=.6, t4=.8, t5=.1, d=1, s=0, r=.8, dly=.35, tm=.1875, fb=.55, rev=.6, vol=.6),
 p("Ghost Chime", eng=GLS, t1=.2, t2=.9, t3=.4, t4=.6, t5=.3, d=2.2, s=0, r=2, cho=.3, rev=.7, vol=.55)])
bank("Data Crush & FX", [
 p("Data Crush", eng=CRU, t1=.5, t2=.4, t3=.5, t4=.3, t5=.2, mono=1, rev=.2),
 p("8-Bit Lead", eng=CRU, t1=.8, t2=.2, t3=.6, t4=1, t5=0, mono=1, gl=.03, rev=.15),
 p("Bit Rain", eng=CRU, t1=.4, t2=.3, t3=.5, t4=.3, t5=.3, d=.3, s=0, r=.2, dly=.3, tm=.25, fb=.5, rev=.3),
 p("Glitch Bass", eng=CRU, t1=.3, t2=.5, t3=.3, t4=.5, t5=.2, mono=1, pit=-12, vol=.8, rev=.1),
 p("Corrupted Pad", eng=CRU, t1=.7, t2=.7, t3=.4, t4=.6, t5=.5, a=.5, r=1.2, cho=.4, rev=.55, vol=.6),
 p("Signal Lost", eng=CRU, t1=.1, t2=.1, t3=.5, t4=.4, t5=.8, mono=1, gl=.2, src=4, dst=3, rate=8, amt=.6, rev=.3, vol=.6),
 p("Modem Dial", eng=GFM, t1=.9, t2=.1, t3=.9, t4=.8, t5=.5, mono=1, a=.002, d=.1, s=.3, r=.1, src=4, dst=0, rate=14, amt=.5, rev=.1, vol=.55),
 p("Firewall", eng=CRU, t1=.9, t2=.6, t3=.7, t4=.7, t5=.4, mono=1, dis=.35, vol=.55, rev=.15),
 p("Static Wind", eng=CRU, t1=.2, t2=.2, t3=.3, t4=0, t5=1, a=1, s=1, r=1.5, cut=2500, rso=.4, src=4, dst=1, rate=.5, amt=.6, rev=.6, vol=.5),
 p("Boot Sequence", eng=LSR, t1=.7, t2=.5, t3=.8, t4=.4, t5=.3, d=.4, s=0, r=.3, dly=.4, tm=.125, fb=.65, rev=.3)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Cp {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
