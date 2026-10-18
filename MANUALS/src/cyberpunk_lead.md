# CYBERPUNK LEAD

Neon lead synthesizer: supersaws, PWM, glitch FM, reese bass, laser plucks, brass, glass bells and bit-crushed FX from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

CYBERPUNK LEAD is a lead synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Neon Saw (a five-saw supersaw with sub and octave layers), Chrome PWM (two pulse waves with a moving pulse width), Glitch FM (two-operator FM with feedback), Reese Bass (detuned saws with a sub and saturation), Laser Pluck (a saw with a pitch dive and a closing filter), Neon Brass (detuned saws with an opening filter), Data Crush (a saw run through sample-rate and bit-depth reduction) and Hollow Glass (FM bells).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb.

CYBERPUNK LEAD targets Trap and Drill leads, Synthwave and Retrowave, Hip-Hop hooks, Afrotrap melodies, G-Funk portamento leads and dark electronic bass lines. The leads, basses and FX load in Mono for clean single-note lines. The Laser Plucks, Neon Brass & Pads and part of the other banks load in Poly mode, which plays up to 16 voices.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **CYBERPUNK_LEAD** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **CYBERPUNK LEAD - Init Patch**: the Neon Saw engine in Poly mode with default settings.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Leads sit well between C5 and C7 in FL Studio naming (middle C is C5). The Reese & Bass presets work an octave or two lower.
5. Shape the sound with the three main controls:
    - **TONE**: the main character of the engine (sub level, PWM depth, FM ratio, pitch dive, crush rate...).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a softer, darker lead.
    - **GLIDE**: slide time between notes. Leave it at zero for straight notes.
6. Adjust **VOLUME** (top right, above REVERB and CHORUS) to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels on a dark steel faceplate with pink and yellow neon accents. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The camel artwork, the "Cyberpunk Lead" title and "By Humpire". The dotted grilles on each side of the artwork are decorative.

**Character knobs (center).** Four small knobs, **TONE, BODY, BRIGHT, WIDTH**, sit left of the **GLIDE** knob. They are four of the five character controls of the selected engine. The fifth, **TEXTURE**, sits to the right of **CUTOFF**, together with **RES** (filter resonance), **ENV AMT** (filter envelope depth) and **DELAY MIX** (delay level).

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, also under this header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**AMP ENVELOPE (left).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**FILTER ENVELOPE (top right).** Five sliders: **ATTACK, DECAY, SUSTAIN, RELEASE** (the filter envelope) and **VELOCITY** (velocity sensitivity of the level, not part of the filter envelope).

**Output and effects (right).** Top row: **VOLUME**, **REVERB**, **CHORUS**. Bottom row: **DISTORTION**, **DLY TIME** (delay time), **FEEDBACK** (delay repeats).

**Menu row.** Five drop-down menus, each with a caption below it: **ENGINE** (Waveform), **LFO SHAPE** (LFO Source), **LFO DESTINATION** (Destination), **FILTER TYPE** and **VOICE MODE** (Mono/Poly). Each menu shows its current value.

**Wheels (bottom left).** **PITCH BEND** springs back to the center when you release it. **MODULATION** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. The keyboard covers MIDI notes 48 to 96: four octaves plus one key. Its key labels run from C2 to C6; FL Studio names the same notes C4 to C8 (middle C is C5).

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

