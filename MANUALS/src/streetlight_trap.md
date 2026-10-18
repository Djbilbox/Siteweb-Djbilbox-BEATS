# STREETLIGHT TRAP

Dark trap synthesizer: 808 subs, trap bells, dark plucks, street keys, neon pads, trap choir, brass stabs and flutes from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |


## Overview

STREETLIGHT TRAP is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: 808 Sub (a sine with a pitch knock and saturation), Trap Bell (an FM bell), Dark Pluck (saws with a closing filter and a pitch dive), Street Keys (an electric-piano model), Neon Pad (detuned saws with a sub), Trap Choir (five saws through vowel formants), Brass Stab (saws with an opening filter) and Flute (a sine with harmonics, breath and vibrato).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. The character is dark and nocturnal.

STREETLIGHT TRAP targets Trap and Drill 808s, bell and pluck melodies, dark pads and choirs, brass stabs and flute hooks. 52 presets load in Poly mode with up to 16 voices; 18 load in Mono mode for single-note lines.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **STREETLIGHT TRAP** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **STREETLIGHT TRAP - Init Patch**: the 808 Sub engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 36 to 84 (C3 to C7 in FL Studio naming, where middle C is C5).
5. Shape the sound with the main controls:
    - **TONE, BODY, BRIGHT, WIDTH, TEXTURE**: the small knobs, which reshape the selected engine.
    - **CUTOFF**: the cutoff of the filter.
    - **GLIDE**: slide time between notes (808 slides).
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJ BILBOX BEATS name and the DB logo.

**Center.** The artwork, the STREETLIGHT TRAP title and "By Humpire". Two clusters of small knobs sit on each side: **TONE, BODY, BRIGHT, WIDTH** to the left of the artwork, and the large **GLIDE** and **CUTOFF** knobs below it.

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, in the same row, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**FILTER ENVELOPE (left, below the LFO).** Four sliders: **ATK, DEC, SUS, REL**. Next to them: **TEXTURE, RES** (filter resonance) and **ENV AMT** (filter envelope depth). **VELOCITY** (velocity sensitivity of the level) is a small knob, with **REVERB**, in the center-right column.

**AMP ENVELOPE (top right).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**EFFECTS (right).** **CHORUS, DISTORTION, DLY MIX** (delay level), **DLY TIME** and **DLY FDBK** (delay repeats) in a row. The **VOLUME** knob sits above the row at the right.

**Menu row.** Five drop-down menus, each with a caption: **ENGINE** (Waveform), **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE**.

**Wheels.** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard.** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard covers MIDI notes 36 to 84 (four octaves plus one key). The key labels on screen run from C1 to C5, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C3 to C7. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

STREETLIGHT TRAP exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | 808 Sub, Trap Bell, Dark Pluck, Street Keys, Neon Pad, Trap Choir, Brass Stab, Flute | 808 Sub | Selects the synthesis engine. |
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
| DLY MIX (Delay Mix) | 0 to 1 | 0 % | Level of the ping-pong delay. |
| DLY TIME (Delay Time) | 0.05 to 1 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| DLY FDBK (Delay Feedback) | 0 to 0.9 | 35 % | Number of delay repeats (0 to 90% feedback). |
| REVERB | 0 to 1 | 25 % | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Mod Wheel | 0 to 1 | 0 % | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT; the larger of this wheel and MIDI CC1 applies. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| 808 Sub | Pitch "knock" at the attack | Adds the octave harmonic | Attack click | No effect | Saturation |
| Trap Bell | Modulator ratio (1.4 to 4.5) | Ring length | FM depth (bell brightness) | Detune between the two carriers | Attack noise |
| Dark Pluck | Pitch-dive depth | Length of the filter closing | Initial opening of the filter | Detune between the two saws | Attack noise |
| Street Keys | No effect | Decay length | Bark: level of the upper partials, stronger with velocity | No effect | Attack noise |
| Neon Pad | No effect | Level of a sine one octave below | Low-pass opening | Detune spread of the five saws | Noise |
| Trap Choir | Vowel (a, o, u, i) | Formant level | Low-pass opening | Detune spread of the five saws | Noise |
| Brass Stab | Time for the filter to open | Output weight | How far the filter opens | Detune of the three saws | Noise |
| Flute | Breath filter frequency | Breath filter sharpness | Level of the harmonics | Vibrato depth | Breath noise level |


