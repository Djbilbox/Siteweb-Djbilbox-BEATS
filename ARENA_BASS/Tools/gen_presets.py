#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets ARENA BASS (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.003, d=.5, s=.8, r=.3, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, fen=0, fa=.002, fdc=.3, fs=0, fr=.3, rso=.15, vel=.5,
         rev=.15, cho=0, dis=0, dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=1, vol=.75)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', fen='fenv', fa='fattack', fdc='fdecay', fs='fsustain', fr='frelease', rso='resonance', vel='velocity', rev='reverb', cho='chorus', dis='distortion',
           dly='delay', tm='time', fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide',
           pit='pitch', ft='filtertype', mono='monopoly', vol='volume')
SUB, SLP, MOG, RSE, GRW, ACD, HRN, KCK = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Arena Sub", [
 p("Arena 808", eng=SUB, t1=.4, t2=.3, t3=.4, t5=.4, d=1.5, s=.8, r=.5, vol=.85, rev=.05),
 p("Stadium Rumble", eng=SUB, t1=.4, t2=.6, t3=.4, t5=1, d=1.6, s=.8, r=.5, dis=.4, vol=.6, rev=.1),
 p("Long Slide Sub", eng=SUB, t1=.3, t2=.3, t3=.3, t5=.3, d=2, s=.9, r=.8, gl=.35, vol=.85, rev=.05),
 p("Locker Room Sub", eng=SUB, t1=.2, t2=.2, t3=.1, t5=.1, d=2.5, s=.9, r=.8, pit=-12, vol=.9, rev=.1),
 p("Hard Knock Bass", eng=SUB, t1=.9, t2=.5, t3=.7, t5=.5, d=.8, s=.5, r=.25, vol=.8, rev=.05),
 p("Short Pump", eng=SUB, t1=.6, t2=.4, t3=.5, t5=.4, d=.35, s=0, r=.15, vol=.85, rev=.05),
 p("Smooth Court", eng=SUB, t1=.1, t2=.1, t3=.1, t5=0, d=1.5, s=.95, r=.4, vol=.9, rev=.05),
 p("Pitch Drop Dunk", eng=SUB, t1=1, t2=.4, t3=.6, t5=.3, d=1.2, s=.6, r=.4, gl=.1, vol=.8, rev=.05),
 p("Dirty Overtime", eng=SUB, t1=.5, t2=.5, t3=.5, t5=.9, d=1.5, s=.8, r=.4, dis=.25, vol=.7, rev=.05),
 p("Scoreboard Sub", eng=SUB, t1=.3, t2=.5, t3=.3, t5=.2, d=1.2, s=.85, r=.6, src=0, dst=2, rate=.5, amt=.3, vol=.85, rev=.15)])
bank("Slap & Pluck Bass", [
 p("Slap Bass", eng=SLP, t1=.9, t2=.5, t3=.7, t4=.6, t5=.5, d=.5, s=0, r=.2, vol=.8, rev=.1),
 p("Pick Bass", eng=SLP, t1=.7, t2=.5, t3=.6, t4=.4, t5=.3, d=.7, s=0, r=.25, vol=.8, rev=.1),
 p("Finger Funk", eng=SLP, t1=.4, t2=.6, t3=.4, t4=.5, t5=.2, d=.9, s=0, r=.3, vol=.85, rev=.1),
 p("Muted Pocket", eng=SLP, t1=.5, t2=.2, t3=.3, t4=.4, t5=.2, d=.3, s=0, r=.12, vol=.85, rev=.05),
 p("Upright Arena", eng=SLP, t1=.3, t2=.7, t3=.3, t4=.7, t5=.1, d=1.1, s=0, r=.4, vol=.85, rev=.15),
 p("Pop Slap", eng=SLP, t1=1, t2=.4, t3=.8, t4=.5, t5=.8, d=.4, s=0, r=.15, vol=.75, rev=.1, vel=.8),
 p("Deep Pluck", eng=SLP, t1=.3, t2=.7, t3=.3, t4=.8, t5=.1, d=1.4, s=0, r=.5, pit=-12, vol=.85, rev=.1),
 p("Chorus Bass", eng=SLP, t1=.5, t2=.5, t3=.5, t4=.5, t5=.3, d=.8, s=0, r=.3, cho=.45, vol=.8, rev=.15),
 p("Dub Pluck", eng=SLP, t1=.4, t2=.6, t3=.4, t4=.6, t5=.2, d=.9, s=0, r=.4, dly=.4, tm=.375, fb=.55, rev=.3),
 p("Walking Court", eng=SLP, t1=.5, t2=.6, t3=.5, t4=.5, t5=.2, d=.8, s=0, r=.3, gl=.04, vol=.85, rev=.15)])
