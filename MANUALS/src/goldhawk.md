# GOLDHAWK

Golden, cinematic leads, brass, pads, plucks, bells and basses for epic beats.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

GOLDHAWK is a 16-voice synthesizer with a royal, cinematic character. It has eight synthesis engines: Hawk Lead (pulse-width-modulated lead), Golden Brass (brass with a self-opening filter), Royal Pad (wide breathing pad), Falcon Pluck (pluck with a pitch dive), Sky Bells (FM bells), Feather Strings (strings with delayed vibrato), Talon Bass (saturated bass with sub) and Eagle Cry (falling, breathy cry for effects). It uses no samples. Every note is generated in real time and responds to the five timbre knobs, the filter and the envelopes.

Each engine runs through a resonant multimode filter with its own envelope, a key-synced LFO and an effects chain: distortion, chorus, stereo ping-pong delay and reverb. A soft limiter at the output keeps the level under control.

GOLDHAWK is built for Trap, Hip-Hop and Drill with an epic or luxurious mood, Afrotrap, cinematic intros and Deep House plucks. Use Hawk Lead for hooks, Golden Brass for fanfares and stabs, Royal Pads for atmospheres, Falcon Plucks and Sky Bells for melodies and arpeggios, Talon Bass for low end and Eagle Cry for transitions.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **GOLDHAWK** (manufacturer **DJBILBOX BEATS**). You can also drag it from **Plugin database > Generators**.
2. The interface opens on **GOLDHAWK - Init Patch**.
3. Click the folder icon at the left of the preset display. Open a bank (for example **Hawk Leads**) and pick a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play notes from your MIDI keyboard, or draw them in the **Piano roll**.
5. Turn **FILTER** to make the sound darker or brighter.
6. Turn **TONE** (below the cassette) to change the character of the engine.
7. Turn **REVERB** to place the sound in a small or large space.
8. Click the disk icon at the right of the preset display to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from half size to double size. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon opens the preset menu, **<** loads the previous preset, the center shows the current preset name, **>** loads the next preset, and the disk icon saves a user preset. The preset menu lists the seven factory banks as submenus, a **User** submenu with your saved presets, and an **Open a .ghpreset file...** item.

**EXPANSION cassette (top left).** Click it to see information about upcoming expansion packs (**EXPANSION VOL.1**) and the link to the DJBILBOX BEATS store.

**Small knobs (below the cassette).** Two rows of four small knobs. First row: **TONE**, **BODY**, **BRIGHT** and **WIDTH**. Second row: **TEXTURE**, **RES** (filter resonance), **ENV** (filter envelope depth) and **DELAY** (delay level). TONE, BODY, BRIGHT, WIDTH and TEXTURE shape the selected engine.

**Logo (top right).** The DJBILBOX BEATS name and logo.

**Emblem (center).** The golden camel-hawk emblem sits between two decorative speaker grilles. The GOLDHAWK title and "By Humpire" are below it.

