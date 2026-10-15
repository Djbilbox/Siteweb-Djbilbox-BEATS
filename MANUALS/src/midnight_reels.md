# MIDNIGHT REELS

Film noir and late-night jazz: vibes, electric piano, muted horns, upright bass, organ, strings and celesta.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

MIDNIGHT REELS is a 16-voice synthesizer for the sound of a smoky jazz club and classic film noir scores. It has eight synthesis engines: Vibes (vibraphone with a motor tremolo), Reel EP (electric piano with a velocity "bark"), Muted Trumpet, Tenor Sax, Upright Bass (plucked string model), Organ (drawbar-style), Noir Strings (detuned string ensemble with tape flutter) and Celesta. It uses no samples. Every note is generated in real time and responds to five timbre knobs, a filter and two envelopes.

Each voice runs through a resonant multimode filter with its own envelope and a per-voice LFO. The output then goes through an effects chain: distortion, chorus, stereo delay and reverb. A soft limiter at the output keeps the level under control.

MIDNIGHT REELS is built for Jazz-Hop and lo-fi Hip-Hop, Boom Bap, Soul and G-Funk (organ and electric piano), dark Trap and Drill (noir strings, celesta, music box) and Deep House chords. It also fits cinematic intros, detective themes and podcast beds. Use the Reel EP and Organ banks for chords, the Trumpet & Sax bank for hooks, the Upright Bass bank for walking lines, and the Vibes, Celesta and Strings banks for atmosphere.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **MIDNIGHT_REELS** (manufacturer **DJBILBOX BEATS**). You can also drag it from **Plugin database > Generators**.
2. The interface opens on **MIDNIGHT REELS - Init Patch** (Vibes engine).
3. Click the folder icon at the left of the preset display. Open a bank (for example **Reel EP**) and pick a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play notes from your MIDI keyboard, or draw them in the **Piano roll**.
5. Turn **FILTER** to make the sound darker or brighter.
6. Turn **BODY** (center, small knob) to change the body of the engine. On Vibes, Reel EP and Celesta it sets how long the notes ring.
7. Turn **REVERB** to place the sound in a small room or a large hall.
8. Click the disk icon at the right of the preset display to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from half size (500 x 373) to double size (2000 x 1492). The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon opens the preset menu, **<** loads the previous preset, the center shows the current preset name, **>** loads the next preset, and the disk icon saves a user preset. The arrows wrap around from the last preset to the first. The preset menu lists the seven factory banks as submenus, a **User** submenu with your saved presets, and an **Open a .mrpreset file...** item.

**EXPANSION cassette (top left).** Click it to see information about upcoming expansion packs (**EXPANSION VOL.1**) and the link to the DJBILBOX BEATS store. Version 1.0 contains no expansion content.

**Logo (top right).** The DJBILBOX BEATS name and logo.

**Emblem (center).** The camel detective, in hat and trench coat, sits between two decorative speaker grilles. The MIDNIGHT REELS title and "By Humpire" are below it.

**Filter envelope, VEL and VOLUME (right, below the logo).** Five vertical sliders: **A**, **D** and **R** are the attack, decay and release of the filter envelope. **VEL** sets the velocity sensitivity of the volume. **VOLUME** is the instrument level.

**LFO and transpose (left, below the cassette).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH** sits in the same row but is the global transpose of the instrument, not an LFO control.

**Volume envelope (left, below RATE/AMOUNT/PITCH).** Four vertical sliders: **ATTACK**, **DECAY**, **SUSTAIN** and **RELEASE**.

**Center controls.** On the left, four small knobs in a square: **TONE**, **BODY**, **BRIGHT** and **WIDTH**. In the middle, **GLIDE** and **FILTER** (cutoff). On the right, three small knobs: **TEXTURE**, **RES** (filter resonance) and **ENV** (filter envelope depth). TONE, BODY, BRIGHT, WIDTH and TEXTURE shape the selected engine.

**Effects (right).** **REVERB**, **CHORUS** and **DISTORTION** on the upper row. **DELAY** (level), **TIME** and **FEEDBACK** on the lower row.

**Menu row.** Five drop-down menus across the panel. Each one shows its current value. From left to right: the engine (**Waveform**, default "Vibes"), the LFO shape (**LFO Source**, default "Sine"), the LFO target (**Destination**, default "Pitch"), the filter mode (**Filter Type**, default "Low Pass") and the voice mode (**Mono/Poly**, default "Poly").

**Wheels (bottom left).** The left wheel bends the pitch and springs back to center when you release the mouse. The right wheel is the modulation wheel and stays where you leave it.

