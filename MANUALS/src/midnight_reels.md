# MIDNIGHT REELS

Film-noir keys, brass, bass and strings: vibes, electric piano, muted trumpet, tenor sax, upright bass, organ, strings and celesta from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

MIDNIGHT REELS is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Vibes (struck bars with a tremolo motor), Reel EP (an electric piano with a velocity-sensitive "bark"), Muted Trumpet, Tenor Sax (with a delayed vibrato), Upright Bass (a plucked string with a body resonance), Organ (drawbar-style), Noir Strings (five detuned saws with a tape-style flutter) and Celesta (bell-like partials).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. The character is dark and smoky, built for late-night, detective-movie atmospheres.

MIDNIGHT REELS targets film-noir and jazz-flavoured Lo-Fi, Hip-Hop and R&B beats, Trip-Hop, soundtrack and cinematic cues, and lounge or slow-jam arrangements. Most presets load in Poly mode with up to 16 voices. Twelve presets (the ten Upright Bass presets plus Late Night Sax and Growl Sax) load in Mono mode for single-note lines.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **MIDNIGHT REELS** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **Init Patch**: the Vibes engine with default settings.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 36 to 84 (C3 to C7 in FL Studio naming, where middle C is C5).
5. Shape the sound with the main controls:
    - **TONE, BODY, BRIGHT, WIDTH, TEXTURE**: the character of the selected engine.
    - **CUTOFF**: the filter cutoff. Turn it down for a darker, tape-like sound.
    - **REVERB** and **CHORUS**: space and width.
6. Adjust **VOLUME** (the rightmost slider, top right) to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. The proportions stay fixed when the host resizes the window.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The detective camel artwork and the MIDNIGHT REELS title with "By Humpire". The dotted grilles at each side of the artwork are decorative.

**FILTER ENVELOPE (top right).** Five vertical sliders: **ATK, DEC, REL** (the filter envelope), **VELOCITY** (velocity sensitivity of the level) and **VOLUME** (output level of the voices).

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE** sits in the same row: it is the global transposition (-12 to +12 semitones), not an LFO control.

**AMP ENVELOPE (left, below the LFO).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Character and filter (center).**

- **TONE, BODY, BRIGHT, WIDTH**: four small knobs to the left of GLIDE.
- **GLIDE** and **CUTOFF**: two large knobs.
- **TEXTURE**, **RES** (filter resonance) and **ENV AMT** (filter envelope depth): three small knobs to the right of CUTOFF.

**Effects (right).** Top row: **REVERB, CHORUS, DISTORTION**. Bottom row: **DLY MIX** (delay level), **DLY TIME** (delay time) and **FEEDBACK** (delay repeats).

**Menu row.** Five drop-down menus, each with a caption below it: **ENGINE**, **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE** (Poly or Mono). Each menu shows its current value.

**Wheels (bottom left).** **PITCH BEND** springs back to the center when you release it. **MOD WHEEL** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. It covers MIDI notes 36 to 84 (four octaves plus one key). The key labels on screen run from C1 to C5, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C3 to C7. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

