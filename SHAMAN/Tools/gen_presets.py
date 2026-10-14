#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets SHAMAN (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.003, d=.4, s=.8, r=.3, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.25, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
DID, TRB, JAW, FLT, CHO, PAD, CHM, DRN = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Didgeridoo", [
 p("Earth Didge", eng=DID, mono=1, pit=-12, t1=.4, t2=.5, t3=.4, t4=.3, t5=.3, vol=.8, rev=.2),
 p("Outback Drone", eng=DID, mono=1, pit=-12, a=.1, t1=.3, t2=.6, t3=.3, t5=.5, rev=.25, vol=.8),
 p("Growl Didge", eng=DID, mono=1, pit=-12, t1=.5, t2=.6, t3=.5, t5=.8, dis=.2, vol=.7),
 p("Wobble Spirit", eng=DID, mono=1, pit=-12, t1=.5, t2=.5, t3=.4, t5=.4, src=0, dst=1, rate=2.5, amt=.5, rev=.25, vol=.8),
 p("Deep Breath", eng=DID, mono=1, pit=-12, a=.25, r=.6, t1=.2, t2=.7, t3=.2, t5=.2, rev=.35, vol=.8),
 p("Rhythm Didge", eng=DID, mono=1, pit=-12, d=.2, s=.3, r=.12, t1=.6, t2=.4, t3=.6, t5=.6, vol=.8),
 p("Sacred Hum", eng=DID, mono=1, a=.3, r=1, t1=.3, t2=.6, t3=.3, t5=.1, cho=.3, rev=.5, vol=.7),
 p("Bright Didge", eng=DID, mono=1, pit=-12, t1=.8, t2=.4, t3=.8, t5=.4, rev=.2, vol=.7),
 p("Slide Didge", eng=DID, mono=1, pit=-12, gl=.2, t1=.5, t2=.5, t3=.4, t5=.5, rev=.2, vol=.8),
 p("Storm Didge", eng=DID, mono=1, pit=-12, t1=.6, t2=.8, t3=.5, t5=.9, dis=.3, rev=.3, vol=.6)])
bank("Tribal Drums", [
 p("Tom Low", eng=TRB, t1=.2, t2=.5, t3=.3, t4=.3, t5=.2, d=.5, s=0, r=.3, rev=.2, vol=.8),
 p("Tom Mid", eng=TRB, t1=.5, t2=.5, t3=.4, t4=.3, t5=.2, d=.4, s=0, r=.25, rev=.2, vol=.8),
 p("Djembe Bass", eng=TRB, t1=.3, t2=.7, t3=.5, t4=.2, t5=.3, d=.35, s=0, r=.2, rev=.2, vol=.8),
 p("Djembe Slap", eng=TRB, t1=.8, t2=.3, t3=.8, t4=.2, t5=.5, d=.2, s=0, r=.15, rev=.15, vol=.8),
 p("Ritual Taiko", eng=TRB, t1=.15, t2=.8, t3=.4, t4=.3, t5=.2, d=.8, s=0, r=.5, rev=.35, vol=.8),
 p("Skin Drum", eng=TRB, t1=.6, t2=.4, t3=.5, t4=.2, t5=.4, d=.3, s=0, r=.2, rev=.25, vol=.8),
 p("War Drum", eng=TRB, t1=.1, t2=.9, t3=.3, t4=.4, t5=.3, d=.9, s=0, r=.6, dis=.2, rev=.4, vol=.7),
 p("Bongo Hi", eng=TRB, t1=.9, t2=.3, t3=.7, t4=.1, t5=.4, d=.15, s=0, r=.1, rev=.15, vol=.8),
 p("Cave Echo", eng=TRB, t1=.3, t2=.6, t3=.4, t4=.3, t5=.2, d=.5, s=0, r=.3, dly=.35, tm=.375, fb=.5, rev=.5, vol=.8),
 p("Thunder Skin", eng=TRB, t1=.2, t2=.8, t3=.6, t4=.5, t5=.7, d=1, s=0, r=.8, rev=.5, vol=.7)])
