#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets CHINTZWAZ (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.01, d=.7, s=.8, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, rso=.15, vel=.5,
         rev=.4, cho=.1, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
SIT, EPK, MEL, FLT, KAL, PAD, BAS, ORG = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Sitar & Strings", [
 p("Sitar Bloom", eng=SIT, t1=.5, t2=.6, t3=.6, t4=.5, t5=.4, d=1.3, s=0, r=.8, rev=.45),
 p("Raga Morning", eng=SIT, t1=.4, t2=.7, t3=.5, t4=.6, t5=.5, d=1.6, s=0, r=1, dly=.3, tm=.375, fb=.45, rev=.5),
 p("Patchouli Pluck", eng=SIT, t1=.6, t2=.4, t3=.7, t4=.4, t5=.3, d=.8, s=0, r=.4, cho=.2, rev=.4),
 p("Jawari Buzz", eng=SIT, t1=.5, t2=.5, t3=.6, t4=.9, t5=.6, d=1.2, s=0, r=.6, rev=.4, vol=.6),
 p("Marigold Drone", eng=SIT, t1=.3, t2=.9, t3=.4, t4=.7, t5=.4, d=2.4, s=0, r=1.5, pit=-12, rev=.6),
 p("Soft Tamboura", eng=SIT, t1=.3, t2=.8, t3=.3, t4=.5, t5=.2, d=2, s=0, r=1.2, cut=5000, rev=.55),
 p("Hippie Strum", eng=SIT, t1=.7, t2=.4, t3=.8, t4=.5, t5=.3, d=.5, s=0, r=.3, cho=.3, rev=.35),
 p("Bazaar Echo", eng=SIT, t1=.5, t2=.5, t3=.6, t4=.6, t5=.4, d=1, s=0, r=.6, dly=.45, tm=.375, fb=.55, rev=.5),
 p("Low Sitar", eng=SIT, t1=.4, t2=.6, t3=.4, t4=.6, t5=.4, d=1.3, s=0, r=.7, pit=-12, rev=.4),
 p("Glass Sitar", eng=SIT, t1=.7, t2=.6, t3=.9, t4=.4, t5=.5, d=1.5, s=0, r=1, pit=12, cho=.3, rev=.6, vol=.6)])
bank("Flower EP", [
 p("Flower Power EP", eng=EPK, t2=.5, t3=.5, d=1.4, s=.3, r=.6, cho=.4, rev=.4, vel=.7),
 p("Velvet Daisy", eng=EPK, t2=.7, t3=.35, d=2, s=.35, r=1, cho=.4, rev=.5),
 p("Bark Petal", eng=EPK, t2=.4, t3=.85, d=1.2, s=.25, r=.5, dis=.15, rev=.25, vel=.9),
 p("Summer Haze Keys", eng=EPK, t2=.6, t3=.4, d=1.8, s=.4, r=.9, cho=.5, rev=.5),
 p("Tremolo Bloom", eng=EPK, t2=.5, t3=.5, d=1.5, s=.3, src=0, dst=2, rate=5, amt=.45, cho=.25, rev=.35),
 p("Tape Wurly", eng=EPK, t2=.5, t3=.35, d=1.6, s=.3, r=.7, cut=3000, cho=.45, rev=.4),
 p("Bell Blossom", eng=EPK, t2=.6, t3=.95, d=1.4, s=.2, r=.8, pit=12, rev=.45),
 p("Low Tide EP", eng=EPK, t2=.8, t3=.3, d=2.6, s=.5, r=1.4, cho=.35, rev=.55, pit=-12),
 p("Lazy Sunday", eng=EPK, t2=.65, t3=.45, d=1.5, s=.3, r=.8, dly=.25, tm=.375, fb=.35, cho=.4, rev=.5),
 p("Wah Garden", eng=EPK, t2=.45, t3=.6, d=1, s=.4, r=.4, fen=.5, fdc=.35, cut=1800, rso=.4, src=0, dst=1, rate=3, amt=.3, rev=.3)])
