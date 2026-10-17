## [1.1.0] - 2026-08-16 - Plus aucun son tiers dans le produit

### Pourquoi
La 1.0.0 embarquait 159 guitares converties depuis Slayer Electric, Urban
Slayer Acoustic et la bibliotheque Player. Redistribuer ces enregistrements
dans un instrument vendu n'est pas couvert par la licence de ces produits, et
masquer leur origine n'aurait fait que rendre le probleme invisible jusqu'a ce
qu'on le remarque. La banque est donc retiree du produit.

### Le resonateur de caisse ne sonnait pas
`BodyResonator::setParams()` n'etait appele nulle part : les coefficients
restaient a zero, `process()` rendait le silence, et monter BODY ne faisait
qu'attenuer la corde. Il est maintenant accorde depuis
`GuitarVoice::setLayerA/B` - la resonance monte de 95 Hz (grande caisse) a
230 Hz (petit corps claquant) avec la quantite de corps. Ce bug etait discret
tant que la couche echantillonnee portait le son ; il ne l'est plus du tout
maintenant que les cordes sont l'instrument entier.

### La couche echantillonnee joue les fichiers de l'utilisateur
- `SampleBank::scanFolder()` lit `Documents/DJBILBOX/ROCKPUNK/Samples`
  (wav / aif / flac, sous-dossiers = categories), fondamentale mesuree au
  premier chargement et retenue ensuite.
- Le dossier est cree au premier lancement avec un mode d'emploi dedans : un
  selecteur vide sans explication ressemble a une panne.
- La ligne d'etat sous la liste ouvre le dossier au clic, et la liste est relue
  a chaque ouverture de la fenetre.
- Le format `.rockpunkbank` et son chargement restent en place : embarquer un
  jour une banque dont on a la licence ne demandera aucune modification du code.

### Les 25 presets ne reposent que sur les cordes
Reecrits entierement sur les deux couches modelisees, l'ampli, le drive et le
delay. `make_presets.py` refuse desormais tout preset qui allumerait `smpOn`, et
le banc de test le verifie fichier par fichier : un preset d'usine doit sonner
sur une installation neuve, dossier d'echantillons vide.

Niveaux recales entre 0,079 et 0,125 RMS. La mesure elle-meme a du etre
stabilisee : la corde est excitee par un bruit non graine, donc deux passages
sur la meme note donnent des niveaux distants d'environ 3 dB - le banc moyenne
maintenant quatre notes reparties sur le manche.

### Livraison
- Installeur Windows : 70 Mo -> 11 Mo.
- Le workflow macOS echoue franchement si un fichier audio se glisse dans
  l'image disque.
- Signature et notarisation Apple cablees, actives par la presence des secrets
  du depot ; sans certificat, l'image embarque la marche a suivre.

## [1.0.0] - 2026-08-16 - Produit livrable (Windows + macOS)