## Factory presets

STREETLIGHT TRAP includes 70 factory presets in 7 banks of 10. 52 presets load in Poly mode and 18 in Mono mode. Mono presets: 808 & Sub Bass (all 10); Trap Choir (1 of 10); Flutes & FX (7 of 10). 7 presets load with a glide.



**808 & Sub Bass:** Streetlight 808, Long Slide 808, Dirty 808, Boom Bap Sub, Hard Knock, Deep Cellar, Short 808, Distorted Rumble, Smooth Sub, Pitch Drop.

**Trap Bells:** Trap Bell, Music Box Trap, Glass Lead, Dark Chimes, Night Bells, Kalimba Trap, Icy Pluck, Haunted Box, Low Bell, Echo Bells.

**Dark Plucks:** Dark Pluck, Trap Arp, Zap Down, Muted Pluck, Soft Pluck, Pluck Wall, Street Harp, Dark Koto, Drill Pluck, Sad Pluck.

**Street Keys:** Street EP, Lofi Keys, Bark EP, Trap Soul Keys, Tremolo Keys, Midnight Rhodes, Wurly Bite, Bell Keys, Rooftop Chords, Dusty Piano.

**Neon Pads & Brass:** Streetlight Pad, Smoke Pad, Alley Strings, Slow Rise, Dream Pad, Dark Drone, Trap Brass, Horn Stab, Soft Brass, Heist Horns.

**Trap Choir:** Trap Choir, Ooo Voices, Ah Gospel, Ee Ghost, Vowel Drift, Whisper Crowd, Low Choir, Chopped Vox, Hymn In The Rain, Hollow Chant.

**Flutes & FX:** Trap Flute, Pan Flute, Breathy Flute, Sad Flute, High Whistle, Flute Echo, Wind Lead, Low Flute, Fog Flute, Siren Flute.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.slpreset` file. The default folder is `DJBILBOX BEATS/STREETLIGHT_TRAP/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .slpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Trap and Drill 808s.** Start from **Streetlight 808** or **Long Slide 808**. Raise TONE for a harder knock on short notes. Raise TEXTURE or add DISTORTION so the 808 stays audible on phone speakers.
2. **808 slides.** With GLIDE above zero, every new note slides from the previous one. For slides on selected notes only, keep GLIDE at 0 and automate it before those notes.
3. **Bell melodies.** On the Trap Bell engine, TONE sets the bell ratio, BODY the ring length and BRIGHT the FM depth. Add DLY MIX for a classic trap bell.
4. **Dark plucks.** The Dark Pluck engine closes its filter after each note: BODY sets how long it takes, BRIGHT how open it starts.
5. **Keys and pads.** The Street Keys and Neon Pads & Brass banks load in Poly. Add CHORUS and REVERB for width.
6. **Brass stabs.** **Trap Brass** and **Heist Horns** use short ATTACK and SUSTAIN values; TONE sets how quickly the filter opens.
7. **Tempo-matched delay.** DLY TIME is in seconds. Use 60 / BPM for a quarter note.

## MIDI, automation and performance

**Notes and channels.** STREETLIGHT TRAP responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead. CC121 (reset all controllers) returns the MIDI mod wheel to zero.

**Other MIDI.** All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. Level, pitch, bend and cutoff changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples). It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** STREETLIGHT TRAP uses no samples. Effects set to zero are bypassed and cost no CPU. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**My 808 is not perfectly centered.**
Each note gets a slight random stereo offset. For a strictly centered sub, reduce the stereo separation of the mixer track in FL Studio, or insert a mono utility after the plugin.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono, which is the default for the 808 and flute presets. Switch it to Poly (up to 16 voices).

**STREETLIGHT TRAP does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **STREETLIGHT TRAP** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.

**Is the delay tempo-synced?**
No. Delay time is in seconds (0.05 to 1 s), and the LFO runs in Hz (0.05 to 20 Hz).
