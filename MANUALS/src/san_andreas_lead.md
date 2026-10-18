# SAN ANDREAS LEAD

G-funk lead synthesizer: whistles, Moog-style leads, square leads, talkbox, funk bass, Rhodes-style keys, strings and clav from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |


## Overview

SAN ANDREAS LEAD is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: G-Funk (a sine and a soft saw with a vibrato that fades in and a "whine" on the second harmonic), Moog (two saws, a square and a sub octave), Square (a PWM lead with two detuned pulses), Talkbox (a saw through vowel formants), Bass (a funk bass with a snapping filter), Rhodes (an FM electric piano), Strings (a lowrider string ensemble) and Clav (a narrow pulse with a fast filter).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. The character is sunny and smooth, built around gliding mono leads.

SAN ANDREAS LEAD targets G-Funk and West Coast Hip-Hop, funk and R&B hooks, Trap leads and lowrider bass lines. 48 presets load in Mono mode (most with glide) and 22 in Poly mode, up to 16 voices.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **SAN ANDREAS LEAD** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **SAN ANDREAS LEAD - Init Patch**: the G-Funk engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 48 to 96 (C4 to C8 in FL Studio naming, where middle C is C5).
5. Shape the sound with the main controls:
    - **TONE, BODY, BRIGHT, WIDTH, TEXTURE**: the small knobs on each side of the artwork, which reshape the selected engine.
    - **CUTOFF**: the cutoff of the filter.
    - **GLIDE**: slide time between notes. Most lead presets use it.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save), under a **PRESET** caption. The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOX BEATS name and the DB logo.

**Center.** The artwork, the SAN ANDREAS LEAD title and "By Humpire".

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, in the same row, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**AMP ENVELOPE (left, below the LFO).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Character controls.** A column of four small knobs to the left of the artwork: **TONE, BODY, BRIGHT, WIDTH**. A column of three to its right: **TEXTURE, RES** (filter resonance) and **ENV AMT** (filter envelope depth). The large **GLIDE** and **CUTOFF** knobs sit below the artwork, with **VOLUME** to their right.

**FILTER ENVELOPE (right).** Five vertical sliders: **ATK, DEC, SUS, REL** and **VELOCITY** (velocity sensitivity of the level).

**Effects (right).** **REVERB, CHORUS, DISTORTION** on the upper row. Under a **DELAY** caption: **TIME**, **FEEDBACK** and **DELAY MIX** (delay level).

**Menu row.** Five drop-down menus, each with a caption: **ENGINE** (Waveform), **LFO SHAPE**, **LFO DEST** (Destination), **FILTER TYPE** and **VOICE MODE**.

**Wheels.** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard.** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard covers MIDI notes 48 to 96 (four octaves plus one key). The key labels on screen run from C2 to C6, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C4 to C8. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

