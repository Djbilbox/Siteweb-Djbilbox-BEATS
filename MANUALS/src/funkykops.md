# FUNKYKOPS

Funk, disco and G-Funk synthesizer: clavinet, electric piano, synth bass, brass, strings, lead, guitar and talkbox sounds from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

FUNKYKOPS is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Clavinet (a plucked-string model with a pickup "bite"), Rhodes (tine and bell partials with velocity "bark"), Synth Bass (a saw, a sub-octave sine and a square blend), Brass (three detuned saws), Strings (five detuned saws), Lead (saw plus square), Guitar (a short plucked string) and Talkbox (saws through vowel formants).

Five small character knobs (VOWEL, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb.

FUNKYKOPS targets Funk, Disco, G-Funk, Hip-Hop and Afrobeat productions: wah clavs, silky electric pianos, rubbery bass lines, horn stabs, string pads, whistle leads and robot-voice talkbox hooks. Most keys, brass, string and guitar presets load in Poly mode with up to 16 voices. The Synth Bass bank and most lead presets load in Mono mode for clean one-note lines.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **FUNKYKOPS** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **FUNKYKOPS - Init Patch**: the Clavinet engine in Poly mode.
3. Click the **Presets** button at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 36 to 96. Presets with TRANSPOSE at -12 (Bootsy Clav, Lowrider Keys, Bus Driver) sound one octave lower than the key you play; Bell EP, Trumpet Call, Closing Credits, Disco Ball Bell and Laser Zap are one octave higher.
5. Shape the sound with the main controls:
    - **CUTOFF**: the filter cutoff. Turn it down for a darker, rounder sound.
    - **BRIGHT**: the brightness of the selected engine.
    - **DECAY** (AMP ENVELOPE) and **SUSTAIN**: how long a note rings.
6. Adjust **VOLUME** to set the level, then click **Save** to store your own version.

## Interface

The window opens at 1000 x 746 pixels. It has a fixed size.

**Preset display (top center).** From left to right: the **Presets** button (preset menu), the current preset name, the **<** and **>** arrows and the **Save** button. The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOX BEATS logo.

**Center.** The camel artwork, the FUNKYKOPS title and "By Humpire". The speaker grilles are decorative.

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. The **TRANSPOSE** knob sits on the same row; it is the global transposition (-12 to +12 semitones) and is not an LFO control.

**AMP ENVELOPE (left, below the LFO).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Center row.** Two small character knobs, **VOWEL** and **BODY**, then the **GLIDE** and **CUTOFF** knobs, then three small character knobs, **BRIGHT, WIDTH** and **TEXTURE**.

**FILTER ENV (top right).** Two sliders, **DEPTH** (how far the filter envelope opens the cutoff) and **DECAY** (its decay and release time), followed by the **RESO** (filter resonance) and **VEL** (velocity sensitivity) sliders. RESO and VEL are separated from the filter envelope by a thin line because they are not part of it.

**Effects and level (right).** **VOLUME** (top), **REVERB, CHORUS, DISTORTION**, then **DELAY MIX, DLY TIME, FEEDBACK**.

**Menu row.** Five drop-down menus, each with a caption below it: **WAVEFORM** (the engine), **LFO SOURCE** (LFO shape), **LFO DESTINATION**, **FILTER TYPE** and **MONO / POLY**. Each menu shows its current value.

**Wheels (bottom left).** **Pitch Bend** springs back to the center when you release it. **Mod Wheel** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard covers MIDI notes 36 to 96 (five octaves, 61 keys). The key labels run from C2 to C7, with middle C (MIDI 60) labeled C4; in FL Studio naming, where middle C is C5, the same range is C3 to C8. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

FUNKYKOPS exposes 32 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Control | Range | Default | What it does |
|---|---|---|---|
| WAVEFORM (Waveform menu) | Clavinet, Rhodes, Synth Bass, Brass, Strings, Lead, Guitar, Talkbox | Clavinet | Selects the synthesis engine. |
| VOWEL (Vowel / Attack) | 0 to 1 | 0.5 | Character control 1. Moves the vowel of the Talkbox engine; no effect on the other engines. |
| BODY (Body / Resonance) | 0 to 1 | 0.5 | Character control 2. Engine-dependent (see the table below). |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width / Detune) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune or sub level). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (pickup bite, noise, square blend...). |
| CUTOFF (Filter Cutoff) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RESO (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| DEPTH (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff by up to 5 octaves. At 0 the filter envelope has no effect. |
| DECAY, FILTER ENV (Filter Decay) | 0.02 to 2 s | 0.3 s | Decay and release time of the filter envelope. The filter envelope attacks in 2 ms and has a sustain level of zero. |
| VEL (Velocity Sensitivity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY, AMP ENVELOPE (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Output level of each voice, before the effects. |
| GLIDE (Glide) | 0 to 2 s | 0 | Portamento. The value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The mod wheel adds to this value. |
| LFO SOURCE (LFO Source menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION (Destination menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| MONO / POLY (Mono/Poly menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (fixed rate). |
| DELAY MIX (Delay Mix) | 0 to 1 | 0 | Level of the ping-pong delay. |
| DLY TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Mod Wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | VOWEL | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Clavinet | No effect | String sustain (longer ring) | String brightness (damping) | No effect | Pickup bite (saturation) |
| Rhodes | No effect | Decay length of the tine and bell partials | Bell "bark", stronger at high velocity | No effect | Attack noise |
| Synth Bass | No effect | No effect | Built-in low-pass opening | Sub-octave sine level | Blend from saw to square |
| Brass | No effect | No effect | Built-in low-pass opening | Detune of the three saws | Breath noise |
| Strings | No effect | No effect | Built-in low-pass opening | Detune of the five saws | No effect |
| Lead | No effect | No effect | Built-in low-pass opening | Detune | No effect |
| Guitar | No effect | String sustain (longer ring) | String brightness (damping) | No effect | No effect |
| Talkbox | Vowel (formant position) | Formant level | Brightness of the carrier | Detune of the three saws | Noise |

## Factory presets

FUNKYKOPS includes 70 factory presets in 7 banks of 10. 47 load in Poly mode and 23 in Mono mode. Mono presets are marked with an asterisk (*) below.

**Clavinet & Funk Keys:** Chicken Scratch, Disco Clav, Wah Patrol, Afro Clav, Stevie Stab, Funk Pickup, Bootsy Clav*, Siren Clav, Dusty Clav, Badge Clav.

**Rhodes & EP:** Rhodes Suitcase, Silk Rhodes, Bark Rhodes, Midnight EP, Disco EP, Tremolo EP, Slow Jam Keys, Wurly Bite, Bell EP, Lowrider Keys.

**Synth Bass (all Mono):** Funk Moog*, Dre Bass*, Rubber Bass*, Sub Cop*, Slap Synth*, G-Funk Whine Bass*, Disco Octave*, Acid Funk*, Bus Driver*, Mean Streets*.

**Brass & Stabs:** Horn Section, Funk Stab, Police Horns, Disco Brass, Swell Brass, Soft Flugel, Big Band Hit, Synth Brass 80, Sax Pad, Trumpet Call.

**Strings & Disco:** Disco Strings, Philly Strings, Cop Show Strings, Mirror Ball Pad, Slow Jam Pad, Staccato Strings, Lush Ensemble, String Machine, Velvet Pad, Closing Credits.

**Leads & Talkbox:** Funk Lead*, Chrome Lead*, G-Funk Whistle*, Talkbox Wah*, Robot Cop*, Vocoder Cruise, Squeal Lead*, Pop Lock Lead*, Midnight Cruiser*, Disco Solo*.

**Guitar & FX:** Chank Guitar, Funk Strum, Wah Guitar, Clean Pluck, Jazz Box, Muted Pick, Siren FX*, Radio Dispatch*, Disco Ball Bell, Laser Zap*.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click **Save** to save the current sound as a `.fkpreset` file. The default folder is `DJBILBOX BEATS/FUNKYKOPS/Presets` inside your Documents folder. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .fkpreset file...** in the same menu to load a file from another location. The preset display shows the file name. User presets store the sound parameters; the wheels are not stored.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Funk clav.** Start from Chicken Scratch or Stevie Stab. BRIGHT sets the string brightness and TEXTURE the pickup bite. Set VEL high (around 80%) so ghost notes and accents in the Piano roll sound different.
2. **Wah.** Wah Patrol uses a Low Pass filter with a low CUTOFF, DEPTH at 50% and a sine LFO on the filter. Raise DEPTH for a deeper quack on every note, or lower the LFO RATE for a slow sweep.
3. **Electric piano.** On the Rhodes engine, BRIGHT sets the bark that appears at high velocity and BODY sets how long the notes ring. Add CHORUS and REVERB for a wide, slow-jam sound (Midnight EP, Slow Jam Keys).
4. **G-Funk whine.** Use G-Funk Whine Bass for the bass and G-Funk Whistle for the lead. Both rely on GLIDE and a small LFO amount on pitch. Play them legato.
5. **Bass lines.** The Synth Bass bank loads in Mono mode. Raise DEPTH and shorten the filter DECAY for a rubbery pluck. Raise WIDTH to bring up the sub-octave sine, and use TEXTURE to move from saw to square.
6. **Horns and strings.** The Brass & Stabs bank plays chords in Poly mode. Raise the ATTACK slider for swells (Swell Brass) or set SUSTAIN to zero for stabs (Funk Stab). On the Strings engine, raise WIDTH and CHORUS for a wider ensemble (Lush Ensemble).
7. **Talkbox.** Choose the Talkbox engine and move VOWEL during the note, or automate it, to make the synth talk. Robot Cop and Talkbox Wah are good starting points.
8. **Slides.** With GLIDE above zero, every new note slides from the previous one, even if the notes do not overlap. For slides on selected notes only, keep GLIDE at 0 and automate it up just before those notes.

## MIDI, automation and performance

**Notes and channels.** FUNKYKOPS responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VEL slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Mod Wheel. When both are used, the higher value wins. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead. CC121 (reset all controllers) sets the CC1 value back to zero.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 32 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend, cutoff, distortion and delay changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** Effects set to zero are bypassed and cost no CPU. Mono mode and short release times keep the voice count low.

## FAQ

**FUNKYKOPS does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **FUNKYKOPS** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Check VOLUME, and check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. Some presets use a low CUTOFF on purpose. Presets with TRANSPOSE -12 play one octave lower than the key. For notes outside the on-screen keyboard range (MIDI 36 to 96), play from a MIDI keyboard or the Piano roll.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The MONO / POLY menu is set to Mono, which is the default for the Synth Bass bank and most lead presets. Switch it to Poly (up to 16 voices).

**The VOWEL knob does nothing.**
VOWEL only acts on the Talkbox engine. Select Talkbox in the WAVEFORM menu.

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.
