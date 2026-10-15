# FUNKYKOPS

Funk, disco and G-Funk keys, basses, brass, strings and talkbox leads from a pure synthesis engine.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

FUNKYKOPS is a 16-voice synthesizer for the classic sounds of funk, disco and West Coast G-Funk. It has eight synthesis engines: Clavinet (percussive plucked keys), Rhodes (electric piano), Synth Bass, Brass, Strings, Lead, Guitar and Talkbox. It uses no samples. Every note is generated in real time and responds to the timbre knobs, the filter and its envelope.

Each engine runs through a resonant multimode filter with a fast decay envelope, the classic tool for funky bass "quacks", clav wah and brass stabs. A key-synced LFO, glide and a Mono mode cover whining G-Funk leads. The effects chain adds distortion, chorus, a stereo ping-pong delay and reverb. A soft limiter at the output keeps the level under control.

FUNKYKOPS is built for G-Funk, Funk, Disco and Boogie, Hip-Hop, Nu-Disco and Deep House. Use the Clavinet and Guitar for rhythm parts, the Rhodes for chords, the Synth Bass for bass lines, the Brass and Strings for stabs and pads, and the Lead and Talkbox engines for melodies.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **FUNKYKOPS** (manufacturer **DJBILBOX BEATS**). You can also drag it from **Plugin database > Generators**.
2. The interface opens on **FUNKYKOPS - Init Patch**.
3. Click **Presets** at the left of the preset display. Open a bank (for example **Synth Bass**) and pick a preset. You can also step through all 70 presets with the **<** and **>** buttons.
4. Play notes from your MIDI keyboard, or draw them in the **Piano roll**.
5. Turn **FILTER** to make the sound darker or brighter.
6. Turn **BRIGHT** to change the brightness of the engine itself.
7. Turn **CHORUS** for width and a vintage shimmer.
8. Click **Save** to store your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from half size to double size. The proportions stay fixed.

**Preset display (top center).** From left to right: the **Presets** button opens the preset menu, the center shows the current preset name, **<** and **>** load the previous and next presets, and **Save** saves a user preset. The preset menu lists the seven factory banks as submenus, a **User** submenu with your saved presets, and an **Open a .fkpreset file...** item.

**Cassette (top left).** Click it to see information about upcoming expansion packs (**EXPANSION VOL.1**) and the link to the DJBILBOX BEATS store.

**Logo (top right).** The DJBILBOX BEATS logo.

**Emblem and speaker grilles (center).** The camel emblem sits between two decorative speaker grilles. The FUNKYKOPS title and "By Humpire" are below the emblem.

