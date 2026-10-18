# HUMPIRE HARDCAP

Look-ahead true-peak limiter with automatic release and momentary, short-term and integrated LUFS meters.

| | |
|---|---|
| Type | Audio effect (master limiter) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 12 |
| Look-ahead | 1 to 5 ms |
| Latency | About half of the look-ahead time, reported to the host |

## Overview

HUMPIRE HARDCAP is a look-ahead limiter for the end of your master chain. **GAIN** raises the input level by up to 24 dB, and the **CEILING** (from -6 to 0 dBTP) sets the maximum output level. Because the limiter looks ahead (1 to 5 ms), it lowers the gain before a peak arrives, so the attack is instantaneous.

With **TRUE PEAK** on, the plugin estimates the peaks between samples with a cubic interpolation, so the output does not exceed the ceiling after conversion. With **AUTO REL** on, the release time adapts automatically to the material (between 40 and 300 ms); with it off, the **RELEASE** knob sets it from 10 to 500 ms.

The screen shows three loudness readings based on the BS.1770 K-weighting method: **MOMENTARY** (400 ms), **SHORT-TERM** (3 seconds) and **INTEGRATED** (since the start), plus a LED strip from -30 to 0 LUFS with a target marker and a gain-reduction bar. When the integrated loudness stays within half a LU of the target for two seconds, **LEVEL UP!** is displayed.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE HARDCAP** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Put HARDCAP on the **last slot** of the Master track.
4. Set the **TARGET** to the loudness you aim for (for example -14 LUFS for streaming) and raise **GAIN** while you watch the gain reduction bar. A few dB of reduction is normal.
5. Keep **TRUE PEAK** on and **CEILING** at -1 dBTP for streaming platforms.
6. Play the whole song and read the **INTEGRATED** value. Raise or lower GAIN until it is near the target.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Screen.** Three large readings (**MOMENTARY**, **SHORT-TERM**, **INTEGRATED**) in LUFS (shown as `---` below -69 LUFS). Under them, the LED strip with the **TARGET** marker (white line). At the bottom, **GAIN REDUCTION** with a bar (full scale 12 dB) and the value in dB.

**Knobs.** **GAIN**, **CEILING**, **RELEASE**, **LOOKAHEAD**, **TARGET**. In SIMPLE mode, **LOOKAHEAD** is hidden.

**Buttons.** **AUTO REL** and **TRUE PEAK** (each is an on/off switch with a LED).

## Parameters

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| GAIN | Input Gain | 0 to 24 dB | 0 dB | Gain before the limiter. |
| CEILING | Ceiling | -6 to 0 dBTP | -1 dBTP | Maximum output level. |
| RELEASE | Release | 10 to 500 ms | 100 ms | Release time when AUTO REL is off. |
| LOOKAHEAD | Lookahead | 1 to 5 ms | 2 ms | Look-ahead time. Longer values are smoother and add more latency. |
| TARGET | Target LUFS | -24 to -6 LUFS | -14 LUFS | Loudness target shown on the meter. |
| AUTO REL | Auto Release | Off / On | On | Automatic release between 40 and 300 ms depending on the gain reduction. |
| TRUE PEAK | True Peak | Off / On | On | Detects peaks between samples. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

HARDCAP has 8 automatable parameters. The loudness meters do not change the audio.

**About the LUFS meters.** The meters follow the BS.1770 method (K-weighting and 100 ms blocks), but the integrated value uses an approximated gating. Use them as a guide, and confirm your final master with the loudness tool you normally use.

**Latency.** The limiter delays the audio by about half of the look-ahead time (1 ms at the default of 2 ms) and reports it to the host.

## Factory presets

HARDCAP includes 12 factory presets.

**Master:** Streaming -14, Club Loud -9, Afrotrap Master, Deep House Smooth, Oriental Dynamic, G-Funk Warm, Hip-Hop Punch, Safe -16 LUFS.

**Bus Drums:** Bus Drums Catch.

**808:** 808 Guard.

**Trap Vocals:** Trap Vocal Safety.

**Kick:** Kick Ceiling.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\HARDCAP\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/HARDCAP/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Streaming.** Start from **Streaming -14** with a ceiling of -1 dBTP.
- **Club loudness.** **Club Loud -9** raises the gain to 7 dB with a faster release.
- **Protect an 808.** **808 Guard** or **Kick Ceiling** on a single track can catch occasional peaks.
- **Do not over-limit.** If the gain reduction bar stays above 4 to 6 dB all the time, lower GAIN.
- **Leave headroom.** Do not put anything after HARDCAP except metering.

## FAQ

**Does it add latency?** Yes, about half of the look-ahead time, reported to the host.

**Why does the INTEGRATED reading show ---?** The signal has been too quiet (below -69 LUFS) or nothing has played yet.

**What does LEVEL UP! mean?** The integrated loudness stayed within half a LU of the target for two seconds.

**Is the meter certified?** No. It is a guide.
