# DZPOWER

Algerian Raï, Chaabi and Casbah sounds from a pure synthesis engine: plucked strings, reed flutes, organ, strings and hand drums.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

DZPOWER is a 16-voice synthesizer dedicated to the sounds of Algerian popular music. It has eight synthesis engines inspired by traditional instruments: Mandole (Algerian mandolin), Qanun (zither), Gasba (reed flute), Ghaita (double-reed zorna), Rai Organ, Chaabi Strings, Derbouka (goblet drum) and Bendir Sub (frame drum turned into a deep sub). It uses no samples. Every sound is generated in real time, so it reacts to the five timbre knobs, the filter and the envelopes.

Each engine runs through a resonant multimode filter with its own envelope, a key-synced LFO and an effects chain: distortion, chorus, stereo ping-pong delay and reverb. A soft limiter at the output keeps the level under control.

DZPOWER is built for Raï and Raï'n'B, Chaabi, Oriental pop, Afrotrap, Hip-Hop and Trap. Use the Mandole and Qanun for melodic hooks, the Gasba and Ghaita for lead lines, the Rai Organ for classic keyboard parts, the strings for pads, the Derbouka for percussion and the Bendir Sub as an 808-style bass.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **DZPOWER** (manufacturer **DJBILBOX BEATS**). You can also drag it from **Plugin database > Generators**.
2. The interface opens on **DZPOWER - Init Patch**.
3. Click the folder icon at the left of the preset display. Open a bank (for example **Mandole**) and pick a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play notes from your MIDI keyboard, or draw them in the **Piano roll**.
5. Turn **CUTOFF** to make the sound darker or brighter.
6. Turn **BRIGHT** (in the left speaker grille) to change the brightness of the engine itself.
7. Turn **REVERB** to place the sound in a small or large space.
8. Click the disk icon at the right of the preset display to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from half size to double size. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon opens the preset menu, **<** loads the previous preset, the center shows the current preset name, **>** loads the next preset, and the disk icon saves a user preset. The preset menu lists the seven factory banks as submenus, a **User** submenu with your saved presets, and an **Open a .dzpreset file...** item.

**EXPANSION cassette (top left).** Click it to see information about upcoming expansion packs (**EXPANSION VOL.1**) and the link to the DJBILBOX BEATS store.

**Logo (top right).** The DJBILBOX BEATS logo and name.

**Speaker grilles and emblem (center).** The camel emblem sits between two perforated speaker grilles that hold the small knobs. The left grille holds **TONE**, **BODY**, **BRIGHT** and **WIDTH**. These four knobs and **TEXTURE** shape the selected engine. The right grille holds **TEXTURE**, **ENV** (filter envelope depth) and **DELAY** (delay level).

**VOLUME and REVERB (top right).** Level of the instrument and amount of reverb.

**ASDR group (right).** Five vertical sliders under the "ASDR" heading: **ATTACK**, **DECAY**, **SUSTAIN** and **RELEASE** of the filter envelope, plus **VELOCITY** (velocity sensitivity of the volume).

