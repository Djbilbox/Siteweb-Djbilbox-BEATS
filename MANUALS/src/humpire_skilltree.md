# HUMPIRE SKILLTREE

Six-band equalizer drawn as a skill tree, with Mid/Side targeting, minimum or linear phase and a real-time FFT analyzer.

| | |
|---|---|
| Type | Audio effect (parametric EQ) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Bands | 6 (low shelf, 4 peaking, high shelf) plus high-pass and low-pass |
| Factory presets | 12 |
| Latency | 0 samples (Minimum phase); 2048 samples (Linear phase) |

## Overview

HUMPIRE SKILLTREE is a six-band parametric equalizer. Each band is a numbered node on a CRT-style screen: you drag the node to change its frequency and gain, turn the mouse wheel over it to change its Q, and double-click it to switch it on or off. The nodes are joined by lines in order of frequency, like a skill tree, and the amber curve shows the combined response of the equalizer.

Band 1 is a **low shelf**, band 6 is a **high shelf**, and bands 2 to 5 are **peaking** filters. A **high-pass** and a **low-pass** filter (24 dB per octave each) sit on either side. The **Target** button sets whether the equalizer processes the whole stereo signal, only the **Mid** (center) signal or only the **Side** signal. The **Phase** button chooses **Minimum** phase (no latency) or **Linear** phase (a 4096-tap FIR, with 2048 samples of latency reported to the host). A real-time **FFT analyzer** is drawn behind the curve.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE SKILLTREE** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Drag a **node** on the screen: left and right change the frequency, up and down change the gain. The label at the top of the screen shows the band, its frequency, its gain and its Q while you hover or drag.
4. Turn the **mouse wheel** over a node to change its Q. Double-click a node to switch the band on or off. A band with a gain of 0 dB, or switched off, does nothing.
5. Use the **HIGH PASS** and **LOW PASS** knobs to remove rumble or harshness, and the **OUTPUT** knob to match the level before and after.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Screen.** The grid shows frequencies from 20 Hz to 20 kHz on a logarithmic scale and gains from -18 dB to +18 dB. The six nodes are numbered 1 to 6 (from the lowest default frequency to the highest). A switched-off node is dimmed. The violet area is the spectrum of the signal after the equalizer, drawn when the analyzer is on.

**Knobs.** **HIGH PASS**, **OUTPUT** and **LOW PASS**, in this order below the screen.

**Buttons.** **STEREO / MID / SIDE** (each click goes to the next target), **MINIMUM / LINEAR** (each click switches the phase mode) and **ANALYZER** (switches the spectrum on or off). When Linear phase is on, the screen shows "LINEAR PHASE LAT 2048" in violet at its top right.

## Parameters

Signal flow: the signal is optionally split into Mid and Side, passes through the six bands and the two filters, and is then scaled by the output gain. With the Mid or Side target, only the chosen signal is processed and the other one is left untouched (delayed to match when Linear phase is on).

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| Node frequency (drag) x6 | Band 1 Freq ... Band 6 Freq | 20 Hz to 20 kHz | 60, 200, 600, 2000, 6000, 12000 Hz | Center frequency of each band (corner frequency for the two shelves). |
| Node gain (drag) x6 | Band 1 Gain ... Band 6 Gain | -18 to +18 dB | 0 dB | Boost or cut of each band. |
| Node Q (mouse wheel) x6 | Band 1 Q ... Band 6 Q | 0.2 to 10 | 0.7, 1, 1, 1, 1, 0.7 | Width of the band: a higher Q is narrower. |
| Node on/off (double-click) x6 | Band 1 On ... Band 6 On | Off / On | On | Switches a band on or off. |
| HIGH PASS | High Pass | 20 Hz to 1 kHz | 20 Hz | 24 dB per octave high-pass filter. At 20 Hz it is off. |
| LOW PASS | Low Pass | 1 kHz to 20 kHz | 20 kHz | 24 dB per octave low-pass filter. At 20 kHz it is off. |
| STEREO / MID / SIDE | Target | Stereo, Mid, Side | Stereo | Which signal the equalizer processes. On a mono track, the target is always Stereo. |
| MINIMUM / LINEAR | Phase | Minimum, Linear | Minimum | Minimum phase has no latency. Linear phase keeps the phase untouched and reports 2048 samples of latency. |
| OUTPUT | Output | -18 to +18 dB | 0 dB | Output gain after the equalizer. |
| ANALYZER | Analyzer | Off / On | On | Shows the spectrum behind the curve. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

The 6 bands use 4 parameters each, so SKILLTREE has 31 automatable parameters in total (24 band parameters, High Pass, Low Pass, Target, Phase, Output, Analyzer and Bypass).

**Linear phase and latency.** In Linear phase mode the plugin recalculates a 4096-tap FIR filter whenever you change a setting, and reports 2048 samples of latency to the host. FL Studio compensates this automatically. While the filter is being rebuilt, audio is briefly paused. For that reason, prefer Minimum phase while you are moving nodes and switch to Linear phase for the final result.

## Factory presets

SKILLTREE includes 12 factory presets in categories named after the material they are made for.

**Trap Vocals:** Trap Vocal Air, Radio Vocal.

**808:** 808 Sub Boost, 808 Clarity.

**Kick:** Kick Punch, Kick Click.

**Oud:** Oud Body, Oud Air.

**Darbouka:** Darbouka Bite.

**Bus Drums:** Bus Drums Glue.

**Master:** Master Wide Air, Master Mono Bass.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\SKILLTREE\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/SKILLTREE/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Clean the low end first.** Raise **HIGH PASS** to 25 to 40 Hz on every track except the kick and the 808 to remove inaudible rumble (see **Kick Click** for an example).
- **Vocal air.** Start from **Trap Vocal Air**: a high-pass at 90 Hz, a small cut around 250 Hz, a boost near 3.5 kHz and a high shelf at 11 kHz.
- **Mid/Side on the master.** Set the target to **Side** and lift band 6 (as in **Master Wide Air**) to open up the top end without touching the center. Set the target to **Mid** and lift band 1 (as in **Master Mono Bass**) to strengthen the bass in the center.
- **Match levels.** Use **OUTPUT** to compensate for boosts so you judge the tone, not the loudness.
- **Check with the analyzer.** Switch the analyzer on to see where the energy is, then switch it off to reduce CPU use.

## FAQ

**Does Linear phase add latency?** Yes: 2048 samples, reported to your host. Minimum phase adds none.

**The nodes are in the wrong order.** Nodes are numbered by band, not by position. Band 1 is always the low shelf and band 6 the high shelf, whatever frequency you give them.

**Can I equalize only the stereo width?** Yes. Set the target to **Side**.

**Why does the screen show no spectrum?** Check that **ANALYZER** is on and that audio is playing through the plugin.

**Where do I find the plugin on macOS?** Copy the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`, then rescan plugins in your host.
