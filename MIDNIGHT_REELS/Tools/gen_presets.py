#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets MIDNIGHT REELS (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.005, d=.5, s=.7, r=.4, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fr=.3, rso=.15, vel=.5,
         rev=.25, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
VIB, EP, TRP, SAX, BAS, ORG, STR, CEL = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Vibes & Mallets", [
 p("Midnight Vibes", eng=VIB, t1=.4, t2=.6, t3=.5, t4=.5, t5=.4, d=1, s=0, r=1, rev=.4),
 p("Smoke Vibes", eng=VIB, t1=.3, t2=.7, t3=.3, t4=.7, t5=.3, d=1.4, s=0, r=1.3, cho=.2, rev=.5),
 p("Hard Mallet Vibes", eng=VIB, t1=.9, t2=.5, t3=.7, t4=.3, t5=.5, d=.8, s=0, r=.6, rev=.3, vel=.8),
 p("Slow Motor", eng=VIB, t1=.4, t2=.7, t3=.4, t4=.9, t5=.1, d=1.6, s=0, r=1.5, rev=.5),
 p("Fast Motor", eng=VIB, t1=.4, t2=.5, t3=.5, t4=.8, t5=.8, d=1, s=0, r=.8, rev=.35),
 p("Rainy Window", eng=VIB, t1=.5, t2=.6, t3=.6, t4=.5, t5=.4, d=1.1, s=0, r=1, pit=12, dly=.3, tm=.35, fb=.5, rev=.55),
 p("Low Vibes", eng=VIB, t1=.3, t2=.6, t3=.3, t4=.5, t5=.3, d=1.2, s=0, r=1, pit=-12, rev=.4),
 p("Dry Vibes", eng=VIB, t1=.5, t2=.2, t3=.5, t4=.2, t5=.4, d=.4, s=0, r=.3, rev=.15),
 p("Ghost Mallets", eng=VIB, t1=.2, t2=.9, t3=.4, t4=.8, t5=.2, d=2, s=0, r=2, cho=.3, rev=.65, vol=.6),
 p("Vibes Delay", eng=VIB, t1=.4, t2=.5, t3=.5, t4=.5, t5=.4, d=1, s=0, r=.8, dly=.4, tm=.375, fb=.55, rev=.35)])
bank("Reel EP", [
 p("Reel Electric Piano", eng=EP, t2=.5, t3=.5, d=1.4, s=.3, r=.6, cho=.3, rev=.3, vel=.7),
 p("Noir Keys", eng=EP, t2=.7, t3=.3, d=2, s=.35, r=1, cho=.3, rev=.45),
 p("Bark EP", eng=EP, t2=.4, t3=.85, d=1.2, s=.25, r=.5, dis=.15, rev=.2, vel=.9),
 p("Lounge EP", eng=EP, t2=.6, t3=.4, d=1.8, s=.4, r=.9, cho=.4, rev=.4),
 p("Tremolo EP", eng=EP, t2=.5, t3=.5, d=1.5, s=.3, src=0, dst=2, rate=5, amt=.45, cho=.2, rev=.3),
 p("Tape EP", eng=EP, t2=.55, t3=.35, d=1.6, s=.3, r=.7, cut=3000, cho=.45, rev=.35),
 p("After Hours", eng=EP, t2=.8, t3=.3, d=2.6, s=.5, r=1.4, cho=.35, rev=.55, pit=-12),
 p("Wurly Bite", eng=EP, t2=.35, t3=.9, d=.9, s=.15, r=.3, dis=.2, rev=.15),
 p("Bell EP", eng=EP, t2=.6, t3=.95, d=1.4, s=.2, r=.8, pit=12, rev=.4),
 p("Detective Chords", eng=EP, t2=.65, t3=.45, d=1.5, s=.3, r=.8, dly=.2, tm=.3, fb=.3, cho=.3, rev=.45)])