SAN ANDREAS LEAD exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | G-Funk, Moog, Square, Talkbox, Bass, Rhodes, Strings, Clav | G-Funk | Selects the synthesis engine. |
| TONE | 0 to 1 | 50 % | Character control 1. Engine-dependent (see the table below). |
| BODY | 0 to 1 | 50 % | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 50 % | Character control 3 (brightness). Engine-dependent. |
| WIDTH | 0 to 1 | 30 % | Character control 4. Engine-dependent (detune, vibrato, sweep speed...). It is not a stereo width control. |
| TEXTURE | 0 to 1 | 20 % | Character control 5. Engine-dependent (noise, click, saturation...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 15 % | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode (state-variable filter, 12 dB per octave). Band Pass gets a small level boost. |
| ENV AMT (Filter Env Amount) | 0 to 1 | 0 % | Filter envelope depth. At 100 % the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| ATK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DEC (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| SUS (Filter Sustain) | 0 to 1 | 0 % | Filter envelope sustain level. |
| REL (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 50 % | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN | 0 to 1 | 70 % | Amplitude envelope sustain level. |
| RELEASE | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME | 0 to 1 (shown in dB) | 0.7 (-3.1 dB) | Output level of each voice, before the effects. |
| GLIDE | 0 to 2 s | 0 s | Portamento. The value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. The LFO restarts at every new note. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 % | LFO depth. The mod wheel (on-screen or MIDI CC1) adds up to 0.8 to this value. |
| LFO SHAPE (LFO Source menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION (Destination menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| VOICE MODE (Mono/Poly menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION | 0 to 1 | 0 % | Saturation on the summed output, with automatic level compensation. |
| CHORUS | 0 to 1 | 0 % | Stereo chorus mix (slow, fixed rate). |
| DELAY MIX | 0 to 1 | 0 % | Level of the ping-pong delay. |
| TIME (Delay Time) | 0.05 to 1 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 35 % | Number of delay repeats (0 to 90% feedback). |
| REVERB | 0 to 1 | 25 % | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Mod Wheel | 0 to 1 | 0 % | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT; the larger of this wheel and MIDI CC1 applies. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| G-Funk | Level of the soft saw added to the sine | Level of the sine (0.6 to 1.0) | Low-pass opening | Vibrato depth (fades in after 0.15 s) | Level of the second-harmonic "whine" |
| Moog | Level of the second saw | Level of the sub octave | Low-pass opening | Detune of the second saw | Level of the square |
| Square | Depth of the pulse-width modulation | Level of the sub octave | Low-pass opening | Detune of the second pulse | Speed of the pulse-width modulation (0.6 to 4.6 Hz) |
| Talkbox | Vowel (a, o, u, i) | Formant level | Level of the direct, unfiltered saw | Speed and depth of the vowel sweep | Noise |
| Bass | Saw level | Sub level | Base opening of the filter | Length of the filter snap | Depth of the filter snap |
| Rhodes | Level of the tine bell partial | Decay length | FM depth | Tremolo speed (4.5 to 7.5 Hz) | Tremolo depth |
| Strings | Output level (0.6 to 1.0) | Level of a sine at the fundamental | Low-pass opening | Detune spread of the five saws | Noise |
| Clav | Depth of the filter snap | Length of the filter snap | Base opening of the filter | Pulse width (0.08 to 0.48) | Resonance of the filter snap |


## Factory presets

SAN ANDREAS LEAD includes 70 factory presets in 7 banks of 10. 22 presets load in Poly mode and 48 in Mono mode. Mono presets: G-Funk Whistles (all 10); Moog Leads (all 10); Square Leads (9 of 10); Talkbox (9 of 10); Funk Bass (all 10). 39 presets load with a glide.



**G-Funk Whistles:** Compton Whistle, Dre Whine, Long Beach Glide, Sunset Boulevard, Whistle Pure, Sharp Whine, Cruise Control, Hydraulic, High Rider, Nate Dogg Air.

**Moog Leads:** Moog Lowrider, Fat Funk Lead, Chronic Lead, Saw Cruiser, Slide Lead, Pimp Lead, Smooth Moog, Acid Cali, Octave Sub Lead, Wide Moog.

**Square Leads:** Square Cruiser, PWM Lowrider, Hollow Square, Chiptune Cali, Wide Pulse, Pulse Slide, Funky Square, Soft Pulse Pad, Buzz Square, Dirty South Lead.

**Talkbox:** Roger Talk, Talk Ah-Oh, Talk Ee-Oo, Wah Mouth, Robot Funk, Slow Vowel, Choir Talk, Whisper Box, Low Talker, Zapp Voice.

**Funk Bass:** Funk Slap, Sub Cruiser, G-Funk Bass, Pop Bass, Wobble Low, 808 Cali, Moog Bass, Dirty Funk, Round Bass, Slide Bass.

**West Coast Keys:** Rhodes Cali, Mellow Rhodes, Bell Rhodes, Tremolo Keys, Funk Clav, Clav Stab, Chrome Pluck, Wah Clav, Piano Funk, Dre Stab.

**Lowrider Strings:** Lowrider Strings, Sunset Pad, Velvet Strings, Smooth Ensemble, Palm Tree Pad, Hot Summer, Cali Night, Stab Strings, Low Ride Brass, Fog Pad.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.sapreset` file. The default folder is `DJBILBOX BEATS/SAN_ANDREAS_LEAD/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .sapreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

Moog, Rhodes and Clav are trademarks of their owners. They are used here only to describe a sound style. DJBILBOX BEATS is not affiliated with the trademark owners.

## Production tips

1. **G-funk whistle.** Start from **Compton Whistle** or **Dre Whine**. Keep GLIDE at 0.1 to 0.3 and play legato; raise WIDTH for a deeper vibrato.
2. **Moog-style leads.** The Moog Leads bank loads in Mono with glide. TONE blends the second saw and BODY adds the sub octave (see **Fat Funk Lead**).
3. **Talkbox.** TONE selects the vowel and WIDTH sweeps between vowels. Set LFO DESTINATION to Timbre for movement (see **Talk Ah-Oh**).
4. **Lowrider bass.** The Funk Bass bank loads in Mono. **Sub Cruiser** and **Slide Bass** use glide for 808-style slides.
5. **West Coast keys.** The West Coast Keys bank loads in Poly: Rhodes-style presets (**Rhodes Cali**, **Mellow Rhodes**) and clav presets (**Funk Clav**, **Clav Stab**).
6. **Layering.** Combine a whistle and a bass in two instances for a full G-funk arrangement.
7. **Delay in time with the beat.** TIME is in seconds. Use 60 / BPM for a quarter note.

## MIDI, automation and performance

**Notes and channels.** SAN ANDREAS LEAD responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead. CC121 (reset all controllers) returns the MIDI mod wheel to zero.

**Other MIDI.** All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. Level, pitch, bend and cutoff changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples). It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** SAN ANDREAS LEAD uses no samples. Effects set to zero are bypassed and cost no CPU. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**My chord only plays one note.**
Most lead, talkbox and bass presets load in Mono. Switch VOICE MODE to Poly (up to 16 voices).

**Every note slides into the next one.**
GLIDE is above zero. Set GLIDE to 0, or automate it only where you want slides.

**SAN ANDREAS LEAD does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **SAN ANDREAS LEAD** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.

**Is the delay tempo-synced?**
No. Delay time is in seconds (0.05 to 1 s), and the LFO runs in Hz (0.05 to 20 Hz).
