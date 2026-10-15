# CYBERPUNK LEAD

Neon lead synthesizer: supersaws, PWM, glitch FM, reese bass, laser plucks, brass, glass bells and bit-crushed FX.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

CYBERPUNK LEAD is a neon, synthwave-flavored lead synthesizer by Humpire / DJBILBOX BEATS. It uses no samples. Eight engines cover the modern and retro-futuristic palette: a five-voice Neon Saw supersaw, a Chrome PWM with moving pulse width, a two-operator Glitch FM, a detuned Reese Bass, a Laser Pluck with a pitch dive, a Neon Brass section, a Data Crush engine with sample-rate and bit reduction, and a Hollow Glass FM bell.

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb. The character is bright, wide and cutting, built to sit on top of a busy beat.

CYBERPUNK LEAD targets Trap and Drill leads, Synthwave and Retrowave, Hip-Hop hooks, Afrotrap melodies, G-Funk portamento leads, plus dark Reese bass lines and glitchy sound effects. It plays up to 16 notes at once. Most lead and bass presets load in Mono mode, many of them with a short glide.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **CYBERPUNK_LEAD** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **CYBERPUNK LEAD - Init Patch**: the Neon Saw engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Leads sit well between C5 and C7 in FL Studio. The Reese & Bass presets are transposed one octave down: play them between C3 and C5.
5. Shape the sound with the three main controls:
    - **TONE**: the main character of the engine (sub level, PWM depth, FM ratio, pitch dive, crush rate...).
    - **FILTER**: the cutoff of the filter. Turn it down for a softer, darker lead.
    - **GLIDE**: slide time between notes. Leave it at zero for straight notes.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels on a dark steel faceplate with pink and yellow neon accents. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The camel artwork between two decorative speaker grilles, the "Cyberpunk Lead" title and "By Humpire".

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH**, also under this header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**Amplitude envelope (left).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Character knobs.** A block of four small knobs next to the amplitude envelope: **TONE, BODY** (top) and **BRIGHT, WIDTH** (bottom). The fifth character control, **TEXTURE**, sits in a row with **RES** (filter resonance) and **ENV** (filter envelope depth) at the right of the FILTER knob. The **DELAY** knob (delay level) sits below RES.

**GLIDE and FILTER.** The two large knobs in the middle of the panel.

**ADSR+VELOCITY section (upper right).** Five sliders without individual labels. From left to right: filter envelope **Attack**, **Decay**, **Sustain**, **Release**, then **Velocity** (velocity sensitivity of the level). The value bubble shows the setting while you drag.

**Output and effects (right).** First row: **VOLUME, REVERB, CHORUS**. Second row: **DISTORTION, TIME** (delay time) and **FEEDBACK** (delay repeats).

**Menu row.** Five drop-down menus, from left to right: **Waveform** (engine), **LFO Source** (LFO shape), **Destination** (LFO target), **Filter Type** and **Mono/Poly**. Each menu shows its current value.

**Wheels (bottom left).** **PITCH BEND** springs back to the center when you release it. **MODULATION** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard starts at the lowest MIDI note (labeled C-2, which is C0 in FL Studio) and does not scroll. It covers MIDI notes 0 to 43, which is very low for lead sounds. Use a MIDI controller or the Piano roll for normal playing.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