bank("Jaw Harp & Drone", [
 p("Jaw Harp", eng=JAW, t1=.5, t2=.5, t3=.5, t4=.3, t5=.3, mono=1, rev=.2),
 p("Twang Spirit", eng=JAW, t1=.7, t2=.6, t3=.7, t4=.3, t5=.4, mono=1, dly=.25, tm=.25, fb=.4, rev=.25),
 p("Mouth Bow", eng=JAW, t1=.4, t2=.4, t3=.4, t4=.2, t5=.2, mono=1, d=.4, s=.3, r=.2, rev=.2),
 p("Buzz Rhythm", eng=JAW, t1=.6, t2=.5, t3=.6, t4=.5, t5=.7, mono=1, src=0, dst=1, rate=6, amt=.4, rev=.15),
 p("Cave Jaw", eng=JAW, t1=.3, t2=.7, t3=.4, t4=.3, t5=.3, mono=1, rev=.5, dly=.35, tm=.375, fb=.5),
 p("Earth Drone", eng=DRN, t1=.4, t2=.5, t3=.3, t4=.4, t5=.2, a=.6, r=1.5, rev=.5, vol=.75),
 p("Stone Circle", eng=DRN, t1=.2, t2=.6, t3=.3, t4=.6, t5=.3, a=.8, r=2, cho=.3, rev=.6, vol=.7),
 p("Root Pulse", eng=DRN, t1=.5, t2=.4, t3=.4, t4=.3, t5=.5, pit=-12, a=.3, r=1, src=0, dst=1, rate=.8, amt=.4, rev=.35, vol=.75),
 p("Dark Hollow", eng=DRN, t1=.3, t2=.8, t3=.2, t4=.5, t5=.4, pit=-12, a=1, r=2, rev=.65, vol=.7),
 p("Ancestor Hum", eng=DRN, t1=.6, t2=.5, t3=.5, t4=.7, t5=.1, a=1.2, s=.9, r=2.5, cho=.4, rev=.7, vol=.65)])
bank("Spirit Flute", [
 p("Spirit Flute", eng=FLT, t1=.5, t2=.4, t3=.4, t4=.3, t5=.3, mono=1, a=.05, rev=.4, vol=.8),
 p("Breath Flute", eng=FLT, t1=.4, t2=.3, t3=.5, t4=.3, t5=.7, mono=1, a=.08, rev=.4, vol=.8),
 p("Bone Whistle", eng=FLT, t1=.8, t2=.2, t3=.8, t4=.2, t5=.3, mono=1, a=.03, rev=.35, vol=.8),
 p("Shaman Pipe", eng=FLT, t1=.5, t2=.5, t3=.5, t4=.4, t5=.4, mono=1, gl=.08, src=0, dst=0, rate=5, amt=.08, rev=.45, vol=.8),
 p("Mist Flute", eng=FLT, t1=.3, t2=.6, t3=.3, t4=.5, t5=.5, mono=1, a=.15, r=.8, cho=.3, rev=.6, vol=.8),
 p("Eagle Cry", eng=FLT, t1=.9, t2=.3, t3=.7, t4=.3, t5=.3, mono=1, gl=.15, dly=.3, tm=.375, fb=.45, rev=.45, vol=.75),
 p("Hollow Reed", eng=FLT, t1=.2, t2=.5, t3=.3, t4=.2, t5=.4, mono=1, rev=.35, vol=.8),
 p("Dream Flute", eng=FLT, t1=.6, t2=.5, t3=.4, t4=.6, t5=.3, a=.2, r=1, cho=.4, rev=.6, vol=.75),
 p("Overblown", eng=FLT, t1=.7, t2=.3, t3=.9, t4=.3, t5=.8, mono=1, dis=.1, rev=.35, vol=.7),
 p("Canyon Wind", eng=FLT, t1=.4, t2=.7, t3=.4, t4=.7, t5=.9, a=.6, r=1.5, rev=.7, vol=.7)])
bank("Chant Choir", [
 p("Chant Ah", eng=CHO, t1=.1, t2=.5, t3=.5, t4=.4, t5=.2, a=.15, r=.6, rev=.45, vol=.8),
 p("Chant Oh", eng=CHO, t1=.4, t2=.5, t3=.5, t4=.4, t5=.2, a=.15, r=.6, rev=.45, vol=.8),
 p("Chant Ee", eng=CHO, t1=.8, t2=.5, t3=.6, t4=.4, t5=.2, a=.15, r=.6, rev=.45, vol=.8),
 p("Elder Voices", eng=CHO, t1=.3, t2=.7, t3=.4, t4=.6, t5=.2, a=.4, r=1, cho=.4, rev=.6, vol=.8),
 p("Tribe Gather", eng=CHO, t1=.5, t2=.8, t3=.5, t4=.8, t5=.3, a=.3, r=1, cho=.4, rev=.55, vol=.75),
 p("Whisper Rite", eng=CHO, t1=.6, t2=.4, t3=.6, t4=.5, t5=.9, a=.5, r=1.2, rev=.65, vol=.8),
 p("Low Throat", eng=CHO, t1=.2, t2=.4, t3=.3, t4=.2, t5=.4, pit=-12, mono=1, a=.1, rev=.35, vol=.8),
 p("Vowel Sweep", eng=CHO, t1=.5, t2=.5, t3=.5, t4=.4, t5=.2, a=.3, r=1, src=0, dst=3, rate=.4, amt=.8, rev=.5, vol=.8),
 p("Sky Choir", eng=CHO, t1=.7, t2=.7, t3=.7, t4=.7, t5=.1, pit=12, a=.6, r=1.5, cho=.5, rev=.7, vol=.75),
 p("Ghost Chant", eng=CHO, t1=.4, t2=.6, t3=.3, t4=.9, t5=.6, a=.8, r=2, dly=.4, tm=.5, fb=.5, rev=.75, vol=.7)])
