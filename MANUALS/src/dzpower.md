# DZPOWER

Algerian Raï, Chaabi and Casbah sounds from eight synthesis engines: plucked strings, reed flutes, organ, strings and hand drums.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

DZPOWER is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines inspired by the instruments of Algerian popular music: Mandole (plucked string), Qanun (zither), Gasba (reed flute), Ghaita (double-reed horn), Rai Organ, Chaabi Strings, Derbouka (hand drum) and Bendir Sub (deep sub with rattle).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a resonant multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb. Up to 16 voices play at once in Poly mode.

DZPOWER targets Raï and Raï'n'B, Chaabi, Oriental pop, Afrotrap, Hip-Hop and Trap. Use the Mandole and Qanun for melodic hooks, the Gasba and Ghaita for lead lines, the Rai Organ and Chaabi Strings for chords and pads, and the Derbouka and Bendir Sub for percussion and sub bass.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **DZPOWER** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **DZPOWER - Init Patch**: the Mandole engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 36 to 96 (C2 to C7 in the labels printed on the keys; C3 to C8 in FL Studio naming, where middle C is C5).
5. Shape the sound with three controls:
    - **TONE**: the main character of the engine (its effect depends on the engine, see the table in the Parameters section).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker, rounder sound.
    - **REVERB**: places the sound in a small or large space.
6. Adjust **VOLUME** (in the MASTER section) to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 500 x 373 up to 2000 x 1492 pixels. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking it opens a message saying that expansion packs are coming soon, with a link to the DJBILBOX BEATS store. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name.

**Center.** The camel emblem and the DZPOWER title between two perforated speaker grilles that hold the small knobs. The left grille holds **TONE, BODY, BRIGHT, WIDTH**. The right grille holds **TEXTURE, FLT ENV** (filter envelope depth) and **DELAY MIX** (delay level).

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, in the same row, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**MASTER section (top right).** **VOLUME** and **REVERB**.

**AMP ENVELOPE (left, below the LFO).** Four vertical sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**FILTER ENVELOPE (right).** Four vertical sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**. The **VELOCITY** slider (velocity sensitivity of the level) sits just to their right, separated by a thin line, because it is not part of the filter envelope.

**Center row.** From left to right: the **GLIDE** knob, the **FILTER** group (**CUTOFF** and **RESONANCE**), the **EFFECTS** group (**CHORUS** and **DISTORTION**) and the **DELAY** group (**TIME** and **FEEDBACK**).

**Menu row.** Five drop-down menus, each with a caption below it: **WAVEFORM** (the engine), **LFO SOURCE** (LFO shape), **LFO DESTINATION**, **FILTER TYPE** and **MONO / POLY**. Each menu shows its current value.

**PITCH & MOD (bottom left).** The **Pitch Bend** wheel springs back to the center when you release it. The **Mod Wheel** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. It also lights up incoming MIDI notes within its range. The keyboard covers MIDI notes 36 to 96: five octaves plus one key, with labels from C2 to C7 (middle C, MIDI 60, is labeled C4; in FL Studio naming the same range reads C3 to C8). It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click a knob or slider to return it to its default value. The mouse wheel also adjusts controls.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

