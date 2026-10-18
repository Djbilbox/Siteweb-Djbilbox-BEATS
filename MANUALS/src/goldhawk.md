# GOLDHAWK

Golden, cinematic synthesizer: hawk leads, brass, pads, plucks, bells, strings, basses and eagle cries from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

GOLDHAWK is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Hawk Lead (two pulse waves with modulated width, a saw and a sub octave), Golden Brass (three saws with a self-opening filter), Royal Pad (five detuned saws and a sub, with a slowly breathing filter), Falcon Pluck (a saw with a pitch dive and a filter that closes), Sky Bells (FM bells), Feather Strings (five saws with a delayed vibrato), Talon Bass (detuned saws, sub and saturation) and Eagle Cry (a falling glissando with formant-filtered breath).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb. The character is royal, wide and cinematic.

GOLDHAWK targets epic and cinematic beats, trap and hip-hop melodies, anthem leads and brass hits, atmospheric pads, bell and pluck motifs, string beds, bass lines and risers or FX. Up to 16 voices can play at once in Poly mode. The lead, bass and cry presets that are meant for single-note lines load in Mono mode.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **GOLDHAWK** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **GOLDHAWK - Init Patch**: the Hawk Lead engine in Poly mode with default settings.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Leads, brass, strings and pads sit well between MIDI notes 48 and 84 (C3 to C6 in FL Studio naming, where middle C is C5). The Talon Bass presets use TRANSPOSE at -12, so they sound one octave lower than the key you play.
5. Shape the sound with the main controls:
    - **TONE**, **BODY**, **BRIGHT**, **WIDTH**, **TEXTURE**: the character of the selected engine (small knobs at the left, under the cassette).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker, rounder sound.
    - **GLIDE**: slide time between notes. Leave it at zero for straight notes.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message saying that expansion packs are coming soon. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The camel artwork, the GOLDHAWK title and "By Humpire". The two dotted grilles on either side of the artwork are decorative.

**Character knobs (under the cassette).** Two rows of small knobs:

- Row 1: **TONE, BODY, BRIGHT, WIDTH**.
- Row 2: **TEXTURE, RESO** (filter resonance), **FLT ENV** (filter envelope depth) and **DLY MIX** (delay level).

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, placed under the LFO header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**AMP ENVELOPE (left).** Four sliders: **ATK, DEC, SUS, REL**.

**FILTER ENVELOPE (top right).** Three sliders: **ATK, DEC/REL, SUS**. The filter envelope uses its decay time for its release too, so there is no separate release slider. The **VEL** slider (velocity sensitivity of the level) sits just to the right of a thin divider line: it is not part of the filter envelope. The **VOLUME** knob is at the far right of this row.

**EFFECTS (right).** **REVERB**, **CHORUS** and **DISTORTION**.

**DELAY (right).** **TIME** and **FEEDBACK** knobs (delay time and delay repeats). The delay level is the **DLY MIX** knob in the character area.

**Center bottom.** The **GLIDE** knob and five menus. Four menus have a label to their left: **WAVEFORM** (engine), **LFO SOURCE** (LFO shape), **LFO DEST** (destination) and **MONO/POLY**. The **CUTOFF** knob and the **FILTER TYPE** menu sit to the right of them. Each menu shows its current value.

**PITCH & MOD wheels (bottom left).** **Pitch Bend** springs back to the center when you release it. **Mod Wheel** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. The keyboard covers MIDI notes 36 to 96 (five octaves plus one key, 36 white keys). The key labels on screen run from C2 to C7, with middle C labeled C4; in FL Studio naming, where middle C is C5, the same range is C3 to C8. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

