# PHANTOM RIDER

Ghost-western synthesizer: theremin, ghost choir, twang guitar, haunted organ, wraith pad, ghost bells, dead bass and whispering wind from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |


## Overview

PHANTOM RIDER is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Theremin (a sine with a vibrato that settles in and a light breath), Ghost Choir (five saws through vowel formants), Twang Guitar (a plucked-string model with a body resonance), Haunted Organ (drawbar-style with a slow flutter), Wraith Pad (five detuned saws with a breathing filter), Ghost Bells (an FM bell), Dead Bass (a sine sub with knock and saturation) and Whisper Wind (noise through two moving formants).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. The character is spectral and lonesome, a ghost-town western mood.

PHANTOM RIDER targets dark Trap and Drill toplines, cinematic and horror scoring, spaghetti-western moods and atmospheric intros. 51 presets load in Poly mode with up to 16 voices; 19 load in Mono mode, most of them with glide.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **PHANTOM RIDER** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **PHANTOM RIDER - Init Patch**: the Theremin engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 36 to 84 (C3 to C7 in FL Studio naming, where middle C is C5).
5. Shape the sound with the main controls:
    - **TONE, BODY, BRIGHT, WIDTH, TEXTURE**: the five small knobs, which reshape the selected engine.
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker sound.
    - **GLIDE**: slide time between notes, important for the theremin and whistle presets.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOX BEATS name and the DB logo.

**Center.** The phantom rider artwork, the PHANTOM RIDER title and "By Humpire".

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, in the same row, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**AMP ENVELOPE (left, below the LFO).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Character controls (center).** Two clusters of small knobs. Left of the center: **TONE, BODY** (upper row) and **BRIGHT, WIDTH** (lower row). Right of the center: **TEXTURE, RES** (filter resonance) and **ENV AMT** (filter envelope depth). The large **GLIDE** and **CUTOFF** knobs sit between the two clusters.

**FILTER ENVELOPE (top right).** Four vertical sliders: **ATK, DEC, REL** and **VELOCITY** (velocity sensitivity of the level). The **VOLUME** knob sits to their right.

**Effects (right).** **REVERB, CHORUS, DISTORTION** on the upper row. Under a **DELAY** caption: **DELAY MIX** (delay level), **TIME** and **FEEDBACK** (delay repeats).

**Menu row.** Five drop-down menus across the panel, each with a caption: **ENGINE** (Waveform), **LFO SHAPE**, **LFO DEST** (Destination), **FILTER TYPE** and **VOICE MODE**. Each menu shows its current value.

**Wheels.** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard.** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard covers MIDI notes 36 to 84 (four octaves plus one key). The key labels on screen run from C1 to C5, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C3 to C7. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

PHANTOM RIDER exposes 34 automatable parameters. The host name is shown in parentheses when it differs from the panel label. The filter envelope has no sustain control: it rises and decays to zero, and ENV AMT sets how far it opens the filter.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Theremin, Ghost Choir, Twang Guitar, Haunted Organ, Wraith Pad, Ghost Bells, Dead Bass, Whisper Wind | Theremin | Selects the synthesis engine. |
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
| Theremin | Vibrato speed (4.8 to 6.8 Hz) | Third harmonic level | Second harmonic level | Vibrato depth (fades in over the first moments of the note) | Breath noise |
| Ghost Choir | Vowel (a, o, u, i) | Formant level | Low-pass opening | Detune spread of the five saws | Noise |
| Twang Guitar | Pluck hardness | String sustain | String brightness | Frequency of the body resonance (1.1 to 2.5 kHz) | Level of the body resonance |
| Haunted Organ | Sub-octave drawbar level | Level of the drawbar one octave above | Upper drawbars level | Depth of the slow pitch flutter | Key click |
| Wraith Pad | No effect | Level of a sine one octave below | Breathing low-pass opening | Detune spread of the five saws | Breath noise |
| Ghost Bells | Modulator ratio (1.4 to 4.5) | Ring length | FM depth (bell brightness) | Detune between the two carriers | Attack noise |
| Dead Bass | Pitch "knock" at the attack | Adds the octave harmonic | Attack click | No effect | Saturation |
| Whisper Wind | Frequency of the first moving formant | Resonance of the first formant | Frequency of the second formant | Speed of the formant movement | Level of a faint sine at the note pitch |


