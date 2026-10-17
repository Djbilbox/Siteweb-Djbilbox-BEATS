# ROCKPUNK GUITAR — README

**ROCKPUNK GUITAR** · guitare rock'n'roll hybride · DJBILBOX BEATS
· code `HUMP`/`RPK1` · C++17 · JUCE 8.0.4
· VST3 + application autonome (Windows), VST3 + AU + application (macOS).

Deux cordes Karplus-Strong, l'ampli « Urban Slayer », un drive à 4 caractères et
un delay synchronisable. 25 presets d'usine. Édition unique : ni essai, ni
activation, ni extension.

**Aucun enregistrement n'est livré** : tout est calculé. Une couche
supplémentaire joue les fichiers que l'utilisateur dépose dans
`Documents/DJBILBOX/ROCKPUNK/Samples`.

## Compilation

```powershell
cmake -B Build -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR=<chemin vers JUCE 8.0.4>
cmake --build Build --config Release --target ROCKPUNK_All --parallel
```

Artefacts : `Build\ROCKPUNK_artefacts\Release\VST3\ROCKPUNK.vst3` et
`...\Standalone\ROCKPUNK.exe`.

> **Chaîne** : JUCE 8 refuse MinGW (`#error "MinGW is not supported"`) — utiliser
> MSVC (VS 2022 Build Tools). `JUCE_USE_DIRECT2D=0` → rendu logiciel, portable.
> `JUCE_MODAL_LOOPS_PERMITTED=1` → dialogues de la cassette dans le VST3.

Aucune banque n'est nécessaire. Si un `.rockpunkbank` est présent dans
`SampleBank/`, CMake le copie dans le bundle ; sinon la couche échantillonnée
lit le dossier de l'utilisateur. Voir `PROJECT_STATUS.md` § 2.

## Cibles

| Cible | Rôle |
|---|---|
| `ROCKPUNK_All` | le plugin et l'application |
| `ROCKPUNK_ConvertSamples` | construit un `.rockpunkbank` depuis vos WAV (hors `ALL`) |
| `ROCKPUNK_RenderTest` | vérification audio sans hôte ni interface (hors `ALL`) |

## Livraison

- **Windows** : `ISCC.exe Installer\ROCKPUNK.iss` → `Dist\ROCKPUNK_1.1.0_Installer.exe`
- **macOS** : `.github/workflows/build-macos.yml` (runner `macos-14`) → `.dmg`.
  Un bundle Mach-O et une image disque Apple ne se fabriquent pas sous Windows.

## Presets

`.rpk` = l'état APVTS en XML. Les 25 presets d'usine sont générés par
`Tools/make_presets.py` et voyagent dans `Contents/Resources/Presets` ; le plugin
les recopie dans `Documents\DJBILBOX\ROCKPUNK\Presets` au premier lancement.
Charger, enregistrer et parcourir se font depuis la **cassette** de l'interface.

## Interface

- Habillage et disposition sont **compilés** dans le binaire
  (`Assets/faceplate.jpg`, `Assets/layout.json`, via `juce_add_binary_data`).
- Pour retoucher les coordonnées sans recompiler : déposer un `layout.json` dans
  `Documents\DJBILBOX\ROCKPUNK\` — il remplace celui d'origine.
- Animation néon : le clavier, les potards et les switches s'allument en jouant.

## Documentation

- `GUIDE_UTILISATION.md` — guide livré au client.
- `PROJECT_STATUS.md` — état, banque de sons, chaîne de compilation, limites.
- `PARAMETER_MAP.md` — les 35 paramètres.
- `SYNTH_ARCHITECTURE.md` — architecture audio.
- `CHANGELOG.md` — historique.
