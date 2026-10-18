# HUMPIRE RESPAWN

Algorithmic reverb with an eight-line feedback delay network, pre-delay, damping, low-cut and high-cut filters and ducking.

| | |
|---|---|
| Type | Audio effect (reverb) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 12 |
| Decay | 0.2 to 20 s |
| Reported tail | 21 seconds |
| Latency | 0 samples |

## Overview

HUMPIRE RESPAWN is an algorithmic reverb built on an **eight-line feedback delay network** with a Householder feedback matrix. The signal first goes through a **pre-delay**, then through six **early reflections**, and then feeds the eight delay lines that create the tail. The **DECAY** knob sets the time for the tail to fall by 60 dB (0.2 to 20 seconds), **SIZE** scales the length of the delay lines, **DAMP** darkens the tail as it decays, and **MOD** adds a slow modulation for a smoother tail.

A **low cut** and a **high cut** filter shape the reverb itself, and **DUCK** lowers the reverb while the input signal is playing, so a vocal stays clear and the reverb blooms in the gaps. The **MIX** knob sets the dry and wet balance.

The screen shows rings that spread from a speaker cone: the number and speed of rings follow the decay time, and the cone pulses with the level of the reverb.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE RESPAWN** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Start from a preset in the **Plate**, **Hall**, **Hammam** or **Cathedral** category.
4. Adjust **DECAY** and **SIZE** to the length and size of the space.
5. Add **PRE-DLY** (10 to 40 ms) to keep the source in front of the reverb.
6. Use **MIX** to place the sound. On a send track, set MIX to 100%.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Knobs.** **DECAY**, **SIZE**, **PRE-DLY**, **DAMP**, **EARLY**, **MOD**, **LOW CUT**, **HIGH CUT**, **DUCK** and **MIX**. In SIMPLE mode, only **DECAY**, **SIZE**, **PRE-DLY**, **DAMP** and **MIX** remain visible.

## Parameters

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| DECAY | Decay | 0.2 to 20 s | 2.0 s | Time for the tail to fall by 60 dB. |
| SIZE | Size | 0.5 to 2 | 1.00 | Scales the delay lines and the early reflections. Larger is a bigger space. |
| PRE-DLY | Pre-Delay | 0 to 200 ms | 15 ms | Delay before the reverb starts. |
| DAMP | Damping | 0 to 1 | 0.40 | High-frequency absorption of the tail (cutoff from 18 kHz down to 1.5 kHz). |
| EARLY | Early | 0 to 1 | 0.50 | Level of the early reflections. |
| MOD | Modulation | 0 to 1 | 0.30 | Depth of the slow modulation of the delay lines. |
| LOW CUT | Low Cut | 20 Hz to 1 kHz | 80 Hz | High-pass filter on the reverb signal. |
| HIGH CUT | High Cut | 2 kHz to 20 kHz | 12 kHz | Low-pass filter on the reverb signal. |
| DUCK | Ducking | 0 to 1 | 0.00 | Amount by which the reverb is lowered while the input is loud. |
| MIX | Mix | 0 to 1 | 0.30 | Dry/wet balance. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

RESPAWN has 11 automatable parameters. The plugin reports a 21-second tail to the host, so reverb tails are not cut when you render.

## Factory presets

RESPAWN includes 12 factory presets.

**Plate:** Plate Vocal, Plate Snare.

**Hall:** Hall Medium, Hall Large.

**Hammam:** Hammam Steam, Hammam Chant.

**Cathedral:** Cathedral Choir, Cathedral Abyss.

**Trap Vocals:** Trap Vocal Space.

**808:** 808 Room.

**Oud:** Oud Chamber.

**Darbouka:** Darbouka Courtyard.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\RESPAWN\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/RESPAWN/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Vocals that stay clear.** **Trap Vocal Space** combines a medium decay with strong ducking (0.7).
- **Small rooms for 808s and drums.** **808 Room** keeps the decay at 0.6 s with a high low cut (300 Hz) so the low end stays tight.
- **Oriental spaces.** **Hammam Steam** and **Hammam Chant** use heavy damping for a warm, steamy tail.
- **Huge spaces.** **Cathedral Abyss** reaches a 15-second decay: use it on a send and automate MIX.
- **CPU.** The reverb is lightweight, but very long decays keep the tail running: pause the track when you are not using it.

## FAQ

**Is there latency?** No.

**Why is nothing happening at MIX 0?** MIX at 0 means 100% dry. Raise it.

**How do I use it on a send?** Set MIX to 100% and send the amount from each track.
