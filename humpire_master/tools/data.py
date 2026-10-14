# -*- coding: utf-8 -*-
"""Catalogue propre : parse catalogue_gumroad.csv + 4 nouveaux synthés du repo."""
import csv, re, os, html
ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
SRC = os.path.join(ROOT, "djbilbox_campaign", "prospection", "catalogue_gumroad.csv")
SITE = "https://humpireplugins.netlify.app"

# corrections de fautes dans les titres
FIX = {"BREACK YA NECK": "BREAK YA NECK", "ETNIC RITMIK": "ETHNIC RITMIK"}

# role: instrument | effect | midi | bundle | soundware | free-tool ; fmt: formats annoncés dans le titre
PLUG = {
 "BLOOD KEYS": ("instrument", "Horror piano", "VST3", "horror piano vst"),
 "JUKEBOX NEW YORK": ("instrument", "Sampler", "VST3", "sampler vst3"),
 "ROBOTALK": ("effect", "Talkbox / Vocoder / Autotune", "VST3, AU", "talkbox vocoder"),
 "HUMPIRE VST & Effects": ("bundle", "Bundle", "VST3", "vst bundle"),
 "HUMPIRE Free VST Collection": ("bundle", "Bundle gratuit", "VST3", "free vst collection"),
 "HUMPIRE VST SUITE": ("bundle", "Bundle", "VST3", "vst suite"),
 "CHOCOLATINE": ("effect", "Bus compressor / Saturation", "VST3", "bus compressor"),
 "LA BAGUETTE": ("effect", "Tape compressor", "VST3", "tape compressor"),
 "CAMEL PILOT": ("instrument", "Bass synth", "VST3", "bass synth"),
 "ALIEN 808": ("instrument", "808 / Sub bass", "VST3", "808 sub bass"),
 "BLOCK GORILLA": ("instrument", "Drum machine / Beat generator", "VST3", "beat generator"),
 "CASBAH CHORDS": ("midi", "Chord & melody generator", "VST3", "chord generator"),
 "TAXI ALGER": ("midi", "Beat generator", "VST3", "beat generator"),
 "CAMEL PEDALS": ("bundle", "Guitar pedals bundle", "VST3, AU, Standalone", "guitar fx"),
 "PHARAOH COMP": ("effect", "Guitar pedal / Compressor", "VST3, AU, Standalone", "guitar fx"),
 "SURF TREMOLO": ("effect", "Guitar pedal / Tremolo", "VST3, AU, Standalone", "guitar fx"),
 "NEON PHASER": ("effect", "Guitar pedal / Phaser", "VST3, AU, Standalone", "guitar fx"),
 "CORAL CHORUS": ("effect", "Guitar pedal / Chorus", "VST3, AU, Standalone", "guitar fx"),
 "AURORA VERB": ("effect", "Guitar pedal / Reverb", "VST3, AU, Standalone", "guitar fx"),
 "CANYON ECHO": ("effect", "Guitar pedal / Delay", "VST3, AU, Standalone", "guitar fx"),
 "LAVA FUZZ": ("effect", "Guitar pedal / Fuzz", "VST3, AU, Standalone", "guitar fx"),
 "STORM RIDER": ("effect", "Guitar pedal / Distortion", "VST3, AU, Standalone", "guitar fx"),
 "CAMEL PEDAL": ("effect", "Guitar pedal / Overdrive", "VST3, AU, Standalone", "guitar fx"),
 "JET FLANGER": ("effect", "Guitar pedal / Flanger", "VST3, AU, Standalone", "guitar fx"),
 "EQ-PRO SPIDER": ("effect", "Equalizer", "VST3, AU", "eq plugin"),
 "MINI MPC HUMPIRE": ("instrument", "Drum machine", "VST3", "drum machine"),
 "THUGLIFE Free Demo": ("instrument", "G-Funk synth (demo)", "VST3", "g-funk synth"),
 "THUGLIFE": ("instrument", "G-Funk synth", "VST3", "g-funk synth"),
 "STATION SYNTH Free Demo": ("instrument", "Synth (demo)", "VST3, AU", "synth vst"),
 "STATION SYNTH": ("instrument", "Synth", "VST3", "synth vst"),
 "MPC 2026": ("instrument", "Beat machine / Drum sampler", "VST3, AU, Standalone", "beat machine"),
 "NEON SYNTH 80s": ("instrument", "Synthwave synth", "VST3", "synthwave synth"),
 "MASTERING": ("effect", "Mastering limiter", "VST3", "mastering limiter"),
 "DJBILBOX PRO BUNDLE": ("bundle", "Bundle", "VST3", "vst bundle"),
 "MATRIX MODULAR": ("effect", "Oriental / West Coast effect", "VST3", "oriental effect"),
 "BIGBASS": ("instrument", "808 / Sub bass", "VST3", "808 sub bass"),
 "VICE CITY": ("instrument", "Polyphonic synth (80s)", "VST3", "80s synth"),
 "ORIENTAL INSTRUMENT Free Demo": ("instrument", "Arabic & ethnic sampler (demo)", "VST3", "oriental vst"),
 "ORIENTAL INSTRUMENT": ("instrument", "Arabic & ethnic sampler", "VST3", "oriental vst"),
}
# faits vérifiés (campagne 13/10/2026 + titres Gumroad)
FACTS = {
 "MPC 2026": "16 pads, 50 kits (G-Funk, Boom Bap, Trap, Drill, House, Techno, Lo-Fi, Afro, Electro, Reggaeton), tempo-synced sequencer, MIDI/keyboard/mouse triggering. No sample bank included: it maps sounds already installed on the computer. VST3/standalone on Windows, AU/VST3/standalone on macOS.",
 "MINI MPC HUMPIRE": "Free VST3 drum machine with 64 pads and 1,000 grooves.",
 "ORIENTAL INSTRUMENT": "7 multisampled instruments, 280 presets in the bundle (35 in the free demo), VST3 for Windows and macOS Universal.",
 "ORIENTAL INSTRUMENT Free Demo": "Free demo of ORIENTAL INSTRUMENT: 35 presets.",
 "THUGLIFE": "G-Funk synth VST3 for West Coast leads and bass. 30-day free demo available (THUGLIFE Free Demo).",
 "THUGLIFE Free Demo": "30-day trial of the THUGLIFE G-Funk synth.",
 "DJBILBOX PRO BUNDLE": "8 VST plugins in a single payment.",
 "HUMPIRE VST SUITE": "7 VST3 plugins, 6,142 presets plus MIDI.",
 "HUMPIRE VST & Effects": "25 VST3 plugins: synths, guitar, vocals and mixing.",
 "HUMPIRE Free VST Collection": "6 free plugins and demos (VST3).",
 "JUKEBOX NEW YORK": "21 real instruments, riff generator, 1,944 presets.",
 "CASBAH CHORDS": "36 genres, maqam modes.",
 "TAXI ALGER": "36 genres, real drummer grooves.",
 "BLOCK GORILLA": "16 pads, beat generator, real drummers.",
 "ALIEN 808": "Pattern generator, slides and glides.",
 "CAMEL PILOT": "Bassline generator.",
 "CAMEL PEDALS": "10 guitar pedal plugins: VST3, AU and standalone.",
 "STATION SYNTH": "4,000 presets bundle.",
 "LA BAGUETTE": "Saturation, wow & flutter.",
 "VOID SIGNALS": "19 GB SFX and cinematic sound design library.",
 "931 Free Rap Beats": "931 instrumentals, 8 genres, MP3/WAV, 2.72 GB; free for profit with credit, no resale of the pack.",
 "GHOST VOICE": "Vocal hooks, chops and acapellas (WAV 24-bit, 4 GB announced).",
}
SOUND_BUCKET = [
 ("G-FUNK", "g-funk"), ("WEST", "west coast"), ("SUNSET", "west coast"), ("LOWRIDER", "west coast"),
 ("TRAP", "trap"), ("DRILL", "trap"), ("VIPER", "trap"), ("HORROR", "horror"), ("NECROPOLIS", "horror"),
 ("ORIENTAL", "oriental"), ("ARABIC", "oriental"), ("AFRO", "afrobeat"), ("HIP-HOP", "hip-hop"),
 ("BOOM BAP", "boom bap"), ("HOUSE", "house"), ("SYNTHWAVE", "synthwave"),
]

