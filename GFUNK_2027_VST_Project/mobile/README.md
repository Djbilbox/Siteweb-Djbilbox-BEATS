# GFUNK 2027 Free - mobile

Synth in `www/` (AudioWorklet engine ported from the plug-in DSP, 100 Free presets from `tools/gen_mobile.py`).

* **iPhone (PWA)**: the `iphone-web` job publishes `www/` on GitHub Pages. Open the URL in Safari, Share > *Add to Home Screen*. iOS 14.5+, turn the silent switch off.
* **Android (APK)**: the `android` job builds `GFUNK_2027_Free.apk` (debug-signed). Copy it to the phone and allow "install unknown apps".
* **Local test**: `cd www && python -m http.server 8765`, then open http://localhost:8765 (Chrome / Safari).

A native App Store / TestFlight iOS build needs a paid Apple Developer account and code signing; the PWA needs neither.
Regenerate presets after editing the plug-in presets: `python tools/gen_mobile.py`.
