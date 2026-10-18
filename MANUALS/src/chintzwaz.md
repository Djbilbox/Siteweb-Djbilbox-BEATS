# CHINTZWAZ

Retro floral synthesizer: sitar, flower-power keys, tape strings, flutes, kalimba, pads and psychedelic organ, from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

CHINTZWAZ is a retro, bohemian synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Sitar Bloom (a plucked string with a buzzing bridge), Flower EP (an electric piano with a bell-like tine), Mellotron (five detuned saws with tape flutter and breath), Hippie Flute (a sine with harmonics, vibrato and breath noise), Kalimba (short metallic tines), Daisy Pad (detuned saws with a sub octave and a slowly breathing filter), Bloom Bass (a filtered triangle/saw with a round sub) and Psyche Organ (a drawbar organ with tremolo).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. The look is a pink brushed-metal faceplate on floral wallpaper.

CHINTZWAZ fits Lo-fi Hip-Hop and Boom Bap loops, psychedelic Soul and Funk, Oriental-flavored melodies (sitar, flute), Afrobeats and Amapiano (kalimba, pads), dreamy pads and retro keys. Most presets load in Poly mode (up to 16 voices). Twelve presets load in Mono mode: one flute-and-tape lead and the bass and organ voices.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **CHINTZWAZ** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **CHINTZWAZ - Init Patch**: the Sitar Bloom engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 48 to 96 (C4 to C8 in FL Studio naming, where middle C is C5). Keys, pads and plucks work well in the middle of that range; the Bloom Bass presets sit lower, so play them from a MIDI keyboard or the Piano roll.
5. Shape the sound with the three main controls:
    - **TONE**: the main character of the engine (pluck brightness, tape color, breath pitch, organ sub drawbar...).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker, vintage tone.
    - **WIDTH**: jawari buzz, tape detune and wobble, flute vibrato, pad detune or organ tremolo, depending on the engine.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DB logo and the DJBILBOXBEATS name.

**Center.** The camel artwork, the CHINTZWAZ title and "By Humpire". The two perforated speaker grilles on each side of the artwork carry the small character knobs (see below).

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, also under this header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**Left speaker grille.** Four small knobs stacked vertically: **TONE, BODY, BRIGHT, WIDTH**.

**Right speaker grille.** Three small knobs stacked vertically: **TEXTURE** (the fifth character control), **RES** (filter resonance) and **ENV AMT** (filter envelope depth).

**AMP ENVELOPE (lower left).** Four sliders: **ATTACK, DECAY, SUSTAIN, RELEASE**.

**GLIDE and CUTOFF.** The **GLIDE** knob sits at the left of the title and the **CUTOFF** knob at the right.

**FILTER ENVELOPE (upper right).** Four sliders: **ATTACK, DECAY, SUSTAIN** (filter envelope) and **VELOCITY** (how much note velocity changes the level).

**MASTER and EFFECTS (right).** **VOLUME** is the large knob in the top right corner. Under the **EFFECTS** header, first row: **REVERB, CHORUS, DISTORTION**. Second row: **DELAY** (delay level), **DLY TIME** (delay time) and **FEEDBACK** (delay repeats).

**Menu row.** Five drop-down menus, each with a caption below it: **ENGINE**, **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE**. Each menu shows its current value.

**Wheels (bottom left).** **PITCH BEND** springs back to the center when you release it. **MOD WHEEL** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. The keyboard covers MIDI notes 48 to 96 (four octaves plus one key). The key labels on screen run from C2 to C6, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C4 to C8. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