**LFO group (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH** sits in the same row but is the global transpose of the instrument.

**Amplitude envelope (left, below the LFO).** Four vertical sliders: **ATTACK**, **DECAY**, **SUSTAIN** and **RELEASE** of the volume envelope.

**Center row.** **GLIDE**, then the **FILTER** group with **CUTOFF** and **RESONANCE**, then **CHORUS**, **DISTORTION**, and the **DELAY** group with **TIME** and **FEEDBACK**.

**Menu row.** Five drop-down menus. Each one shows its current value. From left to right: engine (Waveform), LFO shape (LFO Source), LFO target (Destination), filter mode (Filter Type) and voice mode (Poly / Mono).

**Wheels (bottom left).** **Pitch Bend** springs back to center when you release the mouse. **Modulation** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to audition the sound. It also lights up incoming MIDI notes within its range. In version 1.0 the keyboard covers the lowest part of the MIDI range (keys labeled C-2 to about A0, MIDI notes 0 to 33). Use it to check bass and sub notes. Play melodies and chords from a MIDI keyboard or the Piano roll.

**Using the controls.** Drag a knob up/down or left/right. Drag a slider vertically. The mouse wheel adjusts in small steps. Double-click to return to the default value. Hold Ctrl or Alt while you drag for velocity-sensitive, finer movement. A small bubble shows the current value while you hover or drag.

## Parameters

Names in parentheses are the parameter names shown in FL Studio for automation, when they differ from the panel label. "Amp" means the amplitude (volume) envelope.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Engine menu (Waveform) | Mandole, Qanun, Gasba, Ghaita, Rai Organ, Chaabi Strings, Derbouka, Bendir Sub | Mandole | Selects the synthesis engine. See the engine table below. |
| LFO shape menu (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO waveform. Sample&Hold picks a new random value once per cycle. |
| LFO target menu (Destination) | Pitch, Filter, Amp, Timbre | Pitch | Where the LFO goes. Pitch: up to ±1 semitone. Filter: up to ±3 octaves of cutoff. Amp: tremolo, up to 50% volume dip. Timbre: moves all five timbre knobs together, up to ±0.5. |
| Filter mode menu (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | 12 dB/octave state-variable filter mode. Band Pass gets a small level boost. |
| Voice mode menu (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes. Mono: each new note releases the previous one. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. The LFO restarts at each new note. It is not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The Modulation wheel (or MIDI CC1) adds up to 0.8 on top. |
| PITCH (Pitch) | -12 to +12 semitones, continuous | 0 | Global transpose. Values between semitones detune the whole instrument. |
| GLIDE (Glide) | 0 to 2 s | 0 (off) | Portamento. Above 0, each new note slides from the pitch of the previous note, in Poly and Mono. |
| CUTOFF (Filter) | 40 Hz to 18 kHz | 9 kHz | Filter cutoff frequency. The cutoff follows the played note at 25% (key tracking). |
| RESONANCE (Resonance) | 0 to 1 | 0.15 | Filter resonance (emphasis at the cutoff). |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1, the envelope opens the cutoff by up to 5 octaves. |
| ASDR ATTACK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| ASDR DECAY (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| ASDR SUSTAIN (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| ASDR RELEASE (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| VELOCITY (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the volume. 0 = every note at full level. 1 = full velocity response. |
| Amp ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Volume envelope attack time. |
| Amp DECAY (Decay) | 0.01 to 5 s | 0.4 s | Volume envelope decay time. |
| Amp SUSTAIN (Sustain) | 0 to 1 | 0.7 | Volume envelope sustain level. Set to 0 for plucks and drums. |
| Amp RELEASE (Release) | 0.02 to 8 s | 0.4 s | Volume envelope release time after note-off. |
| TONE (Tone) | 0 to 1 | 0.5 | Engine timbre control 1. See the engine table. |
| BODY (Body) | 0 to 1 | 0.5 | Engine timbre control 2. See the engine table. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Engine timbre control 3. See the engine table. |
| WIDTH (Width) | 0 to 1 | 0.3 | Engine timbre control 4. See the engine table. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Engine timbre control 5. See the engine table. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Instrument level, before the effects. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Soft saturation. Drive rises up to 15x; the level is compensated. |
| CHORUS (Chorus) | 0 to 1 | 0 | Chorus mix (fixed 0.7 Hz modulation). |
| DELAY (Delay) | 0 to 1 | 0 | Level of the stereo ping-pong delay. |
| TIME (Delay Time) | 0.05 to 1 s | 0.35 s | Delay time in seconds. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Delay repeats. Higher values give more echoes. |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. The room also grows larger as you turn it up. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | Bends all notes by up to ±2 semitones. Adds to the MIDI pitch wheel. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | Adds LFO depth (up to +0.8). The higher of this wheel and MIDI CC1 is used. |

Effects order: Distortion, Chorus, Delay, Reverb, then the output soft limiter. Each effect is bypassed while its knob is at zero.

**What the timbre knobs do on each engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Mandole: plucked string with wooden body | Pick hardness and body resonance pitch | String sustain | String brightness (damping) | No effect | Wooden body resonance |
| Qanun: bright plucked zither with a sympathetic octave | No effect | String sustain | String brightness (damping) | Level of the sympathetic octave at the attack | No effect |
| Gasba: reed flute with breath and vibrato | Color of the breath noise | Focus of the breath noise | 2nd and 3rd harmonics | Vibrato depth (fades in during the first half second) | Breath level |
| Ghaita: double reed with nasal resonances | Position of the nasal resonances | Strength of the nasal resonances | Brightness | Detune between the two reeds | Breath noise |
| Rai Organ: drawbars, tremolo, key click | Sub-octave drawbar | Octave drawbar | Upper harmonic drawbars | Tremolo depth and speed | Key click |
| Chaabi Strings: five detuned saws with delayed vibrato | Body resonance pitch | Body resonance level | Brightness | Vibrato depth (starts after 0.2 s) | Bow noise |
| Derbouka: doum tone and tak slap | Pitch drop of the doum | Doum decay | Pitch of the slap | Level of the tak slap | Attack click |
| Bendir Sub: deep sub with jingles | Knock (pitch punch at the attack) | 2nd harmonic | Rattle decay | Saturation | Rattle level |

## Factory presets

DZPOWER includes 70 factory presets in 7 banks of 10. Loading a factory preset resets every parameter before applying the preset. The Pitch Bend and Modulation wheels are not changed.

**Mandole**
Algiers Mandole, Mandole Chaabi, Casbah Pick, Warm Mandole, Tremolo Pick, Low Mandole, Bright Strum, Rai Pluck, Muted Mandole, Desert Night Pluck.

**Qanun & Plucks**
Qanun Dar, Zither Rain, Harp Of Tlemcen, Bright Qanun, Soft Qanun, Oriental Arp, Low Qanun, Glass Qanun, Sahara Strings, Zellij Pluck.

**Gasba & Ghaita**
Gasba Dream, Desert Gasba, Breathy Gasba, Sufi Gasba, High Gasba, Ghaita Call, Zorna Celebration, Nasal Reed, Mizmar Dz, Snake Zorna.

**Rai Organ**
Rai Organ, Oran Night, Full Rai, Rai Lead Organ, Accordion Dz, Vintage Rai Keys, Soft Flute Stop, Rotary Casbah, Tape Rai, Golden Swell.

**Chaabi Strings**
Chaabi Strings, Algiers Violin, Slow Swell, Dark Cellos, High Violins, Pizzicato Dz, Andalusian Pad, Wobble Violin, Suspense Casbah, Rooftop Strings.

**Derbouka & Perc**
Derbouka Doum, Tak Tak, Deep Doum, Slap Derbouka, Tabla Dz, Rai Percussion, Dub Doum, Hand Drum Fill, Crowd Clap, Drop Doum.

**Bendir Sub & Bass**
Bendir Sub, Deep Casbah, Rattle Sub, Long Slide Sub, Pitch Drop Dz, Dirty Bendir, Smooth Sub, Hard Knock Dz, Short Boom, Algiers Rumble.

**User presets.** The disk icon saves the current sound as a `.dzpreset` file in `Documents\DJBILBOX BEATS\DZPOWER\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .dzpreset file...** to load a preset from another folder. Your sound is also saved inside your FL Studio project.

## Production tips

- **Raï lead vibrato.** On most presets the LFO is set to Sine, Pitch, 5 Hz with AMOUNT at 0. Push the Modulation wheel to add vibrato only where you want it. Try **Rai Lead Organ** or **Zorna Celebration**: both are in Mono with a short glide for legato phrases.
- **Quarter-tone inflections.** DZPOWER plays equal temperament. The pitch bend range is ±2 semitones, so a quarter tone is one quarter of the upward wheel travel. For a whole passage, set **PITCH** to +0.5 or -0.5. The knob is continuous.
- **Delay in time with the beat.** TIME is set in seconds. Quarter note = 60 / BPM. Eighth note = 30 / BPM. Dotted eighth = 45 / BPM. At 100 BPM, an eighth note is 0.30 s. At 140 BPM, it is 0.21 s.
- **Afrotrap mandole hooks.** Start from **Casbah Pick** or **Algiers Mandole**. Raise **VELOCITY** to 0.8 or more so that your playing dynamics come through. Lower **BODY** for shorter, tighter notes that leave room for the drums.
- **808-style slides with the Bendir Sub.** Use **Long Slide Sub** or set GLIDE between 0.1 and 0.35 in Mono. Any GLIDE above 0 makes every note slide from the previous one, so set it back to 0 for straight notes. Raise **BODY** to add a 2nd harmonic that stays audible on phone speakers. Add **DISTORTION** around 0.2 for grit.
- **Derbouka grooves.** Each hit combines a pitched doum and a slap. Play low notes for deep doums and higher notes for tighter hits; use **Tak Tak** or **Slap Derbouka** for slap accents. **TONE** controls the pitch drop of the doum. **WIDTH** controls the slap level. Keep Poly mode so that hits overlap naturally. Vary note velocities in the Piano roll for a human feel.
- **Chaabi string pads.** The vibrato of the Chaabi Strings engine starts 0.2 s after the note begins. Short notes stay straight, long notes bloom. Add CHORUS around 0.3 and REVERB around 0.5 for width. **Suspense Casbah** uses a slow filter envelope for intros and builds.
- **Keep chords clean.** The output passes through a soft limiter. Dense chords at high VOLUME saturate gently. Lower **VOLUME** for big chords, and raise it on plucks and drums, which peak higher than they sound.

## MIDI, automation and performance

- **Notes.** DZPOWER responds to all MIDI notes (0 to 127) on all MIDI channels, tuned to equal temperament with A4 = 440 Hz. Middle C is MIDI note 60 (C5 in FL Studio).
- **Polyphony.** 16 voices in Poly mode. When you play more than 16 notes, the oldest note is stolen; the lowest and highest held notes are protected. In Mono mode, each new note releases the previous one (its release tail still sounds). Releasing a note does not bring back an earlier held note.
- **Velocity.** Velocity changes the volume according to the VELOCITY slider.
- **Pitch bend.** ±2 semitones. The MIDI pitch wheel and the on-screen wheel add together.
- **Modulation (CC1).** Adds up to 0.8 to the LFO AMOUNT. The higher of MIDI CC1 and the on-screen wheel is used.
- **Sustain pedal (CC64).** Supported. Aftertouch and other MIDI CCs are ignored. Use host automation for everything else.
- **Automation in FL Studio.** Every parameter in the table can be automated. Move a control on the DZPOWER panel, then choose **Tools > Last tweaked > Create automation clip**. To use a hardware knob, choose **Tools > Last tweaked > Link to controller** and move the knob. Volume, pitch, bend and cutoff changes are smoothed to avoid zipper noise.
- **Latency.** 0 samples. No delay compensation is needed. The plugin reports a 12-second tail to the host so that delay and reverb tails are not cut when you render.
- **CPU.** DZPOWER uses no samples and streams nothing from disk. The CPU load grows with the number of sounding notes. The five-saw Chaabi Strings engine is the heaviest. Effects at zero are bypassed and use no CPU. Long release times keep voices active longer.
- **Sample rates.** 44.1, 48, 88.2 and 96 kHz.

## FAQ

**I hear no sound.**
Check that the DZPOWER channel is routed to a Mixer track and that the channel is not muted. Check VOLUME. With Low Pass selected, a very low CUTOFF can silence the sound. With High Pass selected, a high CUTOFF removes bass presets. Finally, remember that the on-screen keyboard plays very low notes in version 1.0: test with a MIDI keyboard or the Piano roll.

**DZPOWER does not appear in FL Studio.**
DZPOWER is a 64-bit VST3 plugin. Check that `DZPOWER.vst3` is in `C:\Program Files\Common Files\VST3`. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**. Look for it under **Generators**, manufacturer **DJBILBOX BEATS**.

**Plucks and drums sound quieter than pads.**
Plucked and percussive presets have sharp attacks and short notes, so they sound softer at the same peak level. Raise **VOLUME** on these presets.

**Can DZPOWER play maqam scales with quarter tones?**
There is no built-in scale or microtuning. Use the pitch wheel (a quarter tone is a quarter of the upward travel) or the continuous **PITCH** knob for inflections.

**The sound gets quieter when I turn REVERB fully down.**
In version 1.0, the reverb stage also lifts the dry signal by about 6 dB while it is active. At 0 the reverb is bypassed, so the overall level drops. For a dry sound at the usual level, leave REVERB at a very low value such as 0.01, or compensate with VOLUME.
