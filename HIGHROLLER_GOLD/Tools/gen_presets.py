#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets HIGHROLLER GOLD (7 banques x 10)."""
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

bank("Gold Bells & Jackpot", [
 p("Jackpot Bell", eng=BEL, t1=.4, t2=.5, t3=.6, t4=.3, t5=.1, d=1.3, s=0, r=.9, rev=.45, dly=.25, tm=.375, fb=.4),
 p("Gold Coin", eng=BEL, t1=.7, t2=.3, t3=.7, t4=.4, t5=.3, d=.6, s=0, r=.4, pit=12, rev=.35),
 p("Slot Machine", eng=BEL, t1=.8, t2=.2, t3=.8, t4=.6, t5=.4, d=.4, s=0, r=.25, pit=12, dly=.2, tm=.125, fb=.5, rev=.25, vol=.6),
 p("Casino Chimes", eng=BEL, t1=.5, t2=.6, t3=.5, t4=.5, t5=.1, d=1.6, s=0, r=1.2, pit=12, rev=.55, vol=.6),
 p("Velvet Box", eng=BEL, t1=.6, t2=.3, t3=.5, t4=.3, t5=.1, d=.8, s=0, r=.6, rev=.4),
 p("Crystal Chandelier", eng=BEL, t1=.5, t2=.8, t3=.7, t4=.7, t5=.1, d=2.2, s=0, r=1.8, pit=12, cho=.3, rev=.65, vol=.55),
 p("Low Gong", eng=BEL, t1=.3, t2=.9, t3=.4, t4=.3, t5=.3, d=3, s=0, r=2.5, pit=-12, rev=.7),
 p("Ring Of Fortune", eng=BEL, t1=.55, t2=.5, t3=.6, t4=.5, t5=.2, d=1.2, s=0, r=.8, dly=.35, tm=.25, fb=.55, rev=.5),
 p("Lucky Seven", eng=BEL, t1=.35, t2=.4, t3=.55, t4=.3, t5=.2, d=1, s=0, r=.6, src=0, dst=0, rate=6, amt=.05, rev=.4),
 p("Winner Echo", eng=BEL, t1=.5, t2=.5, t3=.6, t4=.4, t5=.1, d=1, s=0, r=.8, dly=.45, tm=.5, fb=.6, rev=.5, vol=.55)])
bank("Velvet Keys", [
 p("Velvet Rhodes", eng=EPK, t2=.5, t3=.5, d=1.4, s=.3, r=.6, cho=.3, rev=.35, vel=.7),
 p("High Stakes Keys", eng=EPK, t2=.6, t3=.4, d=1.8, s=.35, r=.8, cho=.35, rev=.45),
 p("Bark EP", eng=EPK, t2=.4, t3=.85, d=1.2, s=.25, r=.5, dis=.15, rev=.2, vel=.9),
 p("Lounge Keys", eng=EPK, t2=.7, t3=.35, d=2.2, s=.4, r=1, cho=.4, rev=.5),
 p("Tremolo Velvet", eng=EPK, t2=.5, t3=.5, d=1.5, s=.3, src=0, dst=2, rate=5, amt=.45, cho=.2, rev=.3),
 p("After Dark EP", eng=EPK, t2=.8, t3=.3, d=2.6, s=.5, r=1.4, cho=.35, rev=.55, pit=-12),
 p("Wurly Bite", eng=EPK, t2=.35, t3=.9, d=.9, s=.15, r=.3, dis=.2, rev=.15),
 p("Gilded Bell EP", eng=EPK, t2=.6, t3=.95, d=1.4, s=.2, r=.8, pit=12, rev=.4),
 p("Penthouse Chords", eng=EPK, t2=.65, t3=.45, d=1.5, s=.3, r=.8, dly=.2, tm=.3, fb=.3, cho=.3, rev=.45),
 p("Tape Velvet", eng=EPK, t2=.55, t3=.3, d=1.6, s=.3, r=.7, cut=3000, cho=.45, rev=.35)])
