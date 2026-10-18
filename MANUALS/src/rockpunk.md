# ROCKPUNK

Rock'n'roll electric guitar instrument: two modelled strings per note, the "Urban Slayer" amp, four drive types, a filter, an LFO, chorus, reverb and a tempo-syncable delay. 76 factory presets, no recordings.

| | |
|---|---|
| Type | Instrument (modelled electric guitar) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.1.0 |
| Factory presets | 76 |
| Polyphony | 8 voices, or Mono |
| Automatable parameters | 65 |

## Overview

ROCKPUNK is a rock guitar you play from MIDI. It ships **no recording**: every note is calculated. Each note runs two modelled strings (**string A**, a rhythm string with a short sustain and a soft pick, and **string B**, a lead string with a long sustain and an aggressive pick). Both strings use a plucked-string model (Karplus-Strong) with a body resonance. A **layer mix** crossfades between them.

The signal then goes through the **Urban Slayer** amp (gain, tone, an asymmetric saturation and a level control), a **drive** stage with four flavours (Soft, Hard, Asymmetric and Fuzz), a resonant **filter** with its own envelope, an **LFO**, a **chorus**, a ping-pong **delay** (free or synced to the tempo), a **reverb**, a stereo **width** control and a limiter.

An optional **sampled layer** plays your own audio files if you drop them in a folder. None are included, and the 76 factory presets do not need it.

## Installation

**Windows.** Run the installer included in your download. Accept the Windows elevation prompt: the VST3 folder is under Program Files, and the installer cannot write there without administrator rights. The plugin is installed as `C:\Program Files\Common Files\VST3\ROCKPUNK.vst3`. The installer is not code-signed, so Windows SmartScreen may show a warning: click **More info**, then **Run anyway**.

**macOS.** Copy `ROCKPUNK.vst3` to `/Library/Audio/Plug-Ins/VST3` and, for Logic, `ROCKPUNK.component` to `/Library/Audio/Plug-Ins/Components`, or use the installer package of your download. The package is not notarized by Apple: if macOS refuses to open it, use **System Settings > Privacy & Security > Open Anyway**.

After installing, restart your host and force a plugin rescan. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**; the plugin is listed as **ROCKPUNK** under **Generators**.

**Where are the files?**

| What | Windows | macOS |
|---|---|---|
| Plugin | `C:\Program Files\Common Files\VST3\ROCKPUNK.vst3` | `/Library/Audio/Plug-Ins/VST3` (and `Components` for the Audio Unit) |
| Presets (factory and yours) | `Documents\DJBILBOX\ROCKPUNK\Presets` | `Documents/DJBILBOX/ROCKPUNK/Presets` |
| Your audio files | `Documents\DJBILBOX\ROCKPUNK\Samples` | `Documents/DJBILBOX/ROCKPUNK/Samples` |

The factory presets travel inside the plugin and are copied to your Documents folder the first time it runs, without overwriting a file of the same name. Back up the Presets folder: uninstalling never touches it.

## Quick start in FL Studio

1. In the **Channel rack**, click **+** and choose **ROCKPUNK**.
2. Click the **folder button** or the preset name in the preset bar and pick a preset by family, or step through the presets with **<** and **>**. Start with **01 Schoolyard Crunch**.
3. Play from your MIDI keyboard or draw notes in the **Piano roll**. The on-screen keyboard covers three octaves (37 keys, C2 to C5).
4. Make sure at least one of the two **speaker grills** in the center of the panel is lit: they switch string A and string B on and off.
5. Shape the sound with **DISTORTION**, **FILTER**, **REVERB** and the **WAVEFORM** menu (the pick hardness: Soft, Round, Bright or Sharp).
6. Click the **disk button** to save your own version as a preset.

## Interface

Everything is drawn on one faceplate. The window opens at 1000 x 746 pixels and can be resized from 500 to 2000 pixels wide; the whole panel scales and the proportions stay fixed.