CHINTZWAZ exposes 34 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Control | Range | Default | What it does |
|---|---|---|---|
| ENGINE (Waveform) | Sitar Bloom, Flower EP, Mellotron, Hippie Flute, Kalimba, Daisy Pad, Bloom Bass, Psyche Organ | Sitar Bloom | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune, buzz, vibrato, tremolo...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (noise, click, saturation...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (Filter Type) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| ENV AMT (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| FILTER ENVELOPE ATTACK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| FILTER ENVELOPE DECAY (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. It also sets the filter release time after the key is released. |
| FILTER ENVELOPE SUSTAIN (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| AMP ENVELOPE ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| AMP ENVELOPE DECAY (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| AMP ENVELOPE SUSTAIN (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| AMP ENVELOPE RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Output level of each voice, before the effects. Shown in dB. |
| GLIDE (Glide) | 0 to 2 s | 0 | Portamento: the value is roughly the slide time in seconds. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
| TRANSPOSE (Transpose) | -12 to +12 semitones | 0 | Global transposition in whole semitones. |
| RATE (LFO Rate) | 0.05 to 20 Hz | 4 Hz | LFO speed. Not tempo-synced. |
| AMOUNT (LFO Amount) | 0 to 1 | 0 | LFO depth. The mod wheel adds to this value. |
| LFO SHAPE (LFO Source) | Sine, Triangle, Saw, Square, Sample&Hold | Sine | LFO shape. The LFO restarts at every new note. Sample&Hold picks a new random value once per cycle. |
| LFO DESTINATION (Destination) | Pitch, Filter, Amp, Timbre | Pitch | LFO target. Pitch: up to +/-1 semitone (vibrato). Filter: up to +/-3 octaves (wobble). Amp: up to 50% level dip (tremolo). Timbre: moves all five character controls by up to +/-0.5. |
| VOICE MODE (Mono/Poly) | Poly, Mono | Poly | Poly: up to 16 notes at once. Mono: one note at a time, last-note priority. |
| DISTORTION (Distortion) | 0 to 1 | 0 | Saturation on the summed output, with automatic level compensation. |
| CHORUS (Chorus) | 0 to 1 | 0 | Stereo chorus mix (slow, fixed rate). |
| DELAY (Delay Mix) | 0 to 1 | 0 | Level of the ping-pong delay. |
| DLY TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Delay Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| PITCH BEND wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| MOD WHEEL (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Sitar Bloom | Brightness of the pluck | String sustain | String brightness (less damping) | Jawari buzz amount | Level of a resonant formant (about 900 to 2,500 Hz) |
| Flower EP | No effect | Length of the tine decay | Bark (upper partials), stronger with velocity | No effect | Attack noise |
| Mellotron | Center of a mid band (400 to 1,100 Hz) | Level of that mid band | Low-pass opening | Detune of the five saws and tape wobble depth | Wobble speed and breath noise |
| Hippie Flute | Pitch of the breath noise | Resonance of the breath noise | Level of the 2nd and 3rd harmonics | Vibrato depth (fades in after the attack) | Breath noise level |
| Kalimba | Level of the pluck click | Ring length | Level of the upper partials | No effect | Resonance of the kalimba box |
| Daisy Pad | No effect | Sub-octave level | Filter opening, with a slow built-in breathing | Detune of the five saws | Noise |
| Bloom Bass | Blend from triangle to saw | Sub level | Opening of a built-in low-pass that closes during the note | How slowly that low-pass closes | Saturation |
| Psyche Organ | Sub-octave drawbar level | Level of the octave-up drawbar | Level of the upper drawbars | Tremolo speed and depth | Key click |

## Factory presets

CHINTZWAZ includes 70 factory presets in 7 banks of 10. 58 load in Poly mode and 12 in Mono mode (Violin Tape; Breathy Dawn, Sad Daisy, Flute Echo, Wind In Petals and Whistling Gardener; and the Bloom Bass & Organs presets Bloom Bass, Round Sub, Fuzz Petal, Slide Stem, Pluck Root and Vox Garden).

**Sitar & Strings** (Sitar Bloom engine): Sitar Bloom, Raga Morning, Patchouli Pluck, Jawari Buzz, Marigold Drone, Soft Tamboura, Hippie Strum, Bazaar Echo, Low Sitar, Glass Sitar.

**Flower EP** (Flower EP engine): Flower Power EP, Velvet Daisy, Bark Petal, Summer Haze Keys, Tremolo Bloom, Tape Wurly, Bell Blossom, Low Tide EP, Lazy Sunday, Wah Garden.

**Mellotron & Choir** (Mellotron engine): Chintz Mellotron, Strawberry Tape, Soft Choir Tape, Flute Mellotron, Violin Tape, Slow Wobble, Dream Tape Pad, Low Choir Tape, Eight Track, Kaleidoscope.

**Hippie Flutes** (Hippie Flute engine): Hippie Flute, Meadow Pipe, Breathy Dawn, Sad Daisy, High Piccolo, Flute Echo, Wind In Petals, Low Flute, Fog Flute, Whistling Gardener.

**Kalimba & Bells** (Kalimba engine): Daisy Kalimba, Music Box Garden, Thumb Piano, Wind Chimes, Glass Petals, Low Kalimba, Echo Garden, Rain On Leaves, Teacup Bell, Marimba Blush.

**Daisy Pads** (Daisy Pad engine): Daisy Pad, Petal Mist, Sundress Strings, Slow Bloom, Lavender Haze, Warm Chintz, Breeze Drift, Whisper Field, Pulse Of Spring, Twilight Garden.

**Bloom Bass & Organs** (Bloom Bass engine for the first five, Psyche Organ engine for the last five): Bloom Bass, Round Sub, Fuzz Petal, Slide Stem, Pluck Root, Psyche Organ, Vox Garden, Soft Flute Stop, Rotary Daisy, Golden Haze Organ.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.czpreset` file. The default folder is `DJBILBOX BEATS/CHINTZWAZ/Presets` inside your Documents folder. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .czpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Lo-fi and Boom Bap loops.** Start from Tape Wurly or Eight Track. Lower CUTOFF and add a little CHORUS for a dusty, vintage color. Raise DISTORTION slightly to glue the sound.
2. **Sitar and Oriental-flavored melodies.** Load Sitar Bloom or Raga Morning. WIDTH sets the jawari buzz and TEXTURE adds a resonant formant. Add DELAY (Bazaar Echo is a starting point) and set DLY TIME to a quarter or eighth note: 60 / BPM seconds, halved for an eighth.
3. **Psychedelic Soul and Funk keys.** Flower Power EP and Bark Petal respond to velocity: BRIGHT adds bark on hard notes. Use the VELOCITY slider around 60 to 80% so the accents of your Piano roll come through. Wah Garden uses the filter envelope; raise ENV AMT for a stronger wah.
4. **Afrobeats and Amapiano.** Daisy Kalimba, Thumb Piano and Music Box Garden give short, bouncy hooks; Daisy Pad and Sundress Strings hold the harmony. Add REVERB for space and a touch of DELAY for movement.
5. **Tape strings and choirs.** On the Mellotron engine, WIDTH detunes the five saws and adds tape wobble, and TEXTURE sets the wobble speed. Raise the AMP ENVELOPE ATTACK for swells (Dream Tape Pad).
6. **Flute leads.** Play Hippie Flute in Mono mode for clean lines, and add GLIDE (about 0.1 s) for smooth slides. WIDTH sets the vibrato depth; TEXTURE sets the breath.
7. **Slow-moving pads.** Slow Bloom and Daisy Pad use long attacks and releases. Automate CUTOFF over 8 or 16 bars, or set LFO DESTINATION to Filter with a slow Sine for gentle movement (Kaleidoscope uses the Timbre destination).
8. **Bass and organ.** The Bloom Bass presets load in Mono mode. Round Sub keeps TONE low for a soft low end; Fuzz Petal adds saturation through TEXTURE. Psyche Organ and Rotary Daisy play chords in Poly mode, or in Mono mode for lead lines like Vox Garden.

## MIDI, automation and performance

**Notes and channels.** CHINTZWAZ responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen PITCH BEND wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen MOD WHEEL (up to +0.8 on top of AMOUNT, capped at 1). When both are used, the higher value wins. A note that starts later begins from the latest CC1 position. For repeatable results in a song, automate the **Mod Wheel** parameter.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 34 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, transpose, bend, cutoff, distortion and delay changes are smoothed to avoid clicks. Parameters show units in the host (Hz, ms, s, dB, semitones, %).

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** Effects set to zero are bypassed. Fewer simultaneous notes and shorter release times keep the voice count low.

## FAQ

**CHINTZWAZ does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **CHINTZWAZ** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Check VOLUME, and check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. Some presets have a long attack (pads, Slow Bloom), so the sound fades in slowly. For notes outside the on-screen keyboard range (MIDI 48 to 96), play from a MIDI keyboard or the Piano roll.

**The sound is slightly different on every note.**
Each note gets a random starting phase and a slight random stereo offset. This is intended and adds a natural feel.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono. Switch it to Poly (up to 16 voices).

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.
