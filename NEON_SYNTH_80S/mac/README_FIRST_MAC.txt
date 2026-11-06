NEON SYNTH 80s 2.0 - HUMPIRE by Djbilbox BEATS (macOS)
==============================================

INSTALL (macOS 10.15 or later, Intel and Apple Silicon)
1. Right-click NEON_SYNTH_80s_2_Mac.pkg and choose "Open", then "Open" again.
   (Double-clicking may show "cannot be opened because the developer cannot be verified" - this is Apple's
   standard notice for software that is not distributed through the App Store. Right-click > Open is safe.)
   If it still refuses: System Settings > Privacy & Security > scroll down > "Open Anyway".
2. Follow the installer. It installs:
   - NEON SYNTH 80s 2.0.component (Audio Unit)  ->  /Library/Audio/Plug-Ins/Components
   - NEON SYNTH 80s 2.0.vst3                    ->  /Library/Audio/Plug-Ins/VST3
   - NEON SYNTH 80s 2.0.app (standalone)        ->  /Applications
3. Restart your DAW and rescan plug-ins. In Logic Pro: Plug-In Manager > Reset & Rescan Selection.

If a plug-in is blocked on first load, run this once in Terminal:
   sudo xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/Components/NEON SYNTH 80s 2.0.component" "/Library/Audio/Plug-Ins/VST3/NEON SYNTH 80s 2.0.vst3" "/Applications/NEON SYNTH 80s 2.0.app"

UNINSTALL: delete the three items listed above.

Presets: 70 factory presets in 6 banks (Leads, Basses, Pads, Keys & Brass, Plucks, FX). User presets are saved in Documents/DJBILBOX BEATS/NEON SYNTH 80s/Presets.