**Mouse.** Drag knobs and faders up or down. Hold Shift for fine control. Double-click a knob, fader or wheel to return it to its default value. The mouse wheel over a knob or fader nudges it. Hover over any control to read what it does.

**Preset bar (top center).** The folder button or the preset name opens the list of presets sorted by family. **<** and **>** load the previous and next preset. The disk button saves the current sound as a preset (`.rpk` file).

**EXPANSION cassette (top left).** Click it to open a menu:

- **Init patch**: every control back to its default value.
- **Open preset file...**: load any `.rpk` file.
- **Save preset as...**: same as the disk button.
- **Show presets folder**: opens `Documents\DJBILBOX\ROCKPUNK\Presets`.
- **Delay on / Delay synced**: switch the delay on and lock it to the host tempo.
- **Your samples**: the optional sampled layer (see below).
- **About**.

**LFO (left panel).** **RATE** (0.05 to 20 Hz), **AMOUNT** (depth of the LFO on the destination chosen with DESTINATION) and **PITCH** (vibrato depth, whatever the destination).

**Volume envelope (left panel faders).** **ATTACK, DECAY, SUSTAIN, RELEASE**: from a dry, short rhythm sound to a pad that swells in.

**Filter envelope (right panel faders).** **A, S, D, R** shape the filter sweep, in the order shown on the panel. The fifth fader, **VELOCITY**, sets how much your playing dynamics shape each note (at zero, every note attacks the same).

**Knobs.**

- **GLIDE**: time taken to slide from one note to the next.
- **FILTER**: filter cutoff. Turning it switches the filter on if FILTER TYPE is Off.
- **VOLUME**: master level.
- **REVERB**: amount of reverb.
- **CHORUS**: amount of chorus (stereo widening).
- **DISTORTION**: amount of drive after the amp.
- **TIME**: delay time. Turning it switches the delay on.
- **FEEDBACK**: number of delay repeats. Turning it switches the delay on.

**Speaker grills (center, on either side of the logo).** They are switches: click one to mute or restore the matching string. A lit dot at the top means the string is on. If you hear nothing, check that at least one grill is lit.

**Drop-down selectors (bottom).**

- **WAVEFORM**: how hard the pick is: Soft, Round, Bright or Sharp.
- **LFO SOURCE**: shape of the LFO: Sine, Triangle, Square or S&H.
- **DESTINATION**: what the LFO moves: Pitch, Filter, Volume or Drive.
- **FILTER TYPE**: Low-pass, Band-pass, High-pass or Off.
- **MONO/POLY**: Poly (8 notes), or Mono to play one note at a time and let GLIDE join them.

**Wheels (bottom left).** **PITCH BEND** moves the pitch by plus or minus two semitones and springs back to the middle. **MODULATION** opens the vibrato; it also follows MIDI CC1 from your keyboard.

**Keyboard (bottom).** Three octaves, C2 to C5. Click and slide to play; it also lights up with the MIDI notes it receives. The panel has a neon animation: keys, knobs, faders, wheels and grills light up when you hover over them and while you play.

## The sound path

MIDI notes go to a synthesizer of 8 voices. Each voice has two strings (A and B), each with a body resonance. The two strings are blended by the **Layer Mix** (an equal-power crossfade). The mixed signal goes through the **Urban Slayer** amp (pre-gain, a tone filter, an asymmetric saturation that adds second and third harmonics, and an output level), then the **Drive** stage (Soft, Hard, Asymmetric or Fuzz, with its own tone filter), the **Delay** (stereo ping-pong, free or synced, with a low-pass filter in the loop), the **Width** control (mid/side), the **Volume** and a final soft limiter. A note keeps the same position in the stereo field every time you play it (the pan depends on the note name).

## Parameters

ROCKPUNK has 65 automatable parameters. The controls on the faceplate are listed first. Some settings have no knob on the faceplate but can be automated from the parameter list of your host.