MIDNIGHT REELS exposes 34 automatable parameters. The host name is shown in parentheses when it differs from the panel label. The filter envelope has no sustain control: it rises and decays to zero, and ENV AMT sets how far it opens the filter.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Vibes, Reel EP, Muted Trumpet, Tenor Sax, Upright Bass, Organ, Noir Strings, Celesta | Vibes | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 (0 to 100 %) | 50 % | Character control 1. Engine-dependent (see the table below). |
| BODY (Body) | 0 to 1 | 50 % | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 50 % | Character control 3. Engine-dependent. |
| WIDTH (Width / Vibrato) | 0 to 1 | 30 % | Character control 4. Engine-dependent (detune, tremolo, vibrato...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 20 % | Character control 5. Engine-dependent (noise, click, flutter speed...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 15 % | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| ENV AMT (Filter Env Amount) | 0 to 1 | 0 % | Filter envelope depth. At 100 % the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| ATK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DEC (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| REL (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 50 % | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 70 % | Amplitude envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 (shown in dB) | 0.7 (-3.1 dB) | Output level of each voice, before the effects. |
| GLIDE (Glide) | 0 to 2 | 0 | Portamento. The value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 % | LFO depth. The mod wheel (on-screen or MIDI CC1) adds up to 0.8 to this value. |
| LFO SHAPE (LFO Source menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION (Destination menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| VOICE MODE (Mono/Poly menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 % | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 % | Stereo chorus mix (slow, fixed rate). |
| DLY MIX (Delay Mix) | 0 to 1 | 0 % | Level of the ping-pong delay. |
| DLY TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 35 % | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 25 % | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Mod Wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT, together with MIDI CC1 (the larger of the two applies). |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Vibes | Mallet attack noise | Ring length of the bars | Level of the upper partials | Depth of the tremolo motor | Speed of the tremolo motor |
| Reel EP | No effect | Decay length | Bark: level of the upper partials, stronger with velocity | No effect | Attack noise |
| Muted Trumpet | Mute resonance frequency | Mute resonance and level | Low-pass opening | Detune of the two saws | Breath noise at the start of the note |
| Tenor Sax | Formant frequencies | Formant level | Low-pass opening | Vibrato depth (starts after a short delay) | Breath noise |
| Upright Bass | Pluck hardness | String sustain | String brightness | No effect | Body resonance (around 95 Hz) |
| Organ | Sub-octave drawbar level | Second-harmonic drawbar level | Upper drawbars level | Percussion | Key click |
| Noir Strings | Frequency of the mid resonance | Level of the mid resonance | Low-pass opening | Detune spread and tape flutter depth | Tape flutter speed |
| Celesta | Mallet attack noise | Ring length of the bells | Level of the upper partials | No effect | No effect |

## Factory presets

MIDNIGHT REELS includes 70 factory presets in 7 banks of 10. 58 presets load in Poly mode. 12 presets load in Mono mode: Late Night Sax and Growl Sax (Trumpet & Sax bank) and all ten presets of the Upright Bass bank. Late Night Sax and Detective Walk load with a short glide.

**Vibes & Mallets:** Midnight Vibes, Smoke Vibes, Hard Mallet Vibes, Slow Motor, Fast Motor, Rainy Window, Low Vibes, Dry Vibes, Ghost Mallets, Vibes Delay.

**Reel EP:** Reel Electric Piano, Noir Keys, Bark EP, Lounge EP, Tremolo EP, Tape EP, After Hours, Wurly Bite, Bell EP, Detective Chords.

**Trumpet & Sax:** Muted Trumpet, Harmon Mute, Wah Trumpet, Lonely Horn, Trumpet Stab, Tenor Sax, Smoky Sax, Breathy Sax, Late Night Sax, Growl Sax.

**Upright Bass:** Walking Upright, Slap Upright, Deep Pocket, Muted Bass, Jazz Club Bass, Bowed Cello, Detective Walk, Tape Bass, Fat Pluck, Sub Noir.

**Organ & Lounge:** Jazz Organ, Smoky Lounge, Full Drawbars, Gospel Hammond, Church Hall, Perc Organ, Tape Organ, Soft Flute Stop, Rotary Dream, Noir Swell.

**Noir Strings:** Noir Strings, Tape Strings, Slow Swell, Dark Cellos, High Violins, Pizzicato Noir, Film Score Pad, Wobble Reel, Suspense, Rooftop Strings.

**Celesta & Reels:** Music Box Noir, Celesta Dream, Glass Bells, Lullaby Reel, Toy Piano, Haunted Box, Low Celesta, Projector Click, Reel To Reel, Closing Credits.

Two presets use another engine than their bank name suggests: Bowed Cello (Upright Bass bank) uses Noir Strings, and Projector Click and Reel To Reel (Celesta & Reels bank) use Organ and Reel EP.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.mrpreset` file. The default folder is `DJBILBOX BEATS/MIDNIGHT_REELS/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .mrpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

- **Lo-Fi keys:** load **Tape EP**, then lower CUTOFF and raise CHORUS for a worn-tape feel.
- **Chords with glue:** the Reel EP and Organ banks are Poly. Raise REVERB to place chords behind a vocal.
- **Cinematic builds:** **Suspense** and **Slow Swell** have long attacks. Draw long notes and automate CUTOFF or ENV AMT.
- **Walking lines:** the Upright Bass bank is Mono. Write one note at a time and keep VELOCITY above 50% for a more natural touch.
- **Movement:** set LFO DESTINATION to Timbre or Filter and raise the mod wheel (or AMOUNT) for slow evolving textures.
- **Tempo-matched delay:** DLY TIME is in seconds. Use 60 / BPM for a quarter note.

## MIDI and automation

- **Notes:** all MIDI channels are accepted. Velocity changes the level (see VELOCITY) and, for Reel EP, the bark.
- **Pitch bend:** +/-2 semitones, added to the on-screen PITCH BEND wheel.
- **Mod wheel (CC1):** raises the LFO AMOUNT by up to 0.8. It starts from the last position received, even if no note is playing.
- **CC121 (reset all controllers):** returns the MIDI mod wheel to zero.
- **Polyphony:** up to 16 voices in Poly mode. In Mono mode, a new note cuts the previous one (last-note priority).
- **Automation:** the 34 parameters listed above are automatable in your host. The ENGINE, LFO SHAPE, LFO DESTINATION, FILTER TYPE and VOICE MODE menus are automatable choice parameters.
- **Latency:** 0 samples. The output is stereo.

## FAQ

**Does it use samples?** No. Every sound is generated in real time by one of eight engines.

**Can I play chords with the Upright Bass presets?** Switch VOICE MODE to Poly. The Upright Bass bank loads in Mono.

**Why does TRANSPOSE sit next to the LFO knobs?** It is a global transposition in semitones (-12 to +12). It is not an LFO control.

**The keyboard on screen stops at C5. Can I play lower or higher notes?** The on-screen keyboard covers MIDI notes 36 to 84 only. Play outside that range from a MIDI controller or the Piano roll.

**Is the delay tempo-synced?** No. DLY TIME is in seconds (0.05 to 1 s), and the LFO runs in Hz (0.05 to 20 Hz).

**Can I save my own presets?** Yes. The disk icon saves a `.mrpreset` file. Your presets appear in the User submenu of the preset menu.

**Is there a filter sustain?** No. The filter envelope has attack, decay and release. It decays to zero, and ENV AMT sets its depth.
