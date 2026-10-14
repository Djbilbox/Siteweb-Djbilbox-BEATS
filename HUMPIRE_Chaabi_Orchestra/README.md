# HUMPIRE CHAABI ORCHESTRA — VST3 (JUCE 8 / C++20)

Sampler d'instruments chaâbi (8 instruments, SFZ). Moteur sampler **maison** (aucune dépendance hors JUCE).

## Arborescence
```
HUMPIRE_Chaabi_Orchestra/
├─ CMakeLists.txt                  JUCE 8.0.4 (FetchContent ou -DHUMPIRE_JUCE_DIR=...)
├─ Source/
│  ├─ HumpireCore/                 module partagé (cible INTERFACE)
│  │   HumpireTheme.h  HumpireLookAndFeel.*  HumpireKnob.h  HumpireMeter.h
│  │   HumpireZellige.h  HumpirePresetManager.*  (presets + A/B)
│  └─ Plugin/
│      PluginProcessor.*  PluginEditor.*  Params.h (layout APVTS)  Instruments.h
│      SfzParser.*  SampleBank.*  ChaabiSynth.*  (voix, RR, keyswitch, release-trigger, off_by)
│      PhraseBank.*  PhrasePlayer.*  (phrases MIDI + séquenceur 16 pas)
│      FactoryPresets.h  InstrumentArt.h
├─ Samples/{Mandole,Derbouka}/*.sfz   exemples complets (WAV non versionnés)
├─ Tests/SmokeTest.cpp             test headless (option HUMPIRE_BUILD_TESTS)
└─ tools/make_placeholders.py      génère SFZ + samples de test synthétisés
```

## Build rapide
```
python tools/make_placeholders.py --out Samples        # samples de test (optionnel)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --target HumpireChaabiOrchestra_VST3 HumpireChaabiOrchestra_Standalone
```
Samples : `C:\ProgramData\HUMPIRE\Chaabi Orchestra\Samples\<Instrument>\<Instrument>.sfz`
(ou variable d'env `HUMPIRE_SAMPLES`, ou bouton « DOSSIER SAMPLES » dans l'UI).

## Dossiers de samples
```
Samples/
├─ Mandole/   Mandole.sfz  samples/pick/*.wav  samples/tremolo/*.wav
├─ Banjo/     Banjo.sfz     ├─ Violon/  Violon.sfz   ├─ Ney/  Ney.sfz   ├─ Qanun/  Qanun.sfz
├─ Derbouka/  Derbouka.sfz  samples/{dum,tek,ka,slap,ghost}/*.wav
├─ Tar/       Tar.sfz       └─ Bendir/  Bendir.sfz
```
Zones clavier : **12-23** keyswitch (`sw_last`), **24-35** déclencheurs de phrases, **36+** jeu.
Derbouka : 36-37 DUM · 38-39 TEK · 40 KA · 41-42 SLAP · 43-44 GHOST (one_shot, sans pitch).
Tar (séquenceur) : 36 DUM · 38 TEK · 40 KA · 42 ZIL · 45 GHOST.
Opcodes SFZ : voir en-tête de `Source/Plugin/SfzParser.h`.

## État
Livrables 1-4 faits. Reste : Hub + licence Ed25519, installateur Inno Setup, prompts images, pluginval/FL, products.json.
