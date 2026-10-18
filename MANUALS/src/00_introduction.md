# Welcome

Thank you for choosing **DJBILBOX BEATS / HUMPIRE**. This manual covers installation, licensing and the detailed use of every plugin in the collection: the DJBILBOX BEATS synthesizers and the **HUMPIRE LOADOUT** studio effects suite.

Every plugin is designed and programmed in Toulouse, France, by DJBILBOX BEATS, beatmaker and developer. All audio algorithms, interfaces and presets are original creations.

## System requirements

| | |
|---|---|
| Operating system | Windows 10 or 11, 64-bit; or macOS 11 or later (Apple Silicon and Intel) |
| Format | VST3 64-bit (Windows); VST3 and Audio Unit, universal (macOS) |
| Designed for | FL Studio. Any VST3 or Audio Unit host should load the plugins (Ableton Live, Logic Pro, Cubase, Studio One, Reaper, Bitwig...), but no human listening test and no test in every host has been done: try each plugin in your own host. |
| CPU | x64 processor (Intel Core i5 / AMD Ryzen 5 or better recommended) |
| Memory | 4 GB minimum, 8 GB recommended |
| Display | 1280 × 720 minimum; interfaces are resizable |
| Disk space | About 300 MB for the full collection |

## Installation

1. Close your music software (FL Studio or any other host).
2. Unzip the downloaded archive and run the **Setup.exe** installer it contains.
3. Accept the Windows administrator prompt. If Windows SmartScreen shows "Windows protected your PC", click **More info**, then **Run anyway**.
4. Read and accept the license agreement.
5. Choose **Full installation**, or untick the plugins you do not want.
6. Click **Install**. The plugins are copied to the standard Windows VST3 folder:
   `C:\Program Files\Common Files\VST3`
7. This manual and the license agreement are installed in `C:\Program Files\DJBILBOX BEATS\Documentation` and can be opened from the Start menu (**DJBILBOX BEATS** folder).

## Installation on macOS

1. Close your music software.
2. Open the macOS download and run the **.pkg** installer of the product (or the installers of the collection). The VST3 is installed in `/Library/Audio/Plug-Ins/VST3` and the Audio Unit in `/Library/Audio/Plug-Ins/Components`.
3. The installers are not notarized by Apple. If macOS says the package cannot be opened, open **System Settings > Privacy & Security**, scroll down and click **Open Anyway** next to the message about the installer, then confirm. The README in the download shows the same steps.
4. Restart your host and rescan plugins. In Logic Pro, the Audio Units are validated automatically. In FL Studio for macOS, use **Options > Manage plugins** and rescan.

## First use in FL Studio

1. Open FL Studio, then **Options > Manage plugins**.
2. Make sure `C:\Program Files\Common Files\VST3` is listed among the plugin search paths (it is by default).
3. Click **Find installed plugins** (or **Start scan**). The plugins appear under the vendor **DJBILBOX BEATS** or **HUMPIRE by Djbilbox BEATS**.
4. Click the star (favorite) next to each plugin to find it quickly in the **Plugin database** and the browser.
5. **Synthesizers and sampler**: add them to the **Channel rack** (**+** button) and play them from a MIDI keyboard or the Piano roll.
6. **Effects**: open the **Mixer**, select a track, click an empty slot in the effects chain and choose the plugin.

> Tip: in FL Studio, right-click any plugin control and choose **Link to controller** to drive it from your MIDI controller, or **Create automation clip** to automate it.

## Common gestures

| Gesture | Action |
|---|---|
| Drag a knob vertically | Change the value |
| Shift + drag | Fine adjustment |
| Double-click | Reset to the default value |
| Right-click (on supported plugins) | Type a numeric value |
| Mouse wheel | Small steps |
| Bottom-right corner of the window | Resize the interface (on supported plugins) |

## Presets

Each plugin ships with factory presets organized by genre or instrument. Use the **<** and **>** arrows or the preset menu in the top bar. Your own presets are saved in your user folder (`Documents` or `AppData`, depending on the plugin, see the plugin's chapter) and are never overwritten by updates.

## Uninstalling

On Windows, open **Windows Settings > Apps > Installed apps**, find **DJBILBOX BEATS - All Plugins** and click **Uninstall**. On macOS, delete the plugin files from the two folders above. Your own presets are kept.

## License summary

The sounds and music you create with these plugins belong to you and are **royalty-free**: you may use them commercially (streaming, sales, placements, sync licensing, beat sales and leases) with no royalties and no mandatory credit. The license is personal and allows installation on two computers you own. Redistributing the plugins, their presets or their factory sounds on their own is prohibited. The full text is in **EULA_DJBILBOX_BEATS.txt**.

## Support

Before contacting support, check the FAQ section of the plugin concerned. For any question: **djbilboxbeats.com**. Please include the plugin name, your Windows version, your host software and its version, and a precise description of the issue.
