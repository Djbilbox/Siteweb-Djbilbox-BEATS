# CHINTZWAZ

Retro floral synthesizer: sitar, flower-power keys, tape strings, flutes, kalimba, pads and psychedelic organ.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

CHINTZWAZ is a retro, bohemian synthesizer by Humpire / DJBILBOX BEATS. It uses no samples. Eight engines recreate the colors of the psychedelic era: a plucked Sitar Bloom with jawari buzz, a Flower EP electric piano with velocity "bark", a Mellotron tape-string voice with flutter and hiss, a breathy Hippie Flute with vibrato, a Kalimba, a slowly breathing Daisy Pad, a round Bloom Bass and a Psyche Organ with drawbars and tremolo.

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb. The character is warm, colorful and slightly worn, like an old record or a tape loop.

CHINTZWAZ fits Lo-fi Hip-Hop and Boom Bap sample-style loops, psychedelic Soul and Funk, Oriental and Rai melodies (sitar, flute), Afrobeats and Amapiano (kalimba, marimba), Deep House keys and organs, and melodic Trap and R&B. It plays up to 16 notes at once. Flute leads and bass presets load in Mono mode.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **CHINTZWAZ** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **CHINTZWAZ - Init Patch**: the Sitar Bloom engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Keys, pads and plucks sit well between C4 and C7 in FL Studio. Bass presets sit between C2 and C4.
5. Shape the sound with the three main controls:
    - **TONE**: the main character of the engine (pluck hardness, tape color, breath pitch, organ sub drawbar...).
    - **FILTER**: the cutoff of the filter. Turn it down for a darker, vintage tone.
    - **WIDTH**: jawari buzz, tape flutter, flute vibrato or organ tremolo, depending on the engine.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels on a pink brushed-metal faceplate with floral wallpaper. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DB logo and the DJBILBOXBEATS name.

**Left speaker grille.** Four small knobs stacked vertically: **TONE, BODY, BRIGHT, WIDTH**.

**Right speaker grille.** Three small knobs: **TEXTURE** (the fifth character control), **RES** (filter resonance) and **ENV** (filter envelope depth).

**Center.** The camel artwork, the CHINTZWAZ title and "By Humpire". The **GLIDE** knob sits at the left of the title and the **FILTER** knob at the right.

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH**, also under this header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**Amplitude envelope (lower left).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**ADSR+VELOCITY section (upper right).** Four sliders: **ATTACK, DECAY, SUSTAIN** of the filter envelope, and **VELOCITY** (velocity sensitivity of the level). The filter envelope has no separate release slider: its release time follows its DECAY slider. The **VOLUME** knob sits at the right of these sliders.

**Effects (right).** First row: **REVERB, CHORUS, DISTORTION**. Second row: **DELAY** (delay level), **TIME** and **FEEDBACK**.

**Menu row.** Five drop-down menus, from left to right: **Waveform** (engine), **LFO Source** (LFO shape), **Destination** (LFO target), **Filter Type** and **Mono/Poly**. Each menu shows its current value.

**Wheels (bottom left).** **PITCH** is the pitch bend wheel. It springs back to the center when you release it. **MODULATION** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard starts at the lowest MIDI note (labeled C-2, which is C0 in FL Studio) and does not scroll. It covers MIDI notes 0 to 31, which are very low for most CHINTZWAZ sounds. Use a MIDI controller or the Piano roll for normal playing.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

