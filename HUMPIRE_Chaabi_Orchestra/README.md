# HUMPIRE CHAABI ORCHESTRA - VST3 (JUCE 8 / C++20)

Chaabi instrument sampler (8 instruments, SFZ). **In-house sampler engine** (no dependency beyond JUCE).

The plug-in plays out of the box: when no SFZ sample library is found it renders its **built-in modelled
instruments** (physical-modelling / modal / additive synthesis, computed in the background when an instrument
is selected). These are *modelled* instruments, not recordings of real ones. A user SFZ + WAV library, when
present, always takes precedence.

## Layout
```
HUMPIRE_Chaabi_Orchestra/
+- CMakeLists.txt                  JUCE 8.0.4 (FetchContent or -DHUMPIRE_JUCE_DIR=...)
+- Source/
|  +- HumpireCore/                 shared module (INTERFACE target)
|  |   HumpireTheme.h  HumpireLookAndFeel.*  HumpireKnob.h  HumpireMeter.h
|  |   HumpireZellige.h  HumpirePresetManager.*  (presets + A/B)
|  +- Plugin/
|      PluginProcessor.*  PluginEditor.*  Params.h (APVTS layout)  Instruments.h
|      SfzParser.*  SampleBank.*  ChaabiSynth.*  (voices, RR, keyswitch, release-trigger, off_by)
|      PhraseBank.*  PhrasePlayer.*  (MIDI phrases + 16-step sequencer)
|      FactoryPresets.h  InstrumentArt.h
|      Model/                      BUILT-IN MODELLED LIBRARY (pure C++17, std only, no JUCE)
|        ModelLibrary.h            interface + contract + per-instrument key map (read this first)
|        ModelCommon.h             shared DSP helpers (RNG, filters, waveguide string, loops, ...)
|        ModelLibrary.cpp          dispatch + validate()
|        PluckedModels.cpp         mandole, banjo, qanun
|        BowedWindModels.cpp       violin, ney
|        PercussionModels.cpp      derbouka, tar, bendir
+- Samples/{Mandole,Derbouka}/*.sfz   example mappings (WAV files are not versioned)
+- Tests/SmokeTest.cpp             headless test (option HUMPIRE_BUILD_TESTS)
+- tools/
    render_library.cpp             ChaabiRenderLibrary: exports the built-in library as WAV + SFZ
    LibraryWriter.h                WAV/SFZ writer shared by the tool and the smoke test
    analyze_library.py             objective quality gate (numpy only)
    make_placeholders.py           generates simple placeholder SFZ + samples
```

## Quick build
```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release -DHUMPIRE_BUILD_TESTS=ON
cmake --build build --target HumpireChaabiOrchestra_VST3 HumpireChaabiOrchestra_Standalone HumpireSmokeTest ChaabiRenderLibrary
ctest --test-dir build          # runs the smoke test (no sample folder needed)
```
Options: `-DHUMPIRE_JUCE_DIR=<local JUCE clone>`, `-DHUMPIRE_COPY_PLUGIN=OFF`, `-DHUMPIRE_BUILD_TOOLS=OFF`.

## Sound sources
1. **User SFZ library** (optional): `C:\ProgramData\HUMPIRE\Chaabi Orchestra\Samples\<Instrument>\<Instrument>.sfz`,
   or the `HUMPIRE_SAMPLES` environment variable, or the **SAMPLES FOLDER** button in the UI.
2. **Built-in modelled instruments** (default): rendered in a background thread the first time an instrument is
   used (about 0.1-3 s per instrument at 44.1 kHz, at most 25 MB of RAM each). The UI shows *Rendering 63%*, then
   *Built-in modelled instrument*. If a user SFZ cannot be loaded (missing, empty, unreadable samples) the plug-in
   falls back to the built-in instrument and says why in the status line - it is never silent without an explanation.

Folder layout of a user library:
```
Samples/
+- Mandole/   Mandole.sfz  samples/pick/*.wav  samples/tremolo/*.wav
+- Banjo/     Banjo.sfz     +- Violon/  Violon.sfz   +- Ney/  Ney.sfz   +- Qanun/  Qanun.sfz
+- Derbouka/  Derbouka.sfz  samples/{dum,tek,ka,slap,ghost}/*.wav
+- Tar/       Tar.sfz       +- Bendir/  Bendir.sfz
```
Keyboard zones: **12-23** keyswitches (`sw_last`), **24-35** phrase triggers, **36+** playing range.
Derbouka: 36-37 DUM - 38-39 TEK - 40 KA - 41-42 SLAP - 43-44 GHOST (one_shot, no pitch tracking).
Tar (sequencer): 36 DUM - 38 TEK - 40 KA - 42 ZIL - 45 GHOST.
SFZ opcodes: see the header of `Source/Plugin/SfzParser.h`.

## Built-in library: how it works
* `Model/ModelLibrary.h` defines `model::Library` (a list of `model::Zone` = one SFZ region each, with float audio)
  and `model::build(instrumentIndex, sampleRate)`. Each instrument has an offline renderer (see the three family
  files). Renderers are deterministic, thread-safe, allocation-only-inside, and cancellable.
* `Bank::buildFromModel` (SampleBank.*) turns the zones into the same `Region` + `SampleData` objects an SFZ file
  produces, so the existing engine (velocity layers, round robins, loops, keyswitches, release triggers,
  one_shot / off_by groups) plays them unchanged. Identical banks are shared between plug-in instances.
* `model::validate()` checks the contract (coverage of the key range for every velocity / round-robin /
  articulation, loops, budgets, key map) and is run by the smoke test and by the plug-in (problems become warnings).
* The per-instrument key map and level conventions are documented at the end of `Model/ModelLibrary.h`.

## Tools
```
ChaabiRenderLibrary --out /tmp/lib --bits 24            # WAV + SFZ for every instrument (16 / 24 / 32-bit float)
python3 tools/analyze_library.py /tmp/lib               # per-zone metrics + PASS / FAIL gate
python3 tools/analyze_library.py /tmp/lib --instrument Mandole --failures-only --csv m.csv --spectrograms /tmp/specs
```
The exported SFZ files can be loaded by the plug-in as a regular user library.
Gate: peak -24..-1 dBFS, |DC| < 0.002, pitch within 8 cents, no clicks (start / end / loop seam / glitches),
loudness within +-4 dB inside each (articulation, velocity layer) group and across hard layers.

## Built-in library: size and render time (44.1 kHz, one core)
| Instrument | Zones | RAM | Render |
|---|---|---|---|
| Mandole | 70 | 18.3 MB | 1.0 s |
| Banjo | 60 | 10.4 MB | 0.5 s |
| Violin | 58 | 13.3 MB | 1.2 s |
| Ney | 34 | 13.2 MB | 1.0 s |
| Qanun | 60 | 21.3 MB | 1.2 s |
| Derbouka | 82 (57 aliases) | 1.6 MB | 0.05 s |
| Tar | 54 (27 aliases) | 2.7 MB | 0.13 s |
| Bendir | 54 (27 aliases) | 2.3 MB | 0.10 s |

Output has a soft knee above -2 dBFS so stacked voices cannot hard-clip.

## Status
The 8 built-in instruments are physically-motivated models (modal strings, bowed waveguide, additive + noise
winds, modal membranes). They pass the automated gate (pitch, level, loops, clicks) and the smoke test, but they
have not been auditioned by ear and are not recordings: expect a good synthesizer, not a sampled instrument.
Not yet verified: MSVC build, loading in a real DAW, pluginval. Remaining product work: Hub + Ed25519 licence,
Inno Setup installer, image prompts, products.json.
