# ARENA BASS

Stadium-size bass synthesizer: 808 subs, slap, reese, acid, horns and kicks from eight synthesis engines.

| | |
|---|---|
| Type | Synthesizer (instrument) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 70 |
| Latency | 0 samples |

## Overview

ARENA BASS is a bass synthesizer sold under the HUMPIRE brand by DJBILBOX BEATS. It uses no samples. Every sound is generated in real time by one of eight engines: a sine sub with a pitch "knock", a plucked-string slap bass, a Moog Bass engine (two saws, a square and a sub octave), a detuned Stadium Reese, a two-operator FM Growl, an Acid Rubber voice with a self-closing resonant filter, a Stadium Horns brass section and an Arena Kick for pitched kicks and stomps.

Five character controls (TONE, BODY, BRIGHT, WIDTH, TEXTURE) reshape each engine. The sound then goes through a multimode filter with its own envelope, an amplitude envelope, an LFO and a master effects chain: distortion, chorus, ping-pong delay and reverb. The character is big, punchy and direct, made for sport-arena energy and hard-hitting low end.

ARENA BASS targets Trap, Drill and Hip-Hop 808s, Afrotrap and Afro bass lines, G-Funk slides, Deep House and acid bass, plus horn stabs and anthem hooks. Bass presets load in Mono mode for clean, one-note-at-a-time lines. The Stadium Horns bank plays chords in Poly mode with up to 16 voices.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **ARENA_BASS** (vendor **DJBILBOX BEATS**). You can also drag it from the **Plugin database > Generators** browser.
2. The plugin opens on **ARENA BASS - Init Patch**: the Arena Sub engine in Poly mode.
3. Click the **folder icon** at the left of the preset display. Pick a bank, then a preset. You can also step through all 70 presets with the **<** and **>** arrows.
4. Play from your MIDI keyboard or draw notes in the **Piano roll**. Bass lines sit best between MIDI notes 24 and 48 (C2 to C4 in FL Studio naming, where middle C is C5). Presets with TRANSPOSE at -12 sound one octave lower than the key you play.
5. Shape the sound with the three main controls:
    - **TONE**: the main character of the engine (knock, pluck hardness, sub level, FM ratio...).
    - **CUTOFF**: the cutoff of the filter. Turn it down for a darker, rounder bass.
    - **GLIDE**: slide time between notes. Leave it at zero for straight notes.
6. Adjust **VOLUME** (in the MASTER section) to set the level, then click the **disk icon** to save your own version.

## Interface

The window opens at 1000 x 746 pixels. Drag the bottom-right corner to resize it from 50% to 200%. The proportions stay fixed.

**Preset display (top center).** From left to right: the folder icon (preset menu), the **<** arrow, the current preset name, the **>** arrow and the disk icon (save). The arrows wrap around from the last preset to the first.

**EXPANSION cassette (top left).** Clicking the cassette opens an information message about upcoming expansion packs. No expansion content is included with version 1.0.

**Logo (top right).** The DJBILBOXBEATS name and the DB logo.

**Center.** The camel artwork, the ARENA BASS title and "By Humpire". The dotted grille at the left of the artwork is decorative.

**Character panel (top right, slatted panel).** Two rows of small knobs:

- Row 1: **TONE, BODY, BRIGHT, WIDTH, TEXTURE**. The five character controls of the selected engine.
- Row 2: **RES** (filter resonance), **ENV AMT** (filter envelope depth), **DISTORTION** and **DLY MIX** (delay level).

**LFO section (left).** **RATE** and **AMOUNT** set the LFO speed and depth. **TRANSPOSE**, also under this header, is the global transposition (-12 to +12 semitones). It is not an LFO control.

**MASTER section (right).** **VOLUME**, **REVERB** and **CHORUS**.

**Center row.** From left to right:

- **AMP ENVELOPE**: four sliders, **ATTACK, DECAY, SUSTAIN, RELEASE**.
- **GLIDE** and **CUTOFF** knobs.
- **FILTER ENVELOPE**: four sliders, **ATK, DEC, SUS, REL**. The **VELOCITY** slider (velocity sensitivity of the level) sits just to their right.
- **DELAY**: **TIME** and **FEEDBACK** knobs (delay time and delay repeats). The delay level is the **DLY MIX** knob in the character panel.