**LFO group (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH** sits in the same row but is the global transpose of the instrument.

**Amplitude envelope (left, below the LFO).** Four vertical sliders: **A**, **D**, **S** and **R** (attack, decay, sustain and release of the volume envelope).

**ENVELOPE group (right).** Four vertical sliders: **A**, **D** and **S** of the filter envelope, and **VEL** (velocity sensitivity of the volume). The filter envelope release uses the same time as its decay. **VOLUME** is to the right of this group.

**Effects (right).** **REVERB**, **CHORUS** and **DISTORTION** on one row. Below, the **DELAY** group with **TIME** and **FEEDBACK**. The delay level is the small **DELAY** knob below the cassette.

**Center bottom.** **GLIDE** on the left. Four labeled menus in a column: **WAVEFORM** (engine), **LFO SOURCE** (LFO shape), **DESTINATION** (LFO target) and **MONO/POLY** (voice mode). Then the **FILTER** knob (cutoff) and the small **FILTER TYPE** menu.

**Wheels (bottom left).** The left wheel bends the pitch and springs back to center when you release the mouse. The right wheel is the modulation wheel and stays where you leave it.

**On-screen keyboard (bottom).** Click a key to audition the sound. It also lights up incoming MIDI notes within its range. In version 1.0 the keyboard covers the lowest part of the MIDI range (keys labeled C-2 to about A0, MIDI notes 0 to 33). Use it to check bass notes. Play melodies and chords from a MIDI keyboard or the Piano roll.

**Using the controls.** Drag a knob up/down or left/right. Drag a slider vertically. The mouse wheel adjusts in small steps. Double-click to return to the default value. Hold Ctrl or Alt while you drag for velocity-sensitive, finer movement. A small bubble shows the current value while you hover or drag.

## Parameters

Names in parentheses are the parameter names shown in FL Studio for automation, when they differ from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| WAVEFORM (Waveform) | Hawk Lead, Golden Brass, Royal Pad, Falcon Pluck, Sky Bells, Feather Strings, Talon Bass, Eagle Cry | Hawk Lead | Selects the synthesis engine. See the engine table below. |
| LFO SOURCE (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO waveform. Sample&Hold picks a new random value once per cycle. |
| DESTINATION (Destination) | Pitch, Filter, Amp, Timbre | Pitch | Where the LFO goes. Pitch: up to ±1 semitone. Filter: up to ±3 octaves of cutoff. Amp: tremolo, up to 50% volume dip. Timbre: moves all five timbre knobs together, up to ±0.5. |
| FILTER TYPE (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | 12 dB/octave state-variable filter mode. Band Pass gets a small level boost. |
| MONO/POLY (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes. Mono: each new note releases the previous one. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. The LFO restarts at each new note. It is not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The modulation wheel (or MIDI CC1) adds up to 0.8 on top. |
| PITCH (Pitch) | -12 to +12 semitones, continuous | 0 | Global transpose. Values between semitones detune the whole instrument. |
| GLIDE (Glide) | 0 to 2 s | 0 (off) | Portamento. Above 0, each new note slides from the pitch of the previous note, in Poly and Mono. |
| FILTER (Filter) | 40 Hz to 18 kHz | 9 kHz | Filter cutoff frequency. The cutoff follows the played note at 25% (key tracking). |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance (emphasis at the cutoff). |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1, the envelope opens the cutoff by up to 5 octaves. |
| ENVELOPE A (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| ENVELOPE D (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. Also sets the filter envelope release after note-off. |
| ENVELOPE S (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| VEL (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the volume. 0 = every note at full level. 1 = full velocity response. |
| A (Attack) | 0.001 to 5 s | 0.01 s | Volume envelope attack time. |
| D (Decay) | 0.01 to 5 s | 0.4 s | Volume envelope decay time. |
| S (Sustain) | 0 to 1 | 0.7 | Volume envelope sustain level. |
| R (Release) | 0.02 to 8 s | 0.4 s | Volume envelope release time after note-off. |
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
| Pitch bend wheel (Pitch Bend) | -1 to +1 | 0 | Bends all notes by up to ±2 semitones. Adds to the MIDI pitch wheel. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | Adds LFO depth (up to +0.8). The higher of this wheel and MIDI CC1 is used. |

Effects order: Distortion, Chorus, Delay, Reverb, then the output soft limiter. Each effect is bypassed while its knob is at zero.

**What the timbre knobs do on each engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Hawk Lead: two pulse waves with PWM, plus a sub-octave sine | Pulse-width modulation depth | Detune between the two pulses | Brightness | Pulse-width modulation speed | Added saw layer |
| Golden Brass: three saws with a filter that opens on each note | Swell time (how slowly the filter opens) | Weight (level) | How far the filter opens | Detune | Breath noise |
| Royal Pad: five saws and a sub-octave sine, slow breathing filter | No effect | Sub-octave level | Brightness | Detune | Air noise |
| Falcon Pluck: two saws with a pitch dive and a closing filter | Pitch dive at the attack | Filter closing time | Filter sweep amount | Detune | Attack noise |
| Sky Bells: FM bell | FM ratio (bell color, from soft to clangorous) | Decay length | FM brightness (metallic shine) | Detune (beating) | Strike noise |
| Feather Strings: five detuned saws with delayed vibrato | Body resonance pitch | Body resonance level | Brightness | Vibrato depth (starts after 0.2 s) | Bow noise |
| Talon Bass: three detuned saws, sub-octave sine, saturation | Sub-octave level | Detune | Brightness | Saturation | Noise |
| Eagle Cry: falling pitch with harmonics and breath | Fall depth and breath color | 3rd harmonic | 2nd harmonic | Fall speed | Breath level |

## Factory presets

GOLDHAWK includes 70 factory presets in 7 banks of 10. Loading a factory preset resets every parameter before applying the preset. The pitch bend and modulation wheels are not changed.

**Hawk Leads**
Goldhawk Lead, Soaring, Royal Pulse, Golden Wire, Sky Solo, Hawk Wide, Thermal, Eagle Eye, Golden Stab, Altitude.

**Golden Brass**
Golden Brass, Royal Fanfare, Hawk Stab, Imperial Horns, Soft Flugel, Slow Swell, Low Brass, Gold Rush Hit, Trumpet Call, Cinematic Brass.

**Royal Pads**
Royal Pad, Golden Hour, Palace Air, Rising Hawk, Cloud Throne, Warm Gold, Dream Flight, Dusk Strings, Whisper Wind, Pulse Of Gold.

**Falcon Plucks**
Falcon Pluck, Gold Arp, Dive Zap, Muted Talon, Soft Feather, Pluck Choir, Golden Harp, Royal Koto, Rhythm Gate, Sad Hawk.

**Sky Bells & Strings**
Sky Bell, Golden Chimes, Crystal Crown, Temple Bell, Music Box Gold, Feather Strings, Violin Flight, Slow Rise Strings, Dark Cellos, Pizz Gold.

**Talon Bass**
Talon Bass, Gold Sub, Dirty Talon, Wobble Wing, Industrial Hawk, Slide Bass, Acid Talon, Deep Nest, Bass Stab, Meltdown Gold.

**Eagle Cry & FX**
Eagle Cry, Hawk Screech, Distant Call, Falcon Dive, Wind Rush, Mountain Echo, Golden Whistle, Raptor Siren, Night Hunter, Thunder Wing.

**User presets.** The disk icon saves the current sound as a `.ghpreset` file in `Documents\DJBILBOX BEATS\GOLDHAWK\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .ghpreset file...** to load a preset from another folder. Your sound is also saved inside your FL Studio project.

## Production tips

- **Trap and Drill hooks.** **Goldhawk Lead** is in Mono with a short glide and a 0.375 s delay (a dotted eighth at 120 BPM). Each note slides briefly from the previous one. Raise **WIDTH** for a faster pulse-width movement, or **TONE** for a deeper one.
- **Cinematic brass.** On the Golden Brass engine, each note opens its own filter. **TONE** sets how slowly it opens: low for punchy stabs (**Hawk Stab**, **Gold Rush Hit**), high for slow swells (**Slow Swell**, **Cinematic Brass**). For intros, layer **Imperial Horns** with **Low Brass**, which is already transposed one octave down.
- **Plucks and arpeggios.** **Falcon Pluck** and **Gold Arp** work well for Afrotrap and Deep House melodies. **TONE** adds a pitch "dive" at the attack; turn it down for a clean pluck, up for laser zaps (**Dive Zap**). **BODY** sets how long the filter stays open.
- **Bells for dark melodies.** On Sky Bells, **TONE** changes the FM ratio. Most positions give inharmonic, metallic tones. Find a setting that fits your key, then use **BRIGHT** to control the shine and **BODY** for the decay length.
- **Talon Bass as an 808-style bass.** All Talon Bass presets are in Mono and transposed one octave down. For slides, use **Slide Bass** or raise GLIDE: any value above 0 makes every note slide from the previous one. **WIDTH** adds saturation that stays audible on small speakers. Keep **BODY** low for a tight, centered sub.
- **Builds and drops.** **Rising Hawk** combines a slow attack with a slow filter envelope for builds. Automate **FILTER** over 4 or 8 bars for longer risers. Use **Raptor Siren** or **Falcon Dive** on a single long note before the drop.
- **Delay in time with the beat.** TIME is set in seconds. Quarter note = 60 / BPM. Eighth note = 30 / BPM. Dotted eighth = 45 / BPM. At 140 BPM, a dotted eighth is 0.32 s.
- **Keep chords clean.** The output passes through a soft limiter. Dense pad chords at high VOLUME saturate gently. Lower **VOLUME** on big chords, and raise it on short stabs and plucks, which peak higher than they sound.

## MIDI, automation and performance

- **Notes.** GOLDHAWK responds to all MIDI notes (0 to 127) on all MIDI channels, tuned to equal temperament with A4 = 440 Hz. Middle C is MIDI note 60 (C5 in FL Studio).
- **Polyphony.** 16 voices in Poly mode. When you play more than 16 notes, the oldest note is stolen; the lowest and highest held notes are protected. In Mono mode, each new note releases the previous one (its release tail still sounds). Releasing a note does not bring back an earlier held note.
- **Velocity.** Velocity changes the volume according to the VEL slider.
- **Pitch bend.** ±2 semitones. The MIDI pitch wheel and the on-screen wheel add together.
- **Modulation (CC1).** Adds up to 0.8 to the LFO AMOUNT. The higher of MIDI CC1 and the on-screen wheel is used.
- **Sustain pedal (CC64).** Supported. Aftertouch and other MIDI CCs are ignored. Use host automation for everything else.
- **Automation in FL Studio.** Every parameter in the table can be automated. Move a control on the GOLDHAWK panel, then choose **Tools > Last tweaked > Create automation clip**. To use a hardware knob, choose **Tools > Last tweaked > Link to controller** and move the knob. Volume, pitch, bend and cutoff changes are smoothed to avoid zipper noise.
- **Latency.** 0 samples. No delay compensation is needed. The plugin reports a 12-second tail to the host so that delay and reverb tails are not cut when you render.
- **CPU.** GOLDHAWK uses no samples and streams nothing from disk. The CPU load grows with the number of sounding notes. The five-saw Royal Pad and Feather Strings engines are the heaviest. Effects at zero are bypassed and use no CPU. Long release times, common on pads, keep voices active longer.
- **Sample rates.** 44.1, 48, 88.2 and 96 kHz.

## FAQ

**I hear no sound.**
Check that the GOLDHAWK channel is routed to a Mixer track and that the channel is not muted. Check VOLUME. With Low Pass selected, a very low FILTER setting can silence the sound. With High Pass selected, a high FILTER setting removes bass presets. Finally, remember that the on-screen keyboard plays very low notes in version 1.0: test with a MIDI keyboard or the Piano roll.

**GOLDHAWK does not appear in FL Studio.**
GOLDHAWK is a 64-bit VST3 plugin. Check that `GOLDHAWK.vst3` is in `C:\Program Files\Common Files\VST3`. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**. Look for it under **Generators**, manufacturer **DJBILBOX BEATS**.

**The Talon Bass presets sound an octave lower than the notes I play.**
This is intended. Every Talon Bass preset sets **PITCH** to -12 semitones for a deep sub. Set PITCH back to 0 to play at the written pitch.

**TONE has no effect on my pad.**
On the Royal Pad engine, TONE is not used. Use BRIGHT, WIDTH, BODY and TEXTURE to shape the pad. See the engine table in the Parameters section.

**The sound gets quieter when I turn REVERB fully down.**
In version 1.0, the reverb stage also lifts the dry signal by about 6 dB while it is active. At 0 the reverb is bypassed, so the overall level drops. For a dry sound at the usual level, leave REVERB at a very low value such as 0.01, or compensate with VOLUME.
