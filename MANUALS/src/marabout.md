# MARABOUT

Oriental and Afro-mystic synthesizer: kora, balafon, oud, ney, zurna, choir voices, dark pads and voodoo subs from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

MARABOUT is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: **Kora** (plucked harp string with a sympathetic octave), **Balafon** (inharmonic wooden bars with a gourd buzz), **Oud** (soft plucked string with a soundbox resonance), **Ney** (reed flute with filtered breath and vibrato), **Zurna** (double reed with a nasal resonance), **Choir** (five detuned saws through vowel filters), **Dark Pad** (five detuned saws plus a sub sine) and **Sub Voodoo** (sine sub with a pitch "knock" and saturation).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. The character is dark, warm and atmospheric.

MARABOUT fits Afro-trap and Afrobeat melodies, Gnawa and Middle Eastern-flavored loops, Hip-Hop and Drill textures, cinematic pads and ritual drones, and voodoo-style 808 subs. Most presets load in Poly mode with up to 16 voices; 11 presets (leads and subs) load in Mono mode.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **MARABOUT** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **MARABOUT - Init Patch**: the Kora engine in Poly mode with default settings.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Melodic presets sit well between MIDI notes 48 and 84 (C4 to C7 in FL Studio naming, where middle C is C5). Presets with TRANSPOSE at -12 or +12 sound one octave lower or higher than the key you play.
5. Shape the sound with the three main controls:
    - **TONE**: the first character control. Its role depends on the engine (vowel of the Choir, pick softness of the Oud, pitch knock of Sub Voodoo...).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker, rounder sound.
    - **GLIDE**: slide time between notes. Leave it at zero for straight notes.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. The window can be resized from 50% to 200% with the proportions kept.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The camel artwork, the MARABOUT title and "By Humpire". The two dotted grilles beside the artwork are decorative.

**Left, under the cassette.** Three knobs: **LFO RATE** and **LFO AMT** set the LFO speed and depth. **TRANSPOSE** sits next to them but is not an LFO control: it is the global transposition (-12 to +12 semitones).

**AMP ENVELOPE (left).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**Character knobs (center).** Small knobs **TONE, BODY, BRIGHT, WIDTH** (left of GLIDE) and **TEXTURE** (right of CUTOFF): the five character controls of the selected engine. Beside TEXTURE are **RES** (filter resonance) and **FLT ENV** (filter envelope depth). The large **GLIDE** and **CUTOFF** knobs sit between them.

**FILTER ENVELOPE (right).** Four sliders **ATK, DEC, SUS, REL**, with the **VELOCITY** slider (velocity sensitivity of the level) just to their right. The **VOLUME** knob is at the far right of the same row.

**FX (right).** Six knobs in two rows. Top row: **REVERB, CHORUS, DISTORTION**. Bottom row: **DLY MIX** (delay level), **DLY TIME** (delay time) and **FEEDBACK** (delay repeats).

**Menu row.** Five drop-down menus, each with a caption below it: **ENGINE**, **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE**. Each menu shows its current value.

**Wheels (bottom left).** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. The keyboard covers MIDI notes 48 to 96 (four octaves plus one key). The key labels on screen run from C2 to C6, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C4 to C8. The keyboard does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click a control to return it to its default value (standard JUCE behavior). A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