bank("Fire Pads", [
 p("Fire Pad", eng=PAD, t1=.5, t2=.5, t3=.4, t4=.4, t5=.2, a=.5, r=1.2, rev=.5, vol=.75),
 p("Ember Glow", eng=PAD, t1=.3, t2=.6, t3=.3, t4=.5, t5=.2, a=.8, r=1.8, cho=.4, rev=.6, vol=.75),
 p("Campfire", eng=PAD, t1=.6, t2=.4, t3=.5, t4=.3, t5=.6, a=.3, r=1, rev=.4, vol=.75),
 p("Smoke Rise", eng=PAD, t1=.4, t2=.7, t3=.3, t4=.6, t5=.4, a=1.2, r=2.5, rev=.7, vol=.7),
 p("Sacred Flame", eng=PAD, t1=.7, t2=.6, t3=.6, t4=.5, t5=.2, a=.6, r=1.5, cho=.4, rev=.6, vol=.7),
 p("Dusk Ritual", eng=PAD, t1=.3, t2=.5, t3=.3, t4=.7, t5=.3, a=1, r=2, src=0, dst=1, rate=.2, amt=.4, rev=.65, vol=.7),
 p("Sun Dance", eng=PAD, t1=.8, t2=.5, t3=.7, t4=.4, t5=.2, a=.2, r=.8, cho=.3, dly=.3, tm=.375, fb=.4, rev=.4, vol=.7),
 p("Night Watch", eng=PAD, t1=.2, t2=.4, t3=.2, t4=.3, t5=.3, a=1.5, r=3, rev=.75, vol=.7),
 p("Heat Haze", eng=PAD, t1=.5, t2=.8, t3=.5, t4=.8, t5=.5, a=.8, r=1.8, cho=.5, rev=.6, vol=.65),
 p("Spirit Smoke", eng=PAD, t1=.6, t2=.6, t3=.4, t4=.6, t5=.8, a=1, r=2.5, dly=.4, tm=.5, fb=.5, rev=.7, vol=.65)])
bank("Bone Chimes", [
 p("Bone Chimes", eng=CHM, t1=.5, t2=.4, t3=.5, t4=.3, t5=.2, d=1.2, s=0, r=.8, rev=.45, vol=.8),
 p("Shell Bells", eng=CHM, t1=.7, t2=.3, t3=.6, t4=.2, t5=.2, d=1.5, s=0, r=1, pit=12, rev=.5, vol=.7),
 p("Wood Block", eng=CHM, t1=.2, t2=.2, t3=.5, t4=.1, t5=.5, d=.2, s=0, r=.15, rev=.25, vol=.8),
 p("Kalimba Tribe", eng=CHM, t1=.3, t2=.4, t3=.4, t4=.2, t5=.3, d=.7, s=0, r=.5, dly=.3, tm=.375, fb=.4, rev=.4, vol=.8),
 p("Rain Stick", eng=CHM, t1=.9, t2=.6, t3=.8, t4=.6, t5=.9, d=.8, s=0, r=.6, dly=.35, tm=.1875, fb=.55, rev=.5, vol=.6),
 p("Ghost Bells", eng=CHM, t1=.6, t2=.5, t3=.4, t4=.8, t5=.2, d=2, s=0, r=2, cho=.3, rev=.7, vol=.65),
 p("Marimba Wood", eng=CHM, t1=.4, t2=.3, t3=.5, t4=.1, t5=.4, d=.5, s=0, r=.3, rev=.35, vol=.8),
 p("Ritual Gong", eng=CHM, t1=.2, t2=.7, t3=.3, t4=.5, t5=.2, pit=-12, d=2.5, s=0, r=2, rev=.6, vol=.7),
 p("Tin Tribe", eng=CHM, t1=.8, t2=.2, t3=.9, t4=.2, t5=.5, d=.4, s=0, r=.3, rev=.3, vol=.7),
 p("Dream Chimes", eng=CHM, t1=.5, t2=.5, t3=.5, t4=.7, t5=.1, d=2, s=0, r=1.5, cho=.4, dly=.4, tm=.5, fb=.5, rev=.7, vol=.6)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Sh {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
