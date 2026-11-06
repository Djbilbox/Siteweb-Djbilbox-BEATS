GFUNK 2027 - HUMPIRE by Djbilbox BEATS (macOS)
==============================================

INSTALL (macOS 10.15 or later, Intel and Apple Silicon)
1. Right-click GFUNK_2027_Mac.pkg and choose "Open", then "Open" again.
   (Double-clicking may show "cannot be opened because the developer cannot be verified" - this is Apple's
   standard notice for software that is not distributed through the App Store. Right-click > Open is safe.)
   If it still refuses: System Settings > Privacy & Security > scroll down > "Open Anyway".
2. Follow the installer. It installs:
   - GFUNK 2027.component (Audio Unit)  ->  /Library/Audio/Plug-Ins/Components
   - GFUNK 2027.vst3                    ->  /Library/Audio/Plug-Ins/VST3
   - GFUNK 2027.app (standalone)        ->  /Applications
3. Restart your DAW and rescan plug-ins. In Logic Pro: Plug-In Manager > Reset & Rescan Selection.

If a plug-in is blocked on first load, run this once in Terminal:
   sudo xattr -dr com.apple.quarantine "/Library/Audio/Plug-Ins/Components/GFUNK 2027.component" "/Library/Audio/Plug-Ins/VST3/GFUNK 2027.vst3" "/Applications/GFUNK 2027.app"

UNINSTALL: delete the three items listed above.

The full manual is in the Windows package (USER_GUIDE.html) and applies to the macOS version.
