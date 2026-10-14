#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets PHANTOM RIDER (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.01, d=.5, s=.8, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fr=.3, rso=.15, vel=.5,
         rev=.4, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
THE, CHO, GTR, ORG, PAD, BEL, BAS, WND = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Theremin & Whistles", [
 p("Phantom Theremin", eng=THE, t1=.5, t2=.4, t3=.4, t4=.5, t5=.2, a=.08, mono=1, gl=.12, rev=.5),
 p("Ghost Whistle", eng=THE, t1=.3, t2=.2, t3=.2, t4=.3, t5=.4, a=.1, mono=1, gl=.08, pit=12, rev=.55),
 p("Lonesome Wail", eng=THE, t1=.6, t2=.5, t3=.5, t4=.8, t5=.2, a=.15, r=.8, mono=1, gl=.2, dly=.3, tm=.45, fb=.5, rev=.6),
 p("Spectral Sine", eng=THE, t1=.4, t2=.1, t3=.1, t4=.2, t5=0, a=.05, mono=1, rev=.45),
 p("Cold Wind Lead", eng=THE, t1=.5, t2=.3, t3=.3, t4=.5, t5=.9, a=.12, mono=1, gl=.1, rev=.55, vol=.6),
 p("Siren From Beyond", eng=THE, t1=.7, t2=.6, t3=.6, t4=.9, t5=.2, a=.3, r=1, mono=1, gl=.3, src=2, dst=0, rate=.3, amt=.8, rev=.6),
 p("Crystal Voice", eng=THE, t1=.5, t2=.8, t3=.7, t4=.4, t5=.1, a=.06, mono=1, gl=.05, cho=.3, rev=.5),
 p("Low Moan", eng=THE, t1=.3, t2=.7, t3=.3, t4=.6, t5=.3, a=.2, r=.9, mono=1, pit=-12, gl=.15, rev=.6),
 p("Haunt Echo", eng=THE, t1=.5, t2=.4, t3=.4, t4=.5, t5=.2, a=.08, mono=1, gl=.1, dly=.45, tm=.5, fb=.65, rev=.5),
 p("Fading Signal", eng=THE, t1=.4, t2=.3, t3=.3, t4=.7, t5=.5, a=.1, d=1.5, s=.4, r=1.5, mono=1, rev=.65, vol=.6)])
bank("Ghost Choir", [
 p("Phantom Choir", eng=CHO, t1=.15, t2=.5, t3=.5, t4=.5, t5=.3, a=.6, d=.9, s=.9, r=1.4, cho=.3, rev=.6),
 p("Ooo Ghosts", eng=CHO, t1=.7, t2=.55, t3=.35, t4=.6, t5=.3, a=.8, d=1, s=.9, r=1.6, rev=.65),
 p("Ah Specters", eng=CHO, t1=0, t2=.5, t3=.5, t4=.4, t5=.3, a=.5, d=.8, s=.9, r=1.2, cho=.25, rev=.55),
 p("Ee Wraiths", eng=CHO, t1=.95, t2=.5, t3=.6, t4=.5, t5=.4, a=.7, d=.9, s=.85, r=1.4, rev=.6),
 p("Vowel Haunt", eng=CHO, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, a=.5, d=.9, s=.9, r=1.2, src=0, dst=3, rate=.25, amt=.7, rev=.6),
 p("Whisper Gang", eng=CHO, t1=.3, t2=.4, t3=.3, t4=.7, t5=.9, a=.9, d=1, s=.85, r=1.8, rev=.7, vol=.6),
 p("Low Spirits", eng=CHO, t1=.25, t2=.6, t3=.4, t4=.5, t5=.3, a=.6, d=.9, s=.9, r=1.3, pit=-12, rev=.6),
 p("Graveyard Hymn", eng=CHO, t1=.1, t2=.55, t3=.55, t4=.3, t5=.2, a=.4, d=.7, s=.9, r=1, gl=.2, mono=1, src=0, dst=0, rate=5.2, amt=.1, rev=.65),
 p("Dust Chorale", eng=CHO, t1=.6, t2=.5, t3=.6, t4=.8, t5=.3, a=.45, d=.8, s=.9, r=1.2, cho=.5, rev=.6),
 p("Hollow Chant", eng=CHO, t1=.4, t2=.7, t3=.2, t4=.4, t5=.5, a=.7, d=1, s=.85, r=1.5, cut=2500, rev=.65, dly=.25, tm=.5)])
