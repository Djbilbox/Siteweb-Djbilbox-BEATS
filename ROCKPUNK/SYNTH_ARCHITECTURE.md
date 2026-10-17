# ROCKPUNK GUITAR — Architecture audio (SYNTH_ARCHITECTURE.md)

> L'ancienne architecture synthé est archivée dans `Source/Dsp/_archive_synth_engine/`
> et `MODULATION_MATRIX.md` (obsolète). Ce document décrit le moteur **guitare hybride**.

## 1. Vue d'ensemble

```
MIDI (notes) ─▶ Synthesiser (8 voix) ─▶ LAYER A (KS + résonateur) ─┐
                                    └▶ LAYER B (KS + résonateur) ─┴─▶ mix layer (crossfade)
                                                                      │
                                                         AMP « URBAN SLAYER »
                                                         (pre-gain → tone → tanh asym. → level)
                                                                      │
                                                         DRIVE (4 types, ROCKPUNK)
                                                                      │
                                                         DELAY (sync/div/fb/LP, ROCKPUNK)
                                                                      │
                                                         WIDTH (mid/side) → VOLUME → LIMITEUR tanh
                                                                      │
                                                          RMS (pour VU mètre de l'interface)
```

## 2. Voix (GuitarVoice / GuitarString)

- 8 voix polyphoniques ; une voix = **deux cordes Karplus-Strong** (A et B) + un résonateur de caisse
  (bandpass) par corde. Chaque corde a sa propre fréquence (accordage uniforme ici).
- `layerMix` : crossfade égal-puissance entre couche A et couche B.
- Couche A = référence rythmique/clean (sustain court, plectre doux). Couche B = lead/hot
  (sustain long, plectre agressif). Chaque couche a Tone/Body/Sustain/Pick/Level/On.
- **Pan equal-power par note** (`note % 12`) → la même note retombe toujours au même endroit
  du spectre stéréo (choix délibéré, simple et musical).
- `pluck()` excite la corde avec un front d'impulsion filtré (brightness = « Pick ») ;
  le décroissement est contrôlé par « Sustain » ; « Body » règle le poids du résonateur.
- `stopNote` avec `allowTailOff` = true → queue de libération naturelle de la corde.

## 3. Ampli « Urban Slayer »

- `ampGain` (pré-gain) → one-pole LP `ampTone` → saturation **tanh asymétrique**
  (pair + impair : 3e et 2e harmonique) réglée par `ampDist` → `ampLevel`.
- `ampOn` = false → la couche mixée passe en direct (bypass propre).

## 4. Chaîne master

- **Drive** : waveshaper post-ampli (Soft/Hard/Asym/Fuzz) + tone LP (cf. `Drive.h`).
- **Delay** : ping-pong stéréo, temps ms ou synchro BPM (`delaySync` + `delayDiv`),
  feedback, filtre LP dans la boucle (`Delay.h`).
- **Width** : décodage mid/side → gain du canal side.
- **Volume** (lissé) → **limiteur tanh** final (`outLimiter`) → `RMS` mesuré pour l'interface.

## 5. Note / interface

- `PluginProcessor` tient un `juce::MidiKeyboardState keyboardState` + un `std::set<int> activeNotes`.
- `processBlock` : `keyboardState.processNextMidiBuffer(midi, 0, numSamples, true)` puis
  `trackKeyboardNotes()` → les notes sont suivies pour l'**animation néon** de l'interface.
- Le piano cliquable de l'éditeur réinjecte des noteOn/noteOff dans le même `keyboardState`.

## 6. Fichiers

| Fichier | Rôle |
|---|---|
| `Params.h` | Définition des 35 paramètres + helpers (`midiToFreq`, `dbToGain`, `divisionToMs`) |
| `Dsp/GuitarString.h` | Corde Karplus-Strong + résonateur de caisse |
| `Dsp/GuitarVoice.h` | Voix double-couche + `GuitarSound` |
| `Dsp/Amp.h` | Ampli Urban Slayer |
| `Dsp/Drive.h`, `Dsp/Delay.h` | FX signature ROCKPUNK |
| `PluginProcessor.h/.cpp` | Synthesiser + chaîne master + état MIDI pour l'UI |