bank("Mellotron & Choir", [
 p("Chintz Mellotron", eng=MEL, t1=.4, t2=.5, t3=.4, t4=.5, t5=.4, a=.08, d=.8, s=.85, r=.8, cho=.3, rev=.5),
 p("Strawberry Tape", eng=MEL, t1=.5, t2=.5, t3=.35, t4=.7, t5=.7, a=.1, d=1, s=.9, r=1, cut=4000, cho=.3, rev=.5),
 p("Soft Choir Tape", eng=MEL, t1=.3, t2=.7, t3=.3, t4=.5, t5=.3, a=.3, d=1, s=.9, r=1.2, rev=.6),
 p("Flute Mellotron", eng=MEL, t1=.6, t2=.4, t3=.5, t4=.4, t5=.5, a=.06, d=.7, s=.85, r=.7, cho=.25, rev=.45),
 p("Violin Tape", eng=MEL, t1=.5, t2=.6, t3=.5, t4=.8, t5=.4, a=.15, d=.8, s=.9, r=.9, mono=1, gl=.04, rev=.5),
 p("Slow Wobble", eng=MEL, t1=.4, t2=.5, t3=.3, t4=1, t5=.9, a=.4, d=1, s=.9, r=1.2, rev=.55),
 p("Dream Tape Pad", eng=MEL, t1=.4, t2=.6, t3=.3, t4=.6, t5=.5, a=.8, d=1.2, s=.9, r=1.6, cho=.4, rev=.7, vol=.65),
 p("Low Choir Tape", eng=MEL, t1=.3, t2=.8, t3=.25, t4=.5, t5=.3, a=.3, d=.8, s=.9, r=1, pit=-12, rev=.55),
 p("Eight Track", eng=MEL, t1=.5, t2=.4, t3=.4, t4=.6, t5=.6, a=.02, d=.5, s=.7, r=.4, cut=3000, rev=.35),
 p("Kaleidoscope", eng=MEL, t1=.6, t2=.5, t3=.6, t4=.8, t5=.5, a=.4, d=1, s=.85, r=1.4, src=0, dst=3, rate=.4, amt=.5, cho=.4, rev=.65, dly=.3, tm=.45)])
bank("Hippie Flutes", [
 p("Hippie Flute", eng=FLT, t1=.4, t2=.5, t3=.3, t4=.5, t5=.4, a=.06, d=.4, s=.85, r=.4, rev=.5),
 p("Meadow Pipe", eng=FLT, t1=.5, t2=.6, t3=.4, t4=.3, t5=.7, a=.05, d=.4, s=.85, r=.4, rev=.45),
 p("Breathy Dawn", eng=FLT, t1=.6, t2=.4, t3=.35, t4=.4, t5=.9, a=.1, d=.5, s=.8, r=.5, mono=1, rev=.5),
 p("Sad Daisy", eng=FLT, t1=.45, t2=.5, t3=.3, t4=.8, t5=.4, a=.12, d=.5, s=.9, r=.7, mono=1, rev=.55, cho=.2),
 p("High Piccolo", eng=FLT, t1=.5, t2=.5, t3=.2, t4=.5, t5=.3, a=.05, d=.4, s=.85, r=.4, pit=12, rev=.45),
 p("Flute Echo", eng=FLT, t1=.5, t2=.5, t3=.4, t4=.5, t5=.4, a=.06, mono=1, dly=.45, tm=.375, fb=.6, rev=.5),
 p("Wind In Petals", eng=FLT, t1=.7, t2=.7, t3=.5, t4=.6, t5=1, a=.15, mono=1, gl=.15, rev=.5, vol=.55),
 p("Low Flute", eng=FLT, t1=.4, t2=.6, t3=.4, t4=.5, t5=.4, a=.08, d=.5, s=.85, r=.5, pit=-12, rev=.45),
 p("Fog Flute", eng=FLT, t1=.3, t2=.4, t3=.2, t4=.6, t5=.8, a=.4, d=1, s=.9, r=1.2, rev=.7, vol=.55),
 p("Whistling Gardener", eng=FLT, t1=.5, t2=.5, t3=.5, t4=.9, t5=.3, a=.2, s=1, r=.9, mono=1, gl=.3, src=2, dst=0, rate=.3, amt=.7, rev=.5)])
bank("Kalimba & Bells", [
 p("Daisy Kalimba", eng=KAL, t1=.5, t2=.5, t3=.5, t4=.3, t5=.4, d=1, s=0, r=.5, rev=.45),
 p("Music Box Garden", eng=KAL, t1=.6, t2=.4, t3=.6, t4=.3, t5=.2, d=.8, s=0, r=.4, pit=12, rev=.5),
 p("Thumb Piano", eng=KAL, t1=.4, t2=.3, t3=.4, t4=.3, t5=.6, d=.5, s=0, r=.3, rev=.35),
 p("Wind Chimes", eng=KAL, t1=.8, t2=.6, t3=.7, t4=.5, t5=.2, d=1.4, s=0, r=1, pit=12, dly=.35, tm=.25, fb=.55, rev=.65, vol=.55),
 p("Glass Petals", eng=KAL, t1=.6, t2=.8, t3=.8, t4=.5, t5=.2, d=2, s=0, r=1.6, pit=12, cho=.3, rev=.65, vol=.55),
 p("Low Kalimba", eng=KAL, t1=.4, t2=.6, t3=.4, t4=.3, t5=.4, d=1.3, s=0, r=.7, pit=-12, rev=.5),
 p("Echo Garden", eng=KAL, t1=.5, t2=.5, t3=.5, t4=.3, t5=.3, d=.9, s=0, r=.5, dly=.4, tm=.1875, fb=.55, rev=.45),
 p("Rain On Leaves", eng=KAL, t1=.7, t2=.3, t3=.6, t4=.3, t5=.5, d=.4, s=0, r=.3, dly=.35, tm=.125, fb=.5, rev=.5),
 p("Teacup Bell", eng=KAL, t1=.5, t2=.7, t3=.9, t4=.3, t5=.3, d=1.6, s=0, r=1.2, pit=12, rev=.6),
 p("Marimba Blush", eng=KAL, t1=.3, t2=.5, t3=.3, t4=.3, t5=.3, d=.7, s=0, r=.4, cut=4000, rev=.5)])