CHINTZWAZ exposes 34 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Waveform (menu) | Sitar Bloom, Flower EP, Mellotron, Hippie Flute, Kalimba, Daisy Pad, Bloom Bass, Psyche Organ | Sitar Bloom | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (buzz, flutter, vibrato, tremolo...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (noise, breath, hiss, click...). |
| FILTER (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| Filter Type (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| ATTACK, filter section (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DECAY, filter section (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. Also sets the filter envelope release time. |
| SUSTAIN, filter section (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| VELOCITY (Velocity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
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
| Destination (menu) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves. Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| Mono/Poly (menu) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (slow, fixed rate). |
| DELAY (Delay) | 0 to 1 | 0 | Level of the ping-pong delay. |
| TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| PITCH wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| MODULATION wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Sitar Bloom | Pluck hardness | String sustain | Brighter, longer highs | Jawari buzz | Resonant buzz band (900 Hz to 2.5 kHz) |
| Flower EP | No effect | Decay length | "Bark" (bell and upper partials), stronger at high velocity | No effect | Attack noise |
| Mellotron | Tape color (formant from 400 Hz to 1.1 kHz) | Formant amount | Built-in low-pass opening | Detune and tape-flutter depth | Flutter speed and tape hiss |
| Hippie Flute | Pitch of the breath noise | Focus of the breath noise | 2nd and 3rd harmonics | Vibrato depth (5.2 Hz, fades in after the attack) | Breath level |
| Kalimba | Attack click | Decay length | Metallic upper partials | No effect | Box resonance |
| Daisy Pad | No effect | Sub-octave level | Filter opening (the filter slowly "breathes") | Detune spread of the five saws | Air noise |
| Bloom Bass | Triangle to saw blend | Sub-octave level | Depth of the built-in filter sweep | Length of the built-in filter sweep | Saturation |
| Psyche Organ | Sub-octave drawbar | Octave drawbar | Upper drawbars | Tremolo depth and speed (3 to 7 Hz) | Key click |

## Factory presets

CHINTZWAZ includes 70 factory presets in 7 banks of 10. Most presets load in Poly mode. These presets load in Mono mode: Violin Tape, Breathy Dawn, Sad Daisy, Flute Echo, Wind In Petals, Whistling Gardener, Vox Garden, and the five bass presets (Bloom Bass, Round Sub, Fuzz Petal, Slide Stem, Pluck Root).

**Sitar & Strings:** Sitar Bloom, Raga Morning, Patchouli Pluck, Jawari Buzz, Marigold Drone, Soft Tamboura, Hippie Strum, Bazaar Echo, Low Sitar, Glass Sitar.

**Flower EP:** Flower Power EP, Velvet Daisy, Bark Petal, Summer Haze Keys, Tremolo Bloom, Tape Wurly, Bell Blossom, Low Tide EP, Lazy Sunday, Wah Garden.

**Mellotron & Choir:** Chintz Mellotron, Strawberry Tape, Soft Choir Tape, Flute Mellotron, Violin Tape, Slow Wobble, Dream Tape Pad, Low Choir Tape, Eight Track, Kaleidoscope.

**Hippie Flutes:** Hippie Flute, Meadow Pipe, Breathy Dawn, Sad Daisy, High Piccolo, Flute Echo, Wind In Petals, Low Flute, Fog Flute, Whistling Gardener.

**Kalimba & Bells:** Daisy Kalimba, Music Box Garden, Thumb Piano, Wind Chimes, Glass Petals, Low Kalimba, Echo Garden, Rain On Leaves, Teacup Bell, Marimba Blush.

**Daisy Pads:** Daisy Pad, Petal Mist, Sundress Strings, Slow Bloom, Lavender Haze, Warm Chintz, Breeze Drift, Whisper Field, Pulse Of Spring, Twilight Garden.

**Bloom Bass & Organs:** Bloom Bass, Round Sub, Fuzz Petal, Slide Stem, Pluck Root, Psyche Organ, Vox Garden, Soft Flute Stop, Rotary Daisy, Golden Haze Organ.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.czpreset` file. The default folder is `Documents\DJBILBOX BEATS\CHINTZWAZ\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .czpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

Mellotron, Vox and Wurlitzer are trademarks of their respective owners. They are used here only to describe sound styles. DJBILBOX BEATS is not affiliated with these trademark owners.

## Production tips

1. **Oriental and Rai melodies.** Start from Sitar Bloom or Jawari Buzz. WIDTH sets the jawari buzz and TEXTURE moves the buzzing band. For ney-like flute lines, use Hippie Flute in Mono mode with GLIDE between 0.05 and 0.15, and bend notes with the PITCH wheel (up to 2 semitones).
2. **Lo-fi Hip-Hop and Boom Bap keys.** Load Tape Wurly, Lazy Sunday or Velvet Daisy. Lower FILTER to around 3 kHz, add CHORUS, and a touch of DISTORTION for tape grit. On Flower EP, play harder for more "bark": BRIGHT sets how much the velocity brings out.
3. **Psychedelic strings and choirs.** The Mellotron engine likes a slow ATTACK (0.1 to 0.4 s). WIDTH sets the depth of the tape wobble and TEXTURE its speed and hiss. Kaleidoscope sends a slow LFO to Timbre for an evolving texture.
4. **Afrobeats and Amapiano.** Daisy Kalimba, Marimba Blush and Echo Garden work well for log-drum-friendly melodies. Set a short ping-pong delay that follows the tempo: an eighth note is 30 / BPM seconds (0.27 s at 112 BPM).
5. **Trap and R&B pads.** Daisy Pad, Lavender Haze and Breeze Drift give soft beds for vocals. Set Filter Type to High Pass with FILTER around 200 to 300 Hz to keep the pad away from your 808. A slow Sine LFO on Filter (RATE 0.1 to 0.3 Hz) adds movement.
6. **Deep House organ.** Psyche Organ is ready for chord stabs. Shorten RELEASE for tight stabs. WIDTH sets the tremolo. Rotary Daisy adds a fast LFO on Timbre plus chorus for a swirling organ.
7. **Round bass lines.** The Bloom Bass presets play in Mono mode. Use GLIDE for slides (Slide Stem) and TEXTURE for drive (Fuzz Petal). Each note gets a slight random stereo offset: for a strictly centered bass, reduce the stereo separation of the mixer track.
8. **Expressive vibrato.** Set Destination to Pitch, AMOUNT to 0 and RATE between 5 and 6 Hz. Then ride the MODULATION wheel for vibrato on flute and violin presets. Automate the Mod Wheel parameter to repeat the gesture exactly.

## MIDI, automation and performance

**Notes and channels.** CHINTZWAZ responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider. On the Flower EP engine, velocity also brightens the tone.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. Long releases and delays on kalimba and pad presets use more voices. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen PITCH wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen MODULATION wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 34 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend, filter, distortion and delay changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** CHINTZWAZ is lightweight. Effects set to zero are bypassed and cost no CPU. Engines with several oscillators per voice (Mellotron, Daisy Pad, Psyche Organ) cost a little more. Shorter releases reduce the number of sounding voices. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**CHINTZWAZ does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **CHINTZWAZ** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or a low rumble, when I click the on-screen keyboard.**
The on-screen keyboard covers the lowest MIDI notes (0 to 31). Most CHINTZWAZ sounds are designed for the middle and upper range. Play from a MIDI keyboard or the Piano roll, around C4 to C7 in FL Studio.

**Where is the release of the filter envelope?**
There is no separate slider. The filter envelope release uses the same time as its DECAY slider in the ADSR+VELOCITY section. The filter envelope only acts when ENV is above zero.

**The sound keeps moving even with AMOUNT at zero.**
Some engines have built-in movement: the Daisy Pad filter breathes slowly, and WIDTH sets the tape flutter (Mellotron), the vibrato (Hippie Flute) and the tremolo (Psyche Organ). Also check that the MODULATION wheel is at zero, because it adds LFO depth.

**My chord only plays one note.**
The Mono/Poly menu is set to Mono, as on the flute leads and bass presets. Switch it to Poly.
