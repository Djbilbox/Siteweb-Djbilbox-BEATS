# HIGHROLLER GOLD

Casino-lounge synthesizer: gold bells, velvet keys, big band brass, organ, mallets, strings, gold sub and smoky pads from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

HIGHROLLER GOLD is a synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: Gold Bell (FM bell), Velvet Keys (electric-piano style tines with a bell), Big Band Brass (three detuned saws with an opening filter), Vegas Organ (drawbars with percussion and key click), Roulette Mallets (struck bars with tremolo), Silk Strings (five detuned saws with a slow drift), Gold Sub (a sine sub with pitch "knock" and saturation) and Cigar Smoke (a detuned saw pad with breath noise and a filter that slowly breathes).

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and an effects chain: distortion, chorus, ping-pong delay and reverb.

The sound is built for Hip-Hop, Trap and R&B keys and hooks, lounge and soul chords, brass stabs and pads, bell toplines and 808-style subs. The Gold Sub & Bass bank loads in Mono mode for clean one-note lines. All other banks load in Poly mode, with up to 16 voices.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **HIGHROLLER GOLD** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **HIGHROLLER GOLD - Init Patch**: the Gold Bell engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers MIDI notes 48 to 96. Click a key to hear the sound.
5. Shape the sound with the main controls:
    - **TONE**: the main character of the engine (bell ratio, hardness, drawbar level, string or pad color...).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker, rounder sound.
    - **RELEASE** (in AMP ENVELOPE): how long the note rings after you let go.
6. Adjust **VOLUME** to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 500 x 373 up to 2000 x 1492 pixels. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens a message saying that expansion packs are coming soon. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The camel artwork, the HIGHROLLER GOLD title and "By Humpire". The dotted grille around the artwork is decorative.

**Character knobs (row above the artwork).** **TONE, BODY, BRIGHT, WIDTH, TEXTURE**: the five character controls of the selected engine.

**Left side.**

- **LFO** section: **RATE** and **AMOUNT** set the LFO speed and depth.
- **TUNING** section: **TRANSPOSE**, the global transposition (-12 to +12 semitones). It is not an LFO control.
- **AMP ENVELOPE**: four sliders, **ATTACK, DECAY, SUSTAIN, RELEASE**.
- Small knobs **RES** (filter resonance) and **FLT ENV** (filter envelope depth).
- **GLIDE** knob.

**Right side.**

- **FILTER ENVELOPE**: four sliders **ATK, DEC, SUS, REL**, plus the **VELOCITY** slider (velocity sensitivity of the level) just to their right.
- **VOLUME** knob (output level), **CUTOFF** knob (filter cutoff).
- **REVERB, CHORUS, DISTORTION** knobs.
- **DLY MIX** (delay level), **DLY TIME** and **FEEDBACK** knobs.

**Menu row.** Five drop-down menus, each with a caption below it: **ENGINE**, **LFO SHAPE**, **LFO DESTINATION**, **FILTER TYPE** and **VOICE MODE** (Poly or Mono). Each menu shows its current value.

**Wheels (bottom left).** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. The keyboard covers MIDI notes 48 to 96 (four octaves plus one key). The key labels on screen run from C2 to C6, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C4 to C8. It always shows the whole range and does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

