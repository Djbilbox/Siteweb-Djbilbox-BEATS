# WARZONE808

808 bass synthesizer with five waveforms, 7-voice unison, a resonant filter with its own envelope, an LFO and four effects. 60 factory presets plus a 10-preset expansion.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Presets | 60 factory presets in 6 banks, plus 10 from the built-in Expansion Vol.1 |
| Latency | 0 samples |

## Overview

WARZONE808 is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time from one of five waveforms (sine, square, sawtooth, triangle or noise). Each note can be stacked up to seven times in **unison**, detuned by up to 60 cents and spread across the stereo field. The sound then goes through a multimode filter (low pass, band pass or high pass) with a resonance up to Q 20 and its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, delay and reverb.

WARZONE808 is built around the 808. It opens in **Mono** mode with last-note priority and legato: when you hold one note and play another, the pitch glides to the new note without retriggering the envelopes, and releasing the new note returns to the note you still hold. Switch **VOICE MODE** to **Poly** for chords (up to 16 voices).

It targets Trap, Drill and Hip-Hop 808s, reese and growl bass, leads, plucks, pads, risers and war-themed sound effects. WARZONE808 has no pitch-drop envelope: a classic 808 kick needs a layered kick or the filter envelope.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **WARZONE808** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows. Start with **808 Classic** in the 808 Subs bank.
3. Play from your MIDI keyboard or draw notes in the **Piano roll**. 808 notes sit best between MIDI notes 24 and 48 (C2 to C4 in FL Studio naming, where middle C is C5).
4. Shape the sound with the main controls:
    - **CUTOFF**: the cutoff of the filter. Turn it down for a rounder, darker 808.
    - **DISTORTION**: adds harmonics so the 808 stays audible on small speakers.
    - **GLIDE**: slide time between notes, in milliseconds.
5. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first. The preset menu lists the six factory banks (and any expansion pack) as submenus, then a **User** submenu with your saved presets.

