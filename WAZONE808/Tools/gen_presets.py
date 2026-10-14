#!/usr/bin/env python3
"""Genere Source/Params/PresetData.h (60 presets d'usine) et Assets/EXPANSION_WZ_VOL1.wz808exp (10 presets)."""
import json, sys
D = dict(waveform=0, filter=8000, resonance=1, envdepth=0, attack=2, decay=300, sustain=.7, release=250,
         fattack=1, fdecay=300, fsustain=.7, velocity=.4, glide=0, rate=2, amount=0, lfosource=0, destination=0,
         filtertype=0, monopoly=1, volume=1.0, reverb=.1, chorus=0, distortion=0, time=250, feedback=.3, delaymix=0,
         unison=1, detune=0, pitch=0, disttone=5000, chorusrate=1.5, chorusamount=.5, reverbroom=.4, reverbdamp=.5,
         reverbwidth=1, unisonwidth=.5)
SINE, SQR, SAW, TRI, NOISE = 0, 1, 2, 3, 4
LP, BP, HP = 0, 1, 2
PIT, FLT, AMP = 0, 1, 2
BANKS = []
def bank(n, rows): BANKS.append((n, rows))
def p(n, **k): return (n, k)

bank("808 Subs", [
 p("808 Classic", decay=1400, sustain=0, release=450, distortion=.12, filter=900),
 p("808 Boom", decay=2200, sustain=0, release=700, glide=40, filter=700, distortion=.1, reverb=.12),
 p("808 Short", decay=240, sustain=0, release=120, filter=1100),
 p("808 Slide", decay=1600, sustain=.1, release=500, glide=130, filter=800, distortion=.18),
 p("808 Dirty", decay=1300, sustain=0, release=350, distortion=.5, filter=3500, disttone=3200),
 p("808 Triangle", waveform=TRI, decay=1200, sustain=0, release=400, filter=1600, distortion=.2),
 p("808 Deep", decay=2600, sustain=.2, release=800, filter=240, volume=1.15),
 p("808 Tuned Down", decay=1600, sustain=0, release=500, pitch=-5, filter=800, distortion=.15),
 p("808 Punch", decay=900, sustain=0, release=250, filter=300, envdepth=3, fdecay=120, resonance=2.5, distortion=.2),
 p("808 Trap Long", decay=3600, sustain=0, release=1400, glide=60, filter=650, distortion=.1, delaymix=.08, time=375, feedback=.25)])
bank("Dirty & Reese", [
 p("Reese Bass", waveform=SAW, unison=3, detune=18, filter=900, resonance=2, decay=600, sustain=.8, release=300, lfosource=0, destination=FLT, rate=.35, amount=.35, chorus=.25, distortion=.15),
 p("Warzone Growl", volume=.7, waveform=SAW, filter=700, resonance=5, envdepth=2.5, fdecay=450, fsustain=.2, decay=500, sustain=.8, lfosource=1, destination=FLT, rate=4, amount=.4, distortion=.45),
 p("Grit Sub", volume=.7, waveform=SQR, filter=500, decay=500, sustain=.75, release=200, distortion=.4, disttone=2400),
 p("Siren Bass", waveform=SAW, filter=1800, glide=180, decay=600, sustain=.8, lfosource=3, destination=PIT, rate=1.2, amount=.3, distortion=.25),
 p("Detune War", waveform=SAW, unison=5, detune=26, unisonwidth=.8, filter=1500, resonance=1.5, decay=500, sustain=.8, release=350, chorus=.2, distortion=.2),
 p("Tank Bass", volume=.7, waveform=SQR, filter=1400, resonance=2, envdepth=1.5, fdecay=300, fsustain=.3, decay=420, sustain=.7, distortion=.6, disttone=3000),
 p("Mud Reese", waveform=SAW, unison=3, detune=12, filter=420, resonance=1.2, decay=800, sustain=.85, release=500, lfosource=3, destination=FLT, rate=.2, amount=.5),
 p("Rumble", volume=.7, waveform=TRI, filter=600, decay=900, sustain=.8, release=600, distortion=.35, reverb=.15),
 p("Subsonic Drift", waveform=SINE, filter=400, decay=900, sustain=.9, release=900, lfosource=0, destination=PIT, rate=.4, amount=.15, volume=.8),
 p("Overdrive 808", decay=1300, sustain=0, release=400, filter=5000, distortion=.9, disttone=3500, volume=.7)])