**Controls on the faceplate**

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| ATTACK | Attack | 0.5 to 2,000 ms | 3 ms | Volume envelope attack. |
| DECAY | Decay | 1 to 4,000 ms | 600 ms | Volume envelope decay. |
| SUSTAIN | Sustain | 0 to 100 % | 70 % | Volume envelope sustain level. |
| RELEASE | Release | 1 to 5,000 ms | 220 ms | Volume envelope release. |
| GLIDE | Glide | 0 to 1,000 ms | 0 | Time to slide between notes. |
| MONO/POLY | Mono | Poly, Mono | Poly | Mono plays one note at a time. |
| FILTER TYPE | Filter Type | Low-pass, Band-pass, High-pass, Off | Off | Filter mode. |
| FILTER | Filter | 60 Hz to 18 kHz | 14 kHz | Filter cutoff. |
| A, S, D, R (right faders) | Filter Attack, Filter Decay, Filter Sustain, Filter Release | 0.5 to 2,000 ms; 1 to 4,000 ms; 0 to 100 %; 1 to 5,000 ms | 5 ms, 400 ms, 100 %, 300 ms | Filter envelope. |
| VELOCITY | Velocity | 0 to 100 % | 60 % | How much velocity shapes each note. |
| RATE | LFO Rate | 0.05 to 20 Hz | 5 Hz | LFO speed. |
| AMOUNT | LFO Amount | 0 to 100 % | 0 % | LFO depth on the destination. |
| PITCH | LFO Pitch | 0 to 100 % | 0 % | Vibrato depth. |
| LFO SOURCE | LFO Wave | Sine, Triangle, Square, S&H | Sine | LFO shape. |
| DESTINATION | LFO Destination | Pitch, Filter, Volume, Drive | Filter | What the LFO moves. |
| WAVEFORM | Pick Shape | Soft, Round, Bright, Sharp | Round | Pick hardness. |
| VOLUME | Volume | -60 to +6 dB | -3 dB | Master level. |
| REVERB | Reverb | 0 to 100 % | 18 % | Reverb amount. |
| CHORUS | Chorus | 0 to 100 % | 0 % | Chorus amount. |
| DISTORTION | Drive | 0 to 100 % | 20 % | Amount of the drive stage after the amp. |
| TIME | Delay Time | 10 ms to 2 s | 350 ms | Delay time (when not synced). |
| FEEDBACK | Delay Feedback | 0 to 95 % | 40 % | Delay repeats. |
| Pitch Bend wheel | Pitch Bend | -2 to +2 semitones | 0 | Pitch bend. |
| Modulation wheel | Modulation | 0 to 100 % | 0 % | Vibrato depth (also follows CC1). |
| Left speaker grill | Layer A On | Off, On | On | Switches string A on or off. |
| Right speaker grill | Layer B On | Off, On | On | Switches string B on or off. |

**Settings without a faceplate control (host automation)**