NEW = [  # nouveaux synthés présents dans le repo, absents du catalogue Gumroad
 dict(name="VOODOO", sub="Mystic Synth VST3 — 8 physical/modal engines, 70 presets", dir="VOODOO", voices=12,
      cat="Instrument / Synth (physical modeling)",
      engines="Flute, Pan Flute, Shakuhachi (breath + waveguide), Sitar, Oud (Karplus-Strong, jawari + sympathetic strings), Kalimba (inharmonic modes), Erhu, Voodoo Choir (saws + formants)",
      banks="Flutes, Raga & Strings, Africa, Latin America, Mystic Pads, Bass & Plucks, Textures", presets=70,
      controls="LFO (source, destination Pitch/Filter/Amp/Timbre), pitch, glide, filter (LP/HP/BP), ADSR, 5 timbre controls, reverb, chorus, distortion, delay (time, feedback, ping-pong), mono/poly, pitch-bend and mod wheels",
      ext=".voodoopreset", tags=["oriental vst","world instruments","physical modeling","sitar vst","oud vst","flute vst"]),
 dict(name="WARZONE808", sub="808 Bass Synth VST3 — unison, filter envelope, 70 presets", dir="WAZONE808", voices=16,
      cat="Instrument / Bass synth",
      engines="PolyBLEP oscillators, TPT filter, up to 7-voice unison, filter envelope, velocity sensitivity, mono legato mode",
      banks="808 Subs, Dirty & Reese, Leads, Plucks & Keys, Pads & Ambience, FX & Percussion, Expansion Vol.1", presets=70,
      controls="LFO, pitch (±12 st), glide, filter, resonance, filter envelope depth and A/D/S, velocity, unison and detune, ADSR, reverb, chorus, distortion, delay, mono/poly",
      ext=".wz808preset", tags=["808 vst","bass synth","reese bass","trap bass","sub bass","unison synth"]),
 dict(name="FUNKYKOPS", sub="Funk / Disco / G-Funk Synth VST3 — 8 engines, 70 presets", dir="FUNKYKOPS", voices=16,
      cat="Instrument / Synth (funk, disco, G-Funk)",
      engines="Clavinet (Karplus-Strong + bite), Rhodes (tine + bell, velocity bark), Synth Bass, Brass, Strings, Lead, Guitar, Talkbox (saws + formants / vowels)",
      banks="Clavinet & Funk Keys, Rhodes & EP, Synth Bass, Brass & Stabs, Strings & Disco, Leads & Talkbox, Guitar & FX", presets=70,
      controls="LFO (Sine…S&H), glide, filter (LP/HP/BP), ADSR, filter envelope depth and decay, resonance, velocity, vowel/body/bright/width/texture, reverb, chorus, distortion, delay, mono/poly",
      ext=".fkpreset", tags=["funk synth","g-funk vst","clavinet vst","rhodes vst","talkbox vst","disco synth"]),
 dict(name="OCCITANIE", sub="Occitan Instruments Synth VST3 — 8 engines, drone, 70 presets", dir="OCCITANIE", voices=16,
      cat="Instrument / Synth (folk instruments)",
      engines="Cabrette, Vielle (bowed strings, buzz), Galoubet (recorder), Mandoline, Guitare, Accordéon (3 reeds, musette detune), Choir, Carillon (inharmonic modes); DRONE layer on any engine",
      banks="Cabrette & Bagpipe, Vielle & Bowed Strings, Flutes & Galoubet, Plucked Strings, Accordion & Musette, Choirs & Pads, Carillons & Textures", presets=70,
      controls="LFO, glide, filter (LP/HP/BP), ADSR, reed/body/bright/drone/velocity, width, texture, reverb, chorus, distortion, delay, mono/poly",
      ext=".occpreset", tags=["folk instruments vst","world music synth","accordion vst","drone synth","bagpipe vst","european folk"]),
]

