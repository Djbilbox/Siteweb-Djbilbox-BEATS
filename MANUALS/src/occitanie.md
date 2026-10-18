# OCCITANIE

Folk instruments of Southern France: bagpipe, hurdy-gurdy, pipe, mandolin, guitar, accordion, choir and bells, with a built-in drone.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

OCCITANIE is a 16-voice synthesizer inspired by the traditional instruments of Occitania, the region around Toulouse. It has eight synthesis engines: Cabrette (nasal bagpipe with breath), Vielle (bowed strings with the typical buzz), Galoubet (three-hole folk pipe), Mandoline (plucked string with plectrum tremolo), Guitare (plucked string with a soft attack), Accordeon (three detuned reeds, musette style), Choeur (ensemble with vowel formants) and Carillon (inharmonic bells). It uses no samples. Every note is generated in real time.

A **DRONE** control adds a soft tone one octave below each note, on any engine: the classic bagpipe and hurdy-gurdy sound. Each voice then runs through a multimode filter and a per-voice LFO. The output goes through an effects chain: distortion, chorus, stereo delay and reverb. A soft limiter at the output keeps the level under control.

OCCITANIE brings folk color to modern productions. Use the accordion and pipe sounds for Oriental and Rai melodies, the mandolin and guitar for Afrotrap and Drill hooks, the choirs and bells for cinematic Trap intros, and the hurdy-gurdy and bagpipe for medieval, fantasy and world music. It also works for Deep House toplines and folk-flavored Hip-Hop loops.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **OCCITANIE** (manufacturer **DJBILBOX BEATS**). You can also drag it from **Plugin database > Generators**.
2. The interface opens on **OCCITANIE - Init Patch** (Cabrette engine).
3. Click the **Presets** button at the left of the preset display. Open a bank (for example **Accordeon & Musette**) and pick a preset. You can also step through all 70 presets with the **<** and **>** buttons.
4. Play notes from your MIDI keyboard, or draw them in the **Piano roll**.
5. Raise the **DRONE** slider (right) to add a low drone under every note.
6. Move the **REED** slider to change the color of the engine: the nasal tone of the bagpipe, the vowel of the choir, the breath of the pipe.
7. Turn **REVERB** to place the sound in a small room or a large church.
8. Click **Save** at the right of the preset display to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from half size (500 x 373) to double size (2000 x 1492). The proportions stay fixed.

**Preset display (top center).** From left to right: **Presets** opens the preset menu, **<** loads the previous preset, the center shows the current preset name, **>** loads the next preset, and **Save** saves a user preset. The arrows wrap around from the last preset to the first. The preset menu lists the seven factory banks as submenus, a **User** submenu with your saved presets, and an **Open a .occpreset file...** item.

**EXPANSION cassette (top left).** Click it to see information about upcoming expansion packs (**EXPANSION VOL.1**) and the link to the DJBILBOX BEATS store. Version 1.0 contains no expansion content.

**Logo (top right).** The DJBILBOX BEATS name and logo.

**Medallion (center).** The camel warrior, wearing a horned helmet with the Occitan cross, sits between decorative speaker grilles. The OCCITANIE title and "By Humpire" are below it.

