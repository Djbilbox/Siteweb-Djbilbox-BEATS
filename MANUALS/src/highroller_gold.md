# HIGHROLLER GOLD

Casino-luxury synthesizer: golden bells, velvet keys, big band brass, Vegas organ, silk strings and gold 808s from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (VST3 instrument) |
| Format | VST3 64-bit (Windows) |
| Developer | DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

HIGHROLLER GOLD is a synthesizer by Humpire / DJBILBOX BEATS with a high-stakes, gold-and-velvet character. It uses no samples. Every sound is generated in real time by one of eight engines: Gold Bell (FM bell), Velvet Keys (electric piano with tine and bell partials), Big Band Brass (three detuned saws with a swelling filter), Vegas Organ (drawbar organ with percussion and key click), Roulette Mallets (vibraphone-style bars with tremolo), Silk Strings (five-saw ensemble with slow drift), Gold Sub (sine sub with pitch "knock" and saturation) and Cigar Smoke (detuned saw pad with a breathing filter).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, stereo delay and reverb. The result is smooth, expensive-sounding and slightly smoky: jackpot bells, late-night lounge keys, brass fanfares and deep subs.

HIGHROLLER GOLD targets luxury and mafioso Hip-Hop, Trap bells and 808s, G-Funk and West Coast keys, Boom Bap and lo-fi chords, R&B and Deep House pads and organs. It plays up to 16 notes at once. The Gold Sub & Bass bank loads in Mono mode for clean bass lines.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **HIGHROLLER_GOLD** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **HIGHROLLER GOLD - Init Patch**: the Gold Bell engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Keys, bells and pads sit well between C4 and C6 in FL Studio. Bass presets sit between C2 and C4.
5. Shape the sound with the three main controls:
    - **BRIGHT**: the brightness of the engine (FM depth, key "bark", brass opening, upper drawbars...).
    - **BODY**: the length or weight of the sound (ring time for bells, keys and mallets, sub level for pads and basses).
    - **FILTER**: the cutoff of the filter. Turn it down for a darker, warmer, "tape" tone.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the EXPANSION plate under the cassette opens an information message about upcoming expansion packs, with the address of the DJBILBOX BEATS store (djbilboxbeats.gumroad.com). No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Character row (under the preset display).** Five small knobs: **TONE, BODY, BRIGHT, WIDTH, TEXTURE**. They are the character controls of the selected engine.

**Center.** The camel mascot in a fedora, framed by two gold-dotted speaker grilles. The grilles are decorative. Below them: the HIGHROLLER GOLD title and "By Humpire".

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **PITCH**, also under this header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**Amplitude envelope (left).** **ATTACK, DECAY, SUSTAIN, RELEASE** sliders.

**Filter controls (left of the title).** **RES** (resonance) and **ENV** (filter envelope depth), two small knobs. **GLIDE** sits below them.

**Right of the title.** **VOLUME** and, below it, **FILTER** (cutoff).

**Filter envelope (top right).** **A, D, S, R** sliders for the filter envelope, then **VELOCITY** (velocity sensitivity of the level).

**Effects (right).** First row: **REVERB, CHORUS, DISTORTION**. Second row: **DELAY** (delay level), **TIME** and **FEEDBACK**.

**Menu row.** Five drop-down menus, from left to right: **Waveform** (engine), **LFO Source** (LFO shape), **Destination** (LFO target), **Filter Type** and **Mono/Poly**. The menus have no title. Each one shows its current value.

**Wheels (bottom left).** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard starts at the lowest MIDI note (labeled C-2, which is C0 in FL Studio) and does not scroll. It covers MIDI notes 0 to 31, which are very low. Use a MIDI controller or the Piano roll for normal playing.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag or hover.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