CYBERPUNK LEAD exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Waveform (menu) | Neon Saw, Chrome PWM, Glitch FM, Reese Bass, Laser Pluck, Neon Brass, Data Crush, Hollow Glass | Neon Saw | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (octave layer, PWM speed, feedback, detune...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (noise, sub level...). |
| FILTER (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| Filter Type (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| Slider 1, ADSR+VELOCITY (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| Slider 2, ADSR+VELOCITY (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| Slider 3, ADSR+VELOCITY (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| Slider 4, ADSR+VELOCITY (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| Slider 5, ADSR+VELOCITY (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Level of each voice, before the effects. |
| GLIDE (Glide) | 0 to 2 | 0 | Portamento. The value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| PITCH (Pitch) | -12 to +12 semitones | 0 | Global transposition. The control is continuous: use whole values to stay in tune. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The mod wheel adds to this value. |
| LFO Source (menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| Destination (menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| Mono/Poly (menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (slow, fixed rate). |
| DELAY (Delay) | 0 to 1 | 0 | Level of the ping-pong delay. |
| TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| PITCH BEND wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| MODULATION wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Neon Saw | Sub-octave level | Detune of the five saws | Built-in low-pass opening | Level of the octave-up saw | Noise |
| Chrome PWM | Pulse-width modulation depth | Detune of the second pulse | Built-in low-pass opening | Pulse-width modulation speed (0.3 to 5.3 Hz) | Sub-octave level |
| Glitch FM | Modulator ratio (0.5 to 4.0) | Decay time of the FM depth | FM depth | Modulator feedback | Attack noise burst |
| Reese Bass | Sub-octave level | Detune amount | Built-in low-pass opening | Saturation drive | Noise |
| Laser Pluck | Pitch dive at the attack (up to 2 octaves) | Length of the built-in filter sweep | Depth of the built-in filter sweep | Detune of the second saw | Attack noise |
| Neon Brass | Swell time of the built-in filter | Output weight | How far the filter opens | Detune of the three saws | Breath noise |
| Data Crush | Sample rate (higher is cleaner) | Bit depth (2 to 12 bits) | Built-in low-pass opening | Square-wave blend | Noise |
| Hollow Glass | Modulator ratio (1.4 to 4.5) | Decay length | FM depth (bell brightness) | Detune of the bell pair | Attack noise |

The Hollow Glass engine has its own natural decay, like a struck bell. It fades out even while you hold the key. Raise BODY for a longer ring.

## Factory presets

CYBERPUNK LEAD includes 70 factory presets in 7 banks of 10.

**Neon Leads:** Neon Rider, Chrome Hero, Night Drive, Hacker Lead, Laser Solo, Supersaw Wall, Soft Neon, Blade Lead, Octave Jump, Wire Lead.

**Chrome & Pulse:** Chrome Pulse, Wide PWM, Cyber Bass PWM, Fast Wobble, Retro Keys, Sunset Pad, Pulse Stab, Hollow Pulse, Arcade Lead, Pulse Drone.

**Glitch FM:** Glitch Bell, Data Pluck, FM Lead, Metal Keys, Digital Choir, Alien Voice, Cyber Marimba, Circuit Bass, Scan Lines, Neural Net.

**Reese & Bass:** Reese Core, Neon Sub, Dirty Reese, Wobble Grid, Industrial Bass, Cyber Slide, Acid Reese, Subway Bass, Bass Stab, Meltdown.

**Laser Plucks:** Laser Pluck, Neon Arp, Zap Down, Chrome Pluck, Techno Stab, Soft Pluck, Synth Harp, Cyber Koto, Sci-Fi Drop, Pluck Wall.

**Neon Brass & Pads:** Neon Brass, Blade Runner Pad, Synthwave Stab, Chrome Horns, Slow Open, Warm Neon, Hollow Glass, Ice Bell, Crystal Rain, Ghost Chime.

**Data Crush & FX:** Data Crush, 8-Bit Lead, Bit Rain, Glitch Bass, Corrupted Pad, Signal Lost, Modem Dial, Firewall, Static Wind, Boot Sequence.

**Mono presets.** These 32 presets load in Mono mode: all Neon Leads except Supersaw Wall; Cyber Bass PWM, Fast Wobble, Hollow Pulse and Arcade Lead; FM Lead, Alien Voice and Circuit Bass; all Reese & Bass presets; Data Crush, 8-Bit Lead, Glitch Bass, Signal Lost, Modem Dial and Firewall. All other presets load in Poly mode.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.cppreset` file. The default folder is `Documents\DJBILBOX BEATS\CYBERPUNK_LEAD\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .cppreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

Blade Runner is a trademark of its owner. It is used here only to describe a sound style. DJBILBOX BEATS is not affiliated with the trademark owner.

## Production tips

1. **Trap and Drill leads.** Start from Neon Rider or Night Drive (Mono, short glide). A dotted-eighth delay is 45 / BPM seconds (0.375 s at 120 BPM, as in Neon Rider). Lower DELAY when the vocal comes in.
2. **G-Funk portamento lead.** Load Soft Neon or Hollow Pulse in Mono mode. Lower BODY (less detune) and BRIGHT, set FILTER between 2 and 4 kHz and GLIDE between 0.1 and 0.2. Add a gentle vibrato: Destination Pitch, Sine LFO, RATE 5 to 6 Hz, AMOUNT around 0.06 (as in Chrome Hero). Play high, around C6 to C7 in FL Studio.
3. **Synthwave chords and pads.** Supersaw Wall, Sunset Pad, Warm Neon and Synthwave Stab play chords in Poly mode. On Neon Saw, BODY widens the supersaw detune and WIDTH adds an octave-up layer. Add CHORUS and REVERB for the classic 80s wash.
4. **Plucks and arps for Afrotrap and Trap.** Laser Pluck, Neon Arp and Chrome Pluck react well to short notes. TONE sets the "laser" pitch dive. Combine them with FL Studio's channel arpeggiator (Channel settings, Miscellaneous functions tab) and a tempo-matched delay.
5. **Dark Reese bass.** The Reese & Bass presets are in Mono mode and transposed one octave down. BODY sets the detune and WIDTH drives the saturation. Wobble Grid sends a Sine LFO to the filter. Each note gets a slight random stereo offset: for a strictly centered bass, reduce the stereo separation of the mixer track.
6. **FM bells and glitch keys.** On Glitch FM and Hollow Glass, TONE changes the frequency ratio. Some positions sound harmonic, others metallic and inharmonic: sweep TONE slowly and stop where the bell fits your key. BODY sets how long the bright attack lasts.
7. **8-bit and lo-fi FX.** On Data Crush, lower TONE for a lower sample rate and lower BODY for fewer bits (8-Bit Lead). Use FILTER to tame harsh aliasing. The crushing is lighter at 88.2 and 96 kHz project rates: lower TONE further for the same effect.
8. **Expressive vibrato.** Set Destination to Pitch, AMOUNT to 0 and RATE between 5 and 6 Hz. Then ride the MODULATION wheel at the end of long notes. Automate the Mod Wheel parameter to repeat the gesture exactly.

## MIDI, automation and performance

**Notes and channels.** CYBERPUNK LEAD responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the Velocity slider (fifth slider of the ADSR+VELOCITY section).

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen PITCH BEND wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen MODULATION wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend, filter, distortion and delay changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** CYBERPUNK LEAD is lightweight. Effects set to zero are bypassed and cost no CPU. The Neon Saw engine (seven oscillators per voice) costs the most in Poly mode; Mono mode and shorter releases keep the voice count low. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**CYBERPUNK LEAD does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **CYBERPUNK_LEAD** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or a low rumble, when I click the on-screen keyboard.**
The on-screen keyboard covers the lowest MIDI notes (0 to 43). Lead sounds are designed for the middle and upper range. Play from a MIDI keyboard or the Piano roll, around C5 to C7 in FL Studio.

**Which slider is which in the ADSR+VELOCITY section?**
From left to right: filter envelope Attack, Decay, Sustain, Release, then Velocity. The first four only change the sound when ENV is above zero.

**My bell preset fades out while I hold the key.**
The Hollow Glass engine (Hollow Glass, Ice Bell, Crystal Rain, Ghost Chime) has a built-in bell decay. Raise BODY for a longer ring, or switch to Neon Brass or Chrome PWM for a sustained sound.

**My chord only plays one note.**
The Mono/Poly menu is set to Mono, as on most lead and bass presets. Switch it to Poly.
