#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets QUEEN OF XMAS (7 banques x 10), noms de Noel sur les reglages accordes de HIGHROLLER GOLD."""
D = dict(eng=0, cut=9000, a=.005, d=.6, s=.8, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.35, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
BEL, EPK, BRS, ORG, MAL, STR, SUB, SMK = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Church Bells & Chimes", [
 p("Midnight Mass Bell", eng=BEL, t1=.4, t2=.5, t3=.6, t4=.3, t5=.1, d=1.3, s=0, r=.9, rev=.45, dly=.25, tm=.375, fb=.4),
 p("Silver Bells", eng=BEL, t1=.7, t2=.3, t3=.7, t4=.4, t5=.3, d=.6, s=0, r=.4, pit=12, rev=.35),
 p("Jingle Chime", eng=BEL, t1=.8, t2=.2, t3=.8, t4=.6, t5=.4, d=.4, s=0, r=.25, pit=12, dly=.2, tm=.125, fb=.5, rev=.25, vol=.6),
 p("Carol Chimes", eng=BEL, t1=.5, t2=.6, t3=.5, t4=.5, t5=.1, d=1.6, s=0, r=1.2, pit=12, rev=.55, vol=.6),
 p("Snow Globe Box", eng=BEL, t1=.6, t2=.3, t3=.5, t4=.3, t5=.1, d=.8, s=0, r=.6, rev=.4),
 p("Ice Chandelier", eng=BEL, t1=.5, t2=.8, t3=.7, t4=.7, t5=.1, d=2.2, s=0, r=1.8, pit=12, cho=.3, rev=.65, vol=.55),
 p("Cathedral Gong", eng=BEL, t1=.3, t2=.9, t3=.4, t4=.3, t5=.3, d=3, s=0, r=2.5, pit=-12, rev=.7),
 p("Ring In The Year", eng=BEL, t1=.55, t2=.5, t3=.6, t4=.5, t5=.2, d=1.2, s=0, r=.8, dly=.35, tm=.25, fb=.55, rev=.5),
 p("Lucky Star Bell", eng=BEL, t1=.35, t2=.4, t3=.55, t4=.3, t5=.2, d=1, s=0, r=.6, src=0, dst=0, rate=6, amt=.05, rev=.4),
 p("Bell Echo Night", eng=BEL, t1=.5, t2=.5, t3=.6, t4=.4, t5=.1, d=1, s=0, r=.8, dly=.45, tm=.5, fb=.6, rev=.5, vol=.55)])
bank("Fireplace Piano", [
 p("Fireplace Keys", eng=EPK, t2=.5, t3=.5, d=1.4, s=.3, r=.6, cho=.3, rev=.35, vel=.7),
 p("Cozy Cabin Keys", eng=EPK, t2=.6, t3=.4, d=1.8, s=.35, r=.8, cho=.35, rev=.45),
 p("Hot Cocoa EP", eng=EPK, t2=.4, t3=.85, d=1.2, s=.25, r=.5, dis=.15, rev=.2, vel=.9),
 p("Lounge Noel", eng=EPK, t2=.7, t3=.35, d=2.2, s=.4, r=1, cho=.4, rev=.5),
 p("Tremolo Lights", eng=EPK, t2=.5, t3=.5, d=1.5, s=.3, src=0, dst=2, rate=5, amt=.45, cho=.2, rev=.3),
 p("After Midnight Keys", eng=EPK, t2=.8, t3=.3, d=2.6, s=.5, r=1.4, cho=.35, rev=.55, pit=-12),
 p("Candy Cane Bite", eng=EPK, t2=.35, t3=.9, d=.9, s=.15, r=.3, dis=.2, rev=.15),
 p("Starlight Bell Keys", eng=EPK, t2=.6, t3=.95, d=1.4, s=.2, r=.8, pit=12, rev=.4),
 p("Holiday Chords", eng=EPK, t2=.65, t3=.45, d=1.5, s=.3, r=.8, dly=.2, tm=.3, fb=.3, cho=.3, rev=.45),
 p("Tape Christmas", eng=EPK, t2=.55, t3=.3, d=1.6, s=.3, r=.7, cut=3000, cho=.45, rev=.35)])
