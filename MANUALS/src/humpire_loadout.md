# HUMPIRE LOADOUT

Eight studio effects in one payment: EQ, compressor, enhancer, limiter, reverb, delay, lo-fi and performance effects.

| | |
|---|---|
| Type | Audio effects bundle |
| Format | VST3 64-bit (Windows); VST3 + Audio Unit, universal (macOS) |
| Brand / developer | HUMPIRE by DJBILBOX BEATS |
| Version | 1.0 |
| Plugins | 8 |

## Overview

HUMPIRE LOADOUT is a suite of eight effect plugins that share the same look, the same top bar and the same preset system. Each plugin has its own manual, included in this download. All plugins come with factory presets named for the material they are made for (trap vocals, 808, kick, oud, darbouka, drum bus, master).

| Plugin | Type | What it does |
|---|---|---|
| **HUMPIRE SKILLTREE** | Parametric EQ | Six interactive bands, high-pass and low-pass filters, Mid/Side, linear phase and a real-time analyzer. |
| **HUMPIRE CLUTCH** | Compressor | Clean, Punch and Crush modes, sidechain high-pass and parallel mix, with a LED life bar. |
| **HUMPIRE OVERCLOCK** | Enhancer | Density, harmonics and perceived loudness with a Power knob and a two-band split. |
| **HUMPIRE HARDCAP** | Master limiter | Look-ahead true-peak limiter with auto-release and LUFS meters. |
| **HUMPIRE RESPAWN** | Reverb | Eight-line feedback delay network reverb with pre-delay, damping, EQ and ducking. |
| **HUMPIRE LAG** | Stereo delay | BPM sync, filtered feedback, ducking, ping-pong and four morphing snapshots. |
| **HUMPIRE GLITCH** | Lo-fi | Bit crusher, sample-rate reducer, speaker filter and dither on a handheld-console body. |
| **HUMPIRE COMBO** | Performance FX | Stutter, tape stop, filter sweep and reverse with two XY pads and MIDI triggering. |

## Installation

Run the installer from the Windows download, or the package from the macOS download. The installer lets you choose which of the eight plugins to install. On Windows, the plugins are copied to `C:\Program Files\Common Files\VST3` as `HUMPIRE SKILLTREE.vst3`, `HUMPIRE CLUTCH.vst3` and so on. On macOS, copy or install the VST3 to `/Library/Audio/Plug-Ins/VST3` and the Audio Unit to `/Library/Audio/Plug-Ins/Components`. The README in each download explains the one-time security prompt (Windows SmartScreen, macOS Gatekeeper): the installers are not code-signed. Then restart your host and rescan plugins. In FL Studio, open **Options > Manage plugins** and click **Find installed plugins**; the plugins are listed under the vendor **HUMPIRE by Djbilbox BEATS**.

All eight are effects: add them to a **Mixer** insert slot, not to the Channel rack.

## What the plugins have in common

- **Top bar:** preset selection with **<** and **>**, **SAVE**, **A/B**, **UNDO**, **REDO**, **BYPASS** and **IN / OUT** meters.
- **SIMPLE and CLEAN:** SIMPLE hides the advanced knobs. CLEAN replaces the textured body with a flat panel.
- **Resizable window:** 820 x 560 pixels, resizable from 50% to 200% with a fixed aspect ratio.
- **Presets:** factory presets by category, and your own presets in `%APPDATA%\HUMPIRE\<PLUGIN>\Presets` (Windows) or `~/Library/Application Support/HUMPIRE/<PLUGIN>/Presets` (macOS).
- **Automation:** every control can be automated in your host. In FL Studio, use **Tools > Last tweaked > Create automation clip**.
- **Stereo or mono:** all plugins accept mono or stereo tracks.

## Suggested chains

- **Vocal:** SKILLTREE (clean the lows) then CLUTCH (even out the level) then RESPAWN or LAG on a send.
- **808:** OVERCLOCK (add harmonics) then CLUTCH or HARDCAP in a safe preset such as **808 Guard**.
- **Drum bus:** CLUTCH (**Bus Drums Glue**) then OVERCLOCK (**Drum Bus Power**) then GLITCH at low mix for dirt.
- **Master:** SKILLTREE (**Master Wide Air**) then HARDCAP as the very last plugin.
- **Live moves:** COMBO on a bus, triggered from your MIDI keyboard.

## Latency summary

| Plugin | Latency |
|---|---|
| SKILLTREE | 0 samples in Minimum phase, 2048 samples in Linear phase |
| CLUTCH | 0 samples |
| OVERCLOCK | Small, from the 2x oversampling filters (reported to the host) |
| HARDCAP | About half of the look-ahead time (reported to the host) |
| RESPAWN | 0 samples |
| LAG | 0 samples |
| GLITCH | 0 samples |
| COMBO | 0 samples |

## FAQ

**Can I buy the plugins separately?** Yes. Each plugin is also sold on its own.

**Do the plugins work as instruments?** No. They are effects. COMBO can be triggered by MIDI notes but processes audio.

**Where are the individual manuals?** Each plugin has its own PDF manual in the download. The complete manual contains all of them.
