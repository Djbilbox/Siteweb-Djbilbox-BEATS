# ROCKPUNK GUITAR — README

**ROCKPUNK GUITAR** · guitare rock'n'roll hybride · DJBILBOX BEATS
· code `HUMP`/`RPK1` · C++17 · JUCE 8.0.4
· VST3 + application autonome (Windows), VST3 + AU + application (macOS).

Deux cordes Karplus-Strong, l'ampli « Urban Slayer », un LFO, un filtre, un drive
à 4 caractères, un delay synchronisable, une reverb et un chorus. 76 presets d'usine. Édition unique : ni essai, ni
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
  (installeur, licence et guide en anglais : `USER_GUIDE.txt`, `LICENSE.txt`)
- **macOS** : `.github/workflows/build-macos.yml` (runner `macos-14`) → `.dmg`.
  Un bundle Mach-O et une image disque Apple ne se fabriquent pas sous Windows.

## Presets

`.rpk` = l'état APVTS en XML (65 paramètres). Les 76 presets d'usine sont générés
par `Tools/make_presets.py` et voyagent dans `Contents/Resources/Presets` ; le
plugin les recopie dans `Documents\DJBILBOX\ROCKPUNK\Presets` au premier lancement.
Charger, enregistrer et parcourir se font depuis le **bandeau de presets** (dossier,
flèches, disquette) et le menu de la **cassette** EXPANSION. Ne jamais renommer un
preset : les projets enregistrés retrouvent leur preset par son nom.

## Interface

- Éditeur à taille fixe **1000 x 746**, mis à l'échelle par la fenêtre
  (redimensionnable, ratio verrouillé, 500 à 2000 px de large).
- Habillage et positions viennent **du mockup lui-même** : `Tools/make_ui_assets.py`
  mesure l'image maîtresse et produit `Assets/faceplate.jpg` (le mockup sans les
  pièces mobiles), `Assets/sprites.png` (capuchon de potard, curseurs, molettes) et
  `Assets/layout.json` (toutes les positions, en pixels du mockup). Les trois sont
  **compilés** dans le binaire (`juce_add_binary_data`).
- `Source/UI/Art.h` charge ces ressources ; `Source/UI/Panel.h` dessine et teste
  la souris dans le **même repère** (pixels du mockup) : une zone qui réagit est
  par construction la zone dessinée. Plus de `layout.json` utilisateur.
- Pour retoucher une position : modifier `Tools/make_ui_assets.py`, relancer
  `python3 Tools/make_ui_assets.py`, recompiler.
- Animation néon : clavier, potards, curseurs, molettes et grilles s'allument
  au survol et en jouant.

## Vérification

`ROCKPUNK_RenderTest` (cible hors `ALL`, aucun hôte ni fenêtre) : comportement
face aux hôtes (process avant prepare, blocs plus grands que prévu, blocs vides,
mono, taux d'échantillonnage), notes coincées, NaN et DC sur 120 réglages
aléatoires, aller-retour d'état de chacun des 65 paramètres, texte des paramètres,
les 76 presets, et la géométrie de l'interface (chaque contrôle, au centre où il
est dessiné, pilote bien son paramètre ; les 37 touches jouent leur note).

## Documentation

- `USER_GUIDE.txt` — guide livré au client (en anglais, comme toute l'interface et l'installeur).
- `PROJECT_STATUS.md` — état, banque de sons, chaîne de compilation, limites.
- `PARAMETER_MAP.md` — les paramètres (la liste complète, 65, est dans `Source/Params.h`).
- `SYNTH_ARCHITECTURE.md` — architecture audio.
- `CHANGELOG.md` — historique.