**Menu row.** Five drop-down menus, each with a caption below it: **ENGINE** (Waveform), **LFO SHAPE** (LFO Source), **LFO DESTINATION** (Destination), **FILTER TYPE** and **VOICE MODE** (Mono/Poly). Each menu shows its current value.

**Wheels (bottom left).** **Pitch Bend** springs back to the center when you release it. **Modulation** stays where you leave it.

**On-screen keyboard (bottom).** Click a key to play it. Velocity follows the click position: click near the bottom of a key for a louder note. The keyboard covers MIDI notes 36 to 84 (four octaves plus one key). The key labels on screen run from C1 to C5, with middle C labeled C3; in FL Studio naming, where middle C is C5, the same range is C3 to C7. It does not scroll. For notes outside this range, use a MIDI controller or the Piano roll.

**Mouse gestures.** Drag a knob up or right to increase it. Drag sliders and wheels vertically. Double-click any knob or slider to return it to its default value. The mouse wheel also adjusts controls. A value bubble appears while you drag.

## Parameters

Signal flow: engine, then filter, then amplitude envelope (per voice). The voices are summed, then processed by Distortion, Chorus, Delay and Reverb. A DC blocker and a soft limiter protect the output.

ARENA BASS exposes 35 automatable parameters. The host name is shown in parentheses when it differs from the panel label.

| Parameter | Range | Default | Function |
|---|---|---|---|
| ENGINE (Waveform menu) | Arena Sub, Slap Bass, Moog Bass, Stadium Reese, FM Growl, Acid Rubber, Stadium Horns, Arena Kick | Arena Sub | Selects the synthesis engine. |
| TONE (Tone) | 0 to 1 | 0.5 | Character control 1. Its effect depends on the engine (see the table below). |
| BODY (Body) | 0 to 1 | 0.5 | Character control 2. Engine-dependent. |
| BRIGHT (Brightness) | 0 to 1 | 0.5 | Character control 3. Engine-dependent. |
| WIDTH (Width) | 0 to 1 | 0.3 | Character control 4. Engine-dependent (detune, drive, wobble speed...). It is not a stereo width control. |
| TEXTURE (Texture) | 0 to 1 | 0.2 | Character control 5. Engine-dependent (saturation, noise, feedback...). |
| CUTOFF (Filter) | 40 to 18,000 Hz | 9,000 Hz | Filter cutoff. The cutoff follows the keyboard by 25% around middle C. |
| RES (Resonance) | 0 to 1 | 0.15 | Filter resonance. |
| FILTER TYPE (menu) | Low Pass, High Pass, Band Pass | Low Pass | Filter mode. |
| ENV AMT (Filter Env Amount) | 0 to 1 | 0 | Filter envelope depth. At 1 the envelope opens the cutoff up to 5 octaves. At 0 the filter envelope has no effect. |
| ATK (Filter Attack) | 0.002 to 3 s | 0.002 s | Filter envelope attack time. |
| DEC (Filter Decay) | 0.02 to 4 s | 0.3 s | Filter envelope decay time. |
| SUS (Filter Sustain) | 0 to 1 | 0 | Filter envelope sustain level. |
| REL (Filter Release) | 0.02 to 4 s | 0.3 s | Filter envelope release time. |
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
| TIME (Delay Time) | 0.05 to 1.0 s | 0.35 s | Delay time in seconds. Not tempo-synced: for a quarter note, use 60 / BPM (0.5 s at 120 BPM); halve it for an eighth note. |
| FEEDBACK (Feedback) | 0 to 0.9 | 0.35 | Number of delay repeats (0 to 90% feedback). |
| REVERB (Reverb) | 0 to 1 | 0.25 | Reverb amount. Higher values also enlarge the room. |
| Pitch Bend wheel (Pitch Bend) | -1 to +1 | 0 | On-screen pitch bend, +/-2 semitones. Adds to MIDI pitch bend. |
| Modulation wheel (Mod Wheel) | 0 to 1 | 0 | On-screen mod wheel. Adds up to +0.8 to the LFO AMOUNT. |

**Character controls by engine**