bank("Leads", [
 p("War Horn", waveform=SAW, unison=3, detune=14, filter=3800, resonance=1.5, attack=18, decay=500, sustain=.75, release=300, chorus=.3, delaymix=.15, time=330),
 p("Bunker Lead", waveform=SQR, filter=1400, resonance=3, envdepth=3, fdecay=380, fsustain=.35, decay=450, sustain=.7, release=220, glide=40, delaymix=.18, time=375, feedback=.35),
 p("Cavalry Charge", waveform=SAW, unison=3, detune=10, filter=5200, attack=8, decay=350, sustain=.7, release=260, delaymix=.25, time=250, feedback=.4, reverb=.2),
 p("Mono Hero", waveform=SAW, filter=3200, resonance=2, glide=70, decay=400, sustain=.75, release=200, chorus=.15),
 p("Night Vision", waveform=TRI, filter=6000, attack=25, decay=600, sustain=.7, release=450, chorus=.4, reverb=.35, lfosource=0, destination=PIT, rate=5.2, amount=.12),
 p("Sniper", waveform=SQR, filter=2600, resonance=2, envdepth=2, fdecay=140, fsustain=.1, decay=180, sustain=.2, release=150, delaymix=.12),
 p("Siren Lead", waveform=SAW, filter=6500, decay=500, sustain=.85, release=300, lfosource=0, destination=PIT, rate=5.8, amount=.2, delaymix=.2, time=300),
 p("Sci-Fi Comm", waveform=SQR, filter=3000, resonance=4, filtertype=BP, decay=400, sustain=.7, lfosource=1, destination=FLT, rate=7, amount=.25, delaymix=.2, reverb=.25),
 p("Trench Whistle", waveform=TRI, filter=8000, attack=40, decay=300, sustain=.8, release=350, lfosource=0, destination=PIT, rate=5, amount=.1, reverb=.3),
 p("Combat Saw", waveform=SAW, unison=5, detune=20, unisonwidth=.7, filter=4200, attack=6, decay=400, sustain=.75, release=280, chorus=.25, delaymix=.12)])
bank("Plucks & Keys", [
 p("Ammo Pluck", waveform=SQR, filter=900, resonance=2.5, envdepth=3, fdecay=150, fsustain=0, decay=240, sustain=0, release=160, monopoly=0, reverb=.18),
 p("Boom Bap Keys", waveform=TRI, filter=3800, decay=700, sustain=.3, release=350, monopoly=0, chorus=.35, reverb=.25, velocity=.7),
 p("Dusty Keys", waveform=TRI, filter=2200, decay=900, sustain=.25, release=500, monopoly=0, chorus=.2, reverb=.4, lfosource=0, destination=PIT, rate=4.5, amount=.08),
 p("Saw Pluck", waveform=SAW, filter=1100, resonance=2, envdepth=3, fdecay=200, fsustain=0, decay=300, sustain=0, release=200, monopoly=0, delaymix=.14, reverb=.15),
 p("Metal Pluck", waveform=SQR, filter=1800, resonance=8, filtertype=BP, envdepth=2, fdecay=130, fsustain=0, decay=260, sustain=0, release=180, monopoly=0, distortion=.2),
 p("Soldier Bell", waveform=SINE, filter=9000, decay=1200, sustain=0, release=900, monopoly=0, pitch=12, reverb=.4, delaymix=.2, time=300),
 p("Dark Chord", waveform=SAW, unison=3, detune=16, filter=1500, attack=6, decay=700, sustain=.45, release=500, monopoly=0, reverb=.3, chorus=.3),
 p("Rust Keys", waveform=SQR, filter=1700, decay=500, sustain=.25, release=350, monopoly=0, distortion=.18, chorus=.2, reverb=.2, velocity=.6),
 p("Radio Piano", waveform=TRI, filter=1400, resonance=1.5, decay=800, sustain=.2, release=450, monopoly=0, chorus=.25, distortion=.1, reverb=.3, velocity=.8),
 p("Bunker Marimba", waveform=SINE, filter=5000, decay=450, sustain=0, release=300, monopoly=0, pitch=12, reverb=.25, velocity=.6)])
