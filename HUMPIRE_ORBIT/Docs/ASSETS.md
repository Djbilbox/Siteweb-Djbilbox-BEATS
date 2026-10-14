# HUMPIRE ORBIT — Prompts image & assets (livrable 4)

L'UI des plugins est 100 % vectorielle (juce::Path). Ces visuels servent à : **icônes**, **boutique**, **Hub**, **installateur**, et fonds optionnels.
Prompts en anglais (meilleurs résultats des générateurs). Format : coller `STYLE COMMUN` + prompt du plugin.

## STYLE COMMUN (à préfixer)
```
premium cosmic UI key art, deep black background #050508, subtle starfield, soft nebula in violet-blue #7C5CFF,
glowing amber-to-orange accents #F5B544 to #FF7A3D, clean vector-like shapes, volumetric glow, ultra sharp,
no text, no logo, no watermark, centered composition with empty margins, 1:1, high detail, cinematic lighting
```
Négatif : `text, letters, logo, watermark, brand names, photo of real products, people, low contrast, clutter`

## Prompts par plugin
| # | Plugin | Prompt |
|---|--------|--------|
| 1 | SOLAR | `a blazing golden sun at the center, thin glowing elliptical orbit rings around it carrying a flowing luminous pitch curve like a sound wave wrapped in orbits, small cyan and violet planets on the curve` |
| 2 | SATURN | `a ringed gas giant planet in amber and ember tones, four distinct glowing rings each in a different hue, tiny luminous beads orbiting on each ring like singing voices` |
| 3 | TWIN | `two identical twin planets in mirrored orbit, one amber one violet, symmetrical composition, faint light trails linking them, soft binary-star glow between them` |
| 4 | NEBULA | `a vast violet and magenta nebula cloud filled with hundreds of tiny golden stars, density growing toward the center, stars clustering like a crowd of voices` |
| 5 | SOLAR FLARE | `an erupting solar prominence arching off a fiery star surface, molten ember loops, plasma filaments, intense orange glow with cooler violet space behind` |
| 6 | VACUUM | `deep empty space void with fine dust particles and faint glowing motes being drawn in a spiral toward an invisible center, cold blue-violet, minimal, mysterious` |
| 7 | GRAVITY | `a planet cross-section style sphere with three glowing gravity wells (upper, middle, lower zones) shown as concentric distortion ripples, amber core, violet grid lines bending around it` |
| 8 | SIGNAL | `a space probe with a large parabolic antenna transmitting glowing concentric signal arcs, a distorted waveform flowing out of the dish, amber and cyan light, dark space` |
| 9 | SUPERNOVA | `a star exploding at peak brightness, a blinding white-gold flash core, expanding shockwave rings and radiating light beams in amber and ember, debris sparks` |
| 10 | VOID | `a black hole with a bright swirling accretion disk in amber and orange, gravitational lensing ring, tiny bright particles being swallowed into the dark center, violet glow at edges` |

Bundle (boutique) : `all ten cosmic objects (sun, ringed planet, twin planets, nebula, solar flare, void, gravity planet, probe, supernova, black hole) arranged along one large orbital path around a central golden star, epic wide composition` — 16:9.

## Assets à exporter
Nommage : `humpire_<plugin>_<type>_<LxH>.<ext>` — `<plugin>` ∈ `solar saturn twin nebula solarflare vacuum gravity signal supernova void`.

| Type | Taille | Format | Usage |
|------|--------|--------|-------|
| `hero` | 1200×1200 | PNG transparent | élément signature détouré (Hub, boutique) |
| `bg` | 1920×1200 | PNG/WebP | fond optionnel d'éditeur (2× de 960×600) |
| `icon` | 512×512 + 256, 128, 64, 32, 16 | PNG + `.ico` multi-tailles | icône plugin / raccourci / installateur |
| `store` | 1600×900 | JPG q90 | vignette boutique HUMPIRE |
| `social` | 1080×1080, 1080×1920 | JPG | Instagram / TikTok / Shorts |
| Bundle `banner` | 1920×600 | JPG | bannière boutique |
| Bundle `hero` | 3200×1800 | JPG | page d'accueil bundle |
| Installateur | 164×314 (grand), 55×55 (petit) | BMP 24 bits | Inno Setup `WizardImageFile` / `WizardSmallImageFile` |
| Logo | vectoriel | SVG + PNG 1024 | `humpire_logo.svg`, `humpire_mark.svg` (étoile 4 branches) |

Polices à embarquer (OFL) : *Space Grotesk*, *Inter*, *Press Start 2P* — placer les `.ttf` dans `Core/Fonts/` ; sans elles, le thème retombe sur la police système (testé).
