# HUMPIRE LAG

Stereo delay with BPM sync, filtered feedback, ducking, ping-pong mode and four snapshots you can morph between.

| | |
|---|---|
| Type | Audio effect (stereo delay) |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Factory presets | 12 |
| Delay time | 1 ms to 2 s per side |
| Reported tail | 30 seconds |
| Latency | 0 samples |

## Overview

HUMPIRE LAG is a stereo delay with separate left and right times. Each side can run in milliseconds (1 ms to 2 s) or be **synced** to the host tempo with eight divisions: 1/16, 1/8, dotted 1/8, triplet 1/8, 1/4, dotted 1/4, 1/2 and 1/1. The feedback (up to 95%) passes through a **low cut** and a **high cut** filter, and a soft saturator in the feedback path keeps long repeats under control. **DUCK** lowers the echoes while the input signal is loud. **PING-PONG** feeds the input into the left delay only and sends the repeats back and forth between the two sides.

Four **snapshots** (slots A to D) store the time, feedback, filter, ducking and mix settings. Picking a slot makes the delay **morph** smoothly to those settings over a time you set with the **MORPH** knob.

## Quick start in FL Studio

1. Open the **Mixer**, select a track and click an empty slot in its effects chain. Choose **HUMPIRE LAG** (vendor **HUMPIRE by Djbilbox BEATS**). If it is not listed, open **Options > Manage plugins**, check that `C:\Program Files\Common Files\VST3` is in the search paths and click **Find installed plugins**.
2. Click the **preset name** in the top bar and pick a preset, or step through them with **<** and **>**.
3. Click **SYNC** to follow the host tempo, then use the two division buttons to choose the left and right note values. Click a division button to go to the next value.
4. Set **FEEDBACK** for the number of repeats, and **MIX** for the level of the echoes.
5. Use **DUCK** so the echoes appear in the gaps between words or notes.
6. To store a snapshot: click **STORE**, then click a slot (A to D). To recall it, click the slot.

## Interface

**Top bar (all HUMPIRE LOADOUT plugins).** From left to right: the HUMPIRE logo and the plugin name, the **<** and **>** buttons (previous and next preset), the preset name (click it to open the preset menu, sorted by category), **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS**, and the **IN** and **OUT** level meters. At the right end of the bar, the **SIMPLE** and **CLEAN** buttons change the display.

- **SAVE** asks for a name and stores the current settings as a user preset. User presets are listed in the **User** category of the preset menu.
- **A/B** switches between two settings: the first click stores the current settings in slot A and moves to slot B, and each following click swaps between the two.
- **UNDO** and **REDO** step through your recent control changes and preset loads.
- **BYPASS** passes the audio through untouched. The host also sees it as the plugin's bypass parameter.
- **SIMPLE** hides the advanced knobs and keeps the essential ones. **CLEAN** replaces the textured body with a flat, plain panel. These two choices are saved with your project and are not automatable.

The window opens at 820 x 560 pixels. Drag its bottom-right corner to resize it from 50% to 200%; the proportions stay fixed.

**Mouse gestures.** Drag a knob up or down. Shift + drag for fine control. Right-click a knob to type a value. Double-click to reset to the default (when supported by the host). Hover over a control to see its value.

**Screen.** Two lanes, **L** (amber) and **R** (violet). Each lane shows the source and its repeats as vertical bars that fade according to the feedback, and a moving light. The screen also shows the time of each side in milliseconds.

**Knobs.** **TIME L**, **TIME R**, **FEEDBACK**, **LOW CUT**, **HIGH CUT**, **DUCK**, **MIX** and **MORPH**. In SIMPLE mode, **LOW CUT**, **HIGH CUT**, **DUCK** and **MORPH** are hidden.

**Buttons.** **SYNC**, **PING-PONG**, the left division button (shows its current value, default 1/4) and the right division button (default 1/4 D). When SYNC is off, the time knobs set the delay; when SYNC is on, the division buttons do.

**Snapshot row (bottom).** **SLOT A**, **SLOT B**, **SLOT C**, **SLOT D** and **STORE**. A LED shows the active slot.

## Parameters