bank("Twang & Strings", [
 p("Ghost Town Twang", eng=GTR, t1=.5, t2=.6, t3=.6, t4=.5, t5=.5, d=1, s=0, r=.6, rev=.5, dly=.25, tm=.4, fb=.4),
 p("Spaghetti Lead", eng=GTR, t1=.6, t2=.7, t3=.7, t4=.6, t5=.6, d=1.4, s=0, r=.8, mono=1, gl=.04, dly=.3, tm=.375, fb=.45, rev=.55),
 p("Dusty Pick", eng=GTR, t1=.4, t2=.4, t3=.4, t4=.4, t5=.3, d=.7, s=0, r=.4, cut=5000, rev=.4),
 p("Reverb Surf", eng=GTR, t1=.5, t2=.7, t3=.7, t4=.7, t5=.7, d=1.2, s=0, r=.7, cho=.3, rev=.7, dly=.3, tm=.3, fb=.5),
 p("Low Rider Gtr", eng=GTR, t1=.4, t2=.6, t3=.4, t4=.5, t5=.4, d=1, s=0, r=.6, pit=-12, rev=.5, vol=.55),
 p("Muted Bounty", eng=GTR, t1=.7, t2=.2, t3=.5, t4=.4, t5=.4, d=.3, s=0, r=.2, rev=.3, vel=.8),
 p("Harp Of Bones", eng=GTR, t1=.2, t2=.8, t3=.6, t4=.3, t5=.2, d=1.8, s=0, r=1.2, pit=12, rev=.65),
 p("Ride Away", eng=GTR, t1=.55, t2=.65, t3=.65, t4=.55, t5=.5, d=1.2, s=0, r=.7, dly=.35, tm=.45, fb=.55, rev=.55),
 p("Haunted Banjo", eng=GTR, t1=.8, t2=.4, t3=.8, t4=.8, t5=.7, d=.5, s=0, r=.3, rev=.4, vol=.5),
 p("Midnight Pluck", eng=GTR, t1=.3, t2=.5, t3=.5, t4=.5, t5=.3, d=.9, s=0, r=.5, cho=.3, rev=.6, vol=.65)])
bank("Haunted Organs", [
 p("Haunted Organ", eng=ORG, t1=.6, t2=.6, t3=.4, t4=.6, t5=.3, a=.02, d=.3, s=.9, r=.3, rev=.6),
 p("Chapel Of Dust", eng=ORG, t1=.7, t2=.5, t3=.2, t4=.3, t5=.1, a=.05, d=.3, s=.95, r=.5, rev=.75),
 p("Funeral March", eng=ORG, t1=.9, t2=.7, t3=.3, t4=.5, t5=.2, a=.02, d=.3, s=.9, r=.4, pit=-12, rev=.7),
 p("Carnival Of Souls", eng=ORG, t1=.5, t2=.6, t3=.7, t4=.9, t5=.6, d=.3, s=.9, r=.2, cho=.4, rev=.45),
 p("Whisper Pipes", eng=ORG, t1=.2, t2=.8, t3=.1, t4=.2, t5=.1, a=.08, d=.3, s=.9, r=.5, rev=.65),
 p("Cellar Door", eng=ORG, t1=.8, t2=.4, t3=.3, t4=.7, t5=.2, a=.03, d=.4, s=.9, r=.4, cut=2500, rev=.55),
 p("Full Ghost Stops", eng=ORG, t1=.9, t2=.9, t3=.8, t4=.5, t5=.3, a=.02, d=.2, s=.9, r=.3, rev=.6, vol=.55),
 p("Dying Swell", eng=ORG, t1=.7, t2=.6, t3=.3, t4=.4, t5=.1, a=1, d=.5, s=.95, r=2, rev=.7),
 p("Fog Organ", eng=ORG, t1=.5, t2=.5, t3=.2, t4=1, t5=.2, a=.2, d=.4, s=.9, r=.8, cho=.5, rev=.7),
 p("Last Hymn", eng=ORG, t1=.6, t2=.5, t3=.4, t4=.3, t5=.1, a=.04, d=.3, s=.9, r=.6, dly=.3, tm=.5, fb=.5, rev=.6)])
bank("Wraith Pads", [
 p("Wraith Pad", eng=PAD, t2=.4, t3=.4, t4=.5, t5=.2, a=1.2, d=1, s=.9, r=2, cho=.4, rev=.65),
 p("Fog Bank", eng=PAD, t2=.6, t3=.2, t4=.3, t5=.5, a=1.8, d=1, s=.95, r=2.5, rev=.75, pit=-12),
 p("Dead Moon", eng=PAD, t2=.5, t3=.5, t4=.6, t5=.2, a=1, d=.9, s=.9, r=1.8, cho=.5, rev=.6),
 p("Cold Breath", eng=PAD, t2=.3, t3=.7, t4=.7, t5=.7, a=.8, d=.8, s=.85, r=1.5, pit=12, cho=.3, rev=.55),
 p("Rising Dread", eng=PAD, t2=.5, t3=.4, t4=.5, t5=.2, a=2.5, d=1.5, s=.9, r=3, fen=.6, fa=2.2, fdc=1.5, cut=700, rso=.3, rev=.7),
 p("Moaning Strings", eng=PAD, t2=.35, t3=.6, t4=.6, t5=.3, a=.6, d=.8, s=.9, r=1.2, src=0, dst=1, rate=.2, amt=.4, cho=.4, rev=.6),
 p("Dune Spirit", eng=PAD, t2=.2, t3=.5, t4=.6, t5=.9, a=1.5, d=1, s=.9, r=2.5, cut=2200, src=0, dst=1, rate=.15, amt=.5, rev=.65, vol=.6),
 p("Tomb Drone", eng=PAD, t2=.8, t3=.2, t4=.4, t5=.3, a=2, d=1.2, s=.95, r=3, pit=-12, cut=1200, rev=.8, vol=.6),
 p("Pulse Of The Dead", eng=PAD, t2=.55, t3=.35, t4=.5, t5=.2, a=.8, d=.9, s=.9, r=1.5, src=0, dst=2, rate=1.1, amt=.5, rev=.6),
 p("Phantom Veil", eng=PAD, t2=.4, t3=.5, t4=.9, t5=.4, a=1.4, d=1, s=.9, r=2.2, cho=.6, rev=.7, vol=.6)])