CYBERPUNK LEAD exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Neon Saw, Chrome PWM, Glitch FM, Reese Bass, Laser Pluck, Neon Brass, Data Crush, Hollow Glass | Neon Saw | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune, drive, PWM speed...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (noise, click, sub...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| ENV AMT (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| FILTER ATTACK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| FILTER DECAY (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| FILTER SUSTAIN (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| FILTER RELEASE (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Output level of each voice, before the effects. |
| GLIDE (Glide) | 0 to 2 | 0 | Portamento. The value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The mod wheel adds to this value. |
| LFO SHAPE (LFO Source menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION (Destination menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| VOICE MODE (Mono/Poly menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (slow, fixed rate). |
| DLY MIX (Delay Mix) | 0 to 1 | 0 | Level of the ping-pong delay. |
| DLY TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| PITCH BEND wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| MODULATION wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Neon Saw | Level of the sub-octave sine | Supersaw detune | Opening of the built-in low-pass | Level of an added octave-up saw | Noise |
| Chrome PWM | Pulse-width modulation depth | Detune of the second pulse | Opening of the built-in low-pass | PWM speed (0.3 to 5.3 Hz) | Level of the sub-octave sine |
| Glitch FM | Modulator ratio (0.5 to 4.0) | Decay time of the FM depth | FM depth | Modulator feedback | Attack click |
| Reese Bass | Sub-octave level | Detune amount | Opening of the built-in low-pass | Saturation drive | Noise |
| Laser Pluck | Depth of the pitch dive at the attack | Closing time of the built-in filter | Opening of the built-in filter | Detune of the second saw | Attack click |
| Neon Brass | Swell time of the built-in filter | Output weight | How far the filter opens | Detune of the three saws | Noise |
| Data Crush | Crush rate (sample-and-hold length) | Bit depth (2 to 12 bits) | Opening of the built-in low-pass | Level of an added pulse | Noise |
| Hollow Glass | Modulator ratio (1.4 to 4.5) | Bell decay length | FM depth | Detune of the second oscillator | Attack click |

## Factory presets

CYBERPUNK LEAD includes 70 factory presets in 7 banks of 10. 32 presets load in Mono mode and 38 load in Poly mode.

**Neon Leads** (Neon Saw; Poly: Supersaw Wall): Neon Rider, Chrome Hero, Night Drive, Hacker Lead, Laser Solo, Supersaw Wall, Soft Neon, Blade Lead, Octave Jump, Wire Lead.

**Chrome & Pulse** (Chrome PWM): Chrome Pulse, Wide PWM, Cyber Bass PWM, Fast Wobble, Retro Keys, Sunset Pad, Pulse Stab, Hollow Pulse, Arcade Lead, Pulse Drone.

**Glitch FM** (Glitch FM): Glitch Bell, Data Pluck, FM Lead, Metal Keys, Digital Choir, Alien Voice, Cyber Marimba, Circuit Bass, Scan Lines, Neural Net.

**Reese & Bass** (Reese Bass, all Mono): Reese Core, Neon Sub, Dirty Reese, Wobble Grid, Industrial Bass, Cyber Slide, Acid Reese, Subway Bass, Bass Stab, Meltdown.

**Laser Plucks** (Laser Pluck, all Poly): Laser Pluck, Neon Arp, Zap Down, Chrome Pluck, Techno Stab, Soft Pluck, Synth Harp, Cyber Koto, Sci-Fi Drop, Pluck Wall.

**Neon Brass & Pads** (Neon Brass, then Hollow Glass; all Poly): Neon Brass, Blade Runner Pad, Synthwave Stab, Chrome Horns, Slow Open, Warm Neon, Hollow Glass, Ice Bell, Crystal Rain, Ghost Chime.

**Data Crush & FX** (Data Crush, plus Boot Sequence on Laser Pluck and Modem Dial on Glitch FM): Data Crush, 8-Bit Lead, Bit Rain, Glitch Bass, Corrupted Pad, Signal Lost, Modem Dial, Firewall, Static Wind, Boot Sequence.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.cppreset` file. The default folder is `DJBILBOX BEATS/CYBERPUNK_LEAD/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu, and **Open a .cppreset file...** loads a file from anywhere.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Trap and Drill leads.** Start from Neon Rider or Hacker Lead. Raise BRIGHT for a sharper edge, keep GLIDE at 0 for tight notes and add a little DELAY MIX with DLY TIME at 0.25 to 0.5 s.
2. **Synthwave and Retrowave.** Load Night Drive, Supersaw Wall or a preset from Neon Brass & Pads. Raise CHORUS and REVERB, set VOICE MODE to Poly and play chords.
3. **Portamento leads.** With GLIDE above zero, every new note slides from the previous one, even if the notes do not overlap. Around 0.1 to 0.2 gives a classic G-Funk glide. For slides on selected notes only, keep GLIDE at 0 and automate it up just before those notes.
4. **Dark bass lines.** The Reese & Bass bank loads in Mono. On the Reese engine, BODY sets the detune and WIDTH drives the saturation. Set LFO DESTINATION to Filter, LFO SHAPE to Sine and RATE between 1 and 4 Hz for a wobble (see Wobble Grid).
5. **Plucks and arps.** In the Laser Plucks bank, TONE sets how deep the pitch dive is and BODY sets how fast the filter closes. Shorten DECAY and RELEASE for tight arpeggios.
6. **Filter movement.** Raise ENV AMT and shape the FILTER ENVELOPE sliders: a short FILTER DECAY with FILTER SUSTAIN at 0 gives a pluck at the start of each note, a longer FILTER ATTACK gives a swell. Automate CUTOFF over 8 or 16 bars for builds.
7. **Glitch and FX.** In Data Crush, lower TONE for a lower sample rate and lower BODY for fewer bits. Sample&Hold on the LFO with DESTINATION set to Pitch or Timbre gives random movement.
8. **Bells and glass.** The Hollow Glass presets decay naturally (SUSTAIN is 0): play them with long RELEASE and add REVERB. Change TONE to move the FM ratio from soft to metallic.

## MIDI, automation and performance

**Notes and channels.** CYBERPUNK LEAD responds to MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over. In Mono mode, each new note stops the previous one (last-note priority).

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen PITCH BEND wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen MODULATION wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives and the most recent position is applied to new notes.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** Effects set to zero are bypassed and cost no CPU. Mono mode and short release times keep the voice count low.

## FAQ

**CYBERPUNK LEAD does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **CYBERPUNK_LEAD** under **DJBILBOX BEATS**. It is an instrument: load it from the **Generators** list.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Check VOLUME, and check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. With FILTER TYPE on High Pass, low notes are filtered out. Some presets, such as Static Wind, start slowly because of a long ATTACK.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono, which is the default for most lead and bass presets. Switch it to Poly (up to 16 voices).

**Is the LFO or the delay synced to the tempo?**
No. The LFO runs in Hz (0.05 to 20 Hz) and the delay time is set in seconds (0.05 to 1 s).

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.