**On-screen keyboard (bottom).** Click a key to audition the sound. It also lights up incoming MIDI notes within its range. In version 1.0 the keyboard covers the lowest part of the MIDI range (keys labeled C-2 to E1, MIDI notes 0 to 40). Use it to check bass notes. Play melodies and chords from a MIDI keyboard or the Piano roll.

**Using the controls.** Drag a knob up/down or left/right. Drag a slider vertically. The mouse wheel adjusts in small steps. Double-click to return to the default value. Hold Ctrl or Alt while you drag for velocity-sensitive movement. A small bubble shows the current value while you hover or drag.

## Parameters

Names in parentheses are the parameter names shown in FL Studio for automation, when they differ from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Engine menu (Waveform) | Vibes, Reel EP, Muted Trumpet, Tenor Sax, Upright Bass, Organ, Noir Strings, Celesta | Vibes | Selects the synthesis engine. See the engine table below. |
| LFO shape menu (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO waveform. Sample&Hold picks a new random value once per cycle. |
| LFO target menu (Destination) | Pitch, Filter, Amp, Timbre | Pitch | Where the LFO goes. Pitch: up to ±1 semitone. Filter: up to ±3 octaves of cutoff. Amp: tremolo, up to 50% volume dip. Timbre: moves all five timbre knobs together, up to ±0.5. |
| Filter mode menu (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | 12 dB/octave state-variable filter mode. Band Pass gets a small level boost. |
| Voice mode menu (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes. Mono: each new note releases the previous one. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. The LFO restarts at each new note. It is not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The modulation wheel (or MIDI CC1) adds up to 0.8 on top. |
| PITCH (Pitch) | -12 to +12 semitones, continuous | 0 | Global transpose. Values between semitones detune the whole instrument. |
| GLIDE (Glide) | 0 to 2 s | 0 (off) | Portamento. Above 0, each new note slides from the pitch of the previous note, in Poly and Mono. |
| FILTER (Filter) | 40 Hz to 18 kHz | 9 kHz | Filter cutoff frequency. The cutoff follows the played note at 25% (key tracking). |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance (emphasis at the cutoff). |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1, the envelope opens the cutoff by up to 5 octaves. |
| A (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| D (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. The filter envelope has no sustain stage: it always decays back to zero. |
| R (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time, used when you release a note before the decay has finished. |
| VEL (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the volume. 0 = every note at full level. 1 = full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Volume envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Volume envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Volume envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Volume envelope release time after note-off. |
| TONE (Tone) | 0 to 1 | 0.5 | Engine timbre control 1. See the engine table. |
| BODY (Body) | 0 to 1 | 0.5 | Engine timbre control 2. See the engine table. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Engine timbre control 3. See the engine table. |
| WIDTH (Width / Vibrato) | 0 to 1 | 0.3 | Engine timbre control 4. See the engine table. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Engine timbre control 5. See the engine table. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Instrument level, before the effects. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Soft saturation. Drive rises up to 15x; the level is compensated. |
| CHORUS (Chorus) | 0 to 1 | 0 | Chorus mix (fixed 0.7 Hz modulation). |
| DELAY (Delay) | 0 to 1 | 0 | Level of the stereo cross-feedback (ping-pong style) delay. |
| TIME (Delay Time) | 0.05 to 1 s | 0.35 s | Delay time in seconds. Changes glide smoothly, like a tape delay. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Delay repeats. Higher values give more echoes. |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. The room also grows larger as you turn it up. |
| Pitch bend wheel (Pitch Bend) | -1 to +1 | 0 | Bends all notes by up to ±2 semitones. Adds to the MIDI pitch wheel. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | Adds LFO depth (up to +0.8). The higher of this wheel and MIDI CC1 is used. |

Effects order: Distortion, Chorus, Delay, Reverb, then the output soft limiter. Each effect is bypassed while its knob is at zero.

**What the timbre knobs do on each engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Vibes: three bar partials, mallet noise and a motor tremolo | Mallet hardness (attack noise) | Ring length (0.6 to 3.6 s) | Upper partials | Motor tremolo depth (up to 60%) | Motor speed (2 to 9 Hz) |
| Reel EP: tine partials with a bell overtone | No effect | Decay length | Bark: upper partials, stronger at high velocity | No effect | Key noise at the attack |
| Muted Trumpet: two saws through a resonant mute | Mute resonance pitch (600 to 2400 Hz) | Mute resonance sharpness and level | Brightness of the open horn layer | Detune between the two saws | Breath at the start of the note |
| Tenor Sax: reed wave with two formants and delayed vibrato | Formant position (dark to open) | Formant level | Brightness | Vibrato depth (5 Hz; starts after 0.25 s and fades in) | Breath noise |
| Upright Bass: plucked string model with body resonance | Pluck brightness and finger noise | Sustain | String brightness | No effect | Body resonance (low thump around 95 Hz) |
| Organ: six drawbar-style harmonics, percussion and key click | Sub drawbar (one octave below) | Octave drawbar (2nd harmonic) | Upper drawbars (3rd, 4th and 6th harmonics) | Percussion (fast-decaying 2nd harmonic) | Key click |
| Noir Strings: five detuned saws with tape flutter and body resonance | Body resonance pitch | Body resonance level | Brightness | Detune spread and flutter depth | Flutter speed (0.5 to 2 Hz) |
| Celesta: three bell partials and mallet noise | Mallet hardness (attack noise) | Ring length (0.35 to 2.15 s) | Upper partials | No effect | No effect |

On the Organ engine, the main (fundamental) drawbar is fixed. TONE, BODY and BRIGHT add the other drawbars around it.

## Factory presets

MIDNIGHT REELS includes 70 factory presets in 7 banks of 10. Loading a factory preset resets every parameter before applying the preset. The pitch bend and modulation wheels are not changed. Most banks use the engine of the same name; exceptions are noted.

**Vibes & Mallets** (Vibes engine)
Midnight Vibes, Smoke Vibes, Hard Mallet Vibes, Slow Motor, Fast Motor, Rainy Window, Low Vibes, Dry Vibes, Ghost Mallets, Vibes Delay.

**Reel EP** (Reel EP engine)
Reel Electric Piano, Noir Keys, Bark EP, Lounge EP, Tremolo EP, Tape EP, After Hours, Wurly Bite, Bell EP, Detective Chords.

**Trumpet & Sax** (Muted Trumpet and Tenor Sax engines)
Muted Trumpet, Harmon Mute, Wah Trumpet, Lonely Horn, Trumpet Stab, Tenor Sax, Smoky Sax, Breathy Sax, Late Night Sax, Growl Sax.

**Upright Bass** (Upright Bass engine; Bowed Cello uses Noir Strings)
Walking Upright, Slap Upright, Deep Pocket, Muted Bass, Jazz Club Bass, Bowed Cello, Detective Walk, Tape Bass, Fat Pluck, Sub Noir.

**Organ & Lounge** (Organ engine)
Jazz Organ, Smoky Lounge, Full Drawbars, Gospel Hammond, Church Hall, Perc Organ, Tape Organ, Soft Flute Stop, Rotary Dream, Noir Swell.

**Noir Strings** (Noir Strings engine)
Noir Strings, Tape Strings, Slow Swell, Dark Cellos, High Violins, Pizzicato Noir, Film Score Pad, Wobble Reel, Suspense, Rooftop Strings.

**Celesta & Reels** (Celesta engine; Projector Click uses Organ, Reel To Reel uses Reel EP, Closing Credits uses Noir Strings)
Music Box Noir, Celesta Dream, Glass Bells, Lullaby Reel, Toy Piano, Haunted Box, Low Celesta, Projector Click, Reel To Reel, Closing Credits.

**User presets.** The disk icon saves the current sound as a `.mrpreset` file in `Documents\DJBILBOX BEATS\MIDNIGHT_REELS\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .mrpreset file...** to load a preset from another folder. Your sound is also saved inside your FL Studio project.

## Production tips

- **Lo-fi and Jazz-Hop keys.** Start from **Tape EP**: the filter is already at 3 kHz and the chorus adds a light wobble. Raise **VEL** so soft notes stay mellow and hard notes bark. On Reel EP, **BRIGHT** sets the bark, and it grows with velocity. Add a touch of **DISTORTION** (0.1 to 0.2) for warmth, as in **Wurly Bite**.
- **Walking bass for Boom Bap.** All Upright Bass presets are in Mono. **TONE** sets the pluck attack, **BODY** the sustain, **TEXTURE** the low body thump around 95 Hz. Keep **DECAY** short for staccato lines. If the bass sits too low in the mix, raise **VOLUME**: plucked notes peak high but sound quieter than pads.
- **Vibes with or without motor.** On the Vibes engine, **WIDTH** is the motor tremolo depth and **TEXTURE** its speed. Set WIDTH to 0 for a static vibraphone. **Slow Motor** and **Fast Motor** show a deep slow motor and a deep fast motor.
- **Horn hooks.** For sax solos, use **Late Night Sax** (Mono with a short glide). The vibrato starts after a quarter second, so short notes stay straight and long notes sing. Raise **WIDTH** for a wider vibrato. **Wah Trumpet** uses the LFO on **Timbre**: slow the **RATE** down for a hand-wah effect in time with your groove.
- **Organ for Soul, Gospel and G-Funk.** **TONE** adds the sub drawbar, **BODY** the octave, **BRIGHT** the upper drawbars, **WIDTH** the percussion and **TEXTURE** the key click. Add **CHORUS** (0.4 to 0.6) for movement and a little **DISTORTION** for grit.
- **Film noir pads and builds.** On Noir Strings, **WIDTH** sets the detune and tape flutter depth, **TEXTURE** the flutter speed. For a slow reveal, copy the idea of **Suspense**: low **FILTER** (900 Hz), **ENV** at 0.4 and a long filter **A**. Automate **FILTER** over 8 bars for longer risers.
- **Dark Trap melodies.** **Music Box Noir**, **Haunted Box** and **Glass Bells** cut through dense 808s. Music Box Noir and Glass Bells are already transposed one octave up (**PITCH** +12). Keep **REVERB** around 0.5, and lower **FILTER** if they get too sharp.
- **Delay in time with the beat.** TIME is set in seconds. Quarter note = 60 / BPM. Eighth note = 30 / BPM. Dotted eighth = 45 / BPM. At 90 BPM, an eighth note is 0.33 s.

## MIDI, automation and performance

- **Notes.** MIDNIGHT REELS responds to all MIDI notes (0 to 127) on all MIDI channels, tuned to equal temperament with A4 = 440 Hz. Middle C is MIDI note 60 (C5 in FL Studio).
- **Polyphony.** 16 voices in Poly mode. When you play more than 16 notes, the oldest note is cut to make room; the lowest and highest held notes are protected. In Mono mode, each new note releases the previous one (its release tail still sounds). Releasing a note does not bring back an earlier held note.
- **Velocity.** Velocity changes the volume according to the VEL slider. On the Reel EP engine, velocity also adds bark.
- **Pitch bend.** ±2 semitones. The MIDI pitch wheel and the on-screen wheel add together.
- **Modulation (CC1).** Adds up to 0.8 to the LFO AMOUNT. The higher of MIDI CC1 and the on-screen wheel is used. MIDI CC1 is read by the notes that are sounding when you move the wheel.
- **Sustain pedal (CC64).** Supported. Aftertouch and other MIDI CCs are ignored. Use host automation for everything else.
- **Automation in FL Studio.** Every parameter in the table can be automated. Move a control on the MIDNIGHT REELS panel, then choose **Tools > Last tweaked > Create automation clip**. To use a hardware knob, choose **Tools > Last tweaked > Link to controller** and move the knob. Volume, pitch, bend and cutoff changes are smoothed to avoid zipper noise.
- **Latency.** 0 samples. No delay compensation is needed. The plugin reports a 12-second tail to the host so that delay and reverb tails are not cut when you render.
- **CPU.** MIDNIGHT REELS uses no samples and streams nothing from disk. The CPU load grows with the number of sounding notes. The five-saw Noir Strings engine is the heaviest. Effects at zero are bypassed and use no CPU. Long release times keep voices active longer.
- **Sample rates.** 44.1, 48, 88.2 and 96 kHz.

## FAQ

**I hear no sound.**
Check that the MIDNIGHT REELS channel is routed to a Mixer track and that the channel is not muted. Check VOLUME. With Low Pass selected, a very low FILTER setting can silence the sound. With High Pass selected, a high FILTER setting removes bass presets. Finally, remember that the on-screen keyboard plays very low notes in version 1.0: test with a MIDI keyboard or the Piano roll.

**MIDNIGHT REELS does not appear in FL Studio.**
MIDNIGHT REELS is a 64-bit VST3 plugin. Check that `MIDNIGHT_REELS.vst3` is in `C:\Program Files\Common Files\VST3`. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**. Look for **MIDNIGHT_REELS** under **Generators**, manufacturer **DJBILBOX BEATS**.

**Every note has an unwanted vibrato or wobble.**
Check **AMOUNT** and the on-screen modulation wheel: both add LFO depth. If you use a hardware modulation wheel, return it to zero while you hold a note, so that every voice receives the new position.

**The sound gets quieter when I turn REVERB fully down.**
In version 1.0, the reverb stage also lifts the dry signal by about 6 dB while it is active. At 0 the reverb is bypassed, so the overall level drops. For a dry sound at the usual level, leave REVERB at a very low value such as 0.01, or compensate with VOLUME.

**In Mono mode, notes overlap.**
This is normal. Mono mode releases the previous note instead of cutting it, so its release tail still sounds. Shorten **RELEASE** for a tighter monophonic line.