MARABOUT exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Control | Range | Default | What it does |
|---|---|---|---|
| ENGINE (Waveform menu) | Kora, Balafon, Oud, Ney, Zurna, Choir, Dark Pad, Sub Voodoo | Kora | Selects the synthesis engine. |
| TONE (Tone / Vowel) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width / Detune / Vibrato) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune or vibrato depth). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (resonance, buzz, breath noise, saturation...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% (a quarter octave per octave, centered on middle C). |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| FLT ENV (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff by up to 5 octaves. At 0 the filter envelope has no effect. |
| ATK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DEC (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| SUS (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| REL (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 (-3.1 dB) | Output level of each voice, before the effects. Displayed in dB. |
| GLIDE (Glide) | 0 to 2 | 0 | Portamento. Higher values give longer slides. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| LFO RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| LFO AMT (LFO Amount) | 0 to 1 | 0 | LFO depth. The mod wheel adds to this value. |
| LFO SHAPE (LFO Source menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION (Destination menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| VOICE MODE (Mono/Poly menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (slow, fixed rate). |
| DLY MIX (Delay Mix) | 0 to 1 | 0 | Level of the ping-pong delay. |
| DLY TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMT value. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Kora | No effect | String sustain | Brightness of the string (damping) | No effect | Level of the sympathetic octave |
| Balafon | No effect | Decay length of the bars | Level of the upper partials and the mallet click | No effect | Gourd buzz level |
| Oud | Pick softness and soundbox tuning | String sustain | Brightness of the string | No effect | Soundbox resonance level |
| Ney | Pitch of the breath filter | Focus (resonance) of the breath filter | Upper harmonics | Vibrato depth | Breath noise level |
| Zurna | Frequency of the nasal resonances | Level of the nasal resonances | Opening of the built-in low-pass | Detune of the two reeds | Noise level |
| Choir | Vowel, sweeping a, o, u, i | Level of the vowel formants | Opening of the built-in low-pass | Detune spread of the five saws | Breath noise level |
| Dark Pad | No effect | Level of the sub sine | Opening of the built-in low-pass | Detune spread of the five saws | Noise level |
| Sub Voodoo | Depth of the pitch "knock" at the attack | Level of the octave harmonic | Attack click | No effect | Saturation |

## Factory presets

MARABOUT includes 70 factory presets in 7 banks of 10. 59 presets load in Poly mode and 11 load in Mono mode (marked with an asterisk below).

**Kora & Harps:** Kora Dawn, Griot Kora, Night Harp, Bright Kora, Soft Pluck, Desert Harp, Sahel Arp, Moonlit Kora, Low Kora, Glass Strings.

**Balafon & Mallets:** Balafon Classic, Gourd Balafon, Wood Bells, Marimba Moon, Dry Mallet, Rain Balafon, Kalimba Dream, Buzzy Balafon, Ritual Mallets, Ice Xylo.

**Oud & Plucks:** Oud Taksim, Warm Oud, Bright Oud, Saz Night, Low Oud, Desert Guitar, Gnawa Guembri*, Tape Oud, Muted Pick, Oud Dub.

**Ney & Zurna:** Ney Whisper, Desert Ney, Breathy Reed, Sufi Flute, High Ney, Zurna Call, Wedding Zurna*, Nasal Reed, Mizmar, Snake Charmer*.

**Choir & Voices:** Marabout Choir, Ooo Ancestors, Ah Elders, Ee Spirits, Vowel Sweep, Whisper Choir, Low Voices, Muezzin Pad*, Gospel Dust, Hollow Chant.

**Dark Pads & Drones:** Candle Pad, Smoke Drone, Midnight Souk, Shell Glow, Ritual Swell, Voodoo Mist, Dune Wind, Amber Strings, Tomb Choir Pad, Pulse Drone.

**Sub Voodoo & FX:** Voodoo Sub*, 808 Marabout, Deep Hex*, Knock Bass*, Dirty Sub*, Glide Sub*, Ghost Boom, Bell Drop, Spirit Siren*, Hex Zap*.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.mbpreset` file. The default folder is `DJBILBOX BEATS/MARABOUT/Presets` inside your Documents folder. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .mbpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Afro-trap melodies.** Start from Griot Kora, Sahel Arp or Kalimba Dream. Add a little DLY MIX with DLY TIME set to an eighth note (30 / BPM seconds) for bouncy, repeating plucks.
2. **Gnawa and Maghreb grooves.** Gnawa Guembri (Mono, one octave down) and Muted Pick work well as repeating bass or rhythm lines. Raise TEXTURE on the Oud engine for more soundbox resonance.
3. **Wide melodies from the Ney.** On the Ney engine, WIDTH sets the vibrato depth and TEXTURE the breath noise. Raise the ATTACK slider for a slow, breathy entrance (see Ney Whisper).
4. **Lead lines.** Wedding Zurna and Snake Charmer load in Mono with a short glide. Raise GLIDE for longer slides between notes.
5. **Vowel movement.** On the Choir engine, TONE sweeps the vowel from a to o to u to i. Set LFO DESTINATION to Timbre to let the LFO move it automatically (see Vowel Sweep), or automate TONE.
6. **Cinematic pads.** Dark Pads & Drones are Poly pads made for long notes. Add REVERB and CHORUS, and automate CUTOFF over several bars for evolving textures.
7. **Voodoo subs.** On the Sub Voodoo engine, TONE sets the pitch knock, TEXTURE the saturation and BODY adds the octave harmonic. Raise TEXTURE or DISTORTION so the sub stays audible on small speakers.
8. **Slides.** With GLIDE above zero, every new note slides from the previous one. For slides on selected notes only, keep GLIDE at 0 and automate it up just before the notes that should slide.

## MIDI, automation and performance

**Notes and channels.** MARABOUT responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, and new notes start from the latest CC1 value. For repeatable results in a song, automate the **Mod Wheel** parameter instead.

**Other MIDI.** All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend, filter, distortion and delay changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** Distortion, chorus, delay and reverb are bypassed when set to zero and cost no CPU. Short release times keep the voice count low.

## FAQ

**MARABOUT does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **MARABOUT** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Check VOLUME, and check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. Some presets use TRANSPOSE -12 and play low. For notes outside the on-screen keyboard range (MIDI 48 to 96), play from a MIDI keyboard or the Piano roll.

**The sound dies away quickly even if I hold the key.**
Plucked and mallet engines (Kora, Balafon, Oud) decay naturally. Their presets use a SUSTAIN of zero, so the DECAY slider sets how long the note rings. Raise DECAY, and BODY on the engine, for longer notes.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono. Switch it to Poly (up to 16 voices).

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host.