**LFO group (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH** sits in the same row but is the global transpose of the instrument.

**Amplitude envelope (left, below the LFO).** Four vertical sliders: **ATTACK**, **DECAY**, **SUSTAIN** and **RELEASE** of the volume envelope.

**FILTER ENV + VELOCITY group (top right).** Four vertical sliders: **ENV** (filter envelope depth), **DEC** (filter envelope decay), **RES** (filter resonance) and **VEL** (velocity sensitivity of the volume). **VOLUME** is to their right.

**Center row.** From left to right: the small **VOWEL** and **BODY** knobs, **GLIDE**, **FILTER** (cutoff), then the small **BRIGHT**, **WIDTH** and **TEXTURE** knobs. The five small knobs shape the selected engine.

**Effects (right).** **REVERB**, **CHORUS** and **DISTORTION** on the first row. **DELAY** (level), **TIME** and **FEEDBACK** on the second row.

**Menu row.** Five drop-down menus. Each one shows its current value. From left to right: engine (Waveform), LFO shape (LFO Source), LFO target (Destination), filter mode (Filter Type) and voice mode (Poly / Mono).

**Wheels (bottom left, "Pitch Bend / Mod").** The left wheel bends the pitch and springs back to center when you release the mouse. The right wheel is the modulation wheel and stays where you leave it.

**On-screen keyboard (bottom).** Click a key to audition the sound. It also lights up incoming MIDI notes within its range. In version 1.0 the keyboard covers the lowest part of the MIDI range (keys labeled C-2 to about D1, MIDI notes 0 to 38). Use it to check bass notes. Play melodies and chords from a MIDI keyboard or the Piano roll.

**Using the controls.** Drag a knob up/down or left/right. Drag a slider vertically. The mouse wheel adjusts in small steps. Double-click to return to the default value. Hold Ctrl or Alt while you drag for velocity-sensitive, finer movement. A small bubble shows the current value while you hover or drag.

## Parameters

Names in parentheses are the parameter names shown in FL Studio for automation, when they differ from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Engine menu (Waveform) | Clavinet, Rhodes, Synth Bass, Brass, Strings, Lead, Guitar, Talkbox | Clavinet | Selects the synthesis engine. See the engine table below. |
| LFO shape menu (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO waveform. Sample&Hold picks a new random value once per cycle. |
| LFO target menu (Destination) | Pitch, Filter, Amp, Timbre | Pitch | Where the LFO goes. Pitch: up to ±1 semitone. Filter: up to ±3 octaves of cutoff (auto-wah). Amp: tremolo, up to 50% volume dip. Timbre: moves all five timbre knobs together, up to ±0.5. |
| Filter mode menu (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | 12 dB/octave state-variable filter mode. Band Pass gets a small level boost. |
| Voice mode menu (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes. Mono: each new note releases the previous one. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. The LFO restarts at each new note. It is not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The modulation wheel (or MIDI CC1) adds up to 0.8 on top. |
| PITCH (Pitch) | -12 to +12 semitones, continuous | 0 | Global transpose. Values between semitones detune the whole instrument. |
| GLIDE (Glide) | 0 to 2 s | 0 (off) | Portamento. Above 0, each new note slides from the pitch of the previous note, in Poly and Mono. |
| FILTER (Filter) | 40 Hz to 18 kHz | 9 kHz | Filter cutoff frequency. The cutoff follows the played note at 25% (key tracking). |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance (emphasis at the cutoff). |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1, the envelope opens the cutoff by up to 5 octaves. |
| DEC (Filter Decay) | 0.02 to 2 s | 0.3 s | Filter envelope decay. The envelope has a fixed 2 ms attack and no sustain; it falls back over the DEC time, also after note-off. |
| VEL (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the volume. 0 = every note at full level. 1 = full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Volume envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Volume envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Volume envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Volume envelope release time after note-off. |
| VOWEL (Vowel / Attack) | 0 to 1 | 0.5 | Engine timbre control 1. Active on the Talkbox engine only. See the engine table. |
| BODY (Body / Resonance) | 0 to 1 | 0.5 | Engine timbre control 2. See the engine table. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Engine timbre control 3. See the engine table. |
| WIDTH (Width / Detune) | 0 to 1 | 0.3 | Engine timbre control 4. See the engine table. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Engine timbre control 5. See the engine table. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Instrument level, before the effects. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Soft saturation. Drive rises up to 15x; the level is compensated. |
| CHORUS (Chorus) | 0 to 1 | 0 | Chorus mix (fixed 0.7 Hz modulation). |
| DELAY (Delay) | 0 to 1 | 0 | Level of the stereo ping-pong delay. |
| TIME (Delay Time) | 0.05 to 1 s | 0.35 s | Delay time in seconds. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Delay repeats. Higher values give more echoes. |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. The room also grows larger as you turn it up. |
| Pitch bend wheel (Pitch Bend) | -1 to +1 | 0 | Bends all notes by up to ±2 semitones. Adds to the MIDI pitch wheel. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | Adds LFO depth (up to +0.8). The higher of this wheel and MIDI CC1 is used. |

Effects order: Distortion, Chorus, Delay, Reverb, then the output soft limiter. Each effect is bypassed while its knob is at zero.

**What the timbre knobs do on each engine**

| Engine | VOWEL | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Clavinet: plucked string with pickup bite | No effect | String sustain | String brightness (damping) | No effect | Pickup bite (saturation) |
| Rhodes: tine and bell partials, natural decay | No effect | Decay length | Bell "bark" amount (grows with velocity) | No effect | Hammer attack noise |
| Synth Bass: saw/square with a sub-octave sine | No effect | No effect | Oscillator brightness | Sub-octave level | Saw-to-square blend |
| Brass: three detuned saws | No effect | No effect | Brightness | Detune | Breath noise |
| Strings: five-saw ensemble | No effect | No effect | Brightness | Detune (ensemble width) | No effect |
| Lead: two saws plus a square one octave below | No effect | No effect | Brightness | Detune | No effect |
| Guitar: short plucked string | No effect | String sustain | String brightness (damping) | No effect | No effect |
| Talkbox: three saws through two vowel formants | Vowel (from a dark "oo" to an open vowel) | Formant strength | Brightness | Detune | Breath noise |

## Factory presets

FUNKYKOPS includes 70 factory presets in 7 banks of 10. Loading a factory preset resets every parameter before applying the preset. The pitch bend and modulation wheels are not changed.

**Clavinet & Funk Keys**
Chicken Scratch, Disco Clav, Wah Patrol, Afro Clav, Stevie Stab, Funk Pickup, Bootsy Clav, Siren Clav, Dusty Clav, Badge Clav.

**Rhodes & EP**
Rhodes Suitcase, Silk Rhodes, Bark Rhodes, Midnight EP, Disco EP, Tremolo EP, Slow Jam Keys, Wurly Bite, Bell EP, Lowrider Keys.

**Synth Bass**
Funk Moog, Dre Bass, Rubber Bass, Sub Cop, Slap Synth, G-Funk Whine Bass, Disco Octave, Acid Funk, Bus Driver, Mean Streets.

**Brass & Stabs**
Horn Section, Funk Stab, Police Horns, Disco Brass, Swell Brass, Soft Flugel, Big Band Hit, Synth Brass 80, Sax Pad, Trumpet Call.

**Strings & Disco**
Disco Strings, Philly Strings, Cop Show Strings, Mirror Ball Pad, Slow Jam Pad, Staccato Strings, Lush Ensemble, String Machine, Velvet Pad, Closing Credits.

**Leads & Talkbox**
Funk Lead, Chrome Lead, G-Funk Whistle, Talkbox Wah, Robot Cop, Vocoder Cruise, Squeal Lead, Pop Lock Lead, Midnight Cruiser, Disco Solo.

**Guitar & FX**
Chank Guitar, Funk Strum, Wah Guitar, Clean Pluck, Jazz Box, Muted Pick, Siren FX, Radio Dispatch, Disco Ball Bell, Laser Zap.

**User presets.** **Save** stores the current sound as a `.fkpreset` file in `Documents\DJBILBOX BEATS\FUNKYKOPS\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .fkpreset file...** to load a preset from another folder. Your sound is also saved inside your FL Studio project.

## Production tips

- **G-Funk whine lead.** Load **G-Funk Whistle**: Lead engine, Mono, glide 0.1 and a gentle pitch vibrato. Play it high on the keyboard and push the modulation wheel on long notes for a deeper vibrato. Raise GLIDE for longer slides between notes.
- **Funk bass quack.** On the Synth Bass presets, the sound comes from the filter: a low FILTER, a high ENV and a short DEC give the pluck. Raise RES toward 0.5 for more squelch, as in **Rubber Bass** and **Acid Funk**. Raise **WIDTH** for more sub-octave weight.
- **Clav rhythm parts.** **Chicken Scratch** and **Stevie Stab** react strongly to velocity (VEL 0.8 to 0.9). Program short sixteenth notes with varied velocities. Raise **TEXTURE** for more pickup bite.
- **Auto-wah in time.** **Wah Patrol** and **Wah Guitar** use the LFO on the filter. RATE is in Hz: for one wah cycle per beat, set RATE to BPM / 60 (1.67 Hz at 100 BPM). The LFO restarts on each note, so every note gets the same wah shape.
- **Rhodes chords for Hip-Hop and Deep House.** Start from **Rhodes Suitcase** or **Silk Rhodes**. Play harder and raise **BRIGHT** for more bark. **Tremolo EP** uses the LFO on Amp for the classic tremolo. Add CHORUS around 0.3.
- **Disco strings.** **Disco Strings** and **Philly Strings** stack five detuned saws. **WIDTH** sets the ensemble spread. Double the line one octave up with the same preset for the classic disco string sound.
- **Talking lines with the Talkbox engine.** Automate **VOWEL** in FL Studio to make a line "talk". Or set Destination to Timbre and let the LFO move the vowel, as in **Talkbox Wah**. Keep in mind that Timbre moves all five timbre knobs at once.
- **Delay in time with the beat.** TIME is set in seconds. Quarter note = 60 / BPM. Eighth note = 30 / BPM. Dotted eighth = 45 / BPM. At 96 BPM, an eighth note is 0.31 s.

## MIDI, automation and performance

- **Notes.** FUNKYKOPS responds to all MIDI notes (0 to 127) on all MIDI channels, tuned to equal temperament with A4 = 440 Hz. Middle C is MIDI note 60 (C5 in FL Studio).
- **Polyphony.** 16 voices in Poly mode. When you play more than 16 notes, the oldest note is stolen; the lowest and highest held notes are protected. In Mono mode, each new note releases the previous one (its release tail still sounds). Releasing a note does not bring back an earlier held note.
- **Velocity.** Velocity changes the volume according to the VEL slider. On the Rhodes engine, velocity also adds bell "bark".
- **Pitch bend.** ±2 semitones. The MIDI pitch wheel and the on-screen wheel add together.
- **Modulation (CC1).** Adds up to 0.8 to the LFO AMOUNT. The higher of MIDI CC1 and the on-screen wheel is used.
- **Sustain pedal (CC64).** Supported. Aftertouch and other MIDI CCs are ignored. Use host automation for everything else.
- **Automation in FL Studio.** Every parameter in the table can be automated. Move a control on the FUNKYKOPS panel, then choose **Tools > Last tweaked > Create automation clip**. To use a hardware knob, choose **Tools > Last tweaked > Link to controller** and move the knob. Volume, pitch, bend and cutoff changes are smoothed to avoid zipper noise.
- **Latency.** 0 samples. No delay compensation is needed. The plugin reports a 12-second tail to the host so that delay and reverb tails are not cut when you render.
- **CPU.** FUNKYKOPS uses no samples and streams nothing from disk. The CPU load grows with the number of sounding notes. The five-saw Strings engine is the heaviest. Effects at zero are bypassed and use no CPU. Long release times keep voices active longer.
- **Sample rates.** 44.1, 48, 88.2 and 96 kHz.

## FAQ

**I hear no sound.**
Check that the FUNKYKOPS channel is routed to a Mixer track and that the channel is not muted. Check VOLUME. With Low Pass selected, a very low FILTER setting can silence the sound. With High Pass selected, a high FILTER setting removes bass presets. Finally, remember that the on-screen keyboard plays very low notes in version 1.0: test with a MIDI keyboard or the Piano roll.

**FUNKYKOPS does not appear in FL Studio.**
FUNKYKOPS is a 64-bit VST3 plugin. Check that `FUNKYKOPS.vst3` is in `C:\Program Files\Common Files\VST3`. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**. Look for it under **Generators**, manufacturer **DJBILBOX BEATS**.

**Can I sing into the Talkbox engine?**
No. The Talkbox engine is a synthesizer: it shapes saw waves with two vowel formants. FUNKYKOPS has no audio input. Change the vowel with the VOWEL knob, its automation, or the LFO set to Timbre. VOWEL only acts on the Talkbox engine. Some other timbre knobs are also inactive on certain engines; see the engine table in the Parameters section.

**The sound gets quieter when I turn REVERB fully down.**
In version 1.0, the reverb stage also lifts the dry signal by about 6 dB while it is active. At 0 the reverb is bypassed, so the overall level drops. For a dry sound at the usual level, leave REVERB at a very low value such as 0.01, or compensate with VOLUME.

**Are the Clavinet and Rhodes engines samples of the original instruments?**
No. They are synthesized models that evoke those instruments. Clavinet, Rhodes, Moog and Wurlitzer are trademarks of their respective owners. They are used only to describe a sound character. DJBILBOX BEATS is not affiliated with these companies.