bank("Trumpet & Sax", [
 p("Muted Trumpet", eng=TRP, t1=.3, t2=.5, t3=.5, t4=.4, t5=.4, a=.03, d=.3, s=.85, r=.25, rev=.35),
 p("Harmon Mute", eng=TRP, t1=.55, t2=.7, t3=.6, t4=.3, t5=.3, a=.03, d=.3, s=.8, r=.2, rev=.35),
 p("Wah Trumpet", eng=TRP, t1=.4, t2=.6, t3=.5, t4=.4, t5=.3, a=.02, d=.3, s=.8, src=0, dst=3, rate=3, amt=.5, rev=.35),
 p("Lonely Horn", eng=TRP, t1=.25, t2=.4, t3=.4, t4=.5, t5=.5, a=.08, d=.5, s=.85, r=.5, rev=.55, dly=.25, tm=.45, fb=.4),
 p("Trumpet Stab", eng=TRP, t1=.5, t2=.6, t3=.7, t4=.5, t5=.2, a=.01, d=.2, s=0, r=.15, fen=.7, fdc=.15, rev=.3, vel=.9),
 p("Tenor Sax", eng=SAX, t1=.4, t2=.5, t3=.5, t4=.5, t5=.3, a=.04, d=.4, s=.85, r=.3, rev=.35),
 p("Smoky Sax", eng=SAX, t1=.3, t2=.6, t3=.35, t4=.6, t5=.6, a=.06, d=.5, s=.85, r=.4, rev=.5),
 p("Breathy Sax", eng=SAX, t1=.5, t2=.4, t3=.4, t4=.5, t5=.9, a=.08, d=.5, s=.8, r=.4, rev=.45),
 p("Late Night Sax", eng=SAX, t1=.35, t2=.55, t3=.3, t4=.8, t5=.4, a=.07, d=.5, s=.9, r=.5, mono=1, gl=.06, rev=.55, dly=.2, tm=.4),
 p("Growl Sax", eng=SAX, t1=.6, t2=.6, t3=.7, t4=.4, t5=.3, a=.03, d=.4, s=.8, r=.25, dis=.3, mono=1, rev=.3)])
bank("Upright Bass", [
 p("Walking Upright", eng=BAS, t1=.4, t2=.6, t3=.4, t5=.4, d=.8, s=0, r=.3, mono=1, vol=.85, rev=.1),
 p("Slap Upright", eng=BAS, t1=.9, t2=.5, t3=.7, t5=.3, d=.5, s=0, r=.2, mono=1, vol=.8, rev=.1, vel=.8),
 p("Deep Pocket", eng=BAS, t1=.3, t2=.7, t3=.3, t5=.5, d=1, s=0, r=.35, mono=1, pit=-12, vol=.9, rev=.1),
 p("Muted Bass", eng=BAS, t1=.5, t2=.2, t3=.3, t5=.3, d=.35, s=0, r=.15, mono=1, vol=.85, rev=.05),
 p("Jazz Club Bass", eng=BAS, t1=.4, t2=.65, t3=.5, t5=.6, d=1.1, s=0, r=.4, mono=1, rev=.25, vol=.8),
 p("Bowed Cello", eng=STR, t1=.3, t2=.9, t3=.25, t4=.3, t5=.2, a=.25, d=1.5, s=.85, r=.8, mono=1, pit=-12, vol=.8, rev=.3),
 p("Detective Walk", eng=BAS, t1=.5, t2=.5, t3=.45, t5=.4, d=.7, s=0, r=.25, mono=1, gl=.04, vol=.85, rev=.15),
 p("Tape Bass", eng=BAS, t1=.4, t2=.6, t3=.3, t5=.5, d=.9, s=0, r=.3, mono=1, cut=1800, dis=.1, vol=.85, rev=.15),
 p("Fat Pluck", eng=BAS, t1=.6, t2=.8, t3=.5, t5=.8, d=.9, s=0, r=.3, mono=1, vol=.8, rev=.1),
 p("Sub Noir", eng=BAS, t1=.2, t2=.9, t3=.2, t5=.9, d=1.3, s=0, r=.5, mono=1, pit=-12, vol=.9, rev=.1)])
bank("Organ & Lounge", [
 p("Jazz Organ", eng=ORG, t1=.6, t2=.7, t3=.4, t4=.6, t5=.5, a=.005, d=.2, s=.9, r=.12, cho=.3, rev=.3),
 p("Smoky Lounge", eng=ORG, t1=.7, t2=.5, t3=.2, t4=.2, t5=.2, a=.01, d=.3, s=.9, r=.2, cho=.4, rev=.45),
 p("Full Drawbars", eng=ORG, t1=.9, t2=.9, t3=.9, t4=.4, t5=.4, d=.2, s=.9, r=.12, cho=.3, rev=.3, vol=.6),
 p("Gospel Hammond", eng=ORG, t1=.8, t2=.8, t3=.7, t4=.7, t5=.6, d=.2, s=.9, r=.12, cho=.45, dis=.15, rev=.3, vol=.6),
 p("Church Hall", eng=ORG, t1=.6, t2=.5, t3=.3, t4=.1, t5=.1, a=.03, d=.3, s=.95, r=.4, rev=.65),
 p("Perc Organ", eng=ORG, t1=.5, t2=.6, t3=.5, t4=1, t5=.5, d=.2, s=.9, r=.1, cho=.2, rev=.25),
 p("Tape Organ", eng=ORG, t1=.6, t2=.5, t3=.3, t4=.3, t5=.3, d=.3, s=.9, r=.2, cut=2500, cho=.5, rev=.4),
 p("Soft Flute Stop", eng=ORG, t1=.2, t2=.8, t3=.1, t4=.1, t5=.1, a=.02, d=.3, s=.9, r=.25, rev=.4),
 p("Rotary Dream", eng=ORG, t1=.6, t2=.6, t3=.5, t4=.3, t5=.3, d=.3, s=.9, r=.2, src=0, dst=3, rate=6, amt=.3, cho=.6, rev=.4),
 p("Noir Swell", eng=ORG, t1=.7, t2=.6, t3=.3, t4=.1, t5=.1, a=.9, d=.5, s=.95, r=1.2, cho=.3, rev=.6)])