| Control | Host name | Range | Default | Function |
|---|---|---|---|---|
| TIME L | Time L | 1 to 2000 ms | 375 ms | Left delay time when SYNC is off. |
| TIME R | Time R | 1 to 2000 ms | 375 ms | Right delay time when SYNC is off. |
| SYNC | Sync | Off / On | Off | Follows the host tempo. Without a tempo from the host, 120 BPM is assumed. |
| Left division button | Division L | 1/16, 1/8, 1/8 D, 1/8 T, 1/4, 1/4 D, 1/2, 1/1 | 1/4 | Left note value when SYNC is on. |
| Right division button | Division R | Same | 1/4 D | Right note value when SYNC is on. |
| FEEDBACK | Feedback | 0 to 0.95 | 0.35 | Amount of signal sent back into the delay. |
| LOW CUT | Low Cut | 20 Hz to 1 kHz | 100 Hz | High-pass filter inside the feedback loop. |
| HIGH CUT | High Cut | 1 kHz to 20 kHz | 8 kHz | Low-pass filter inside the feedback loop. |
| DUCK | Ducking | 0 to 1 | 0.00 | Lowers the echoes while the input is loud. |
| MIX | Mix | 0 to 1 | 0.30 | Level of the echoes added to the dry signal. |
| PING-PONG | Ping-Pong | Off / On | Off | Sends the repeats back and forth between left and right. |
| MORPH | Morph | 20 ms to 3 s | 500 ms | Time taken to move to a snapshot. |
| BYPASS | Bypass | Off / On | Off | Passes the audio through. |

LAG has 13 automatable parameters. The echoes are added to the dry signal, which always passes through unchanged. The delay time changes smoothly, like a tape delay.

**Snapshots.** A snapshot stores seven values: Time L, Time R, Feedback, Low Cut, High Cut, Duck and Mix. It does not store Sync, the divisions or Ping-Pong. The four slots start with these values (Time L / Time R in ms, Feedback, Low Cut in Hz, High Cut in Hz, Duck, Mix):

| Slot | Time L | Time R | Feedback | Low Cut | High Cut | Duck | Mix |
|---|---|---|---|---|---|---|---|
| A | 375 | 375 | 0.35 | 100 | 8000 | 0.00 | 0.30 |
| B | 250 | 375 | 0.50 | 200 | 6000 | 0.30 | 0.35 |
| C | 90 | 120 | 0.20 | 150 | 10000 | 0.00 | 0.25 |
| D | 800 | 1200 | 0.70 | 300 | 4000 | 0.50 | 0.45 |

Snapshots are saved with your project. While the delay morphs, the knobs move by themselves.

## Factory presets

LAG includes 12 factory presets.

**Trap Vocals:** Trap Vocal 1/8, Ad-Lib Ping-Pong, Dotted Hook.

**808:** 808 Slap.

**Kick:** Kick Echo Tight.

**Oud:** Oud Wide Echo, Oud Desert.

**Darbouka:** Darbouka Roll.

**Bus Drums:** Bus Drums Space.

**Master:** Master Haze, Dub Throw, Infinite Ghost.

**User presets.** SAVE writes an `.xml` file in `%APPDATA%\HUMPIRE\LAG\Presets` on Windows (the `AppData\Roaming` folder of your user account) and in `~/Library/Application Support/HUMPIRE/LAG/Presets` on macOS. Files in this folder appear in the **User** category. Loading a factory preset first resets every parameter to its default, then applies the preset.

## Production tips

- **Hooks and vocals.** **Trap Vocal 1/8** and **Dotted Hook** use a synced delay with ducking so the words stay clear.
- **Wide echoes.** Use different times on the left and right (**Oud Wide Echo**) or enable ping-pong (**Ad-Lib Ping-Pong**).
- **Slapback.** **808 Slap** uses very short times with a low feedback.
- **Dub throws.** Raise FEEDBACK to 0.75 (**Dub Throw**) and automate MIX or the snapshot slots.
- **Transitions.** Store a dry snapshot in slot A and an extreme one in slot D, then switch to D at the end of a bar with a long MORPH.
- **Keep the low end clean.** Raise LOW CUT so that repeats of a bass do not muddy the mix.

## FAQ

**The delay does not follow my tempo.** Click SYNC. If your host does not send a tempo, the plugin uses 120 BPM.

**What do the division values mean?** D is dotted (1.5 times longer), T is triplet (two thirds as long).

**How do I store a snapshot?** Click STORE (it lights up), then click the slot. The current settings are stored in that slot.

**Does it add latency?** No.