bank("Big Band Brass", [
 p("Big Band Brass", eng=BRS, t1=.3, t2=.7, t3=.6, t4=.5, t5=.2, a=.03, d=.4, s=.8, r=.3, cho=.2, rev=.35),
 p("High Roller Horns", eng=BRS, t1=.2, t2=.8, t3=.8, t4=.6, t5=.1, a=.01, d=.3, s=.6, r=.25, cho=.25, rev=.35),
 p("Brass Stab", eng=BRS, t1=.2, t2=.8, t3=.7, t4=.5, t5=.1, a=.005, d=.25, s=.1, r=.2, rev=.3, vel=.8),
 p("Soft Flugel", eng=BRS, t1=.6, t2=.5, t3=.3, t4=.4, t5=.1, a=.1, d=.6, s=.8, r=.5, rev=.45),
 p("Slow Swell", eng=BRS, t1=.8, t2=.6, t3=.5, t4=.4, t5=.1, a=.4, d=.8, s=.85, r=.7, rev=.5),
 p("Vegas Fanfare", eng=BRS, t1=.3, t2=.9, t3=.9, t4=.8, t5=.2, a=.02, d=.4, s=.7, r=.3, cho=.3, rev=.5, vol=.6),
 p("Muted Horn", eng=BRS, t1=.5, t2=.4, t3=.2, t4=.3, t5=.1, a=.05, d=.5, s=.8, r=.3, cut=2500, rev=.35),
 p("Sax Section", eng=BRS, t1=.5, t2=.6, t3=.4, t4=.6, t5=.3, a=.06, d=.5, s=.8, r=.4, cho=.3, rev=.4),
 p("Disco Hit", eng=BRS, t1=.3, t2=.8, t3=.8, t4=.5, t5=.1, a=.003, d=.2, s=0, r=.15, rev=.3),
 p("Cigar Lounge Horn", eng=BRS, t1=.6, t2=.5, t3=.3, t4=.4, t5=.2, a=.08, d=.6, s=.8, r=.6, mono=1, gl=.05, rev=.5, dly=.25, tm=.4)])
bank("Vegas Organ", [
 p("Vegas Organ", eng=ORG, t1=.6, t2=.7, t3=.4, t4=.6, t5=.5, a=.005, d=.2, s=.9, r=.12, cho=.3, rev=.3),
 p("Smoky Lounge", eng=ORG, t1=.7, t2=.5, t3=.2, t4=.2, t5=.2, a=.01, d=.3, s=.9, r=.2, cho=.4, rev=.45),
 p("Full Drawbars", eng=ORG, t1=.9, t2=.9, t3=.9, t4=.4, t5=.4, d=.2, s=.9, r=.12, cho=.3, rev=.3, vol=.55),
 p("Gospel Gold", eng=ORG, t1=.8, t2=.8, t3=.7, t4=.7, t5=.6, d=.2, s=.9, r=.12, cho=.45, dis=.15, rev=.3, vol=.55),
 p("Casino Floor", eng=ORG, t1=.5, t2=.6, t3=.7, t4=.9, t5=.6, d=.3, s=.9, r=.2, cho=.4, rev=.4),
 p("Perc Organ", eng=ORG, t1=.5, t2=.6, t3=.5, t4=1, t5=.5, d=.2, s=.9, r=.1, cho=.2, rev=.25),
 p("Soft Flute Stop", eng=ORG, t1=.2, t2=.8, t3=.1, t4=.1, t5=.1, a=.02, d=.3, s=.9, r=.25, rev=.4),
 p("Rotary Dream", eng=ORG, t1=.6, t2=.6, t3=.5, t4=.3, t5=.3, d=.3, s=.9, r=.2, src=0, dst=3, rate=6, amt=.3, cho=.6, rev=.4),
 p("Tape Organ", eng=ORG, t1=.6, t2=.5, t3=.3, t4=.3, t5=.3, d=.3, s=.9, r=.2, cut=2500, cho=.5, rev=.4),
 p("Golden Swell", eng=ORG, t1=.7, t2=.6, t3=.3, t4=.1, t5=.1, a=.9, d=.5, s=.95, r=1.2, cho=.3, rev=.6)])
bank("Mallets & Strings", [
 p("Roulette Vibes", eng=MAL, t1=.4, t2=.6, t3=.5, t4=.5, t5=.4, d=1, s=0, r=1, rev=.4),
 p("Gold Marimba", eng=MAL, t1=.5, t2=.2, t3=.4, t4=.1, t5=.3, d=.5, s=0, r=.3, rev=.3),
 p("Chip Stack", eng=MAL, t1=.8, t2=.2, t3=.7, t4=.2, t5=.4, d=.3, s=0, r=.2, pit=12, rev=.25, vel=.8),
 p("Slow Motor Vibes", eng=MAL, t1=.3, t2=.7, t3=.4, t4=.9, t5=.1, d=1.6, s=0, r=1.5, rev=.5),
 p("Dealer's Hand", eng=MAL, t1=.5, t2=.4, t3=.6, t4=.4, t5=.5, d=.8, s=0, r=.5, dly=.3, tm=.1875, fb=.5, rev=.35),
 p("Silk Strings", eng=STR, t1=.4, t2=.5, t3=.4, t4=.5, t5=.3, a=.35, d=.7, s=.9, r=1, cho=.4, rev=.5),
 p("Velvet Strings", eng=STR, t1=.5, t2=.5, t3=.3, t4=.8, t5=.7, a=.4, d=.7, s=.9, r=1.1, cut=3500, cho=.3, rev=.45),
 p("Slow Rise Strings", eng=STR, t1=.4, t2=.6, t3=.35, t4=.5, t5=.3, a=1.4, d=1, s=.95, r=1.8, rev=.6),
 p("Dark Cellos", eng=STR, t1=.3, t2=.8, t3=.25, t4=.4, t5=.2, a=.3, d=.7, s=.9, r=.9, pit=-12, rev=.5),
 p("Pizz Casino", eng=STR, t1=.5, t2=.4, t3=.5, t4=.2, t5=.1, a=.002, d=.25, s=0, r=.15, rev=.3, vel=.8, vol=.95)])