bank("Sleigh Bells", [
 p("Sleigh Ride Jingle", eng=BRS, t1=.3, t2=.7, t3=.6, t4=.5, t5=.2, a=.03, d=.4, s=.8, r=.3, cho=.2, rev=.35),
 p("Lowrider Sleigh", eng=BRS, t1=.2, t2=.8, t3=.8, t4=.6, t5=.1, a=.01, d=.3, s=.6, r=.25, cho=.25, rev=.35),
 p("Jingle Stab", eng=BRS, t1=.2, t2=.8, t3=.7, t4=.5, t5=.1, a=.005, d=.25, s=.1, r=.2, rev=.3, vel=.8),
 p("Soft Shake", eng=BRS, t1=.6, t2=.5, t3=.3, t4=.4, t5=.1, a=.1, d=.6, s=.8, r=.5, rev=.45),
 p("Slow Jingle Swell", eng=BRS, t1=.8, t2=.6, t3=.5, t4=.4, t5=.1, a=.4, d=.8, s=.85, r=.7, rev=.5),
 p("Holiday Fanfare Bells", eng=BRS, t1=.3, t2=.9, t3=.9, t4=.8, t5=.2, a=.02, d=.4, s=.7, r=.3, cho=.3, rev=.5, vol=.6),
 p("Muffled Bells", eng=BRS, t1=.5, t2=.4, t3=.2, t4=.3, t5=.1, a=.05, d=.5, s=.8, r=.3, cut=2500, rev=.35),
 p("Reindeer Run", eng=BRS, t1=.5, t2=.6, t3=.4, t4=.6, t5=.3, a=.06, d=.5, s=.8, r=.4, cho=.3, rev=.4),
 p("Jingle Hit", eng=BRS, t1=.3, t2=.8, t3=.8, t4=.5, t5=.1, a=.003, d=.2, s=0, r=.15, rev=.3),
 p("Midnight Sleigh", eng=BRS, t1=.6, t2=.5, t3=.3, t4=.4, t5=.2, a=.08, d=.6, s=.8, r=.6, mono=1, gl=.05, rev=.5, dly=.25, tm=.4)])
bank("Christmas Organ", [
 p("Christmas Organ", eng=ORG, t1=.6, t2=.7, t3=.4, t4=.6, t5=.5, a=.005, d=.2, s=.9, r=.12, cho=.3, rev=.3),
 p("Chapel Lounge", eng=ORG, t1=.7, t2=.5, t3=.2, t4=.2, t5=.2, a=.01, d=.3, s=.9, r=.2, cho=.4, rev=.45),
 p("Full Choir Stops", eng=ORG, t1=.9, t2=.9, t3=.9, t4=.4, t5=.4, d=.2, s=.9, r=.12, cho=.3, rev=.3, vol=.55),
 p("Gospel Noel", eng=ORG, t1=.8, t2=.8, t3=.7, t4=.7, t5=.6, d=.2, s=.9, r=.12, cho=.45, dis=.15, rev=.3, vol=.55),
 p("Mall Santa Organ", eng=ORG, t1=.5, t2=.6, t3=.7, t4=.9, t5=.6, d=.3, s=.9, r=.2, cho=.4, rev=.4),
 p("Perc Carol", eng=ORG, t1=.5, t2=.6, t3=.5, t4=1, t5=.5, d=.2, s=.9, r=.1, cho=.2, rev=.25),
 p("Silent Night Flute", eng=ORG, t1=.2, t2=.8, t3=.1, t4=.1, t5=.1, a=.02, d=.3, s=.9, r=.25, rev=.4),
 p("Rotary Snow", eng=ORG, t1=.6, t2=.6, t3=.5, t4=.3, t5=.3, d=.3, s=.9, r=.2, src=0, dst=3, rate=6, amt=.3, cho=.6, rev=.4),
 p("Tape Carol", eng=ORG, t1=.6, t2=.5, t3=.3, t4=.3, t5=.3, d=.3, s=.9, r=.2, cut=2500, cho=.5, rev=.4),
 p("Candle Swell", eng=ORG, t1=.7, t2=.6, t3=.3, t4=.1, t5=.1, a=.9, d=.5, s=.95, r=1.2, cho=.3, rev=.6)])
bank("Snow Glock & Winter Strings", [
 p("Snow Glock", eng=MAL, t1=.4, t2=.6, t3=.5, t4=.5, t5=.4, d=1, s=0, r=1, rev=.4),
 p("Nutcracker Mallets", eng=MAL, t1=.5, t2=.2, t3=.4, t4=.1, t5=.3, d=.5, s=0, r=.3, rev=.3),
 p("Ice Chips", eng=MAL, t1=.8, t2=.2, t3=.7, t4=.2, t5=.4, d=.3, s=0, r=.2, pit=12, rev=.25, vel=.8),
 p("Slow Snow Vibes", eng=MAL, t1=.3, t2=.7, t3=.4, t4=.9, t5=.1, d=1.6, s=0, r=1.5, rev=.5),
 p("Elf Workshop", eng=MAL, t1=.5, t2=.4, t3=.6, t4=.4, t5=.5, d=.8, s=0, r=.5, dly=.3, tm=.1875, fb=.5, rev=.35),
 p("Winter Strings", eng=STR, t1=.4, t2=.5, t3=.4, t4=.5, t5=.3, a=.35, d=.7, s=.9, r=1, cho=.4, rev=.5),
 p("Velvet Snow Strings", eng=STR, t1=.5, t2=.5, t3=.3, t4=.8, t5=.7, a=.4, d=.7, s=.9, r=1.1, cut=3500, cho=.3, rev=.45),
 p("Slow Rise Winter", eng=STR, t1=.4, t2=.6, t3=.35, t4=.5, t5=.3, a=1.4, d=1, s=.95, r=1.8, rev=.6),
 p("Cold Night Cellos", eng=STR, t1=.3, t2=.8, t3=.25, t4=.4, t5=.2, a=.3, d=.7, s=.9, r=.9, pit=-12, rev=.5),
 p("Pizz Snowflakes", eng=STR, t1=.5, t2=.4, t3=.5, t4=.2, t5=.1, a=.002, d=.25, s=0, r=.15, rev=.3, vel=.8, vol=.95)])