HIGHROLLER GOLD exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Gold Bell, Velvet Keys, Big Band Brass, Vegas Organ, Roulette Mallets, Silk Strings, Gold Sub, Cigar Smoke | Gold Bell | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune, tremolo depth, drift...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (noise, saturation, speed...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| FLT ENV (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| ATK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DEC (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| SUS (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| REL (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
| VELOCITY (Velocity Sensitivity) | 0 to 1 | 0.5 | How much note velocity changes the level. 0: all notes at the same level. 1: full velocity response. |
| ATTACK (Attack) | 0.001 to 5 s | 0.01 s | Amplitude envelope attack time. |
| DECAY (Decay) | 0.01 to 5 s | 0.4 s | Amplitude envelope decay time. |
| SUSTAIN (Sustain) | 0 to 1 | 0.7 | Amplitude envelope sustain level. |
| RELEASE (Release) | 0.02 to 8 s | 0.4 s | Amplitude envelope release time. |
| VOLUME (Volume) | 0 to 1 | 0.7 | Output level of each voice, before the effects. Shown in dB. |
| GLIDE (Glide) | 0 to 2 | 0 | Portamento. Higher values give a slower slide. Each new note slides from the pitch of the previous note, in Mono and in Poly mode. 0 turns glide off. |
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
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Gold Bell | Ratio of the FM modulator (bell color) | Length of the ring | FM depth (brightness of the strike) | Slight detune between the two carriers | Noise click at the attack |
| Velvet Keys | No effect | Length of the tines | "Bark" (upper partials), stronger with higher velocity | No effect | Attack noise |
| Big Band Brass | Time the filter takes to open | Output weight | How far the filter opens | Detune of the three saws | Breath noise |
| Vegas Organ | Level of the sub-octave drawbar | Level of the 2nd harmonic drawbar | Level of the upper drawbars | Percussion at the attack | Key click |
| Roulette Mallets | Mallet noise at the attack | Length of the ring | Level of the upper partials | Tremolo depth | Tremolo speed |
| Silk Strings | Position of the body resonance | Level of the body resonance | Filter opening | Detune and drift of the five saws | Speed of the slow drift |
| Gold Sub | Pitch "knock" at the attack | Adds the octave harmonic | Attack click | No effect | Saturation |
| Cigar Smoke | No effect | Level of the sub layer | How far the filter opens | Detune of the five saws | Breath noise |

## Factory presets

HIGHROLLER GOLD includes 70 factory presets in 7 banks of 10. The Gold Sub & Bass bank loads in Mono mode. In the Big Band Brass bank, only Cigar Lounge Horn is in Mono mode. Every other preset loads in Poly mode.

**Gold Bells & Jackpot:** Jackpot Bell, Gold Coin, Slot Machine, Casino Chimes, Velvet Box, Crystal Chandelier, Low Gong, Ring Of Fortune, Lucky Seven, Winner Echo.

**Velvet Keys:** Velvet Rhodes, High Stakes Keys, Bark EP, Lounge Keys, Tremolo Velvet, After Dark EP, Wurly Bite, Gilded Bell EP, Penthouse Chords, Tape Velvet.

**Big Band Brass:** Big Band Brass, High Roller Horns, Brass Stab, Soft Flugel, Slow Swell, Vegas Fanfare, Muted Horn, Sax Section, Disco Hit, Cigar Lounge Horn.

**Vegas Organ:** Vegas Organ, Smoky Lounge, Full Drawbars, Gospel Gold, Casino Floor, Perc Organ, Soft Flute Stop, Rotary Dream, Tape Organ, Golden Swell.

**Mallets & Strings:** Roulette Vibes, Gold Marimba, Chip Stack, Slow Motor Vibes, Dealer's Hand (Roulette Mallets engine), then Silk Strings, Velvet Strings, Slow Rise Strings, Dark Cellos, Pizz Casino (Silk Strings engine).

**Gold Sub & Bass:** Gold 808, Long Slide Gold, Dirty Gold, Boom Bap Sub, Hard Knock, Deep Vault, Short Gold, Smooth Sub, Pitch Drop Gold, Rumble Royale.

**Cigar Smoke Pads:** Cigar Smoke, Smoky Backroom, Velvet Curtain, Golden Haze, High Roller Rise, Midnight Casino, Whisper Table, Vault Drone, Pulse Of Luck, Final Hand.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.hgpreset` file. The default folder is `DJBILBOX BEATS/HIGHROLLER_GOLD/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .hgpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

## Production tips

1. **Hooks and toplines.** Start from Jackpot Bell, Gold Coin or Crystal Chandelier. Raise TONE to change the bell color and BODY for a longer ring. Add REVERB and a little DLY MIX with DLY TIME at 0.375 s for a sparkling top layer.
2. **Soul and R&B chords.** The Velvet Keys bank plays chords in Poly mode. Raise BRIGHT for more bite on hard notes, and keep the VELOCITY slider around 60 to 80% so your Piano roll accents come through. Add CHORUS for width.
3. **Brass stabs and swells.** In Big Band Brass, shorten DECAY and SUSTAIN for stabs (Brass Stab, Disco Hit) or raise ATTACK for swells (Slow Swell). TONE sets how fast the filter opens.
4. **Organ beds.** Vegas Organ mixes drawbar levels with TONE, BODY and BRIGHT. WIDTH adds percussion at the attack and TEXTURE adds key click. Set LFO DESTINATION to Amp for a tremolo, as a starting point for rotary-style movement.
5. **Vibes and marimba.** In Roulette Mallets, WIDTH sets the tremolo depth and TEXTURE its speed. Lower RELEASE for dry, tight hits.
6. **Strings and pads.** Silk Strings and Cigar Smoke use detuned saws. Raise ATTACK and RELEASE for slow pads, and WIDTH for a thicker sound. In Cigar Smoke, BRIGHT opens the filter and the filter slowly breathes by itself.
7. **808 subs.** Load a Gold Sub & Bass preset (Gold 808, Long Slide Gold). Raise TONE for a harder knock. Raise TEXTURE or add a little DISTORTION so the sub stays audible on phone and laptop speakers.
8. **Slides.** With GLIDE above zero, every new note slides from the previous one, even if the notes do not overlap. For slides on selected notes only, keep GLIDE at 0 and automate it up just before the notes that should slide.
9. **Filter movement.** Raise FLT ENV and shorten the filter DEC slider for a plucked attack. For a wobble, set LFO DESTINATION to Filter and RATE between 1 and 4 Hz.

## MIDI, automation and performance

**Notes and channels.** HIGHROLLER GOLD responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. A new note starts with the latest CC1 value. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend and cutoff changes are smoothed to avoid clicks. Distortion and delay level changes are smoothed too.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**Output.** Stereo output. Each note gets a slight random position in the stereo field.

## FAQ

**HIGHROLLER GOLD does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **HIGHROLLER GOLD** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Check VOLUME, and check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. Some presets have a very slow ATTACK (pads and swells) or a long fade-in, so hold the note for a moment. For notes outside the on-screen keyboard range (MIDI 48 to 96), play from a MIDI keyboard or the Piano roll.

**My sub is not perfectly centered.**
Each note gets a slight random stereo offset. For a strictly centered sub, reduce the stereo separation of the mixer track in FL Studio, or insert a mono utility after HIGHROLLER GOLD. If the sub is also weak on small speakers, add harmonics with TEXTURE, BODY or DISTORTION.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono, which is the default for the Gold Sub & Bass presets. Switch it to Poly (up to 16 voices).

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.
