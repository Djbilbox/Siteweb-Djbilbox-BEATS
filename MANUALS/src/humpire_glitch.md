# HUMPIRE GLITCH

Lo-fi effect on a handheld-console body: bit crusher from 1 to 16 bits, sample-rate reducer, speaker filter and dither.

| | |
|---|---|
| Type | Audio effect (lo-fi / bitcrusher) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 20 |
| Bit depth | 1 to 16 bits (fractional values allowed) |
| Sample rate | 500 Hz to 48 kHz |
| Latency | 0 samples |

## Overview

HUMPIRE GLITCH degrades the sound on purpose. The signal goes through a **drive** stage (soft saturation), a **sample-rate reducer** (sample and hold, with intentional aliasing), a **dither** stage and a **bit quantizer**. The **speaker** switch then adds a small-speaker simulation: the signal is summed to mono, filtered by a high-pass and a low-pass whose cutoffs follow the **SIZE** knob, and softly saturated.

The interface is a handheld console. A four-tone LCD shows the current settings (bits, rate, dither, speaker and mix). The **D-pad** changes the bit depth (up and down) and the sample rate (left and right). Button **A** cycles the dither and button **B** switches the speaker on or off.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE GLITCH** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Lower **BITS** and **RATE** until the sound gets the crunch you want. 8 bits and 16 kHz is a classic starting point.
4. Use **MIX** to blend the crushed sound with the original.
5. Switch the speaker on with button **B** for a radio or telephone sound, and adjust **SIZE**.
6. Click **SELECT** (SIMPLE) to show or hide the advanced knobs.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. In GLITCH, the SIMPLE and CLEAN switches are the **SELECT** and **START** buttons of the console.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SELECT** (SIMPLE) hides the advanced knobs and keeps the essential ones. **START** (CLEAN) replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable. GLITCH starts in SIMPLE mode.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**LCD (center).** Two lines of text: `BITS 8.0 RATE 48K` and `DITH OFF SPK OFF MIX 100`. In bypass, the LCD shows `** BYPASS **` and is dimmed. A LED at the upper left shows that the plugin is powered.

**D-pad (bottom left).** Up raises the bits by one, down lowers them by one. Right raises the sample rate by about 26%, left lowers it by the same factor.

**Buttons.** **A** (**DITHER**): each click goes to the next dither mode (Off, RPDF, TPDF). **B** (**SPEAKER**): switches the speaker filter on or off. **SELECT** (**SIMPLE**) and **START** (**CLEAN**) are on/off switches with a LED.

**Knobs.** **BITS**, **RATE**, **SIZE**, **DRIVE**, **MIX** and **OUT**. In SIMPLE mode, only **BITS**, **RATE** and **MIX** are visible.

## Parameters

Signal flow: input, drive, sample-rate reducer, dither, bit quantizer, speaker filter (if on), mix with the dry signal, output gain.

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| BITS (or D-pad up/down) | Bits | 1 to 16 bits | 8.0 | Bit depth of the quantizer. Fractional values are allowed. |
| RATE (or D-pad left/right) | Sample Rate | 500 Hz to 48 kHz | 48 kHz | Target sample rate of the reducer. At 48 kHz the reducer does nothing. |
| DRIVE | Drive | 0 to 100 % | 0 % | Soft saturation before the crusher. Transparent at 0. |
| B button | Speaker | Off / On | Off | Mono small-speaker filter with soft saturation. |
| SIZE | Speaker Size | 0 to 100 % | 40 % | Size of the speaker: high-pass from 500 Hz (small) to 120 Hz (large), low-pass from 3.5 kHz (small) to 8 kHz (large). |
| A button | Dither | Off, RPDF, TPDF | Off | Noise added before quantization to smooth the distortion of low bit depths. |
| MIX | Mix | 0 to 100 % | 100 % | Dry/wet balance. |
| OUT | Output | -24 to +12 dB | 0 dB | Output gain. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

GLITCH has 9 automatable parameters. The settings are smoothed over about 30 ms to avoid clicks. The speaker filter gets its signal from the crushed audio and can be mixed with the dry signal.

## Factory presets

GLITCH includes 20 factory presets, grouped by the instrument they suit.

**Trap Vocals:** Radio Crush, Telephone Ad-Lib, Dusty Vocal Chop, Megaphone Hook.

**808:** Dirty Sub 12bit, 808 Grit, Sampler Crunch 808.

**Kick:** 8-Bit Punch, Boom Bap Dust, Hard Clip Kick.

**Oud:** Cassette Oud, Old Souk Radio, Ghost Strings.

**Darbouka:** Darbouka Alley, Tin Can Tabla.

**Bus Drums:** Crushed Drum Bus, Parallel Dirt, Lo-Fi Loop Glue.

**Master:** Master Tape Hint, Old Console Preview.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\GLITCH\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/GLITCH/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Dusty vocal chops.** **Dusty Vocal Chop** uses 9 bits, 22 kHz and TPDF dither at 70% mix.
- **Radio and telephone.** **Radio Crush** and **Telephone Ad-Lib** turn the speaker on with a small size.
- **Gritty 808s.** **Dirty Sub 12bit** and **808 Grit** keep the sub but add texture. Check the low end after crushing.
- **Parallel dirt.** Keep MIX at 25% to 50% on a drum bus (**Parallel Dirt**, **Crushed Drum Bus**).
- **Tape feel.** **Master Tape Hint** uses 15 bits at 15% mix, a very light touch.
- **Extreme.** **Old Console Preview** uses 4 bits and 8 kHz.

## FAQ

**Does it add latency?** No. The plugin reports 0 samples.

**Why do I only see three knobs?** GLITCH starts in SIMPLE mode. Click SELECT to see all six.

**What does dither do?** It adds a little noise before the quantizer, so quiet details are less distorted. RPDF is a flat noise, TPDF is a triangular noise that fully decorrelates the error.

**Why does it sound worse when I lower the rate?** That is the point: the sample-rate reducer creates aliasing on purpose.