DZPOWER exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Control | Range | Default | What it does |
|---|---|---|---|
| WAVEFORM menu (Waveform) | Mandole, Qanun, Gasba, Ghaita, Rai Organ, Chaabi Strings, Derbouka, Bendir Sub | Mandole | Selects the synthesis engine. |
| TONE (Tone) | 0 to 100% | 50% | Character control 1. Engine-dependent. |
| BODY (Body) | 0 to 100% | 50% | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 100% | 50% | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 100% | 30% | Character control 4. Engine-dependent (vibrato, detune, tremolo, drive...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 100% | 20% | Character control 5. Engine-dependent (noise, click, body resonance...). |
| CUTOFF (Filter Cutoff) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RESONANCE (Resonance) | 0 to 100% | 15% | Filter resonance. |
| FILTER TYPE menu (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| FLT ENV (Filter Env Amount) | 0 to 100% | 0% | Filter envelope depth. At 100% the envelope opens the cutoff by up to 5 octaves. At 0% the filter envelope has no effect. |
| Filter ATTACK (Filter Attack) | 2 ms to 3 s | 2 ms | Filter envelope attack time. |
| Filter DECAY (Filter Decay) | 20 ms to 4 s | 300 ms | Filter envelope decay time. |
| Filter SUSTAIN (Filter Sustain) | 0 to 100% | 0% | Filter envelope sustain level. |
| Filter RELEASE (Filter Release) | 20 ms to 4 s | 300 ms | Filter envelope release time. |
| VELOCITY (Velocity Sensitivity) | 0 to 100% | 50% | How much note velocity changes the level. 0%: all notes at the same level. 100%: full velocity response. |
| Amp ATTACK (Attack) | 1 ms to 5 s | 10 ms | Amplitude envelope attack time. |
| Amp DECAY (Decay) | 10 ms to 5 s | 400 ms | Amplitude envelope decay time. |
| Amp SUSTAIN (Sustain) | 0 to 100% | 70% | Amplitude envelope sustain level. |
| Amp RELEASE (Release) | 20 ms to 8 s | 400 ms | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 (shown in dB, -inf to 0 dB) | 0.7 (-3.1 dB) | Output level of each voice, before the effects. |
| GLIDE (Glide) | 0 to 2 s | 0 | Portamento. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 100% | 0% | LFO depth. The mod wheel (on-screen or MIDI CC1) adds up to 80% to this value. |
| LFO SOURCE menu (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION menu (Destination) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| MONO / POLY menu (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 100% | 0% | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 100% | 0% | Stereo chorus mix (fixed rate). |
| DELAY MIX (Delay Mix) | 0 to 100% | 0% | Level of the ping-pong delay. |
| TIME (Delay Time) | 50 ms to 1 s | 350 ms | Delay time. Not tempo-synced: for a quarter note, use 60 / BPM seconds (0.5 s at 120 BPM). |
| FEEDBACK (Delay Feedback) | 0 to 90% | 35% | Number of delay repeats. |
| REVERB (Reverb) | 0 to 100% | 25% | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -2 to +2 semitones | 0 | On-screen pitch bend. Adds to MIDI pitch bend (also +/-2 semitones). |
| Mod Wheel (Mod Wheel) | 0 to 100% | 0% | On-screen mod wheel. Adds up to 80% to the LFO AMOUNT. |

**What the character controls do on each engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Mandole | Pluck hardness and body resonance frequency | String sustain | String brightness (damping) | No effect | Level of the body resonance |
| Qanun | No effect | String sustain | String brightness (damping) | Level of the octave-up sympathetic attack | No effect |
| Gasba | Breath filter frequency | Breath filter resonance | Level of the 2nd and 3rd harmonics | Vibrato depth (about 5 Hz, fades in) | Breath noise level |
| Ghaita | Frequency of the nasal resonances | Level of the nasal resonances | Opening of the reed low-pass | Detune between the two reeds | Noise level |
| Rai Organ | Sub-octave drawbar level | Octave-up drawbar level | Level of the 3rd, 4th and 6th harmonic drawbars | Tremolo depth and speed | Key click |
| Chaabi Strings | Center frequency of the body band | Level of the body band | Opening of the low-pass | Vibrato depth (starts after 0.2 s) | Noise level |
| Derbouka | Depth of the pitch drop at the attack | Length of the doum | Frequency of the "tak" slap noise | Level of the "tak" slap | Click |
| Bendir Sub | Pitch knock at the attack | Octave harmonic level | Length of the rattle | Saturation drive | Level of the rattle |

## Factory presets

DZPOWER includes 70 factory presets in 7 banks of 10. 55 presets load in Poly mode and 15 in Mono mode: the whole Bendir Sub & Bass bank, plus Breathy Gasba, Zorna Fete, Snake Zorna, Rai Lead Organ and Violon Alger.

**Mandole:** Mandole Alger, Mandole Chaabi, Casbah Pick, Warm Mandole, Tremolo Pick, Low Mandole, Bright Strum, Rai Pluck, Muted Mandole, Desert Night Pluck.

**Qanun & Plucks:** Qanun Dar, Zither Rain, Harp Of Tlemcen, Bright Qanun, Soft Qanun, Oriental Arp, Low Qanun, Glass Qanun, Sahara Strings, Zellij Pluck.

**Gasba & Ghaita:** Gasba Reve, Desert Gasba, Breathy Gasba, Sufi Gasba, High Gasba, Ghaita Call, Zorna Fete, Nasal Reed, Mizmar Dz, Snake Zorna.

**Rai Organ:** Rai Organ, Oran Night, Full Rai, Rai Lead Organ, Accordeon Dz, Vintage Rai Keys, Soft Flute Stop, Rotary Casbah, Tape Rai, Golden Swell.

**Chaabi Strings:** Chaabi Strings, Violon Alger, Slow Swell, Dark Cellos, High Violins, Pizzicato Dz, Andalous Pad, Wobble Violin, Suspense Casbah, Rooftop Strings.

**Derbouka & Perc:** Derbouka Doum, Tak Tak, Deep Doum, Slap Derbouka, Tabla Dz, Rai Percussion, Dub Doum, Hand Drum Fill, Crowd Clap, Drop Doum.

**Bendir Sub & Bass:** Bendir Sub, Deep Casbah, Rattle Sub, Long Slide Sub, Pitch Drop Dz, Dirty Bendir, Smooth Sub, Hard Knock Dz, Short Boom, Rumble Alger.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.dzpreset` file. The default folder is `DJBILBOX BEATS/DZPOWER/Presets` inside your Documents folder. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .dzpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

- **Hooks.** Start from a Mandole or Qanun preset, add a little DELAY MIX and set TIME to an eighth note (30 / BPM seconds) for a rolling Raï-style pluck line.
- **Lead lines.** Use a Mono preset (Breathy Gasba, Zorna Fete) and add GLIDE for slides between notes. Raise WIDTH on the Gasba for more vibrato.
- **Pads.** On Chaabi Strings, raise ATTACK and RELEASE in the AMP ENVELOPE and add CHORUS and REVERB for a wide string bed.
- **Percussion.** On Derbouka, TONE sets the depth of the pitch drop, BODY the length of the doum and WIDTH the level of the slap. Play different notes to tune the drum.
- **Sub bass.** The Bendir Sub presets are Mono. Keep CUTOFF high and use TRANSPOSE (-12) to drop an octave. Add DISTORTION to make the sub audible on small speakers.
- **Movement.** Set LFO DESTINATION to Filter and AMOUNT around 30% for a slow wobble, or to Timbre to make the character controls drift.
- **Expression.** Raise VELOCITY to make notes you play softly quieter and harder hits louder. Map the mod wheel to add LFO depth while you play.

## MIDI, automation and performance

- **Notes.** All 16 MIDI channels are accepted and play the same sound.
- **Velocity.** Controls the level, scaled by the VELOCITY slider.
- **Pitch bend.** MIDI pitch bend moves the pitch by +/-2 semitones, added to the on-screen Pitch Bend wheel.
- **Mod wheel (CC1).** Adds up to 80% to the LFO AMOUNT. CC121 (reset all controllers) resets it to zero.
- **Polyphony.** Up to 16 voices in Poly mode. Mono mode plays one note at a time with last-note priority.
- **Automation.** All 35 parameters appear in the host's automation list with units (%, Hz, ms, dB, semitones).
- **Latency.** 0 samples. The output is stereo.
- **Not supported.** Aftertouch is ignored. The delay and LFO are not synced to the host tempo. There is no microtonal tuning.

## FAQ

**I hear no sound.** Check that the channel is routed to a mixer track, that VOLUME is above zero and that CUTOFF is not at its minimum. Also check the AMP ENVELOPE: with SUSTAIN at zero, notes fade out by themselves. Many plucked presets are designed that way.

**DZPOWER does not appear in FL Studio.** Run the plugin manager scan again (**Options > Manage plugins > Find installed plugins**) and check that the VST3 file is in the standard VST3 folder.

**Some presets are quieter than others.** Plucks and drums are short and peak quickly, so they sound quieter than sustained pads at the same VOLUME. Raise VOLUME or balance the level in the mixer.

**Can I play chords on every preset?** Yes. Switch MONO / POLY to Poly. Mono presets, such as the Bendir Sub bank, are meant for single-note lines.

**Can DZPOWER play quarter tones?** No. The plugin has no microtonal tuning. Use a pitch bend or the GLIDE control to approach in-between pitches by ear.

**Does it use samples?** No. Every sound is generated in real time by the synthesis engines.