## Factory presets

PHANTOM RIDER includes 70 factory presets in 7 banks of 10. 51 presets load in Poly mode and 19 in Mono mode. Mono presets: Theremin & Whistles (all 10); Ghost Choir (1 of 10); Twang & Strings (1 of 10); Dead Bass & Wind (7 of 10). 13 presets load with a glide.



**Theremin & Whistles:** Phantom Theremin, Ghost Whistle, Lonesome Wail, Spectral Sine, Cold Wind Lead, Siren From Beyond, Crystal Voice, Low Moan, Haunt Echo, Fading Signal.

**Ghost Choir:** Phantom Choir, Ooo Ghosts, Ah Specters, Ee Wraiths, Vowel Haunt, Whisper Gang, Low Spirits, Graveyard Hymn, Dust Chorale, Hollow Chant.

**Twang & Strings:** Ghost Town Twang, Spaghetti Lead, Dusty Pick, Reverb Surf, Low Rider Gtr, Muted Bounty, Harp Of Bones, Ride Away, Haunted Banjo, Midnight Pluck.

**Haunted Organs:** Haunted Organ, Chapel Of Dust, Funeral March, Carnival Of Souls, Whisper Pipes, Cellar Door, Full Ghost Stops, Dying Swell, Fog Organ, Last Hymn.

**Wraith Pads:** Wraith Pad, Fog Bank, Dead Moon, Cold Breath, Rising Dread, Moaning Strings, Dune Spirit, Tomb Drone, Pulse Of The Dead, Phantom Veil.

**Ghost Bells:** Ghost Bell, Church Of Dust, Chain Chimes, Spirit Glass, Toll For Me, Music Box Ghost, Wind Chimes, Hollow Toll, Cursed Kalimba, Echoing Hall.

**Dead Bass & Wind:** Dead Bass, Grave Sub, Knock Knock, Rotten Sub, Hearse Drone, Whisper Wind, Howling Gale, Dead Harmonica, Tumbleweed, Ghost Rider Wind.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.prpreset` file. The default folder is `DJBILBOX BEATS/PHANTOM_RIDER/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .prpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Lonesome leads.** Start from **Phantom Theremin** or **Ghost Whistle**. They load in Mono with glide: play legato lines. WIDTH sets the vibrato depth.
2. **Western twang.** On the Twang Guitar engine, TONE sets the pluck hardness and BRIGHT the string brightness (see **Spaghetti Lead**).
3. **Chords in the dark.** The Haunted Organs and Wraith Pads banks load in Poly mode and sound best with long RELEASE and REVERB.
4. **Bells for intros.** On the Ghost Bells engine, BODY sets the ring length and BRIGHT the FM depth. A long reverb and a dotted delay make an intro.
5. **Dead bass.** The Dead Bass engine is a sine sub with a pitch knock: **Dead Bass** and **Grave Sub** are Mono presets for 808-style lines.
6. **Wind and atmosphere.** The Whisper Wind engine is filtered noise: **Ghost Rider Wind** and **Howling Gale** add atmosphere under a beat.
7. **Movement.** Set LFO DESTINATION to Timbre or Filter and raise the mod wheel (or AMOUNT) for slow evolving textures.

## MIDI, automation and performance

**Notes and channels.** PHANTOM RIDER responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead. CC121 (reset all controllers) returns the MIDI mod wheel to zero.

**Other MIDI.** All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 34 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. Level, pitch, bend and cutoff changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples). It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** PHANTOM RIDER uses no samples. Effects set to zero are bypassed and cost no CPU. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**Can I play chords with the theremin presets?**
They load in Mono. Switch VOICE MODE to Poly for chords.

**Is there a filter sustain?**
No. The filter envelope has attack, decay and release. It decays to zero, and ENV AMT sets its depth.

**PHANTOM RIDER does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **PHANTOM RIDER** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.

**Is the delay tempo-synced?**
No. Delay time is in seconds (0.05 to 1 s), and the LFO runs in Hz (0.05 to 20 Hz).