bank("Ghost Bells", [
 p("Ghost Bell", eng=BEL, t1=.3, t2=.5, t3=.5, t4=.3, t5=.1, d=1.4, s=0, r=1, rev=.6),
 p("Church Of Dust", eng=BEL, t1=.4, t2=.8, t3=.6, t4=.4, t5=.1, d=2.2, s=0, r=1.8, pit=-12, rev=.75),
 p("Chain Chimes", eng=BEL, t1=.7, t2=.4, t3=.8, t4=.6, t5=.3, d=1, s=0, r=.8, pit=12, dly=.3, tm=.375, fb=.5, rev=.6, vol=.6),
 p("Spirit Glass", eng=BEL, t1=.5, t2=.6, t3=.7, t4=.5, t5=.1, d=1.6, s=0, r=1.2, pit=12, rev=.65, vol=.6),
 p("Toll For Me", eng=BEL, t1=.35, t2=.9, t3=.6, t4=.3, t5=.2, d=3, s=0, r=2.5, pit=-12, rev=.7),
 p("Music Box Ghost", eng=BEL, t1=.6, t2=.3, t3=.5, t4=.3, t5=.1, d=.8, s=0, r=.6, pit=12, rev=.5),
 p("Wind Chimes", eng=BEL, t1=.8, t2=.5, t3=.7, t4=.8, t5=.1, d=1.2, s=0, r=1, pit=12, dly=.35, tm=.25, fb=.55, rev=.65, vol=.55),
 p("Hollow Toll", eng=BEL, t1=.2, t2=.6, t3=.4, t4=.3, t5=.3, d=1.5, s=0, r=1.2, cut=3500, rev=.6),
 p("Cursed Kalimba", eng=BEL, t1=.55, t2=.2, t3=.4, t4=.3, t5=.4, d=.5, s=0, r=.4, rev=.45),
 p("Echoing Hall", eng=BEL, t1=.4, t2=.7, t3=.5, t4=.5, t5=.1, d=2, s=0, r=1.5, dly=.4, tm=.5, fb=.6, rev=.75, vol=.55)])
bank("Dead Bass & Wind", [
 p("Dead Bass", eng=BAS, t1=.3, t2=.3, t3=.3, t5=.2, a=.005, d=1.2, s=.8, r=.35, mono=1, vol=.85, rev=.2),
 p("Grave Sub", eng=BAS, t1=.2, t2=.2, t3=.2, t5=.1, d=1.5, s=.9, r=.5, mono=1, pit=-12, vol=.9, rev=.15),
 p("Knock Knock", eng=BAS, t1=.9, t2=.5, t3=.7, t5=.4, d=.6, s=.3, r=.2, mono=1, vol=.8, rev=.2),
 p("Rotten Sub", eng=BAS, t1=.4, t2=.5, t3=.4, t5=.9, d=1, s=.8, r=.3, mono=1, dis=.25, vol=.7, rev=.2),
 p("Hearse Drone", eng=BAS, t1=.2, t2=.3, t3=.2, t5=.3, a=.3, d=1.5, s=.9, r=.8, mono=1, gl=.2, vol=.85, rev=.3),
 p("Whisper Wind", eng=WND, t1=.4, t2=.4, t3=.4, t4=.4, t5=.1, a=.8, d=1, s=.9, r=1.5, rev=.65),
 p("Howling Gale", eng=WND, t1=.6, t2=.7, t3=.6, t4=.8, t5=.2, a=1.2, d=1, s=.95, r=2, src=0, dst=1, rate=.2, amt=.4, rev=.7, vol=.6),
 p("Dead Harmonica", eng=WND, t1=.3, t2=.8, t3=.3, t4=.3, t5=.7, a=.1, d=.5, s=.85, r=.5, mono=1, gl=.06, rev=.55),
 p("Tumbleweed", eng=WND, t1=.5, t2=.3, t3=.5, t4=.6, t5=.2, a=.5, d=.8, s=.8, r=1, pit=12, dly=.3, tm=.5, fb=.5, rev=.6, vol=.6),
 p("Ghost Rider Wind", eng=WND, t1=.7, t2=.5, t3=.7, t4=.5, t5=.4, a=.6, d=1, s=.9, r=1.4, mono=1, gl=.15, rev=.7, vol=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Pr {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