bank("Gold Sub & Bass", [
 p("Gold 808", eng=SUB, t1=.4, t2=.3, t3=.4, t5=.4, a=.003, d=1.5, s=.8, r=.5, mono=1, vol=.85, rev=.05),
 p("Long Slide Gold", eng=SUB, t1=.3, t2=.3, t3=.3, t5=.3, d=2, s=.9, r=.8, mono=1, gl=.35, vol=.85, rev=.05),
 p("Dirty Gold", eng=SUB, t1=.5, t2=.5, t3=.5, t5=.9, d=1.5, s=.8, r=.4, mono=1, dis=.25, vol=.7, rev=.05),
 p("Boom Bap Sub", eng=SUB, t1=.2, t2=.2, t3=.2, t5=.2, d=1, s=.7, r=.3, mono=1, vol=.9, rev=.05),
 p("Hard Knock", eng=SUB, t1=.9, t2=.5, t3=.7, t5=.5, d=.8, s=.5, r=.25, mono=1, vol=.8, rev=.05),
 p("Deep Vault", eng=SUB, t1=.2, t2=.2, t3=.1, t5=.1, d=2.5, s=.9, r=.8, mono=1, pit=-12, vol=.9, rev=.1),
 p("Short Gold", eng=SUB, t1=.6, t2=.4, t3=.5, t5=.4, d=.35, s=0, r=.15, mono=1, vol=.85, rev=.05),
 p("Smooth Sub", eng=SUB, t1=.1, t2=.1, t3=.1, t5=0, d=1.5, s=.95, r=.4, mono=1, vol=.9, rev=.05),
 p("Pitch Drop Gold", eng=SUB, t1=1, t2=.4, t3=.6, t5=.3, d=1.2, s=.6, r=.4, mono=1, gl=.1, vol=.8, rev=.05),
 p("Rumble Royale", eng=SUB, t1=.4, t2=.6, t3=.4, t5=1, d=1.6, s=.8, r=.5, mono=1, dis=.4, vol=.6, rev=.1)])
bank("Cigar Smoke Pads", [
 p("Cigar Smoke", eng=SMK, t2=.4, t3=.4, t4=.5, t5=.2, a=1.2, d=1, s=.9, r=2, cho=.4, rev=.65),
 p("Smoky Backroom", eng=SMK, t2=.6, t3=.2, t4=.3, t5=.5, a=1.8, d=1, s=.95, r=2.5, rev=.75, pit=-12),
 p("Velvet Curtain", eng=SMK, t2=.5, t3=.5, t4=.6, t5=.2, a=1, d=.9, s=.9, r=1.8, cho=.5, rev=.6),
 p("Golden Haze", eng=SMK, t2=.3, t3=.7, t4=.7, t5=.4, a=.8, d=.8, s=.85, r=1.5, pit=12, cho=.3, rev=.55),
 p("High Roller Rise", eng=SMK, t2=.5, t3=.4, t4=.5, t5=.2, a=2.5, d=1.5, s=.9, r=3, fen=.6, fa=2.2, fdc=1.5, cut=700, rso=.3, rev=.7),
 p("Midnight Casino", eng=SMK, t2=.35, t3=.6, t4=.6, t5=.3, a=.6, d=.8, s=.9, r=1.2, src=0, dst=1, rate=.2, amt=.4, cho=.4, rev=.6),
 p("Whisper Table", eng=SMK, t2=.2, t3=.5, t4=.6, t5=.9, a=1.5, d=1, s=.9, r=2.5, cut=2200, src=0, dst=1, rate=.15, amt=.5, rev=.65, vol=.6),
 p("Vault Drone", eng=SMK, t2=.8, t3=.2, t4=.4, t5=.3, a=2, d=1.2, s=.95, r=3, pit=-12, cut=1200, rev=.8, vol=.6),
 p("Pulse Of Luck", eng=SMK, t2=.55, t3=.35, t4=.5, t5=.2, a=.8, d=.9, s=.9, r=1.5, src=0, dst=2, rate=1.1, amt=.5, rev=.6),
 p("Final Hand", eng=SMK, t2=.4, t3=.5, t4=.9, t5=.4, a=1.4, d=1, s=.9, r=2.2, cho=.6, rev=.7, vol=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Hg {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