def slugify(s):
    s = s.lower().replace("&", "and")
    return re.sub(r"[^a-z0-9]+", "-", s).strip("-")

def parse_price(raw):
    raw = raw.strip()
    cur = "EUR" if "€" in raw else "USD"
    num = re.sub(r"[^0-9,\.]", "", raw).replace(",", ".")
    val = float(num) if num else 0.0
    return val, cur, raw.endswith("+")

def load():
    problems, fixed = [], []
    prods = []
    with open(SRC, encoding="utf-8") as f:
        rows = list(csv.DictReader(f))
    seen = set()
    for r in rows:
        title = r["nom"].strip()
        m = re.match(r"^(.*?)\s+[—-]\s+(.*)$", title)
        name, sub = (m.group(1), m.group(2)) if m else (title, "")
        if name == "DJBILBOX BEATS":
            name, sub = f"DJBILBOX BEATS {sub}", ""
            fixed.append(f"Nom ambigu désambiguïsé : « {name} » (deux produits portaient « DJBILBOX BEATS »)")
        name0 = name
        for bad, good in FIX.items():
            if bad in name:
                name = name.replace(bad, good); fixed.append(f"Faute de titre corrigée : « {bad} » → « {good} » ({r['url']})")
        if name == name0 and sub.endswith("  "): sub = sub.strip()
        if "HAUNTED SOULS" in title: pass
        if r["url"] in seen:
            problems.append(f"Doublon URL ignoré : {r['url']}"); continue
        seen.add(r["url"])
        price, cur, pwyw = parse_price(r["prix_affiche"])
        meta = PLUG.get(name)
        if meta: role, ptype, fmt, kw = meta
        else: role, ptype, fmt, kw = "soundware", "Sample pack / loops", "WAV", "sample pack"
        t = (name + " " + sub).upper()
        genres = sorted({g for k, g in SOUND_BUCKET if k in t})
        slug = slugify(name)
        facts = FACTS.get(name, "")
        status = r["statut_visible"]
        sales = int(r["ventes_affichees"] or 0)
        prods.append(dict(
            id=slug, sku=("HMP-" if role != "soundware" else "DJB-") + slug.upper()[:40],
            name=name, subtitle=sub, full_title=f"{name} — {sub}" if sub else name,
            brand="HUMPIRE" if role != "soundware" else "DJBILBOX BEATS",
            role=role, product_type=ptype, formats=fmt, keyword=kw, genres=genres,
            price=price, currency=cur, pay_what_you_want=pwyw, is_free=(price == 0.0),
            price_display=r["prix_affiche"], url=r["url"], status=status, published=(status == "Published"),
            sales_displayed=sales, facts=facts, image_link="", kind=("plugin" if role in ("instrument","effect","midi","bundle") else "soundware"),
            source="catalogue_gumroad.csv"))
    # tri : produits légitimes
    for p in prods:
        if not p["published"]: problems.append(f"Produit non publié exclu des flux : {p['name']} ({p['url']})")
    return prods, problems, fixed

def new_products():
    out = []
    for n in NEW:
        out.append(dict(n, id=slugify(n["name"]), brand="HUMPIRE", kind="plugin", role="instrument",
                        version="1.0.0", formats="VST3, Standalone", price=None, url=None, image_link="",
                        source=f"{n['dir']}/README.md + CMakeLists.txt"))
    return out