**EXPANSION cassette (top left).** The cassette shows the name of the loaded expansion (Expansion Vol.1). Click it for a menu: **Load a .wz808exp pack...** loads another expansion pack from a file, and **Open the packs folder** opens the folder where packs are loaded automatically at start-up.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**LFO group (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, in the same row, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**AMP ENVELOPE (left, below the LFO).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Center row.** From left to right: **RESO** (filter resonance) and **ENV AMT** (filter envelope depth), the large **GLIDE** and **CUTOFF** knobs, then **UNISON**, **DETUNE** and **DLY MIX** (delay level).

**FILTER ENVELOPE (top right).** Four vertical sliders: **ATK, DEC, SUS** and **VELOCITY** (velocity sensitivity of the level). The filter envelope shares its release time with the amp envelope.

**Right panel.** **VOLUME, REVERB, CHORUS** on the upper row, **DISTORTION, DLY TIME, DLY FDBK** on the lower row.

**Menu row.** Five drop-down menus, each with a caption: **ENGINE** (Waveform), **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE**. Each menu shows its current value.

**Wheels (bottom left).** **PITCH BEND** springs back to the center when you release it. **MOD WHEEL** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. The keyboard covers MIDI notes 36 to 84 (four octaves plus one key). The key labels on screen run from C1 to C5, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C3 to C7. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. A value bubble appears while you drag.

## Parameters

Signal flow: oscillator (with unison), filter, amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb, then by a soft saturation (tanh) on the output. A DC blocker protects the output.

WARZONE808 exposes 38 automatable parameters. The host name is shown in parentheses when it differs from the panel label. Seven of them have no knob on the panel but can be automated from your host: Distortion Tone, Chorus Rate, Chorus Depth, Reverb Room, Reverb Damping, Reverb Width and Unison Width.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Sine, Square, Sawtooth, Triangle, Noise | Sine | Oscillator waveform. Noise is used for explosions, gunshots, wind and percussion. |
| CUTOFF (Filter) | 20 to 20,000 Hz | 8,000 Hz | Filter cutoff. |
| RESO (Resonance) | 0.5 to 20 Q | 1 Q | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, Band Pass, High Pass | Low Pass | Filter mode. |
| ENV AMT (Filter Env Depth) | 0 to 8 octaves | 0 | How far the filter envelope opens the cutoff. At 0 the filter envelope has no effect. |
| ATK (Filter Attack) | 0.5 to 3,000 ms | 1 ms | Filter envelope attack time. |
| DEC (Filter Decay) | 5 to 4,000 ms | 300 ms | Filter envelope decay time. |
| SUS (Filter Sustain) | 0 to 1 | 70 % | Filter envelope sustain level. The filter release is the amp RELEASE. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 40 % | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.5 to 3,000 ms | 2 ms | Amplitude envelope attack time. |
| DECAY (Decay) | 5 to 4,000 ms | 300 ms | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 70 % | Amplitude envelope sustain level. |
| RELEASE (Release) | 5 to 5,000 ms | 250 ms | Amplitude envelope release time (also the filter envelope release). |
| VOLUME (Volume) | 0 to 1.5 (shown in dB) | 1.0 (0 dB) | Master level. |
| GLIDE (Glide) | 0 to 1,000 ms | 0 | Portamento time. In Mono mode, glide is legato: the new note slides without retriggering. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 2 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 % | LFO depth. The mod wheel (on-screen or MIDI CC1) adds to this value. |
| LFO SHAPE (LFO Source menu) | Sine, Square, Sawtooth, Triangle, Noise | Sine | LFO shape. |
| LFO DESTINATION (Destination menu) | Pitch, Filter, Amplitude | Pitch | LFO target: vibrato, filter wobble or tremolo. |
| VOICE MODE (Mono/Poly menu) | Poly, Mono | Mono | Mono: one note at a time, last-note priority, legato glide. Poly: up to 16 voices; when all voices are busy, a voice in release is taken first. |
| UNISON (Unison) | 1 to 7 | 1 | Number of detuned copies of each note. |
| DETUNE (Unison Detune) | 0 to 60 cents | 0 | Detune between the unison copies. |
| Unison Width (no knob) | 0 to 1 | 50 % | Stereo spread of the unison copies. |
| DISTORTION (Distortion) | 0 to 1 | 0 % | Distortion amount (drive up to 24 dB with an increasing mix). |
| Distortion Tone (no knob) | 100 to 8,000 Hz | 5,000 Hz | Tone of the distortion. |
| CHORUS (Chorus) | 0 to 1 | 0 % | Chorus mix. |
| Chorus Rate (no knob) | 0.1 to 8 Hz | 1.5 Hz | Chorus modulation speed. |
| Chorus Depth (no knob) | 0 to 1 | 50 % | Chorus modulation depth. |
| DLY MIX (Delay Mix) | 0 to 1 | 0 % | Level of the delay. |
| DLY TIME (Delay Time) | 50 to 1,000 ms | 250 ms | Delay time in milliseconds. Not tempo-synced: for a quarter note, use 60,000 / BPM (500 ms at 120 BPM). |
| DLY FDBK (Delay Feedback) | 0 to 0.9 | 30 % | Number of delay repeats. |
| REVERB (Reverb) | 0 to 1 | 10 % | Reverb mix. |
| Reverb Room (no knob) | 0 to 1 | 40 % | Room size. |
| Reverb Damping (no knob) | 0 to 1 | 50 % | High-frequency damping of the reverb. |
| Reverb Width (no knob) | 0 to 1 | 100 % | Stereo width of the reverb. |
| PITCH BEND (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| MOD WHEEL (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds to the LFO AMOUNT, together with MIDI CC1 (the larger of the two applies). |

## Factory presets

WARZONE808 includes 60 factory presets in 6 banks of 10, plus the 10 presets of the built-in **Expansion Vol.1**. Of the 60 factory presets, 34 load in Mono mode (808 Subs (all 10); Dirty & Reese (all 10); Leads (all 10); FX & Percussion (4 of 10)) and 26 in Poly mode. 9 load with a glide and 12 use unison.

**808 Subs:** 808 Classic, 808 Boom, 808 Short, 808 Slide, 808 Dirty, 808 Triangle, 808 Deep, 808 Tuned Down, 808 Punch, 808 Trap Long.

**Dirty & Reese:** Reese Bass, Warzone Growl, Grit Sub, Siren Bass, Detune War, Tank Bass, Mud Reese, Rumble, Subsonic Drift, Overdrive 808.

**Leads:** War Horn, Bunker Lead, Cavalry Charge, Mono Hero, Night Vision, Sniper, Siren Lead, Sci-Fi Comm, Trench Whistle, Combat Saw.

**Plucks & Keys:** Ammo Pluck, Boom Bap Keys, Dusty Keys, Saw Pluck, Metal Pluck, Soldier Bell, Dark Chord, Rust Keys, Radio Piano, Bunker Marimba.

**Pads & Ambience:** Foxhole Pad, Smoke Screen, Fallout, Radar Sweep, Dark Drone, Night Patrol, Ghost Radio, Cold War Strings, Wasteland, Evac Dawn.

**FX & Percussion:** Explosion, Gunshot, Helicopter, Air Raid, Radio Chatter, Tank Rumble, Laser, Impact, Static Burst, Kick 808.

**Expansion Vol.1:** Vol1 Sub Drop, Vol1 Grunt, Vol1 Convoy, Vol1 Signal Flare, Vol1 Command Keys, Vol1 Barracks Pad, Vol1 Drone Strike, Vol1 Pluck Squad, Vol1 Rust Lead, Vol1 Ghost Comm.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.wz808preset` file. The default folder is `DJBILBOX BEATS/WARZONE808/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu.

**Expansion packs.** An expansion pack is a `.wz808exp` text file (JSON, format `warzone808-expansion`) that contains a name and a list of presets. Packs placed in `DJBILBOX BEATS/WARZONE808/Expansions` inside your Documents folder are loaded at start-up, and their presets appear in the preset menu under the pack name. The cassette menu also lets you load a pack from any location. An invalid file is refused with a message and does not change the plugin.

The host also sees the presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Trap and Drill 808s.** Start from **808 Classic** or **808 Trap Long**. The 808 presets use a sine, a long decay, no sustain and a low cutoff. Raise DISTORTION or open the CUTOFF a little so the 808 is audible on phone speakers.
2. **808 slides.** In Mono mode with GLIDE above zero, each new note slides from the previous one without retriggering (see **808 Slide**). For slides on selected notes only, keep GLIDE at 0 and automate it before the notes that should slide.
3. **Punch.** **808 Punch** uses the filter envelope (ENV AMT at 3 octaves with a short decay) for a hard attack.
4. **Reese and growl.** **Reese Bass** and **Detune War** use sawtooth waves in unison. Move the filter with the LFO set to Filter for a wobble, as in **Warzone Growl**.
5. **Leads.** **War Horn** and **Combat Saw** use unison for width. Add a little DLY MIX with DLY TIME set from 60,000 / BPM.
6. **Pads and risers.** The Pads & Ambience bank loads in Poly with long ATTACK and RELEASE values. **Radar Sweep** uses the LFO on the filter and a delay. **Air Raid** (FX & Percussion bank) uses glide and the LFO on pitch for a siren.
7. **War effects.** The FX & Percussion bank uses the Noise waveform: **Explosion**, **Gunshot**, **Helicopter** and **Radio Chatter**. Layer them under your drops.
8. **Layering a kick.** There is no pitch-drop envelope. Layer a kick sample or use **Kick 808** (a sine with a fast filter envelope) in a second instance.

## MIDI, automation and performance

**Notes and channels.** WARZONE808 responds to all MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** In Poly mode, up to 16 voices. When all voices are in use, a voice that is already releasing is taken over first. In Mono mode, a list of held notes is kept: the most recent note plays, and releasing it returns to the note you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen PITCH BEND wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen MOD WHEEL. When both are used, the higher value wins. CC121 resets the MIDI wheels and the sustain pedal.

**Sustain pedal.** CC64 is supported. All Notes Off and All Sound Off messages stop the sound.

**Automation in FL Studio.** All 38 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. Volume, cutoff and transpose changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples). It reports an 8-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** WARZONE808 uses no samples. Unison multiplies the number of oscillators per note: use fewer unison voices on dense Poly parts.

## FAQ

**WARZONE808 does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **WARZONE808** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**My chord only plays one note.**
The plugin starts in Mono mode. Switch VOICE MODE to Poly (up to 16 voices).

**Every note slides into the next one.**
GLIDE is above zero. Set GLIDE to 0, or automate it only where you want slides.

**Does it have a pitch envelope for 808 kicks?**
No. There is no pitch-drop envelope. Use **Kick 808** for a short sine kick, or layer a kick.

**How do I add more presets?**
Load a `.wz808exp` expansion pack with the cassette menu, or place it in the Expansions folder.

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.

**Is the delay tempo-synced?**
No. Delay time is set in milliseconds (50 to 1,000 ms) and the LFO runs in Hz (0.05 to 20 Hz).