bank("Moog Bass", [
 p("Arena Moog", eng=MOG, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, d=.3, s=.6, r=.15, fen=.7, fdc=.25, rso=.4, cut=500, vol=.8),
 p("Fat Moog", eng=MOG, t1=.7, t2=.6, t3=.4, t4=.5, t5=.3, d=.5, s=.7, r=.2, fen=.5, fdc=.35, rso=.3, cut=450, vol=.8),
 p("Rubber Moog", eng=MOG, t1=.4, t2=.4, t3=.6, t4=.3, t5=.4, d=.2, s=.4, r=.1, fen=.9, fdc=.12, rso=.55, cut=700, vol=.8),
 p("Sub Moog", eng=MOG, t1=.9, t2=.5, t3=.3, t4=.2, t5=.1, d=.6, s=.8, r=.25, fen=.3, fdc=.4, rso=.2, cut=300, vol=.85),
 p("Slide Moog", eng=MOG, t1=.5, t2=.5, t3=.5, t4=.3, t5=.3, d=.4, s=.7, r=.2, gl=.2, fen=.6, fdc=.3, rso=.4, cut=500, vol=.8),
 p("Pump Bass", eng=MOG, t1=.6, t2=.5, t3=.5, t4=.4, t5=.2, d=.2, s=.5, r=.1, fen=.8, fdc=.15, rso=.35, cut=600, vol=.8),
 p("Octave Moog", eng=MOG, t1=.5, t2=.5, t3=.7, t4=.6, t5=.5, d=.3, s=.6, r=.15, fen=.7, fdc=.2, rso=.4, cut=800, vol=.75),
 p("Hot Seat", eng=MOG, t1=.4, t2=.6, t3=.4, t4=.5, t5=.8, d=.4, s=.7, r=.2, dis=.2, fen=.6, fdc=.3, rso=.5, cut=550, vol=.7),
 p("Bass Stab", eng=MOG, t1=.5, t2=.5, t3=.6, t4=.4, t5=.2, d=.2, s=0, r=.1, fen=.8, fdc=.12, cut=600, vol=.8),
 p("Warm Anthem", eng=MOG, t1=.6, t2=.7, t3=.3, t4=.6, t5=.2, d=.6, s=.8, r=.3, fen=.3, fdc=.4, rso=.2, cut=400, vol=.85, cho=.2)])
bank("Reese & Growl", [
 p("Stadium Reese", eng=RSE, t1=.5, t2=.5, t3=.4, t4=.2, pit=-12, vol=.8, rev=.1),
 p("Dirty Reese", eng=RSE, t1=.4, t2=.6, t3=.5, t4=.7, pit=-12, dis=.2, vol=.75, rev=.1),
 p("Wobble Arena", eng=RSE, t1=.5, t2=.5, t3=.4, t4=.4, pit=-12, src=0, dst=1, rate=3, amt=.7, rso=.4, cut=500, vol=.8, rev=.1),
 p("Industrial Reese", eng=RSE, t1=.3, t2=.7, t3=.6, t4=.8, pit=-12, dis=.3, vol=.7, rev=.05),
 p("Sub Reese", eng=RSE, t1=.8, t2=.3, t3=.3, t4=.1, pit=-12, vol=.85, rev=.05),
 p("FM Growl", eng=GRW, t1=.4, t2=.4, t3=.5, t4=.4, t5=.3, pit=-12, vol=.75, rev=.1),
 p("Talking Bass", eng=GRW, t1=.6, t2=.5, t3=.7, t4=.6, t5=.4, pit=-12, src=0, dst=3, rate=2, amt=.5, vol=.7, rev=.1),
 p("Robot Growl", eng=GRW, t1=.3, t2=.3, t3=.8, t4=.9, t5=.6, pit=-12, dis=.2, vol=.65, rev=.1),
 p("Metal Growl", eng=GRW, t1=.9, t2=.5, t3=.6, t4=.5, t5=.8, pit=-12, vol=.65, rev=.1),
 p("Slow Growl", eng=GRW, t1=.5, t2=.5, t3=.5, t4=.1, t5=.2, pit=-12, gl=.1, vol=.75, rev=.15)])
bank("Acid & Rubber", [
 p("Acid Rubber", eng=ACD, t1=.3, t2=.4, t3=.6, t5=.3, d=.25, s=.4, r=.1, cut=700, rso=.4, vol=.75),
 p("Squelch", eng=ACD, t1=.2, t2=.3, t3=.8, t5=.5, d=.2, s=.3, r=.08, rso=.6, cut=500, vol=.7),
 p("Rubber Band", eng=ACD, t1=.4, t2=.2, t3=.7, t5=.2, d=.15, s=.2, r=.08, vol=.75),
 p("Acid Slide", eng=ACD, t1=.3, t2=.5, t3=.6, t5=.4, d=.4, s=.5, r=.15, gl=.15, rso=.5, vol=.75),
 p("Pulse Acid", eng=ACD, t1=.9, t2=.4, t3=.6, t5=.3, d=.3, s=.4, r=.1, rso=.4, vol=.7),
 p("Dirty Acid", eng=ACD, t1=.3, t2=.4, t3=.7, t5=.9, d=.3, s=.4, r=.1, dis=.25, vol=.65),
 p("Open Acid", eng=ACD, t1=.3, t2=.6, t3=.9, t5=.3, d=.6, s=.6, r=.2, rso=.3, vol=.7),
 p("Low Rubber", eng=ACD, t1=.4, t2=.5, t3=.5, t5=.3, d=.4, s=.5, r=.15, pit=-12, vol=.8),
 p("Pump Squelch", eng=ACD, t1=.2, t2=.3, t3=.8, t5=.6, d=.15, s=.2, r=.08, src=0, dst=1, rate=8, amt=.3, vol=.7),
 p("Buzzer Beater", eng=ACD, t1=.3, t2=.5, t3=.7, t5=.5, d=.5, s=.6, r=.2, dly=.3, tm=.1875, fb=.45, vol=.7)])