bank("Santa 808 & Bass", [
 p("Santa 808", eng=SUB, t1=.4, t2=.3, t3=.4, t5=.4, a=.003, d=1.5, s=.8, r=.5, mono=1, vol=.85, rev=.05),
 p("Long Slide Sleigh", eng=SUB, t1=.3, t2=.3, t3=.3, t5=.3, d=2, s=.9, r=.8, mono=1, gl=.35, vol=.85, rev=.05),
 p("Dirty Snow 808", eng=SUB, t1=.5, t2=.5, t3=.5, t5=.9, d=1.5, s=.8, r=.4, mono=1, dis=.25, vol=.7, rev=.05),
 p("Boom Bap Noel", eng=SUB, t1=.2, t2=.2, t3=.2, t5=.2, d=1, s=.7, r=.3, mono=1, vol=.9, rev=.05),
 p("Hard Knock Chimney", eng=SUB, t1=.9, t2=.5, t3=.7, t5=.5, d=.8, s=.5, r=.25, mono=1, vol=.8, rev=.05),
 p("Deep Freeze", eng=SUB, t1=.2, t2=.2, t3=.1, t5=.1, d=2.5, s=.9, r=.8, mono=1, pit=-12, vol=.9, rev=.1),
 p("Short Santa", eng=SUB, t1=.6, t2=.4, t3=.5, t5=.4, d=.35, s=0, r=.15, mono=1, vol=.85, rev=.05),
 p("Smooth Snow Sub", eng=SUB, t1=.1, t2=.1, t3=.1, t5=0, d=1.5, s=.95, r=.4, mono=1, vol=.9, rev=.05),
 p("Pitch Drop Present", eng=SUB, t1=1, t2=.4, t3=.6, t5=.3, d=1.2, s=.6, r=.4, mono=1, gl=.1, vol=.8, rev=.05),
 p("Rumble Reindeer", eng=SUB, t1=.4, t2=.6, t3=.4, t5=1, d=1.6, s=.8, r=.5, mono=1, dis=.4, vol=.6, rev=.1)])
bank("Diva Voices", [
 p("Queen Of Xmas", eng=SMK, t1=.1, t2=.5, t3=.5, t4=.5, t5=.2, a=.12, d=.8, s=.9, r=.9, cho=.25, dly=.2, tm=.375, fb=.3, rev=.5),
 p("Diva Ah", eng=SMK, t1=0, t2=.5, t3=.45, t4=.4, t5=.15, a=.1, d=.8, s=.9, r=.8, rev=.45),
 p("Diva Ooh", eng=SMK, t1=1, t2=.6, t3=.4, t4=.4, t5=.15, a=.15, d=.8, s=.9, r=1, rev=.5),
 p("Whistle Note", eng=SMK, pit=12, t1=.3, t2=.3, t3=.7, t4=.6, t5=.1, a=.08, d=.6, s=.85, r=.8, mono=1, gl=.12, dly=.25, tm=.375, fb=.35, rev=.55),
 p("Gospel Run", eng=SMK, mono=1, gl=.08, t1=.2, t2=.6, t3=.5, t4=.3, t5=.2, a=.04, d=.6, s=.85, r=.5, rev=.4),
 p("Choir Of Angels", eng=SMK, t1=.5, t2=.6, t3=.4, t4=.7, t5=.25, a=.6, d=1, s=.95, r=1.6, cho=.6, rev=.7, vol=.6),
 p("Silent Night Voice", eng=SMK, t1=.7, t2=.7, t3=.3, t4=.3, t5=.1, a=.3, d=1, s=.9, r=1.4, rev=.65),
 p("Breathy Diva", eng=SMK, t1=.3, t2=.4, t3=.6, t4=.5, t5=.8, a=.15, d=.8, s=.9, r=1, rev=.5),
 p("Mistletoe Duet", eng=SMK, t1=.4, t2=.5, t3=.5, t4=.5, t5=.2, a=.12, d=.8, s=.9, r=.9, cho=.5, rev=.5),
 p("Snowfall Echo", eng=SMK, t1=.2, t2=.5, t3=.5, t4=.45, t5=.2, a=.1, d=.8, s=.9, r=.9, dly=.45, tm=.375, fb=.55, rev=.55)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Sr {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