bank("Daisy Pads", [
 p("Daisy Pad", eng=PAD, t2=.4, t3=.4, t4=.5, t5=.2, a=1, d=1, s=.9, r=2, cho=.5, rev=.65),
 p("Petal Mist", eng=PAD, t2=.6, t3=.2, t4=.3, t5=.5, a=1.8, d=1, s=.95, r=2.5, rev=.75, pit=-12),
 p("Sundress Strings", eng=PAD, t2=.35, t3=.65, t4=.5, t5=.1, a=.4, d=.8, s=.9, r=1, cho=.5, rev=.5),
 p("Slow Bloom", eng=PAD, t2=.5, t3=.4, t4=.5, t5=.2, a=2.5, d=1.5, s=.9, r=3, fen=.6, fa=2.2, fdc=1.5, cut=700, rso=.3, rev=.7),
 p("Lavender Haze", eng=PAD, t2=.3, t3=.7, t4=.7, t5=.3, a=.8, d=.8, s=.85, r=1.5, pit=12, cho=.5, rev=.6),
 p("Warm Chintz", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.1, a=.6, d=.9, s=.9, r=1.2, cho=.4, rev=.5),
 p("Breeze Drift", eng=PAD, t2=.35, t3=.6, t4=.6, t5=.3, a=.8, d=.8, s=.9, r=1.5, src=0, dst=1, rate=.2, amt=.4, cho=.4, rev=.6),
 p("Whisper Field", eng=PAD, t2=.2, t3=.5, t4=.6, t5=.9, a=1.5, d=1, s=.9, r=2.5, cut=2200, src=0, dst=1, rate=.15, amt=.5, rev=.65, vol=.6),
 p("Pulse Of Spring", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.2, a=.8, d=.9, s=.9, r=1.5, src=0, dst=2, rate=1.1, amt=.5, rev=.6),
 p("Twilight Garden", eng=PAD, t2=.4, t3=.5, t4=.9, t5=.4, a=1.4, d=1, s=.9, r=2.2, cho=.6, rev=.7, vol=.6)])
bank("Bloom Bass & Organs", [
 p("Bloom Bass", eng=BAS, t1=.3, t2=.5, t3=.4, t4=.4, t5=.3, a=.005, d=.8, s=.7, r=.2, mono=1, vol=.8, rev=.1, cho=0),
 p("Round Sub", eng=BAS, t1=.1, t2=.7, t3=.2, t4=.3, t5=.1, d=1, s=.9, r=.3, mono=1, vol=.85, rev=.1, cho=0),
 p("Fuzz Petal", eng=BAS, t1=.6, t2=.4, t3=.6, t4=.4, t5=.8, d=.6, s=.6, r=.2, mono=1, dis=.2, vol=.7, rev=.1, cho=0),
 p("Slide Stem", eng=BAS, t1=.3, t2=.5, t3=.4, t4=.4, t5=.3, d=.8, s=.8, r=.3, mono=1, gl=.25, vol=.8, rev=.1, cho=0),
 p("Pluck Root", eng=BAS, t1=.5, t2=.4, t3=.5, t4=.2, t5=.3, d=.3, s=.2, r=.1, mono=1, vol=.85, rev=.1, cho=0),
 p("Psyche Organ", eng=ORG, t1=.6, t2=.7, t3=.4, t4=.6, t5=.5, a=.005, d=.2, s=.9, r=.12, cho=.35, rev=.4),
 p("Vox Garden", eng=ORG, t1=.5, t2=.6, t3=.7, t4=.7, t5=.4, a=.005, d=.2, s=.9, r=.1, mono=1, gl=.03, cho=.3, rev=.35),
 p("Soft Flute Stop", eng=ORG, t1=.2, t2=.8, t3=.1, t4=.2, t5=.1, a=.02, d=.3, s=.9, r=.25, rev=.45),
 p("Rotary Daisy", eng=ORG, t1=.6, t2=.6, t3=.5, t4=.5, t5=.3, d=.3, s=.9, r=.2, src=0, dst=3, rate=6, amt=.3, cho=.6, rev=.4),
 p("Golden Haze Organ", eng=ORG, t1=.7, t2=.6, t3=.3, t4=.3, t5=.1, a=.8, d=.5, s=.95, r=1.2, cho=.3, rev=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Cz {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