HIGHROLLER GOLD exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| Waveform (menu) | Gold Bell, Velvet Keys, Big Band Brass, Vegas Organ, Roulette Mallets, Silk Strings, Gold Sub, Cigar Smoke | Gold Bell | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune, percussion, tremolo depth...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (attack noise, key click, saturation...). |
| FILTER (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| Filter Type (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode (12 dB per octave). |
| ENV (Filter Env) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| A (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| D (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| S (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| R (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
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
| DELAY (Delay) | 0 to 1 | 0 | Level of the stereo delay. The repeats cross between the left and right channels. |
| TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Gold Bell | FM ratio (1.4 to 4.5): from warm bell to metallic chime | Ring time of the bell | FM depth (strike brightness) | Slight detune of a second tone (beating) | Strike noise |
| Velvet Keys | No effect | Decay length of the tine and bell partials | "Bark": level of the upper partials, stronger on hard hits | No effect | Hammer noise at the attack |
| Big Band Brass | Swell time of the built-in filter (fast to slow) | Output weight | How far the built-in filter opens | Detune of the three saws | Breath noise |
| Vegas Organ | Sub-octave drawbar (16') | Octave drawbar (4') | Upper drawbars (2 2/3', 2', 1 1/3') | Percussion (short decaying octave) | Key click |
| Roulette Mallets | Mallet attack noise | Decay length | Level of the upper partials | Tremolo depth (up to 60%) | Tremolo speed (2 to 9 Hz) |
| Silk Strings | Body resonance frequency (350 to 850 Hz) | Body resonance level | Built-in low-pass opening | Detune and slow pitch drift | Drift speed (0.5 to 2 Hz) |
| Gold Sub | Pitch "knock" at the attack | Adds the octave harmonic | Attack click | No effect | Saturation |
| Cigar Smoke | No effect | Sub-octave level | Built-in low-pass opening (with a slow "breathing" movement) | Detune of the five saws | Breath noise |

The Gold Bell, Velvet Keys and Roulette Mallets engines decay naturally, like the real instruments. Holding a key does not sustain them forever. Raise BODY for a longer ring.

## Factory presets

HIGHROLLER GOLD includes 70 factory presets in 7 banks of 10. All presets load in Poly mode, except the Gold Sub & Bass bank and Cigar Lounge Horn, which load in Mono mode.

**Gold Bells & Jackpot:** Jackpot Bell, Gold Coin, Slot Machine, Casino Chimes, Velvet Box, Crystal Chandelier, Low Gong, Ring Of Fortune, Lucky Seven, Winner Echo.

**Velvet Keys:** Velvet Rhodes, High Stakes Keys, Bark EP, Lounge Keys, Tremolo Velvet, After Dark EP, Wurly Bite, Gilded Bell EP, Penthouse Chords, Tape Velvet.

**Big Band Brass:** Big Band Brass, High Roller Horns, Brass Stab, Soft Flugel, Slow Swell, Vegas Fanfare, Muted Horn, Sax Section, Disco Hit, Cigar Lounge Horn.

**Vegas Organ:** Vegas Organ, Smoky Lounge, Full Drawbars, Gospel Gold, Casino Floor, Perc Organ, Soft Flute Stop, Rotary Dream, Tape Organ, Golden Swell.

**Mallets & Strings:** Roulette Vibes, Gold Marimba, Chip Stack, Slow Motor Vibes, Dealer's Hand (Roulette Mallets engine); Silk Strings, Velvet Strings, Slow Rise Strings, Dark Cellos, Pizz Casino (Silk Strings engine).

**Gold Sub & Bass:** Gold 808, Long Slide Gold, Dirty Gold, Boom Bap Sub, Hard Knock, Deep Vault, Short Gold, Smooth Sub, Pitch Drop Gold, Rumble Royale.

**Cigar Smoke Pads:** Cigar Smoke, Smoky Backroom, Velvet Curtain, Golden Haze, High Roller Rise, Midnight Casino, Whisper Table, Vault Drone, Pulse Of Luck, Final Hand.

Some presets are transposed with PITCH. One octave up: Gold Coin, Slot Machine, Casino Chimes, Crystal Chandelier, Gilded Bell EP, Chip Stack, Golden Haze. One octave down: Low Gong, After Dark EP, Dark Cellos, Deep Vault, Smoky Backroom, Vault Drone.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.hgpreset` file. The default folder is `Documents\DJBILBOX BEATS\HIGHROLLER_GOLD\Presets`. Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .hgpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

Rhodes and Wurlitzer are trademarks of their respective owners. Preset names that refer to them only describe a sound style. DJBILBOX BEATS is not affiliated with the trademark owners.

## Production tips

1. **Trap bells and melodies.** Start from Jackpot Bell or Gold Coin. TONE changes the FM ratio, so small moves change the bell color a lot: settle on a value that sounds in tune with your track. Raise BRIGHT for a harder strike. Set TIME to 60 / BPM x 0.75 for a dotted-eighth echo.
2. **Luxury Hip-Hop keys.** Use the Velvet Keys bank (Velvet Rhodes, Penthouse Chords). The "bark" responds to velocity, so set VELOCITY around 0.7 to 0.9 and vary note velocities in the Piano roll. Add CHORUS between 0.3 and 0.45 for a wide, glossy electric piano.
3. **G-Funk and West Coast.** Pair a Vegas Organ preset (Smoky Lounge, Perc Organ) with a Gold Sub bass. On the organ, WIDTH adds the percussion and TEXTURE the key click. For a whining lead, set Mono mode with GLIDE around 0.1.
4. **Boom Bap and lo-fi.** Load Tape Velvet or Tape Organ. Pull FILTER down to 2,000 to 3,500 Hz, add a touch of DISTORTION (0.1 to 0.2) and use REVERB sparingly so the chords stay dry and dusty.
5. **808s and subs.** Gold 808 and Hard Knock are punchy 808-style subs; Short Gold is a short, plucked version. TONE sets the knock and TEXTURE the saturation, which helps the bass on small speakers. Long Slide Gold has glide set for 808 slides: every new note slides from the previous one.
6. **Brass stabs and fanfares.** Brass Stab and Disco Hit are short and punchy. For slow swells, raise TONE (slower filter swell) and ATTACK, as in Slow Swell. Short presets are quieter by nature: raise VOLUME.
7. **R&B and Deep House pads.** The Cigar Smoke Pads bank has long attacks and releases. For a moving pad, set Destination to Filter with a slow RATE (0.1 to 0.3 Hz), as in Midnight Casino. High Roller Rise uses the filter envelope (ENV) for a slow opening.
8. **Rotary-style organ.** Rotary Dream uses the Timbre destination with CHORUS to suggest a spinning speaker. Move RATE between 1 and 6 Hz to switch between slow and fast movement.

## MIDI, automation and performance

**Notes and channels.** HIGHROLLER GOLD responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider. On Velvet Keys, velocity also controls the "bark".

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note releases the previous one (last-note priority); a long RELEASE can briefly overlap the two notes. Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead.

**Other MIDI.** The sustain pedal (CC64) is supported. In Mono mode, avoid the pedal: notes held by the pedal keep sounding together. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend, filter, distortion and delay changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** HIGHROLLER GOLD is lightweight. Effects set to zero are bypassed and cost no CPU. The five-saw engines (Silk Strings, Cigar Smoke) use a little more CPU than the others. Short release times keep the voice count low. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**HIGHROLLER GOLD does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **HIGHROLLER_GOLD** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
The on-screen keyboard covers the lowest MIDI notes (0 to 31), and most of them are below what speakers can reproduce. Play from a MIDI keyboard or the Piano roll around C4 to C6 (FL Studio naming). Also check VOLUME, and check that FILTER is not closed with Filter Type set to Low Pass.

**My bell or electric piano dies out while I still hold the key.**
Gold Bell, Velvet Keys and Roulette Mallets decay on their own, like real bells and pianos. Raise BODY for a longer ring, and raise SUSTAIN and RELEASE if the amplitude envelope also cuts the note.

**My chord only plays one note.**
The Mono/Poly menu is set to Mono, which is the case for the Gold Sub & Bass presets and Cigar Lounge Horn. Switch it to Poly.

**The sound gets squashed when I play loud chords.**
A soft limiter protects the output. Lower VOLUME, DISTORTION or the number of notes, and use the FL Studio mixer to set the final level.