### Banque de sons interne : le plugin transporte ses guitares
- `Tools/extract_mse.py` : decodage des instruments Maize (`.mse`) de **Slayer
  Electric** (63) et **Urban Slayer Acoustic** (52). Le format est un conteneur
  dont l'habillage occupe la tete (un PNG brut puis un XML `<player>` aux images
  en base64 JUCE) et dont les echantillons sont en queue : une entree par WAV
  (chemin d'origine, canaux, frequence, nombre de trames) suivie du PCM 16 bits
  concatene en fin de fichier.
- `Tools/convert_samples.cpp` : rogne, normalise, mesure la note fondamentale et
  encode en FLAC les 159 sources (Electric + Acoustic + 44 guitares Player) dans
  un conteneur unique `SampleBank/ROCKPUNK.rockpunkbank` - 148 Mo de WAV pour
  30 Mo de banque. La fondamentale est gravee dans l'index : le plugin ne lance
  plus d'autocorrelation a chaque chargement.
- `Source/Dsp/SampleBank.h` : lecture par projection memoire ; seules les pages
  reellement jouees sont chargees.

### Plus aucun chemin de la machine de developpement
- L'habillage et `layout.json` sont **compiles** dans le binaire (`juce_add_binary_data`).
  L'editeur lisait le dossier de compilation : la fenetre etait vide partout ailleurs.
- La couche echantillonnee lisait `C:/ProgramData/Player/SampleLibrary/Guitar`,
  le dossier d'un autre produit : remplacee par la banque interne.
- Les presets etaient cherches sur le Bureau ; ils vivent maintenant dans
  `Documents\DJBILBOX\ROCKPUNK\Presets`. L'installeur tournant en mode eleve ne
  peut pas y ecrire, donc les presets d'usine voyagent dans `Contents/Resources`
  et le plugin les y recopie au premier lancement, sans ecraser les fichiers
  deja presents.
- `smpSel` passe de 0..127 a 0..255 : la banque compte 159 entrees.

### 25 presets d'usine
- Noms dans l'esprit rock'n'roll / AC-DC, inventes plutot qu'empruntes a des
  titres existants. Rythmiques crunch, solos, gros gain, clairs, acoustiques.
- Tous mesures et cales autour de 0,10 RMS : l'ecart initial allait de 0,015 a
  0,31, soit 26 dB de saut au changement de preset.
- Les presets acoustiques gardent l'ampli enclenche a distorsion nulle : c'est le
  seul etage de gain de la chaine, sans lui ils arrivaient 15 dB sous les autres.

### Banc de test sans interface
- `Tests/render_test.cpp` : ouvre la banque, decode des echantillons temoins,
  joue les cordes, trois points de la banque et **les 25 presets**, et echoue si
  quoi que ce soit est muet. Un produit qui s'installe puis ne joue rien est la
  panne que rien d'autre ne revele avant le client.

### Livraison
- Windows : installeur Inno Setup (VST3 + application autonome + documentation),
  `ROCKPUNK_v1.0.0_WINDOWS.zip`.
- macOS : `.github/workflows/build-macos.yml` (runner macos-14, arm64 + x86_64)
  produit VST3 + AU + application et le `.dmg`. Ni bundle Mach-O ni image disque
  Apple ne peuvent etre fabriques sous Windows.
- Edition unique : ni essai, ni code d'activation, ni extension vendue a part.
- Icone et visuels d'installeur generes par `Tools/make_icon.py`.

## [0.2.2] - 2026-08-16 - Layout reconstruit sur les mesures reelles

### Layout complet sur la geometrie du mockup
- Source de verite : rockpunk_ui_dimensions.json/csv (mesures humaines des controles reels), echelle design = mesure x 0.8.
- Cassette en haut-gauche, LFO RATE/AMOUNT/PITCH, ADSR (4 sliders verticaux), wheels pitch/mod, GLIDE/FILTER centraux, 6 potards a droite, 5 faders droits, 5 combos, clavier en bas ; patch browser reste visuel.
- LevelSlider : support vertical ; les grilles de haut-parleur = switchs Layer A/B ; les combos = CyclingSwitch en recadre statique.
- Mapping parametres documente (PROJECT_STATUS 1quater) ; ampGain/ampDist/driveTone restent en automation.
- Suppression des captions codees (l'image porte ses textes).
- Compilation Release x64 MSVC OK, .vst3 recopie a la racine.

## [0.2.1] - 2026-08-16 - Recadres auto-detectes sur l'artwork reel

### Fin de la calibration des sprites
- Detection de cercles par Hough sur le mockup 2K (detect.cs) : localisation des potards reels dans l'image (120 cercles, sortie JSON invariante).
- match.ps1 : chaque controle est associe au potard reel le plus proche (affectation gloutonne, sans doublon, confiance <= 60 px design). Les boites des composants sont recentrees sur les potards detectes (hit-areas + sprites alignes) et la taille de recadre provient du rayon mesure.
- layout.json : section sprites gravee avec cx/cy/size par controle (source = detected pour les automatiques, box sinon) - override manuel possible.
- RockpunkLookAndFeel.h : SpriteSpec gagne cx/cy (centre de recadre en px design, -1 = centre de la boite) ; balayage realiste 270 degres (135->405) pour les potards, 0->180 pour les switches ; liseres sombres en bord du clip circulaire.
- PluginEditor.cpp : parsing des overrides cx/cy depuis layout.json.
- Les sliders de niveau restent sur leur boite (pas de recentrage sur un potard).
- Recompilation Release x64 MSVC reussie, .vst3 recopie a la racine.

# CHANGELOG

## [0.2.0] — 2026-08-16 — Sprites depuis l'artwork (RockpunkLookAndFeel)

### Fini les rectangles génériques par-dessus l'image
- **`Filmstrip.h`** : sprite sheet générique (frames en grille, dessin aspect-fit).
- **`RockpunkLookAndFeel.h`** : découpe les **vrais contrôles depuis le mockup** —
  potards et switches rotatifs sont recadrés depuis l'image de fond puis **pré-rotés en
  bandes de sprites** (masque circulaire, coins transparents). Sliders, VU, cassette et
  piano utilisent des recadres directs de leur zone.
- Rendu par sprites : `drawRotarySlider`, `drawLinearSlider`, `drawButtonBackground`,
  `drawToggleButton` (LookAndFeel standard) + `drawKnobSprite/drawSwitchSprite/
  drawSliderSprite/drawCropSprite` (composants ROCKPUNK).
- **`AnimatedKnob`** : l'image du potard tourne réellement (filmstrip) ; le halo néon
  reste une surcouche d'animation « énergie de note ».
- **`CyclingSwitch`** : sprite rotatif à états discrets (2 pour ON/OFF, 4 pour DRIVE TYPE).
- **`LevelSlider`** : track recadrée + remplissage néon translucide.
- **`VirtualPiano`** : clavier = recadre de l'image ; les touches allument en néon.
- **`VuMeter`** / **`ExpansionCassette`** : corps = recadre de l'artwork ; boutons
  transparents (artwork visible) avec neon hover/press.
- **`layout.json`** : nouvelle section `"sprites"` pour régler `size`/`frames`/`box`
  (taille de recadre, nombre d'images du filmstrip, boîte source) sans recompiler.
- Compilation **Release x64 MSVC réussie**, `.vst3` recopié à la racine.

### À vérifier (humain)
- Le recadre (`size`) doit englober le potard réel : ajuster via `layout.json` si la
  rotation révèle du fond voisin.
- Alignement des touches du piano recadrées vs nos rectangles de glow.
- Le son n'est toujours pas écouté.

## [0.1.0] — 2026-08-16 — ROCKPUNK GUITAR (première compilation)

### Re-architecture
- **ROCKPUNK devient ROCKPUNK GUITAR** : moteur synthé archivé dans `Source/Dsp/_archive_synth_engine/`
  (Envelope/Filter/Lfo/Oscillator/Voice). Conservé : `Drive.h`, `Delay.h`.
- Nouveau moteur : **8 voix Karplus-Strong double couche** (Layer A = rythmique/clean, Layer B = lead/hot),
  résonateur de caisse, pan equal-power par note, crossfade `layerMix`.
- Nouvel ampli **« Urban Slayer »** : pre-gain → tone LP → saturation tanh asymétrique → level.
- Chaîne master : Drive → Delay → Width (mid/side) → Volume lissé → Limiteur tanh → RMS (VU).

### Interface
- Piano virtuel néon custom (JUCE 8 : `drawWhiteKey/drawBlackKey` sont `final` → composant maison).
- Knobs chrome + halo néon violet/rose, switches cycliques, sliders, VU mètres, cassette d'expansion
  (INIT/LOAD/SAVE/prev/next, presets `.rpk`).
- **Fond = mockup JPEG** du projet ; positionnement piloté par **`layout.json`** (design 1120×836).
- Animation néon déclenchée par les notes actives (suivi `MidiKeyboardState` + `activeNotes`).

### Build
- Bascule MSVC (Visual Studio 2022) — MinGW bloqué par JUCE 8.
- `JUCE_USE_DIRECT2D=0`, `JUCE_MODAL_LOOPS_PERMITTED=1`, `ROCKPUNK_IMAGE_DIR`.
- Compilation **Release x64 réussie** ; `.vst3` validé et copié à la racine.
- Corrections compat JUCE 8 : `currentNote` privé → `voiceNote` ; `getVoices()` → `getNumVoices()/getVoice(i)` ;
  `Slider::getValueNormalised()` → `getNormalisableRange().convertTo0to1()` ; `TextButton::textColourId` →
  `textColourOffId/textColourOnId` ; `createPluginFilter()` ajouté ; `inline` sur les fonctions de layout.

### Limites connues
- **Le modèle d'IA ne peut pas lire les images** → l'alignement des contrôles face au mockup est
  **« À VÉRIFIER (humain) »** (ajuster `layout.json`).
- Le son n'a pas été écouté (défauts raisonnables, à valider à l'oreille dans un hôte).
- Doc synthé `MODULATION_MATRIX.md`, `VISUAL_VARIATION.md`, `PRODUCT_BRIEF.md` à réviser (contexte hérité).