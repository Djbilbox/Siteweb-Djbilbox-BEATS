# HUMPIRE COMBO

Four one-shot performance effects (stutter, tape stop, filter sweep and reverse) on a game-controller panel with two XY pads and MIDI learn.

| | |
|---|---|
| Type | Audio effect (performance FX, MIDI-triggered) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 12 |
| Effects | Stutter, Tape Stop, Sweep, Reverse |
| MIDI | Notes 36 to 39 by default, MIDI learn |
| Audio buffer | 4 seconds |
| Latency | 0 samples |

## Overview

HUMPIRE COMBO is an effect you play. Four buttons trigger four one-shot effects while you hold them: **STUTTER**, **TAPE STOP**, **SWEEP** and **REVERSE**. Two XY pads (the two sticks of the controller) set how each effect behaves, and two triggers set the drive and the effect mix.

- **Stutter** repeats the last slice of audio. The length follows the host tempo and comes from the **left pad X axis**, in four steps: 0.125, 0.25, 0.5 or 1 beat.
- **Tape stop** slows the audio down to a halt, like a turntable or tape stopping. Its duration comes from the **right pad X axis**, from 0.1 to 2 seconds.
- **Sweep** closes a resonant low-pass filter from 18 kHz down to a target frequency from the **left pad Y axis** (40 Hz to 18 kHz).
- **Reverse** plays the most recent audio backwards. The length follows the host tempo and comes from the **right pad Y axis**: 0.25, 0.5, 1 or 2 beats.

The four effects can also be triggered from **MIDI notes**. By default, notes 36, 37, 38 and 39 trigger Stutter, Tape Stop, Sweep and Reverse. **MIDI LEARN** lets you choose other notes.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE COMBO** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Make sure the plugin receives MIDI if you want to play it from a keyboard (in FL Studio, open the plugin wrapper settings and select a MIDI input port for your controller). Mouse use needs no MIDI.
4. Press and hold **STUTTER**, **TAPE STOP**, **SWEEP** or **REVERSE** with the mouse. The effect runs while you hold the button.
5. Move the XY pads to change the rate, the filter, the tape time and the reverse length.
6. Raise the **L TRIG** slider for drive and lower the **R TRIG** slider for a lower effect mix.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display. COMBO has no advanced knobs, so SIMPLE has no visible effect.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Display (top center).** The four effect names with a dot for each active effect, the current stutter length in milliseconds, the sweep target frequency in Hz, and, at the bottom, either `NOTES 36-39` (the current MIDI note range) or `LEARN: PRESS FX THEN NOTE` while MIDI learn is on.

**Buttons (under the display).** **STUTTER**, **TAPE STOP**, **SWEEP** and **REVERSE** are active while the button is held down. **MIDI LEARN** is an on/off switch.

**Left stick.** **X: STUTTER RATE**, **Y: FILTER** (sweep target).

**Right stick.** **X: TAPE TIME**, **Y: REVERSE LEN**.

**Triggers.** **L TRIG: DRIVE** and **R TRIG: MIX**, two vertical sliders between the sticks.

## Parameters

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| STUTTER button | FX stutter | Off / On | Off | Holds the stutter effect. |
| TAPE STOP button | FX tapestop | Off / On | Off | Holds the tape stop effect. |
| SWEEP button | FX sweep | Off / On | Off | Holds the filter sweep. |
| REVERSE button | FX reverse | Off / On | Off | Holds the reverse effect. |
| Left stick X | Stick L X (Stutter Rate) | 0 to 1 | 0.50 | Stutter length in four steps: below 0.25 = 0.125 beat, 0.25 to 0.5 = 0.25 beat, 0.5 to 0.75 = 0.5 beat, above 0.75 = 1 beat. |
| Left stick Y | Stick L Y (Filter) | 0 to 1 | 0.50 | Sweep target cutoff, from 40 Hz (0) to 18 kHz (1), on a logarithmic scale. |
| Right stick X | Stick R X (Tape Time) | 0 to 1 | 0.50 | Tape stop time, from 0.1 s (0) to 2 s (1). |
| Right stick Y | Stick R Y (Reverse Len) | 0 to 1 | 0.50 | Reverse length: 1/4, 1/2, 1 or 2 beats in four steps. |
| L TRIG | Trigger L Drive | 0 to 1 | 0.00 | Saturation applied to the effect signal (up to 10x gain, level compensated). |
| R TRIG | Trigger R Mix | 0 to 1 | 1.00 | Amount of the effect blended with the dry signal. |
| MIDI LEARN | MIDI Learn | Off / On | Off | Not automatable. Assigns an effect to a MIDI note. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

COMBO has 13 parameters; MIDI Learn and the interface switches are not automatable. The MIDI note assignments are saved with your project.

**MIDI mapping.** The plugin accepts MIDI notes. By default: note 36 = Stutter, 37 = Tape Stop, 38 = Sweep, 39 = Reverse. The effect runs while the note is held. To remap: click **MIDI LEARN**, hold one of the four effect buttons (the display shows `LEARN: PRESS FX THEN NOTE`), then play the note you want. Learning stops after one note.

## Factory presets

COMBO includes 12 factory presets. They set the sticks and the triggers; they do not turn effects on by themselves.

**Trap Vocals:** Trap Stutter 1/16, Ad-Lib Throw.

**808:** 808 Tape Dive, 808 Slow Stop.

**Kick:** Kick Roll 1/32, Kick Reverse.

**Oud:** Oud Reverse Swell, Oud Filter Close.

**Darbouka:** Darbouka Stutter.

**Bus Drums:** Bus Drums Freeze, Bus Filter Drop.

**Master:** Master Tape Stop.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\COMBO\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/COMBO/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Fills and transitions.** Hold **STUTTER** for the last beat of a bar. Move the left stick left for faster stutters.
- **808 dives.** **808 Tape Dive** and **808 Slow Stop** use long tape-stop times.
- **Reverse swells.** **Oud Reverse Swell** uses the longest reverse length.
- **Filter drops.** **Bus Filter Drop** and **Oud Filter Close** lower the sweep target.
- **Automate the buttons.** In FL Studio, you can create automation clips of the four FX buttons for repeatable moves.

## FAQ

**Nothing happens when I play MIDI notes.** Check that the plugin has a MIDI input in your host (in FL Studio, open the plugin wrapper settings and select a MIDI input port). The default notes are 36 to 39.

**Do the effects follow my tempo?** The stutter and reverse lengths do. The tape stop time is in seconds. Without a tempo from the host, 120 BPM is assumed.

**Why do the effects stop when I release the button?** They are one-shot effects held by the button or the note, by design.

**How long is the buffer?** The plugin keeps the last 4 seconds of audio.