GOLDHAWK exposes 34 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| WAVEFORM (Waveform menu) | Hawk Lead, Golden Brass, Royal Pad, Falcon Pluck, Sky Bells, Feather Strings, Talon Bass, Eagle Cry | Hawk Lead | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune, saturation, pitch-fall speed...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (noise, saw level, breath...). |
| CUTOFF (Filter Cutoff) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RESO (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| FLT ENV (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| ATK, filter (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DEC/REL, filter (Filter Decay/Release) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. The same value is used as the filter release time. |
| SUS, filter (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| VEL (Velocity Sensitivity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATK, amp (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DEC, amp (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUS, amp (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| REL, amp (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Output level of each voice, before the effects. |
| GLIDE (Glide) | 0 to 2 | 0 | Portamento. The value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The mod wheel adds to this value. |
| LFO SOURCE (menu) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DEST (Destination menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| MONO/POLY (menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (fixed rate). |
| DLY MIX (Delay Mix) | 0 to 1 | 0 | Level of the ping-pong delay. |
| TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Mod Wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Hawk Lead | Depth of the pulse-width modulation | Detune between the two pulse voices | Built-in low-pass opening | Speed of the pulse-width modulation | Level of the added saw |
| Golden Brass | Time the built-in filter takes to open | Output weight | How far the filter opens | Detune of the three saws | Breath noise |
| Royal Pad | No effect | Level of the sub octave | Built-in low-pass opening (modulated by a slow breathing movement) | Detune of the five saws | Noise |
| Falcon Pluck | Depth of the pitch dive at the attack | Length of the filter closing | Starting brightness of the filter | Detune of the second saw | Attack click (noise burst) |
| Sky Bells | Frequency ratio of the FM modulator | Length of the decay and of the FM brightness | FM depth | Detune of the second voice | Strike noise |
| Feather Strings | Position of the built-in formant filter | Level of the formant filter | Built-in low-pass opening | Vibrato depth | Noise |
| Talon Bass | Sub-octave level | Detune of the three saws | Built-in low-pass opening | Saturation drive | Noise |
| Eagle Cry | Depth of the falling pitch glide, and formant position | Level of the third harmonic | Level of the second harmonic | Speed of the pitch fall | Level of the formant-filtered breath |

## Factory presets

GOLDHAWK includes 70 factory presets in 7 banks of 10. 27 presets load in Mono mode and 43 load in Poly mode.

**Hawk Leads** (Hawk Lead engine): Goldhawk Lead, Soaring, Royal Pulse, Golden Wire, Sky Solo, Hawk Wide, Thermal, Eagle Eye, Golden Stab, Altitude. Eight load in Mono mode; Hawk Wide and Golden Stab load in Poly mode.

**Golden Brass** (Golden Brass engine): Golden Brass, Royal Fanfare, Hawk Stab, Imperial Horns, Soft Flugel, Slow Swell, Low Brass, Gold Rush Hit, Trumpet Call, Cinematic Brass. Only Trumpet Call loads in Mono mode.

**Royal Pads** (Royal Pad engine): Royal Pad, Golden Hour, Palace Air, Rising Hawk, Cloud Throne, Warm Gold, Dream Flight, Dusk Strings, Whisper Wind, Pulse Of Gold. All load in Poly mode.

**Falcon Plucks** (Falcon Pluck engine): Falcon Pluck, Gold Arp, Dive Zap, Muted Talon, Soft Feather, Pluck Choir, Golden Harp, Royal Koto, Rhythm Gate, Sad Hawk. All load in Poly mode.

**Sky Bells & Strings** (Sky Bells and Feather Strings engines): Sky Bell, Golden Chimes, Crystal Crown, Temple Bell, Music Box Gold, Feather Strings, Violin Flight, Slow Rise Strings, Dark Cellos, Pizz Gold. The first five use the Sky Bells engine and the last five use Feather Strings. Only Violin Flight loads in Mono mode.

**Talon Bass** (Talon Bass engine): Talon Bass, Gold Sub, Dirty Talon, Wobble Wing, Industrial Hawk, Slide Bass, Acid Talon, Deep Nest, Bass Stab, Meltdown Gold. All load in Mono mode with TRANSPOSE at -12.

**Eagle Cry & FX** (Eagle Cry engine): Eagle Cry, Hawk Screech, Distant Call, Falcon Dive, Wind Rush, Mountain Echo, Golden Whistle, Raptor Siren, Night Hunter, Thunder Wing. Seven load in Mono mode; Wind Rush, Night Hunter and Thunder Wing load in Poly mode.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.ghpreset` file. The default folder is `DJBILBOX BEATS/GOLDHAWK/Presets` inside your Documents folder. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .ghpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Anthem leads.** Start from Goldhawk Lead or Soaring. The Hawk Lead engine uses a pulse-width modulation: raise TONE for a deeper movement and WIDTH for a faster one. Use a little GLIDE (0.05 to 0.15) and play legato lines.
2. **Brass hits and fanfares.** Load Royal Fanfare, Hawk Stab or Gold Rush Hit in Poly mode and play chords. TONE sets how slowly the filter opens: keep it low for punchy hits, raise it for swells. Add REVERB and CHORUS for a wider sound.
3. **Cinematic pads and risers.** Royal Pad, Golden Hour and Cloud Throne have long attacks and releases. In Rising Hawk, the filter envelope (FLT ENV, with a long filter ATK) opens the filter slowly for a riser.
4. **Pluck and bell motifs.** In the Falcon Plucks and Sky Bells banks, shorten or lengthen the amplitude DEC slider to set the note length. On Sky Bells, TONE changes the FM ratio and BRIGHT the FM depth: small moves give very different bell colors.
5. **Strings.** Feather Strings adds a vibrato that starts about 0.2 seconds after the note. Raise WIDTH for a deeper vibrato and BODY for more formant color. Use a slow ATK for string beds.
6. **Bass lines.** The Talon Bass presets are in Mono mode and transposed down one octave. Raise TONE for more sub, WIDTH for more saturation. For a slide bass, see Slide Bass (GLIDE 0.25). For an acid sound, see Acid Talon (high FLT ENV with a short DEC/REL).
7. **Cries and FX.** Eagle Cry, Hawk Screech and Falcon Dive use the falling glissando of the Eagle Cry engine. Raise TONE for a larger pitch fall and TEXTURE for more breath. Wind Rush is a noise-based swell for transitions.
8. **Movement.** Set LFO DEST to Filter with a Sine LFO shape and a RATE between 1 and 4 Hz for a wobble. Set LFO DEST to Pitch with a small AMOUNT for vibrato, then bring the mod wheel up to add it only when you want it.

## MIDI, automation and performance

**Notes and channels.** GOLDHAWK responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VEL slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Mod Wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead. CC121 (reset all controllers) sets the CC1 value back to zero.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 34 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend and cutoff changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** Effects set to zero are bypassed and cost no CPU. Mono mode and short release times keep the voice count low. Pads with long releases and high polyphony use the most CPU.

## FAQ

**GOLDHAWK does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **GOLDHAWK** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Some presets use TRANSPOSE -12 and play very low. Short presets (stabs, Pizz Gold) are also quieter: raise VOLUME. Check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. For notes outside the on-screen keyboard range (MIDI 36 to 96), play from a MIDI keyboard or the Piano roll.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The MONO/POLY menu is set to Mono, which is the setting of the lead, bass and cry presets. Switch it to Poly (up to 16 voices).

**The filter envelope has no release slider.**
The filter envelope has three sliders: ATK, DEC/REL and SUS. The DEC/REL time is used for both the decay and the release of the filter envelope. The VEL slider next to them controls velocity sensitivity, not the filter.

**Does the delay or the LFO follow the tempo of my project?**
No. Delay time is set in seconds (0.05 to 1 s) and the LFO runs in Hz (0.05 to 20 Hz).

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.
