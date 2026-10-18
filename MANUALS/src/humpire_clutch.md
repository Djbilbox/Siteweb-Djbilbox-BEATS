# HUMPIRE CLUTCH

Compressor with Clean, Punch and Crush modes, soft knee, sidechain high-pass, parallel mix and a LED life bar.

| | |
|---|---|
| Type | Audio effect (compressor) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Modes | Clean, Punch, Crush |
| Factory presets | 12 |
| Latency | 0 samples |

## Overview

HUMPIRE CLUTCH is a feed-forward compressor with a soft knee. A one-pole **sidechain high-pass** filter keeps low frequencies from triggering the compression, and the detector is stereo-linked (it follows the louder of the two channels). The **MIX** knob blends the dry signal with the compressed signal for parallel compression.

The **MODE** button switches between three characters. **Clean** compresses exactly as the knobs say. **Punch** triples the attack time (plus 4 ms) and shortens the release to 70%, so the start of each note passes through before the compression grabs it. **Crush** multiplies the ratio by 1.8 (up to 30:1), shortens the attack to 30% and adds saturation to the compressed signal.

The screen shows gain reduction as a **life bar** of 32 segments that drains as you compress (24 dB of reduction empties it), a numeric **GR** reading, a **HIT!** flash when a sudden reduction occurs and a history graph of the last moments.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE CLUTCH** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Play your track and lower **THRESH** until the life bar drains by 3 to 6 dB on the loud notes.
4. Choose the **RATIO**. Low values (2 to 3) are gentle. High values (8 and more) limit the signal.
5. Adjust **ATTACK** and **RELEASE**. A slower attack lets more of the transient through.
6. Use **MAKEUP** to bring the level back, and **MIX** below 100% for parallel compression.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Screen.** At the top, **VITALITY** and the gain reduction in dB (**GR**). Below, the life bar: green-amber at the right when there is little reduction, going to orange and red as reduction grows. Under the bar, **HIT!** appears when a new reduction of more than 0.6 dB hits. At the bottom, the reduction history: the line goes down the screen as the reduction increases (24 dB at the bottom).

**Knobs.** **THRESH**, **RATIO**, **ATTACK**, **RELEASE**, **KNEE**, **SC HPF**, **MAKEUP**, **MIX**. In SIMPLE mode, **KNEE** and **SC HPF** are hidden.

**Button.** **CLEAN / PUNCH / CRUSH** (each click goes to the next mode).

## Parameters

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| MODE button | Mode | Clean, Punch, Crush | Clean | Compression character. See the Overview. |
| THRESH | Threshold | -40 to 0 dB | -18 dB | Level above which compression starts. |
| RATIO | Ratio | 1:1 to 20:1 | 4:1 | Amount of compression. In Crush mode the ratio is multiplied by 1.8 (up to 30:1). |
| ATTACK | Attack | 0.1 to 100 ms | 10 ms | How fast the compressor reacts. Punch mode makes it slower (3x + 4 ms), Crush mode faster (30%). |
| RELEASE | Release | 10 to 1000 ms | 120 ms | How fast the compression lets go. Punch mode shortens it to 70%. |
| KNEE | Knee | 0 to 24 dB | 6 dB | Softness of the transition around the threshold. |
| MAKEUP | Makeup | -12 to +24 dB | 0 dB | Gain applied after compression. |
| SC HPF | SC HPF | 20 to 500 Hz | 60 Hz | High-pass filter in the detector path only. The audio itself is not filtered. |
| MIX | Mix | 0 to 1 | 1.00 | Blend between the dry signal (0) and the compressed signal (1). |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

CLUTCH has 10 automatable parameters. In Crush mode the compressed signal passes through a saturator before the mix.

## Factory presets

CLUTCH includes 12 factory presets.

**Trap Vocals:** Trap Vocal Lead, Ad-Lib Squash, Vocal Glue.

**808:** 808 Tamer, 808 Sustain.

**Kick:** Kick Punch, Kick Crush.

**Oud:** Oud Smooth.

**Darbouka:** Darbouka Snap.

**Bus Drums:** Bus Drums Glue, Parallel Smash.

**Master:** Master Gentle.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\CLUTCH\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/CLUTCH/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Vocals.** Start from **Trap Vocal Lead** or **Vocal Glue**. Keep gain reduction between 3 and 6 dB.
- **808s.** **808 Tamer** controls peaks with a high ratio and a slow release. Lower **SC HPF** (25 to 30 Hz in the presets) so the sub still triggers the detector.
- **Drum punch.** Use **Punch** mode on kicks and snares: the slower attack keeps the click.
- **Parallel smash.** Choose **Crush** mode, lower the **MIX** to 40% to 70% and bring back the level with **MAKEUP** (see **Parallel Smash** and **Ad-Lib Squash**).
- **Bus glue.** Use a ratio of 2:1 with a slow attack and a long release (see **Bus Drums Glue**).

## FAQ

**Does it add latency?** No. CLUTCH has no look-ahead.

**What does the HIT! text mean?** The gain reduction suddenly grew by more than 0.6 dB: the compressor just caught a loud hit.

**Why do I not see KNEE and SC HPF?** They are hidden in SIMPLE mode. Click SIMPLE at the top right to show them.

**Is it stereo-linked?** Yes. The detector follows the louder channel, so the stereo image does not shift.