| Engine | TONE | BODY | BRIGHT | WIDTH | TEXTURE |
|---|---|---|---|---|---|
| Arena Sub | Pitch "knock" at the attack | Adds the octave harmonic | Attack click | No effect | Saturation |
| Slap Bass | Pluck hardness | String sustain | Brighter, longer highs | Low body resonance (around 95 Hz) | Slap click |
| Moog Bass | Sub-octave level | Output weight | Built-in low-pass opening | Detune of the second saw | Blend from saws to square |
| Stadium Reese | Sub-octave level | Detune amount | Built-in low-pass opening | Saturation drive | Noise |
| FM Growl | Modulator ratio (0.5 to 3.5) | No effect | FM depth (growl intensity) | Speed of the built-in wobble (0.5 to 12.5 Hz) | Modulator feedback (grit) |
| Acid Rubber | Saw to pulse blend | Length of the built-in filter sweep | Depth of the built-in filter sweep | No effect | Saturation |
| Stadium Horns | Swell time of the built-in filter | Output weight | How far the filter opens | Detune of the three saws | Breath noise |
| Arena Kick | Pitch-drop depth | Decay length | Click | No effect | Saturation |

## Factory presets

ARENA BASS includes 70 factory presets in 7 banks of 10. All banks except Stadium Horns load in Mono mode. In the Stadium Horns bank, only Lead Horn is in Mono mode (with a short glide).

**Arena Sub:** Arena 808, Stadium Rumble, Long Slide Sub, Locker Room Sub, Hard Knock Bass, Short Pump, Smooth Court, Pitch Drop Dunk, Dirty Overtime, Scoreboard Sub.

**Slap & Pluck Bass:** Slap Bass, Pick Bass, Finger Funk, Muted Pocket, Upright Arena, Pop Slap, Deep Pluck, Chorus Bass, Dub Pluck, Walking Court.

**Moog Bass:** Arena Moog, Fat Moog, Rubber Moog, Sub Moog, Slide Moog, Pump Bass, Octave Moog, Hot Seat, Bass Stab, Warm Anthem.

**Reese & Growl:** Stadium Reese, Dirty Reese, Wobble Arena, Industrial Reese, Sub Reese, FM Growl, Talking Bass, Robot Growl, Metal Growl, Slow Growl.

**Acid & Rubber:** Acid Rubber, Squelch, Rubber Band, Acid Slide, Pulse Acid, Dirty Acid, Open Acid, Low Rubber, Pump Squelch, Buzzer Beater.

**Stadium Horns:** Stadium Horns, Fanfare, Brass Stab, Victory Horns, Soft Flugel, Slow Swell, Hit Section, Low Horns, Lead Horn, Anthem.

**Kicks & Stomps:** Arena Kick, Stomp, Long Boom, Punch Kick, Sub Kick, Crowd Stomp, Dirty Kick, Tom Fill, Drop Impact, Laser Kick.

**Loading a preset** first resets every parameter to its default value, then applies the preset. The two wheels are not changed.

**User presets.** Click the disk icon to save the current sound as a `.abpreset` file. The default folder is `DJBILBOX BEATS/ARENA_BASS/Presets` inside your Documents folder (Windows and macOS). Files in this folder appear in the **User** submenu of the preset menu. Use **Open a .abpreset file...** in the same menu to load a file from another location. The preset display shows the file name.

The host also sees the 70 factory presets as a program list. Your project saves the full state of the plugin, including the preset name.

Moog is a trademark of its owner. It is used here only to describe a sound style. DJBILBOX BEATS is not affiliated with the trademark owner.

## Production tips