bank("Pads & Ambience", [
 p("Foxhole Pad", waveform=SAW, unison=5, detune=22, unisonwidth=.9, filter=1600, attack=900, decay=900, sustain=.8, release=1400, monopoly=0, chorus=.45, reverb=.5),
 p("Smoke Screen", waveform=TRI, filter=2500, attack=1100, decay=800, sustain=.85, release=1800, monopoly=0, reverb=.65, chorus=.3),
 p("Fallout", waveform=SAW, unison=3, detune=14, filter=900, resonance=2.5, attack=700, decay=900, sustain=.8, release=1500, monopoly=0, lfosource=3, destination=FLT, rate=.18, amount=.5, reverb=.55),
 p("Radar Sweep", waveform=SAW, filter=2000, resonance=4, attack=500, decay=900, sustain=.8, release=1200, monopoly=0, lfosource=0, destination=FLT, rate=.3, amount=.6, delaymix=.25, time=500, feedback=.5, reverb=.4),
 p("Dark Drone", waveform=SAW, unison=3, detune=9, filter=450, attack=1500, sustain=1, release=2500, monopoly=0, lfosource=0, destination=FLT, rate=.12, amount=.4, reverb=.6, volume=.9),
 p("Night Patrol", waveform=TRI, filter=3500, attack=600, sustain=.9, release=1600, monopoly=0, chorus=.5, reverb=.55, lfosource=0, destination=PIT, rate=4.8, amount=.1),
 p("Ghost Radio", waveform=SQR, filter=1800, filtertype=BP, resonance=3, attack=800, sustain=.8, release=1500, monopoly=0, lfosource=4, destination=FLT, rate=1.5, amount=.3, reverb=.6),
 p("Cold War Strings", waveform=SAW, unison=7, detune=30, unisonwidth=1, filter=2800, attack=1000, decay=800, sustain=.85, release=1700, monopoly=0, chorus=.4, reverb=.5),
 p("Wasteland", waveform=SQR, filter=800, attack=900, sustain=.8, release=2000, monopoly=0, lfosource=3, destination=AMP, rate=.25, amount=.4, reverb=.7, delaymix=.2, time=620, feedback=.55),
 p("Evac Dawn", waveform=SAW, unison=5, detune=15, filter=3000, attack=1400, decay=900, sustain=.85, release=2200, monopoly=0, chorus=.4, reverb=.6, pitch=12, volume=.9)])
bank("FX & Percussion", [
 p("Explosion", waveform=NOISE, filter=4500, filtertype=LP, decay=1800, sustain=0, release=800, reverb=.45, distortion=.3, monopoly=0, volume=.9),
 p("Gunshot", waveform=NOISE, filter=6000, decay=140, sustain=0, release=100, reverb=.25, distortion=.4, monopoly=0, volume=.9),
 p("Helicopter", waveform=NOISE, filter=1400, filtertype=BP, resonance=2, attack=60, decay=800, sustain=.9, release=500, lfosource=1, destination=AMP, rate=11, amount=.7, reverb=.3, monopoly=0),
 p("Air Raid", waveform=SAW, filter=5000, attack=100, decay=900, sustain=.9, release=900, glide=250, lfosource=3, destination=PIT, rate=.45, amount=.7, reverb=.35),
 p("Radio Chatter", waveform=NOISE, filter=2600, filtertype=BP, resonance=6, decay=300, sustain=.5, release=200, lfosource=1, destination=AMP, rate=9, amount=.5, delaymix=.15, monopoly=0),
 p("Tank Rumble", waveform=NOISE, filter=180, resonance=2, attack=40, decay=900, sustain=.9, release=700, lfosource=0, destination=AMP, rate=7, amount=.3, distortion=.2, monopoly=0),
 p("Laser", waveform=SQR, filter=7000, decay=260, sustain=0, release=150, glide=300, pitch=12, delaymix=.2, time=200, reverb=.25),
 p("Impact", waveform=SINE, filter=700, decay=700, sustain=0, release=400, distortion=.5, reverb=.5, glide=90, volume=1.1),
 p("Static Burst", waveform=NOISE, filter=9000, filtertype=HP, decay=220, sustain=0, release=150, delaymix=.25, time=130, feedback=.5, monopoly=0),
 p("Kick 808", waveform=SINE, filter=200, envdepth=4, fdecay=60, fsustain=0, decay=420, sustain=0, release=120, distortion=.2, resonance=1.5, volume=1.1)])

