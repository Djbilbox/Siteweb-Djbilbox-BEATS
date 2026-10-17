# Fiche produit (JSON) — schema

One file per product: `MARKETING/fiches/<id>.json` (id = lowercase slug, e.g. `arena-bass`).

```json
{
  "id": "arena-bass",
  "name": "ARENA BASS",                       // display name, exactly as shown by the plugin
  "group": "synths | loadout | orbit | chaabi",
  "category": "instrument | effect",
  "kind": "Bass Synthesizer",                 // short label under the brand on the thumbnail (<= 28 chars)
  "tagline": "...",                           // <= 120 chars, concrete, no hype without a fact
  "sub": "...",                               // subtitle for the website page (<= 70 chars)
  "gumroad_title": "ARENA BASS - Bass Synthesizer VST3 Plugin (70 Presets) | Trap, Drill, Afrobeat",
  "summary": "...",                           // 1-2 sentences for the store summary
  "about": ["paragraph 1", "paragraph 2", "paragraph 3"],
  "features": [ {"icon": "fa-wave-square", "t": "short title", "d": "one sentence, factual"} ],   // 6-8 items
  "specs": [ {"k": "Format", "v": "VST3 64-bit (Windows)"} ],
  "includes": ["..."],
  "genres": ["Trap", "Drill"],
  "tags": ["bass synth", "808"],              // <= 10 store tags
  "badges": ["70 presets", "8 engines", "Royalty-free"],   // 3 short badges for the thumbnail, first one is highlighted
  "faq": [ {"q": "...", "a": "..."} ],
  "accent": "#4fc3f7",                        // optional brand accent for the visuals
  "fs_thumb": "72px", "fs_cover": "104px"     // optional title font sizes (long names need smaller)
}
```
Facts only: every number, feature and claim must be verifiable in the source code or the factory presets.
