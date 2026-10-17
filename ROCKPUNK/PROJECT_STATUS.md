# ROCKPUNK GUITAR — État du projet

> Dernière mise à jour : 2026-08-16 — version **1.1.0**, produit livrable.

## 1. Ce qu'est le produit

Guitare électrique rock'n'roll hybride, édition unique (ni essai, ni code
d'activation, ni extension vendue à part), livrée pour **Windows et macOS**.

**ROCKPUNK ne livre aucun enregistrement.** Chaque note est calculée ; c'est ce
qui rend le produit vendable sans dépendre de la licence de qui que ce soit.

| Source | Nature |
|---|---|
| Corde A / Corde B | Karplus-Strong, 8 voix, 2 cordes par note, résonateur de caisse |
| Couche échantillonnée | les fichiers que **l'utilisateur** dépose, aucun livré |
| Ampli « Urban Slayer » | gain, distorsion asymétrique, tonalité, niveau |

Puis l'étage Drive (SOFT / HARD / ASYM / FUZZ) et le Delay (libre ou synchronisé).

## 2. La couche échantillonnée

Elle joue `Documents/DJBILBOX/ROCKPUNK/Samples` : `.wav`, `.aif`, `.flac`,
sous-dossiers compris (le premier niveau sert de catégorie). Le dossier est créé
au premier lancement avec un mode d'emploi dedans, et la fondamentale de chaque
fichier est mesurée au premier chargement puis retenue.

La 1.0.0 embarquait 159 guitares converties depuis Slayer Electric, Urban Slayer
Acoustic et la bibliothèque Player. Redistribuer ces enregistrements dans un
instrument vendu n'est pas couvert par leur licence : la banque a été retirée du
produit et de l'historique du dépôt.

Le format `.rockpunkbank` et son chargement restent en place (`SampleBank::open`,
`Tools/convert_samples.cpp`) : embarquer un jour un contenu dont on a la licence
ne demandera aucune modification du code. Il suffit de déposer le fichier dans
`SampleBank/` et de reconfigurer — CMake le détecte et le copie dans le bundle.

## 3. Où vont les fichiers

| Quoi | Windows | macOS |
|---|---|---|
| Plugin | `C:\Program Files\Common Files\VST3\ROCKPUNK.vst3` | `/Library/Audio/Plug-Ins/VST3` (+ `Components` pour l'AU) |
| Presets d'usine | dans le bundle, `Contents\Resources\Presets` | idem |
| Cartouches utilisateur | `Documents\DJBILBOX\ROCKPUNK\Presets` | `Documents/DJBILBOX/ROCKPUNK/Presets` |
| Échantillons utilisateur | `Documents\DJBILBOX\ROCKPUNK\Samples` | `Documents/DJBILBOX/ROCKPUNK/Samples` |

L'installeur Windows s'exécute **en mode élevé** : les « Documents » qu'il voit
sont ceux de l'administrateur. Les presets d'usine voyagent donc dans le bundle,
et c'est le plugin qui les recopie chez l'utilisateur au premier lancement — sans
écraser un fichier de même nom déjà présent.

## 4. Chaîne de compilation

- **JUCE 8.0.4** : `C:\Users\djbil\Documents\DJBILBOX\JUCE`
- **CMake** : celui du paquet WinLibs (`%LOCALAPPDATA%\Microsoft\WinGet\Packages\BrechtSanders.WinLibs...\mingw64\bin\cmake.exe`)
- **MSVC** via VS 2022 Build Tools — **MinGW est refusé par JUCE 8**
  (`juce_TargetPlatform.h` : `#error "MinGW is not supported"`).
- **Inno Setup 6** : `%LOCALAPPDATA%\Programs\Inno Setup 6\ISCC.exe`

```
cmake -B Build -G "Visual Studio 17 2022" -A x64 -DJUCE_DIR=<JUCE>
cmake --build Build --config Release --target ROCKPUNK_All --parallel
ISCC.exe Installer\ROCKPUNK.iss
```

Cibles hors `ALL` : `ROCKPUNK_ConvertSamples` (banque) et `ROCKPUNK_RenderTest`.

## 5. Vérification

`Tests/render_test.cpp` — sans hôte ni interface :

```
cmake --build Build --config Release --target ROCKPUNK_RenderTest
Build/ROCKPUNK_RenderTest_artefacts/Release/ROCKPUNK_RenderTest.exe
```

Joue chaque corde séparément, vérifie que BODY produit bien du son (le
résonateur naissait muet), écrit un fichier témoin dans le dossier utilisateur
pour vérifier qu'il est lu, mesuré et joué, puis passe **les 25 presets** — en
refusant tout preset qui exigerait un échantillon.

La corde est excitée par un bruit non graîné : deux passages sur la même note
donnent des niveaux distants d'environ 3 dB. Le banc moyenne donc quatre notes
réparties sur le manche, sans quoi caler les presets revient à courir après du
bruit de mesure.

Dernier passage : **tout est vert**, 25 presets entre 0,079 et 0,125 RMS.

## 6. macOS

`.github/workflows/build-macos.yml`, runner `macos-14`, arm64 + x86_64 :
VST3 + AU + application autonome, puis `hdiutil` pour le `.dmg`. Ni bundle
Mach-O ni image disque Apple ne se fabriquent sous Windows — c'est la même
recette que THUGLIFE.

Dépôt privé : `github.com/Djbilbox/ROCKPUNK`.

## 7. Ce qu'un humain doit encore vérifier

1. **L'alignement de l'interface** face au mockup : le modèle qui a placé les
   contrôles ne voit pas les images, les coordonnées viennent de mesures et
   d'une détection de cercles. Un `layout.json` déposé dans
   `Documents\DJBILBOX\ROCKPUNK\` remplace celui compilé, sans recompiler.
2. **L'écoute** : le banc de test prouve qu'il y a du signal, pas qu'il est beau.
3. Le bundle macOS n'est **ni signé ni notarisé** : au premier lancement il faut
   passer par *Confidentialité et sécurité → Ouvrir quand même*.
4. Rien à trancher côté droits : le produit ne contient plus aucun
   enregistrement tiers.