# expansion VOL.1 : 10 presets au format RuntimePreset (JSON)
def rp(name, cat, **k):
    d = dict(name=name, category=cat); d.update(k); return d
EXP = [
 rp("Vol1 Sub Drop", "Bass", oscWaveform=0, filterCutoffHz=500, attackMs=2, decayMs=2400, sustainLevel=0, releaseMs=900, glideMs=120, distortionDriveDb=3, distortionMix=.3, reverbMix=.1),
 rp("Vol1 Grunt", "Bass", masterVolume=.75, oscWaveform=2, filterCutoffHz=900, filterResonance=3, filterEnvDepthOct=2, decayMs=500, sustainLevel=.7, distortionDriveDb=12, distortionMix=.6),
 rp("Vol1 Convoy", "Bass", oscWaveform=2, unisonVoices=3, unisonDetune=15, unisonWidth=.7, filterCutoffHz=1100, decayMs=700, sustainLevel=.8, chorusMix=.3, chorusRateHz=.8),
 rp("Vol1 Signal Flare", "Lead", oscWaveform=1, filterCutoffHz=3500, attackMs=10, decayMs=500, sustainLevel=.7, delayTimeMs=375, delayFeedback=.4, delayMix=.2, reverbMix=.3),
 rp("Vol1 Command Keys", "Keys", oscWaveform=3, filterCutoffHz=2800, decayMs=800, sustainLevel=.3, releaseMs=400, chorusMix=.3, reverbMix=.3, velocity=.7),
 rp("Vol1 Barracks Pad", "Pad", oscWaveform=2, unisonVoices=5, unisonDetune=20, unisonWidth=.9, filterCutoffHz=1800, attackMs=900, decayMs=900, sustainLevel=.85, releaseMs=1600, chorusMix=.4, reverbMix=.5),
 rp("Vol1 Drone Strike", "FX", oscWaveform=4, filterCutoffHz=700, filterResonance=3, attackMs=300, decayMs=1500, sustainLevel=.6, releaseMs=1200, reverbMix=.5),
 rp("Vol1 Pluck Squad", "Pluck", oscWaveform=1, filterCutoffHz=1000, filterResonance=2.5, filterEnvDepthOct=3, filterDecayMs=150, filterSustain=0, decayMs=240, sustainLevel=0, releaseMs=160),
 rp("Vol1 Rust Lead", "Lead", oscWaveform=2, filterCutoffHz=3000, glideMs=60, decayMs=400, sustainLevel=.75, distortionDriveDb=9, distortionMix=.4, delayMix=.15, delayTimeMs=300),
 rp("Vol1 Ghost Comm", "FX", oscWaveform=1, filterCutoffHz=2200, filterType=1, filterResonance=4, decayMs=400, sustainLevel=.6, lfoRateHz=7, lfoAmount=.3, lfoDestination=1, reverbMix=.4)]

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>', '#include <utility>',
           'namespace Wz {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
           'inline const std::vector<PresetDef>& factoryPresets() { static const std::vector<PresetDef> L = {']
    n = 0
    for b, rows in BANKS:
        assert len(rows) == 10, b
        for name, k in rows:
            for key in k: assert key in D, (name, key)
            kv = dict(D); kv.update(k)
            items = ', '.join('{"%s", %sf}' % (x, repr(float(kv[x]))) for x in D)
            out.append('  {"%s", "%s", {%s}},' % (b, name, items)); n += 1
    out.append('}; return L; } }')
    assert n == 60
    return '\n'.join(out) + '\n'

if __name__ == '__main__':
    open(sys.argv[1], 'w').write(emit())
    json.dump(dict(format="warzone808-expansion", name="Expansion Vol.1", author="DJBILBOX BEATS / Humpire", presets=EXP),
              open(sys.argv[2], 'w'), indent=1)