bank("Stadium Horns", [
 p("Stadium Horns", eng=HRN, t1=.3, t2=.7, t3=.6, t4=.5, t5=.2, a=.02, d=.4, s=.8, r=.3, mono=0, cho=.2, rev=.35),
 p("Fanfare", eng=HRN, t1=.2, t2=.8, t3=.8, t4=.6, t5=.1, a=.01, d=.3, s=.7, r=.25, mono=0, cho=.25, rev=.4),
 p("Brass Stab", eng=HRN, t1=.2, t2=.8, t3=.7, t4=.5, t5=.1, a=.005, d=.25, s=.1, r=.2, mono=0, rev=.3, vel=.8),
 p("Victory Horns", eng=HRN, t1=.3, t2=.9, t3=.9, t4=.8, t5=.2, a=.02, d=.4, s=.7, r=.3, mono=0, cho=.3, rev=.5, vol=.6),
 p("Soft Flugel", eng=HRN, t1=.6, t2=.5, t3=.3, t4=.4, t5=.1, a=.1, d=.6, s=.8, r=.5, mono=0, rev=.45),
 p("Slow Swell", eng=HRN, t1=.8, t2=.6, t3=.5, t4=.4, t5=.1, a=.4, d=.8, s=.85, r=.7, mono=0, rev=.5),
 p("Hit Section", eng=HRN, t1=.3, t2=.8, t3=.8, t4=.5, t5=.1, a=.003, d=.2, s=0, r=.15, mono=0, rev=.3),
 p("Low Horns", eng=HRN, t1=.4, t2=.7, t3=.4, t4=.5, t5=.2, a=.04, d=.5, s=.8, r=.3, pit=-12, mono=0, rev=.4),
 p("Lead Horn", eng=HRN, t1=.4, t2=.6, t3=.6, t4=.3, t5=.2, a=.03, d=.4, s=.8, r=.3, gl=.05, rev=.4, dly=.25, tm=.375),
 p("Anthem", eng=HRN, t1=.5, t2=.7, t3=.7, t4=.7, t5=.2, a=.06, d=.5, s=.85, r=.5, mono=0, cho=.35, rev=.6, vol=.6)])
bank("Kicks & Stomps", [
 p("Arena Kick", eng=KCK, t1=.5, t2=.4, t3=.5, t5=.3, d=.4, s=0, r=.1, vol=.85, rev=.05),
 p("Stomp", eng=KCK, t1=.6, t2=.5, t3=.7, t5=.4, d=.35, s=0, r=.1, vol=.85, rev=.2),
 p("Long Boom", eng=KCK, t1=.4, t2=.9, t3=.4, t5=.2, d=1.2, s=0, r=.5, vol=.85, rev=.2),
 p("Punch Kick", eng=KCK, t1=.8, t2=.2, t3=.8, t5=.5, d=.2, s=0, r=.08, vol=.85, rev=.05),
 p("Sub Kick", eng=KCK, t1=.2, t2=.6, t3=.2, t5=.1, d=.6, s=0, r=.2, pit=-12, vol=.9, rev=.05),
 p("Crowd Stomp", eng=KCK, t1=.5, t2=.5, t3=.9, t5=.3, d=.5, s=0, r=.15, vol=.75, rev=.5),
 p("Dirty Kick", eng=KCK, t1=.5, t2=.4, t3=.6, t5=1, d=.3, s=0, r=.1, dis=.25, vol=.7, rev=.05),
 p("Tom Fill", eng=KCK, t1=.3, t2=.5, t3=.5, t5=.2, d=.5, s=0, r=.2, pit=12, vol=.8, rev=.3),
 p("Drop Impact", eng=KCK, t1=1, t2=1, t3=.6, t5=.3, d=2, s=0, r=.8, pit=-12, vol=.8, rev=.5),
 p("Laser Kick", eng=KCK, t1=1, t2=.3, t3=.4, t5=.2, d=.3, s=0, r=.1, vol=.8, rev=.1)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Ab {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
