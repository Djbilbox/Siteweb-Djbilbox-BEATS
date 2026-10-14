# HUMPIRE LOADOUT — Prompts visuels (Firefly / Midjourney) + assets

## Style commun (à coller dans TOUS les prompts)
```
photorealistic product render, studio lighting from top-left, soft key light + cool rim light,
shallow reflections, brushed metal, matte rubber, glossy plastic, tiny LED glow, subtle dust-free
clean surface, black #0B0B0F and amber-gold #F5B544 to orange #FF7A3D palette with violet #8A5CFF
accent LEDs, orthographic FRONT view, perfectly flat-on, centered, no perspective, no hands, no text,
no logos, no brand names, original design, 8k, dark neutral background (#0B0B0F)
```
Négatif : `logo, brand name, trademark, text, watermark, perspective, hands, cables, clutter`.
Astuce : générer le **corps SANS contrôles** (zones vides/creux) → les knobs/LED/écrans sont dessinés en code.

## Prompts (suffixe à ajouter après le style commun)
| # | Plugin | Prompt |
|---|--------|--------|
| 1 | SKILLTREE | `front panel of an arcade cabinet monitor: curved CRT glass screen with strong glass reflections and slight scanline bloom, thick black plastic bezel, below it a control deck with 6 empty round recesses for arcade buttons, gold trim, amber backlight` |
| 2 | CLUTCH | `black and gold rack hardware box, 2U, anodized black faceplate with gold engraved lines, empty recesses for three large knobs, horizontal slot for an LED life bar, side ventilation slits, rubber feet not visible` |
| 3 | OVERCLOCK | `gamer hardware enclosure, matte black aluminium, angular cut corners, large circular empty well for a power knob surrounded by a thin glass ring light, circular fan grille on the right with 7 blade silhouettes, amber accent lines` |
| 4 | HARDCAP | `premium high-gloss black lacquer loudspeaker enclosure front view, gold accents, large perforated metal grille, two empty brushed-metal knob wells with amber rings, narrow empty LED strip window at the top` |
| 5 | RESPAWN | `living-room hi-fi amplifier speaker, visible paper speaker cone with dust cap in the center behind a thin transparent protective mesh, wooden-black cabinet, gold trim, empty knob recesses on the bottom bar` |
| 6 | LAG | `retro memory-cartridge console front panel: 4 horizontal slots labelled by empty plates for cartridges A B C D, small round LED wells above each slot, wide empty timeline display window, matte dark grey with gold edges` |
| 7 | GLITCH | `original retro handheld game console, beige-grey textured plastic, large dark screen bezel on top with empty screen area, empty cross-shaped recess bottom-left, two empty round button recesses bottom-right on a diagonal, two empty pill-shaped recesses, speaker grille slots, small power LED well, 2D flat front view` |
| 8 | COMBO | `original matte-black game controller with gold accents, front view, four glossy 3D face-button wells, cross-shaped D-pad well, two analog-stick wells with amber ring, small empty OLED window in the middle, two trigger slots on the top edge` |

> Note : pour GLITCH le rendu procédural en code est déjà fourni (aucun PNG requis). Un PNG 2x remplace le fond sans changer le code.

## Assets à exporter (par plugin, `<id>` = skilltree|clutch|overclock|hardcap|respawn|lag|glitch|combo)
| Fichier | Contenu | Taille |
|---|---|---|
| `<id>_body_2x.png` | corps SANS contrôles (fond) | 2× la taille de base (voir tableau) |
| `<id>_body_1x.png` | idem, réduit (optionnel) | taille de base |
| `knob_main_filmstrip.png` | 101 frames verticales, knob principal | 128×(128×101) @1x · 256×(256×101) @2x |
| `knob_small_filmstrip.png` | 101 frames, petit knob | 80×(80×101) @1x |
| `<id>_overlay_glass.png` | reflets de verre (alpha), posés au-dessus de l'écran | taille de l'écran ×2 |
| `<id>_led_{off,on,glow}.png` | sprites LED | 32×32 |
| `<id>_btn_{up,down}.png` | boutons 3D (si non dessinés en code) | 112×112 @2x |
| `humpire_logo.svg` | logo vectoriel | — |

Nommage : minuscules, `_` uniquement, suffixe `_2x` pour la haute résolution (JUCE BinaryData n'accepte pas `@`).
Format : PNG 8 bits RGBA, sRGB, sans profil ICC.

| Plugin | Taille de base (1x) | 2x |
|---|---|---|
| Skilltree | 900×640 | 1800×1280 |
| Clutch | 820×440 | 1640×880 |
| Overclock | 700×480 | 1400×960 |
| Hardcap | 640×560 | 1280×1120 |
| Respawn | 760×520 | 1520×1040 |
| Lag | 820×480 | 1640×960 |
| **Glitch** | **760×560** | **1520×1120** |
| Combo | 800×520 | 1600×1040 |
(la barre commune occupe les 40 premiers px)
