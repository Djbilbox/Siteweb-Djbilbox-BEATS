# HUMPIRE OVERCLOCK

Enhancer that adds density, harmonics and perceived loudness with a Power knob and a two-band split.

| | |
|---|---|
| Type | Audio effect (enhancer) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 12 |
| Oversampling | 2x |
| Latency | Reported to the host (the oversampling filters) |

## Overview

HUMPIRE OVERCLOCK splits the signal into a **low band** and a **high band** at an adjustable frequency (80 Hz to 4 kHz), using a Linkwitz-Riley 4th-order crossover. Each band goes through a saturator running at 2x oversampling to limit aliasing. The low band receives **density**, the high band receives generated **harmonics**.

**POWER** sets the amount of drive of the low band. **CURVE** morphs the saturation curve from soft and asymmetric (rich in even harmonics) to a hard clip. **DENSITY** sets how much of the saturated low band replaces the dry one. **HARMONICS** sets the drive and the amount of the high band. The output gain is automatically compensated as POWER rises, and a soft limit at the end of the chain keeps peaks under control.

On the screen, a ring and a spinning fan follow the Power setting: the ring goes from amber to red and the fan accelerates as POWER rises.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE OVERCLOCK** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Raise **POWER** until the sound feels denser and louder.
4. Use **DENSITY** to blend the effect with the original signal, and **CURVE** to choose between a smooth and an aggressive character.
5. Open **HARMONICS** and **SPLIT** (hidden in SIMPLE mode) to decide which part of the spectrum is enriched.
6. Compare with the **BYPASS** button and adjust **OUTPUT** so the level matches.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Screen.** On the left, the power ring with the value as a percentage (**POWER %**). On the right, the animated fan.

**Knobs.** **POWER**, **DENSITY**, **CURVE**, **HARMONICS**, **SPLIT** and **OUTPUT**. In SIMPLE mode, **HARMONICS** and **SPLIT** are hidden.

## Parameters

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| POWER | Power | 0 to 1 | 0.35 | Drive of the low band. The output is compensated (divided by 1 + 1.5 x Power). |
| DENSITY | Density | 0 to 1 | 0.80 | Amount of saturated low band mixed in. |
| CURVE | Curve | 0 to 1 | 0.30 | 0 = soft, asymmetric curve. 1 = hard clip. |
| HARMONICS | Harmonics | 0 to 1 | 0.30 | Drive and amount of the generated harmonics in the high band. |
| SPLIT | Split | 80 Hz to 4 kHz | 400 Hz | Crossover frequency between the low and high bands. |
| OUTPUT | Output | -18 to +12 dB | 0 dB | Output gain after the effect. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

OVERCLOCK has 7 automatable parameters. Peaks above 0.9 are bent by a soft limiter. A DC blocker removes any offset created by the asymmetric curve.

## Factory presets

OVERCLOCK includes 12 factory presets.

**Trap Vocals:** Vocal Presence, Ad-Lib Heat.

**808:** 808 Harmonics, Sub Thicken.

**Kick:** Kick Weight, Kick Click.

**Oud:** Oud Warmth.

**Darbouka:** Darbouka Bite.

**Bus Drums:** Drum Bus Power.

**Master:** Loud Master, Overclocked!, Air Only.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\OVERCLOCK\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/OVERCLOCK/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **808s.** **808 Harmonics** and **Sub Thicken** use a low split (120 to 150 Hz) so the sub gains harmonics that carry on small speakers.
- **Vocals.** **Vocal Presence** uses a split near 800 Hz and a medium amount of harmonics for presence without harshness.
- **Drum bus.** **Drum Bus Power** adds weight at a split around 300 Hz.
- **Air.** **Air Only** keeps the low band almost untouched and lifts the highs from 3 kHz.
- **Extreme.** **Overclocked!** pushes every control: lower OUTPUT to stay in control.

## FAQ

**Does it add latency?** A small one, from the oversampling filters. The plugin reports it to your host.

**Why is the level lower when I raise POWER?** The output is compensated so that more drive does not simply mean louder. Use OUTPUT to adjust.

**Why do I not see HARMONICS and SPLIT?** They are hidden in SIMPLE mode.