| Host name | Range | Default | Function |
|---|---|---|---|
| Layer Mix | 0 to 100 % | 50 % | Crossfade between string A and string B. |
| Width | 0 to 100 % | 80 % | Stereo width (mid/side). |
| Limiter | Off, On | On | Final soft limiter. |
| Layer A Blend, Tone, Body, Sustain, Pick | 0 to 100 % | 80, 60, 30, 35, 30 % | Blend, tone, body resonance, sustain and pick brightness of string A. |
| Layer A Level | 0 to 100 % | 100 % | Level of string A. |
| Layer B Blend, Tone, Body, Sustain, Pick | 0 to 100 % | 90, 25, 25, 55, 70 % | Same controls for string B. |
| Layer B Level | 0 to 100 % | 100 % | Level of string B. |
| Amp On | Off, On | On | Switches the Urban Slayer amp. Off passes the strings straight through. |
| Amp Gain | 0 to 24 dB | 6 dB | Amp pre-gain. |
| Amp Distortion | 0 to 100 % | 55 % | Amount of asymmetric saturation. |
| Amp Tone | 100 Hz to 10 kHz | 2.5 kHz | Tone filter of the amp. |
| Amp Level | -12 to +12 dB | 0 dB | Amp output level. |
| Drive Type | Soft, Hard, Asym, Fuzz | Soft | Character of the drive stage. |
| Drive Tone | 100 Hz to 10 kHz | 2.5 kHz | Tone filter of the drive stage. |
| Delay On | Off, On | Off | Switches the delay (turning TIME or FEEDBACK also switches it on). |
| Delay Sync | Off, On | Off | Locks the delay time to the host tempo. |
| Delay Division | 1/16, 1/8T, 1/8, 1/4T, 1/4, 1/2, 1 bar, 2 bar, 4 bar | 1/8 | Note value when synced. |
| Delay Mix | 0 to 100 % | 25 % | Level of the delay. |
| Delay LP | 100 Hz to 20 kHz | 5 kHz | Low-pass filter inside the delay loop. |
| Reverb Size | 0 to 100 % | 60 % | Size of the reverb. |
| Filter Resonance | 0.5 to 8 | 0.9 | Resonance of the filter. |
| Sample On, Sample Index, Sample Root Offset, Sample Level, Sample Loop, Sample Attack, Sample Release | see below | Off, 0, 0, 80 %, Off, 5 ms, 250 ms | Controls of the optional sampled layer. |

The sampled layer defaults to Off, so it does not affect any factory preset.

## Your own guitars (optional)

The sampled layer plays your files. Drop `.wav`, `.aif` or `.flac` files in `Documents\DJBILBOX\ROCKPUNK\Samples`, then click the cassette and choose **Your samples**, and pick a file. It plays across the whole keyboard. ROCKPUNK measures the note of each file to tune the keyboard; if the measurement lands beside the note (chords and riffs are ambiguous), use the **Sample Root Offset** parameter from your host to correct it. Sub-folders become categories in the list (for example `Samples/Rhythm/palm-mute-E.wav`). The folder is empty after installation, and that is normal: the factory presets only use the two modelled strings. Use only recordings you have the right to use.

## Factory presets

ROCKPUNK includes 76 factory presets. They are numbered and grouped by family, and all of them are set to the same level, so moving from one to another gives no jump in volume. None of them needs the Samples folder.

**Crunch rhythm (14 presets, numbers 1 to 14):** 01 Schoolyard Crunch, 02 Dirty Alley Riff, 03 Bar Fight Boogie, 04 Blacktop Shuffle, 05 Stadium Stomp, 06 Highway Dust, 07 Loud And Guilty, 08 Steel Toe Boots, 09 Rattlecan Red, 10 Curfew Breaker, 11 Two Bar Warning, 12 Half Past Loud, 13 Smokestack Strut, 14 Rust Belt Anthem.

**Leads (12 presets, numbers 15 to 26):** 15 Leather Jacket Lead, 16 The Long Encore, 17 Last Call, 18 Screaming Treble, 19 Third Encore, 20 Pocket Knife Lead, 21 Gutter Star, 22 Barbed Wire Bend, 23 Nine Volt Heart, 24 Siren Lead, 25 Empty Stage, 26 Backstage Pass.

**High gain (10 presets, numbers 27 to 36):** 27 Amp On Fire, 28 Six String Riot, 29 Whiskey Throttle, 30 Gravel And Gasoline, 31 Broken Bottle Blues, 32 Iron Lung Crunch, 33 Wrecking Yard, 34 Bad Ignition, 35 Full Throttle Choir, 36 Sweat And Sparks.

**Clean (12 presets, numbers 37 to 48):** 37 Clean Getaway, 38 Chrome Bumper, 39 First Gear, 40 Train Yard Twang, 41 Back Row Boogie, 42 Cold Start Clean, 43 Lipstick Pickup, 44 Motel Neon, 45 Paper Cup Whiskey, 46 Alley Cat Strut, 47 Cellar Door, 48 Closing Time Chords.

