# VOODOO

World-instrument synthesizer: flutes, sitar, oud, kalimba, erhu and a voodoo choir from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |


## Overview

VOODOO is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Flute, Pan Flute and Shakuhachi (three breathy flutes made of harmonics, band-filtered breath noise and an attack chiff), Sitar (a plucked-string model with a jawari buzz and sympathetic strings), Oud (a plucked-string model), Kalimba (four inharmonic modes, like a struck metal tine), Erhu (two detuned saws through formants) and Voodoo Choir (five detuned saws through formants).

Five timbre controls (BREATH, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter, a volume envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb. VOODOO has no filter envelope, no resonance knob and no velocity-sensitivity control: BODY also sets the filter resonance, and velocity always affects the level of each note (from 35% at the softest to 100% at the hardest). The character is mystic and worldly.

VOODOO targets world-music melodies, Afrobeat and Latin hooks, Trap and Drill ethnic toplines and cinematic and ritual atmospheres. 62 presets load in Poly mode with up to 16 voices; 8 load in Mono mode (the Bass & Plucks bank) for single-note lines.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **VOODOO** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **VOODOO - Init Patch**: the Flute engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 48 to 96 (C4 to C8 in FL Studio naming, where middle C is C5).
5. Shape the sound with the main controls:
    - **BREATH, BODY, BRIGHT, WIDTH, TEXTURE**: the five TIMBRE sliders, which reshape the selected engine.
    - **CUTOFF**: the cutoff of the filter.
    - **REVERB**: space around the sound.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (left, below the preset display).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Center.** The artwork, the VOODOO title and "By Humpire".

**LFO group (left).** Under the **LFO** caption, three rows of two knobs: **RATE** and **AMOUNT** (LFO speed and depth), then **TRANSPOSE** (global transposition, -12 to +12 semitones; not an LFO control) and **GLIDE**, then **CUTOFF** and **VOLUME**.

**AMP ENVELOPE (center left).** Four sliders: **ATK, DEC, SUS, REL**.

**TIMBRE (center right).** Five sliders: **BREATH, BODY, BRIGHT, WIDTH, TEXTURE**.

**EFFECTS (right).** **REVERB** and **CHORUS** on the first row, **DISTORTION** and **DLY MIX** (delay level) on the second row, **DLY TIME** and **DLY FDBK** (delay repeats) on the third row.

**Menu row.** Five drop-down menus, each with a caption: **ENGINE** (Waveform), **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE**. Each menu shows its current value.

**Wheels.** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard.** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard covers MIDI notes 48 to 96 (four octaves plus one key). The key labels on screen run from C2 to C6, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C4 to C8. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

VOODOO exposes 28 automatable parameters. The host name is shown in parentheses when it differs from the panel label. VOODOO has no filter envelope, no filter resonance knob and no velocity-sensitivity control, so those parameters do not exist. The timbre sliders are the parameters named Breath / Vowel, Body / Resonance, Brightness, Width / Detune and Texture in the host.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Flute, Pan Flute, Shakuhachi, Sitar, Oud, Kalimba, Erhu, Voodoo Choir | Flute | Selects the synthesis engine. |
| BREATH slider (Breath / Vowel) | 0 to 1 | 50 % | Timbre control 1. Engine-dependent: breath, vowel or formant position (see the table below). |
| BODY slider (Body / Resonance) | 0 to 1 | 50 % | Timbre control 2. Engine-dependent. On every engine, BODY also sets the filter resonance (from a flat response at 0 to a gentle peak at 1). |
| BRIGHT slider (Brightness) | 0 to 1 | 50 % | Timbre control 3: brightness. Engine-dependent. |
| WIDTH slider (Width / Detune) | 0 to 1 | 30 % | Timbre control 4. Engine-dependent (detune, doubling, sympathetic strings...). It is not a stereo width control. |
| TEXTURE slider (Texture) | 0 to 1 | 20 % | Timbre control 5. Engine-dependent (chiff, buzz, noise...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode (state-variable filter, 12 dB per octave). Band Pass gets a small level boost. |
| ATK (Attack) | 0.001 to 5 s | 0.01 s | Volume envelope attack time. |
| DEC (Decay) | 0.01 to 5 s | 0.4 s | Volume envelope decay time. |
| SUS (Sustain) | 0 to 1 | 70 % | Volume envelope sustain level. |
| REL (Release) | 0.02 to 8 s | 0.4 s | Volume envelope release time. |
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
| PITCH BEND | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| MOD WHEEL | 0 to 1 | 0 % | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT; the larger of this wheel and MIDI CC1 applies. |

**Character controls by engine**

| Engine | BREATH | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Flute | Balance of breath noise and tone (more breath, less tone) | Focus of the breath band (resonance of the air noise) | Level of the upper harmonics | Level of a second, slightly detuned oscillator | Chiff (attack noise) |
| Pan Flute | Balance of breath noise and tone (more breath, less tone) | Focus of the breath band (resonance of the air noise) | Level of the upper harmonics | Level of a second, slightly detuned oscillator | Chiff (attack noise) |
| Shakuhachi | Balance of breath noise and tone (more breath, less tone) | Focus of the breath band (resonance of the air noise) | Level of the upper harmonics | Level of a second, slightly detuned oscillator | Chiff (attack noise) |
| Sitar | No effect | String sustain | String brightness | Level of the sympathetic strings | Jawari buzz (saturation) |
| Oud | No effect | String sustain | String brightness | No effect | No effect |
| Kalimba | No effect | Ring length | Level of the upper partials | No effect | Attack noise |
| Erhu | Frequency of the two formants | Formant level | Low-pass opening | Detune between the two saws | Noise |
| Voodoo Choir | Frequency of the two formants | Formant level | Low-pass opening | Detune spread of the five saws | Noise |


## Factory presets

VOODOO includes 70 factory presets in 7 banks of 10. 62 presets load in Poly mode and 8 in Mono mode. Mono presets: Bass & Plucks (8 of 10). 8 presets load with a glide.



**Flutes:** Mystic Reed, Moon Whistle, Shaman Breath, Jungle Dawn, Andean Mist, Quena Ghost, Bamboo Spirit, Silver Vibrato, Desert Ney, Temple Whisper.

**Raga & Strings:** Raga Dusk, Sitar Buzz, Oud Nights, Gypsy Oud, Tanpura Drone, Veena Glow, Erhu Tears, Sarangi Moan, Bazaar Pluck, Maqam Sunrise.

**Afrique:** Mbira Rain, Kalimba Fire, Savanna Thumb, Kora Dreams, Balafon Sun, Sanza Bells, Tribal Marimba, Griot Pluck, Zulu Chime, Nile Echo.

**Amerique Latine:** Charango Sun, Zampona Rain, Quena Mountain, Cumbia Pluck, Machu Dawn, Tango Bow, Rio Marimba, Cantina Oud, Andes Echo, Amazon Mist.

**Pads Mystiques:** Voodoo Choir, Swamp Spirits, Ghost Priestess, Candle Ritual, Bone Orchard, Mystic Dawn, Moon Chant, Veil of Smoke, Hollow Temple, Obsidian Hymn.

**Bass & Plucks:** Hex Bass, Root Doctor, Gris-Gris Sub, Cursed Pluck, Zombie Walk, Crossroads, Bayou Thump, Mojo Pluck, Skull Drone, Midnight Mojo.

**Textures:** Fog Over Bayou, Whispers in Smoke, Cauldron, Rain Dance, Ritual Fire, Spirit Wind, Dark Bazaar, Jinn Voices, Ancient Tape, Voodoo Doll.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.voodoopreset` file. The default folder is `DJBILBOX BEATS/VOODOO/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .voodoopreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Flutes.** Start from the Flutes bank. BREATH sets how airy the flute is, BRIGHT its harmonics and TEXTURE the chiff at the start of each note. Add a little vibrato with the LFO set to Pitch (RATE 5 Hz, AMOUNT 0.1) and the mod wheel.
2. **Raga and Arabic strings.** The Raga & Strings bank uses the Sitar, Oud and Erhu engines. On the Sitar engine, TEXTURE drives the jawari buzz and WIDTH the sympathetic strings (see **Sitar Buzz**).
3. **African and Latin hooks.** The Afrique and Amerique Latine banks use the Kalimba, Oud, flute and choir engines. Keep the notes short and add a dotted delay.
4. **Mystic pads.** The Pads Mystiques bank uses the Voodoo Choir engine. BREATH moves the formants; raise ATK and REL and add REVERB.
5. **Bass plucks.** The Bass & Plucks bank loads in Mono, mostly with glide: **Hex Bass** and **Root Doctor** make deep ethnic bass lines.
6. **Movement.** Set LFO DESTINATION to Timbre or Filter and raise the mod wheel (or AMOUNT) for slow evolving textures.
7. **Tempo-matched delay.** DLY TIME is in seconds. Use 60 / BPM for a quarter note.

## MIDI, automation and performance

**Notes and channels.** VOODOO responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY response (fixed: there is no velocity-sensitivity control).

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead. CC121 (reset all controllers) returns the MIDI mod wheel to zero.

**Other MIDI.** All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 28 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. Level, pitch, bend and cutoff changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples). It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** VOODOO uses no samples. Effects set to zero are bypassed and cost no CPU. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**Is there a filter envelope or a resonance knob?**
No. Shape the sound over time with the volume envelope and the LFO. BODY raises the filter resonance gently while it shapes the engine.

**Is there a velocity control?**
No. Velocity always changes the level of each note, from 35% at the softest to 100% at the hardest.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono, which is the default for the Bass & Plucks bank. Switch it to Poly (up to 16 voices).

**VOODOO does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **VOODOO** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.

**Is the delay tempo-synced?**
No. Delay time is in seconds (0.05 to 1 s), and the LFO runs in Hz (0.05 to 20 Hz).