**LFO group (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH** sits under the same "LFO" caption but is the global transpose of the instrument, not an LFO control.

**Volume envelope (left, below the LFO group).** Four vertical sliders: **ATTACK**, **DECAY**, **SUSTAIN** and **RELEASE**.

**Timbre sliders and VOLUME (right, below the speaker grille).** Five short vertical sliders: **REED**, **BODY**, **BRIGHT**, **DRONE** and **VEL** (velocity sensitivity). The **VOLUME** knob is to their right.

**Effects (right).** **REVERB**, **CHORUS** and **DISTORTION** on the upper row. **DELAY** (level), **TIME** and **FEEDBACK** on the lower row.

**Center bottom.** Two small knobs, **WIDTH** and **TEXTURE**, then the **FILTER** knob (cutoff) and the **GLIDE** knob.

**Menu row.** Five drop-down menus across the panel. Each one shows its current value. From left to right: the engine (**Waveform**, default "Cabrette"), the LFO shape (**LFO Source**, default "Sine"), the LFO target (**Destination**, default "Pitch"), the filter mode (**Filter Type**, default "Low Pass") and the voice mode (**Mono/Poly**, default "Poly").

**Wheels (bottom left).** The left wheel bends the pitch and springs back to center when you release the mouse. The right wheel is the modulation wheel and stays where you leave it.

**On-screen keyboard (bottom).** Click a key to audition the sound. It also lights up incoming MIDI notes within its range. In version 1.0 the keyboard covers the lowest part of the MIDI range (keys labeled C-2 to about D1, MIDI notes 0 to 38). Use it to check low notes. Play melodies and chords from a MIDI keyboard or the Piano roll.

**Using the controls.** Drag a knob up/down or left/right. Drag a slider vertically. The mouse wheel adjusts in small steps. Double-click to return to the default value. Hold Ctrl or Alt while you drag for velocity-sensitive movement. A small bubble shows the current value while you hover or drag.

## Parameters

Names in parentheses are the parameter names shown in FL Studio for automation, when they differ from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Engine menu (Waveform) | Cabrette, Vielle, Galoubet, Mandoline, Guitare, Accordeon, Choeur, Carillon | Cabrette | Selects the synthesis engine. See the engine table below. |
| LFO shape menu (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO waveform. Sample&Hold picks a new random value once per cycle. |
| LFO target menu (Destination) | Pitch, Filter, Amp, Timbre | Pitch | Where the LFO goes. Pitch: up to ±1 semitone. Filter: up to ±3 octaves of cutoff. Amp: tremolo, up to 50% volume dip. Timbre: moves REED, BODY, BRIGHT, WIDTH and TEXTURE together, up to ±0.5. |
| Filter mode menu (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | 12 dB/octave state-variable filter mode. Band Pass gets a small level boost. |
| Voice mode menu (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes. Mono: each new note releases the previous one. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. The LFO restarts at each new note. It is not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The modulation wheel (or MIDI CC1) adds up to 0.8 on top. |
| PITCH (Pitch) | -12 to +12 semitones, continuous | 0 | Global transpose. Values between semitones detune the whole instrument. |
| GLIDE (Glide) | 0 to 2 s | 0 (off) | Portamento. Above 0, each new note slides from the pitch of the previous note, in Poly and Mono. |
| FILTER (Filter) | 40 Hz to 18 kHz | 9 kHz | Filter cutoff frequency. The cutoff follows the played note at 25% (key tracking). |
| REED (Reed / Vowel) | 0 to 1 | 0.5 | Engine timbre control 1: reed color, vowel or breath. See the engine table. |
| BODY (Body / Resonance) | 0 to 1 | 0.5 | Engine timbre control 2. See the engine table. BODY also raises the filter resonance slightly. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Engine timbre control 3: brightness. See the engine table. |
| WIDTH (Width / Detune) | 0 to 1 | 0.3 | Engine timbre control 4: detune and width. See the engine table. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Engine timbre control 5: breath, buzz or tremolo. See the engine table. |
| DRONE (Drone) | 0 to 1 | 0 | Level of a softened saw one octave below each note. Works on every engine. |
| VEL (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the volume. 0 = every note at full level. 1 = full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Volume envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Volume envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Volume envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Volume envelope release time after note-off. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Instrument level, before the effects. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Soft saturation. Drive rises up to 15x; the level is compensated. |
| CHORUS (Chorus) | 0 to 1 | 0 | Chorus mix (fixed 0.7 Hz modulation). |
| DELAY (Delay) | 0 to 1 | 0 | Level of the stereo cross-feedback (ping-pong style) delay. |
| TIME (Delay Time) | 0.05 to 1 s | 0.35 s | Delay time in seconds. Changes glide smoothly, like a tape delay. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Delay repeats. Higher values give more echoes. |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. The room also grows larger as you turn it up. |
| Pitch bend wheel (Pitch Bend) | -1 to +1 | 0 | Bends all notes by up to ±2 semitones. Adds to the MIDI pitch wheel. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | Adds LFO depth (up to +0.8). The higher of this wheel and MIDI CC1 is used. |

Effects order: Distortion, Chorus, Delay, Reverb, then the output soft limiter. Each effect is bypassed while its knob is at zero. OCCITANIE has no filter envelope: shape the sound over time with the volume envelope and the LFO.

**What the timbre controls do on each engine**

| Engine | REED | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Cabrette: two saws with a nasal double formant and an attack puff | Formant position (nasal color) | Formant level | Brightness | Detune between the two saws | Breath noise and attack puff |
| Vielle: two saws with formants and a buzzing bridge | Formant position | Formant level | Brightness | Detune between the two saws | Buzz (bridge saturation) |
| Galoubet: soft harmonic tone with breath | Breath amount (more air, less tone) | Breath focus (resonance of the air noise) | Upper harmonics | Second, detuned pipe layer | Chiff (attack noise) |
| Mandoline: plucked string model with plectrum tremolo | No effect | Sustain | Brightness | No effect | Plectrum tremolo depth (14 Hz) |
| Guitare: plucked string model with a soft attack | No effect | Sustain (long at high values) | Brightness | No effect | No effect |
| Accordeon: three detuned reeds with formants | Formant position | Formant level | Brightness | Musette detune between the reeds | Bellows noise |
| Choeur: five detuned saws with vowel formants | Vowel (formant position) | Formant level | Brightness | Ensemble detune | Breath noise |
| Carillon: four inharmonic bell partials | No effect | Ring length | Upper partials | No effect | Strike noise |

On every engine, BODY also controls the filter resonance (from flat at 0 to a gentle peak at 1).

## Factory presets

OCCITANIE includes 70 factory presets in 7 banks of 10. Loading a factory preset resets every parameter before applying the preset. The pitch bend and modulation wheels are not changed. Most banks use the engine of the same name; exceptions are noted. Bank and preset names are shown exactly as they appear in the plugin (French, with place names of the region such as Capitole, Garonne, Albi and Carcassonne).

**Cabrette & Cornemuse** (Cabrette engine)
Capitole Cabrette, Canal du Midi, Garonne Drone, Pic du Midi, Fete Votive, Cabreta Triste, Bal Bagpipe, Corbieres, Pastel Bleu, Saint Sernin.

**Vielle & Cordes frottees** (Vielle engine)
Roue de Cordes, Trompette Buzz, Bourdon Albigeois, Menestrel, Troubadour, Chant Cathare, Rouergue, Brume du Tarn, Chanterelle, Cordes sur Ciel.

**Flutes & Galoubet** (Galoubet engine)
Galoubet Joyeux, Tambourin Air, Flute de Berger, Fifre Camargue, Pastoral, Mistral, Tramontane, Flute Cathare, Sifflet Occitan, Aube Corbieres.

**Cordes pincees** (Guitare engine; Mandoline Toulousaine, Mandoline Tremolo, Cistre, Plectre Doux uses Mandoline)
Mandoline Toulousaine, Mandoline Tremolo, Guitare Gitane, Luth Medieval, Cistre, Guitare Nylon Soft, Plectre Doux, Strum du Midi, Pizz Menestrel, Theorbe Grave.

**Accordeon & Musette** (Accordeon engine)
Musette Toulouse, Bal Musette, Valse Violette, Accordeon Cabaret, Java du Midi, Soufflet Lent, Harmonium Chapelle, Orgue de Barbarie, Gavotte, Cassoulet Musette.

**Choeurs & Pads** (Choeur engine; Brume Ariege uses Vielle)
Choeur Cathare, Montsegur, Carcassonne Pad, Cite Medievale, Voix d Oc, Plainchant Albi, Lune d Oc, Abbaye Moissac, Brume Ariege, Crypte Saint Sernin.

**Carillons & Textures** (Carillon engine; Vent d Autan uses Galoubet; Orage Cevenol uses Vielle)
Carillon Capitole, Cloches Saint Etienne, Angelus du Soir, Clocher d Albi, Glockenspiel Rose, Cristaux Ariege, Rosee Garonne, Vent d Autan, Orage Cevenol, Etoiles d Oc.

**User presets.** **Save** writes the current sound as a `.occpreset` file in `Documents\DJBILBOX BEATS\OCCITANIE\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .occpreset file...** to load a preset from another folder. Your sound is also saved inside your FL Studio project.

## Production tips

- **Bagpipe lines.** Start from **Canal du Midi**: Cabrette engine, Mono, short glide and DRONE at 0.5. **REED** moves the nasal formant, **TEXTURE** adds breath and a puff at each attack. The drone is part of each voice: it follows the note you play and stops with it. For a fixed drone under a moving melody, hold one long low note on a second OCCITANIE channel.
- **Oriental and Rai melodies.** Use the Accordeon engine with **WIDTH** between 0.6 and 0.85 for a wide musette detune. Switch to **Mono** and set **GLIDE** to 0.05 to 0.1 s for slides between notes. Use the pitch wheel (±2 semitones) for ornaments. For vibrato, set the LFO to **Pitch**, **RATE** to 5 or 6 Hz and push the modulation wheel only on long notes.
- **Hurdy-gurdy buzz.** On the Vielle engine, **TEXTURE** drives the buzzing bridge. **Trompette Buzz** pushes it to 0.85. Combine it with **DRONE** and a slow **ATTACK** (0.1 to 0.3 s) for a bowed, medieval feel.
- **Plucked hooks for Afrotrap and Drill.** **Mandoline Toulousaine** and **Strum du Midi** cut through a busy beat. On the Mandoline engine, **TEXTURE** adds the fast plectrum tremolo. On both string engines, **BODY** sets how long the string rings and **BRIGHT** how bright it sounds. **Theorbe Grave** gives a deep plucked bass line in Mono.
- **Choirs and chant.** On the Choeur engine, **REED** selects the vowel. **Voix d Oc** sends a slow LFO to **Timbre** so the vowel keeps moving. Add **DRONE** (0.3 to 0.4) for a plainchant effect, as in **Plainchant Albi**.
- **Bells for intros.** On the Carillon engine, **BODY** sets the ring length and **BRIGHT** the upper partials. Set **PITCH** to -12 for large tower bells (**Cloches Saint Etienne**). A dotted delay and a long reverb turn a single hit into an intro.
- **Layering in a Trap or Hip-Hop mix.** Folk sounds can fill the low mids. Choose **High Pass** in the filter mode menu and raise **FILTER** to 150 to 300 Hz to make room for the 808 and the kick. Lower **DRONE** on dense chords.
- **Delay in time with the beat.** TIME is set in seconds. Quarter note = 60 / BPM. Eighth note = 30 / BPM. Dotted eighth = 45 / BPM. At 100 BPM, a quarter note is 0.6 s.

## MIDI, automation and performance

- **Notes.** OCCITANIE responds to all MIDI notes (0 to 127) on all MIDI channels, tuned to equal temperament with A4 = 440 Hz. Middle C is MIDI note 60 (C5 in FL Studio).
- **Polyphony.** 16 voices in Poly mode. When you play more than 16 notes, the oldest note is cut to make room; the lowest and highest held notes are protected. In Mono mode, each new note releases the previous one (its release tail still sounds). Releasing a note does not bring back an earlier held note.
- **Velocity.** Velocity changes the volume according to the VEL slider.
- **Pitch bend.** ±2 semitones. The MIDI pitch wheel and the on-screen wheel add together.
- **Modulation (CC1).** Adds up to 0.8 to the LFO AMOUNT. The higher of MIDI CC1 and the on-screen wheel is used. MIDI CC1 is read by the notes that are sounding when you move the wheel.
- **Sustain pedal (CC64).** Supported. Aftertouch and other MIDI CCs are ignored. Use host automation for everything else.
- **Automation in FL Studio.** Every parameter in the table can be automated. Move a control on the OCCITANIE panel, then choose **Tools > Last tweaked > Create automation clip**. To use a hardware knob, choose **Tools > Last tweaked > Link to controller** and move the knob. Volume, pitch, bend and cutoff changes are smoothed to avoid zipper noise.
- **Latency.** 0 samples. No delay compensation is needed. The plugin reports a 12-second tail to the host so that delay and reverb tails are not cut when you render.
- **CPU.** OCCITANIE uses no samples and streams nothing from disk. The CPU load grows with the number of sounding notes. The five-saw Choeur engine is the heaviest. Effects at zero are bypassed and use no CPU. Long release times, common on pads and bells, keep voices active longer.
- **Sample rates.** 44.1, 48, 88.2 and 96 kHz.

## FAQ

**I hear no sound.**
Check that the OCCITANIE channel is routed to a Mixer track and that the channel is not muted. Check VOLUME. With Low Pass selected, a very low FILTER setting can silence the sound. With High Pass selected, a high FILTER setting removes low notes. Finally, remember that the on-screen keyboard plays very low notes in version 1.0: test with a MIDI keyboard or the Piano roll.

**OCCITANIE does not appear in FL Studio.**
OCCITANIE is a 64-bit VST3 plugin. Check that `OCCITANIE.vst3` is in `C:\Program Files\Common Files\VST3`. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**. Look for **OCCITANIE** under **Generators**, manufacturer **DJBILBOX BEATS**.

**The drone changes pitch with my melody.**
This is by design. DRONE adds a tone one octave below each note you play, so it moves with the melody. For a fixed drone, play one sustained low note on a separate channel.

**Where is the filter resonance control?**
OCCITANIE has no separate resonance knob. BODY raises the resonance gently while it shapes the engine.

**The sound gets quieter when I turn REVERB fully down.**
In version 1.0, the reverb stage also lifts the dry signal by about 6 dB while it is active. At 0 the reverb is bypassed, so the overall level drops. For a dry sound at the usual level, leave REVERB at a very low value such as 0.01, or compensate with VOLUME.