**Bare strings (8 presets, numbers 49 to 56):** 49 Sunday Sermon, 50 Porch Light, 51 Open Road Steel, 52 Double Denim, 53 Ashtray Ballad, 54 Slow Burn Sunday, 55 Tin Roof Rumble, 56 Velvet Bruise.

**Nocturnal (8 presets, numbers 57 to 64):** 57 Midnight Mechanic, 58 Neon Garage, 59 Nightshift Riff, 60 Cheap Hotel, 61 Static Highway, 62 Last Bus Home, 63 Diesel Sunday, 64 Amp In The Rain.

**Textures (8 presets, numbers 65 to 72):** 65 Feedback Chapel, 66 Warehouse Echo, 67 Dust Devil, 68 Overdrive Sunset, 69 Rooftop Riot, 70 Chrome And Bone, 71 Dead Battery, 72 Nightshift Wah.

**Low end (4 presets, numbers 73 to 76):** 73 Rusty Thunder, 74 Iron Ballast, 75 Anvil Chorus, 76 Bottom Feeder.

**Loading a preset** replaces the current settings with the preset. **User presets** are saved as `.rpk` files in `Documents\DJBILBOX\ROCKPUNK\Presets` and appear in the preset list. A `.rpk` file is the plugin state in XML. Do not rename a factory preset file: saved projects find their preset by its name.

## Production tips

1. **Riffs.** Start from the Crunch rhythm family (01 to 14): the dry attack and short notes suit palm-muted riffs. Play power chords in Poly mode.
2. **Leads.** The Leads family sustains and uses delay and vibrato. Set MONO/POLY to Mono and raise GLIDE for slides.
3. **High gain.** The High gain family uses the Fuzz and Asymmetric drive types. Lower VOLUME if the level feels too hot.
4. **Clean parts.** The Clean family uses the amp on a clean gain with chorus and delay.
5. **Use the strings.** Switch one of the two speaker grills off to hear string A (rhythm) or string B (lead) alone.
6. **Delay in time.** Choose **Delay synced** in the cassette menu, then set the Delay Division from your host's parameter list.
7. **Movement.** Set DESTINATION to Filter or Drive and raise AMOUNT for textures (see the Textures family).

## MIDI, automation and performance

**Notes.** ROCKPUNK responds to MIDI notes on any MIDI channel, with up to 8 voices. Velocity shapes each note according to the VELOCITY fader.

**Pitch bend and modulation.** MIDI pitch bend covers plus or minus two semitones. CC1 and the MODULATION wheel open the vibrato.

**Automation in FL Studio.** All 65 parameters can be automated. Move a control in the plugin, then use **Tools > Last tweaked > Create automation clip**. Use **Tools > Last tweaked > Link to controller** to assign a hardware knob.

## FAQ

**The plugin does not appear in my DAW.**
Check that the plugin scan includes `C:\Program Files\Common Files\VST3`, then run a full rescan. In FL Studio, use **Options > Manage plugins > Find installed plugins**.

**I hear nothing.**
Make sure at least one of the two speaker grills is lit: they switch the two strings, and with both off there is nothing left to play. Also check that the plugin is not bypassed and that MIDI reaches it.

**The "Your samples" list is empty.**
That is the normal state of a fresh install: ROCKPUNK ships no recording. Drop your files in `Documents\DJBILBOX\ROCKPUNK\Samples` (cassette, then **Your samples**, then **Open the Samples folder**).

**Where are my presets?**
In `Documents\DJBILBOX\ROCKPUNK\Presets`. Back this folder up: uninstalling does not touch it.

**Can I sell the music I make with ROCKPUNK?**
Yes, royalty-free and with no credit required. See the license included in the download.

**Does it sound like a real guitar?**
It is a physical model, not a recording. Its character is tight and punchy, built for rock and punk riffs and leads. Audition the presets in your own projects.

## Support

djbilboxbeats.com - support@djbilboxbeats.com