1. **Trap and Drill 808s.** Start from Arena 808 or Long Slide Sub. Raise TONE for a harder knock on short notes. Raise TEXTURE or add a little DISTORTION so the 808 stays audible on phone and laptop speakers.
2. **808 slides.** With GLIDE above zero, every new note slides from the previous one, even if the notes do not overlap. For slides on selected notes only, keep GLIDE at 0 and automate it up just before the notes that should slide.
3. **G-Funk bass.** Use the Moog Bass bank (Slide Moog, Fat Moog). Set GLIDE around 0.1 to 0.2 and play legato lines. Raise ENV AMT and shorten the filter DEC slider for a rounder "pluck" at the start of each note.
4. **Afrotrap and Afro bass.** The Slap & Pluck bank (Muted Pocket, Finger Funk, Pick Bass) gives bouncy, percussive lines. These presets are quieter by nature, so raise VOLUME. Use the VELOCITY slider around 60 to 80% so accents in your Piano roll come through.
5. **Deep House and acid.** Load Arena Moog or a preset from the Acid & Rubber bank. Automate CUTOFF over 8 or 16 bars. For movement, set LFO DESTINATION to Filter with a Square or Sample&Hold LFO shape, as in Pump Squelch.
6. **Reese and wobble.** On the Stadium Reese engine, BODY sets the detune and WIDTH drives the saturation. Set LFO DESTINATION to Filter, LFO SHAPE to Sine and RATE between 1 and 4 Hz for a wobble (see Wobble Arena).
7. **Horn hooks and anthems.** The Stadium Horns bank plays chords in Poly mode. Raise the ATTACK slider for swells (Slow Swell) or shorten DECAY and SUSTAIN for stabs (Hit Section). Add REVERB and CHORUS for a stadium-size sound.
8. **Kicks and stomps.** Load the Kicks & Stomps presets in a second ARENA BASS instance, so the kick and the 808 keep separate envelopes. Play kicks around MIDI notes 28 to 40 (E2 to E3 in FL Studio naming). TONE sets the pitch drop, BODY the length and BRIGHT the click. Tom Fill is transposed one octave up for fills.

## MIDI, automation and performance

**Notes and channels.** ARENA BASS responds to all 128 MIDI notes on any MIDI channel. Velocity controls the level according to the VELOCITY slider.

**Polyphony.** Up to 16 voices in Poly mode. When you play a 17th note, a voice is taken over and cut immediately. In Mono mode, each new note stops the previous one (last-note priority). Releasing the newest note does not bring back an earlier note that you still hold.

**Pitch bend.** MIDI pitch bend covers +/-2 semitones. The on-screen Pitch Bend wheel adds another +/-2 semitones.

**Mod wheel (CC1).** CC1 raises the LFO depth, like the on-screen Modulation wheel. When both are used, the higher value wins. CC1 acts on the notes that are sounding when the message arrives, so move the wheel while you play. For repeatable results in a song, automate the on-screen **Mod Wheel** parameter instead.

**Other MIDI.** The sustain pedal (CC64) is supported. All Notes Off and All Sound Off messages stop the sound. Aftertouch is ignored.

**Automation in FL Studio.** All 35 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob. You can also pick parameters from the plugin wrapper menu (**Browse parameters**). Level, pitch, bend, filter, distortion and delay changes are smoothed to avoid clicks.

**Latency and tail.** The plugin adds no latency (0 samples), so no delay compensation is needed. It reports a 12-second tail to the host so reverb and delay trails are not cut when rendering.

**CPU.** ARENA BASS is lightweight. Effects set to zero are bypassed and cost no CPU. Mono mode and short release times keep the voice count low. The plugin works at 44.1, 48, 88.2 and 96 kHz.

## FAQ

**ARENA BASS does not appear in FL Studio.**
Open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths, then click **Find installed plugins**. The plugin is listed as **ARENA_BASS** under **DJBILBOX BEATS**. It is an instrument: load it in the Channel rack, not in a Mixer insert slot.

**I hear nothing, or almost nothing, when I click the on-screen keyboard.**
Some presets use TRANSPOSE -12 and play very low, below what small speakers can reproduce. Check VOLUME, and check that CUTOFF is not turned all the way down with FILTER TYPE set to Low Pass. For notes outside the on-screen keyboard range (MIDI 36 to 84), play from a MIDI keyboard or the Piano roll.

**My 808 is not perfectly centered.**
Each note gets a slight random stereo offset. For a strictly centered sub, reduce the stereo separation of the mixer track in FL Studio, or insert a mono utility after ARENA BASS. If the bass is also weak on small speakers, add harmonics with TEXTURE, BODY or DISTORTION.

**Every note slides into the next one.**
GLIDE is above zero. Glide applies to every new note. Set GLIDE to 0, or automate it only where you want slides.

**My chord only plays one note.**
The VOICE MODE menu is set to Mono, which is the default for bass presets. Switch it to Poly (up to 16 voices).

**Where do I install it on macOS?**
Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host. Both are universal builds.