bank("Noir Strings", [
 p("Noir Strings", eng=STR, t1=.4, t2=.5, t3=.4, t4=.5, t5=.3, a=.35, d=.7, s=.9, r=1, cho=.4, rev=.5),
 p("Tape Strings", eng=STR, t1=.5, t2=.5, t3=.3, t4=.8, t5=.7, a=.4, d=.7, s=.9, r=1.1, cut=3500, cho=.3, rev=.45),
 p("Slow Swell", eng=STR, t1=.4, t2=.6, t3=.35, t4=.5, t5=.3, a=1.4, d=1, s=.95, r=1.8, rev=.6),
 p("Dark Cellos", eng=STR, t1=.3, t2=.8, t3=.25, t4=.4, t5=.2, a=.3, d=.7, s=.9, r=.9, pit=-12, rev=.5),
 p("High Violins", eng=STR, t1=.6, t2=.3, t3=.6, t4=.5, t5=.3, a=.4, d=.7, s=.9, r=1, pit=12, cho=.3, rev=.55),
 p("Pizzicato Noir", eng=STR, t1=.5, t2=.4, t3=.5, t4=.2, t5=.1, a=.002, d=.25, s=0, r=.15, rev=.3, vel=.8, vol=.95),
 p("Film Score Pad", eng=STR, t1=.45, t2=.6, t3=.4, t4=.6, t5=.4, a=.8, d=1, s=.9, r=1.6, cho=.4, rev=.65, vol=.65),
 p("Wobble Reel", eng=STR, t1=.4, t2=.5, t3=.35, t4=1, t5=.9, a=.5, d=.8, s=.9, r=1.2, rev=.5),
 p("Suspense", eng=STR, t1=.3, t2=.7, t3=.2, t4=.7, t5=.5, a=1.8, d=1, s=.95, r=2.5, fen=.4, fa=1.5, fdc=2, cut=900, rev=.7, vol=.6),
 p("Rooftop Strings", eng=STR, t1=.5, t2=.5, t3=.5, t4=.4, t5=.3, a=.5, d=.8, s=.9, r=1.4, dly=.2, tm=.45, fb=.4, cho=.3, rev=.55)])
bank("Celesta & Reels", [
 p("Music Box Noir", eng=CEL, t1=.4, t2=.5, t3=.5, t4=.3, t5=.2, d=1, s=0, r=.8, pit=12, rev=.5),
 p("Celesta Dream", eng=CEL, t1=.3, t2=.7, t3=.4, t4=.3, t5=.2, d=1.4, s=0, r=1.2, cho=.2, rev=.55),
 p("Glass Bells", eng=CEL, t1=.5, t2=.8, t3=.9, t4=.3, t5=.2, d=1.6, s=0, r=1.4, pit=12, dly=.3, tm=.4, fb=.5, rev=.6, vol=.6),
 p("Lullaby Reel", eng=CEL, t1=.3, t2=.5, t3=.3, t4=.3, t5=.2, d=1, s=0, r=.8, cut=4000, cho=.3, rev=.5),
 p("Toy Piano", eng=CEL, t1=.8, t2=.3, t3=.7, t4=.3, t5=.2, d=.5, s=0, r=.3, rev=.25, vel=.8),
 p("Haunted Box", eng=CEL, t1=.4, t2=.7, t3=.6, t4=.3, t5=.2, d=1.5, s=0, r=1.2, src=0, dst=0, rate=.8, amt=.15, rev=.65, dly=.3, tm=.5, fb=.55),
 p("Low Celesta", eng=CEL, t1=.4, t2=.6, t3=.4, t4=.3, t5=.2, d=1.2, s=0, r=1, pit=-12, rev=.45),
 p("Projector Click", eng=ORG, t1=.1, t2=.1, t3=.9, t4=0, t5=1, a=.002, d=.05, s=0, r=.05, rev=.2, vol=.5),
 p("Reel To Reel", eng=EP, t2=.8, t3=.9, d=1.4, s=.1, r=.9, pit=12, cho=.5, dly=.35, tm=.3, fb=.6, rev=.55, vol=.55),
 p("Closing Credits", eng=STR, t1=.4, t2=.6, t3=.3, t4=.7, t5=.5, a=1.2, d=1, s=.95, r=2.2, cho=.4, rev=.7, vol=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Mr {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
