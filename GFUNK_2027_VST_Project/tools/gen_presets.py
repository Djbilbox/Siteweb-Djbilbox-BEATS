"""Procedural factory library: 10 categories x 100 presets = 1000 sounds.

Every preset is a full parameter set built from a musically-constrained category
template, a flavour modifier and seeded random variation. The five original
signature sounds are hand-set and sit first in their categories.
"""
import math
import random

from gfunk_spec import PARAMS, PARAM_INDEX

PER_CAT = 100

CATEGORIES = ["WHISTLE LEADS", "SYNTH LEADS", "SUB BASS", "BOOGIE BASS", "TALKBOX VOCAL",
              "PADS", "KEYS", "BRASS STABS", "PLUCKS BELLS", "ARPS SEQUENCES"]


def U(a, b):
    return ("u", a, b)


def L(a, b):
    return ("l", a, b)


def C(*v):
    return ("c", v)


def draw(rng, spec):
    if not isinstance(spec, tuple):
        return spec
    k = spec[0]
    if k == "u":
        return rng.uniform(spec[1], spec[2])
    if k == "l":
        return math.exp(rng.uniform(math.log(spec[1]), math.log(spec[2])))
    return rng.choice(spec[1])


TEMPLATES = {
    0: dict(o1wave=U(0, .9), o1oct=C(0, 0, 1), o1level=.85, o2wave=C(0, 1), o2oct=C(0, 1, 0), o2fine=U(-12, 12),
            o2level=U(0, .35), sub=U(0, .15), uni=C(1, 1, 2, 3), unidetune=U(.05, .2), unispread=.5, ftype=C(0, 1),
            cutoff=L(2500, 9000), reso=U(.05, .3), keytrack=U(.4, .9), fenvamt=U(0, .25), fD=U(200, 500), fS=U(.3, .8),
            aA=U(2, 25), aD=U(150, 500), aS=U(.75, 1), aR=U(120, 400), glide=U(120, 380), mode=C(1, 2, 2),
            lfo1rate=U(4.5, 6.5), lfo1pitch=U(.15, .4), drive=U(.1, .35), wobble=U(.1, .35), chorus=U(.2, .5),
            dlymix=U(0, .25), dlytime=C(2, 3), dlyfb=U(.25, .45), revmix=U(.15, .4), revsize=U(.45, .8)),
    1: dict(o1wave=C(2, 2, 3, 1.5), o2wave=C(2, 3), o2fine=U(5, 18), o2semi=C(0, 0, 0, 7, 12, -12), o2level=U(.4, .8),
            uni=C(1, 2, 3, 5), unidetune=U(.2, .6), unispread=U(.5, 1), ftype=1, cutoff=L(1500, 7000), reso=U(.1, .45),
            keytrack=U(.3, .8), fenvamt=U(.2, .6), aA=U(1, 15), aD=U(200, 700), aS=U(.6, .95), aR=U(100, 500),
            fD=U(150, 600), fS=U(.1, .5), glide=U(0, 120), mode=C(0, 1, 2), lfo1rate=U(4.5, 6.5), lfo1pitch=U(0, .25),
            lfo1pwm=U(0, .4), drive=U(.1, .4), chorus=U(.2, .5), dlymix=U(.05, .3), revmix=U(.1, .35)),
    2: dict(o1wave=C(0, 0, 1), o1oct=C(-1, -1, 0), o1level=.9, o2wave=C(0, 1, 2), o2oct=-1, o2level=U(0, .3),
            sub=U(.5, 1), ftype=C(0, 1), cutoff=L(150, 600), reso=U(.05, .3), keytrack=U(.2, .7), fenvamt=U(.1, .5),
            aA=U(1, 3), aD=U(150, 500), aS=U(.6, 1), aR=U(40, 200), fD=U(100, 400), fS=U(0, .3), glide=U(0, 90),
            mode=C(1, 2), drive=U(.2, .6), wobble=U(0, .15), chorus=0, revmix=U(0, .08), dlymix=0),
    3: dict(o1wave=C(2, 3, 2), o1oct=C(-1, 0), o2wave=C(2, 3), o2oct=-1, o2fine=U(-10, 10), o2level=U(.5, .9),
            sub=U(.2, .6), uni=C(1, 1, 2), ftype=1, cutoff=L(250, 1100), reso=U(.2, .6), keytrack=U(.3, .8),
            fenvamt=U(.4, .9), fD=U(80, 300), fS=U(0, .2), fR=U(60, 200), aA=U(1, 3), aD=U(150, 400), aS=U(.3, .7),
            aR=U(40, 160), glide=U(20, 140), mode=C(1, 2), drive=U(.25, .6), wobble=U(.05, .25), chorus=U(0, .2),
            revmix=U(0, .1), dlymix=0),
    4: dict(o1wave=C(2, 3), o2wave=C(2, 2, 3), o2fine=U(4, 14), o2level=U(.4, .8), sub=U(0, .25), uni=C(1, 2, 3),
            ftype=C(2, 2, 1), cutoff=L(500, 2200), reso=U(.45, .85), keytrack=U(.2, .6), fenvamt=U(-.2, .5),
            lfo1rate=U(.3, 5), lfo1shape=C(0, 1, 4), lfo1cut=U(.25, .7), lfo2rate=U(.2, 1.5), lfo2cut=U(-.4, .4),
            aA=U(5, 80), aD=U(300, 800), aS=U(.5, .9), aR=U(150, 600), glide=U(0, 100), mode=C(0, 1, 2),
            drive=U(.2, .5), wobble=U(.1, .4), chorus=U(.3, .6), revmix=U(.1, .35), dlymix=U(0, .2)),
    5: dict(o1wave=C(2, 2, 1, 3), o2wave=C(2, 1, 3), o2fine=U(6, 20), o2level=U(.5, .9), sub=U(0, .3),
            uni=C(3, 4, 5, 7), unidetune=U(.3, .7), unispread=U(.7, 1), ftype=C(0, 1), cutoff=L(900, 6000),
            reso=U(.05, .35), keytrack=U(.3, .7), fenvamt=U(0, .4), aA=L(250, 2500), aD=U(600, 2500),
            aS=U(.6, .95), aR=L(900, 5000), fA=U(200, 1500), fD=U(800, 3000), fS=U(.3, .8), fR=L(900, 5000),
            lfo1rate=U(.1, .6), lfo1cut=U(0, .35), lfo1pwm=U(0, .5), lfo1shape=C(0, 1), lfo2rate=U(.2, 1),
            lfo2cut=U(-.2, .2), drive=U(0, .25), wobble=U(.1, .4), phaser=C(0, 0, .3, .5), chorus=U(.4, .8),
            dlymix=U(0, .25), revmix=U(.3, .65), revsize=U(.6, .95), mode=0),
    6: dict(o1wave=C(1, 0, 1, 1.5), o2wave=C(0, 1), o2oct=C(0, 1, 1), o2fine=U(-6, 6), o2level=U(.2, .6),
            sub=U(0, .3), uni=C(1, 2), ftype=C(0, 1), cutoff=L(1800, 7000), reso=U(.05, .25), keytrack=U(.5, 1),
            fenvamt=U(.15, .5), aA=U(1, 4), aD=L(400, 2200), aS=U(0, .45), aR=U(150, 700), fD=U(200, 900),
            fS=U(0, .3), lfo1rate=U(4, 6), lfo1pitch=U(0, .12), lfo2amp=C(0, 0, .2), lfo2rate=U(3, 6), mode=0,
            drive=U(.1, .35), wobble=U(.05, .25), chorus=U(.3, .7), phaser=C(0, 0, .3), revmix=U(.15, .4),
            dlymix=U(0, .2)),
    7: dict(o1wave=C(2, 2, 3), o2wave=2, o2fine=U(4, 12), o2level=U(.6, .95), sub=U(.1, .4), uni=C(2, 3),
            unidetune=U(.15, .35), ftype=1, cutoff=L(500, 1800), reso=U(.05, .3), keytrack=U(.3, .7),
            fenvamt=U(.45, .9), fA=U(40, 160), fD=U(250, 700), fS=U(.35, .7), fR=U(100, 300), aA=U(25, 120),
            aD=U(250, 700), aS=U(.6, .9), aR=U(100, 320), lfo1pitch=U(0, .12), lfo1rate=U(4.5, 6), glide=U(0, 60),
            mode=C(0, 0, 2), drive=U(.25, .5), wobble=U(.05, .2), chorus=U(.15, .4), revmix=U(.15, .35)),
    8: dict(o1wave=C(1, 2, 0, 3), o1oct=C(0, 1), o2wave=C(1, 0), o2oct=C(1, 2, 0), o2semi=C(0, 7, 12, 5),
            o2level=U(.2, .6), sub=U(0, .2), uni=C(1, 2, 3), ftype=C(0, 1), cutoff=L(900, 5000), reso=U(.1, .45),
            keytrack=U(.5, 1), fenvamt=U(.4, .9), fA=1, fD=L(80, 500), fS=0, fR=U(80, 300), aA=U(1, 3),
            aD=L(150, 900), aS=U(0, .1), aR=U(100, 500), mode=0, drive=U(0, .3), chorus=U(.15, .5),
            dlytime=C(1, 2, 3), dlymix=U(.15, .4), dlyfb=U(.3, .55), revmix=U(.15, .4)),
    9: dict(arp=C(1, 1, 2, 3, 4), arprate=C(1, 3, 3, 4, 5), arpoct=C(1, 2, 3, 4), arpgate=U(.3, .85),
            o1wave=C(2, 3, 2, 1), o2wave=C(2, 3), o2fine=U(3, 12), o2level=U(.3, .7), uni=C(1, 2, 3), ftype=1,
            cutoff=L(1000, 5000), reso=U(.15, .5), keytrack=U(.4, .9), fenvamt=U(.3, .8), fD=L(80, 400),
            fS=U(0, .3), aA=U(1, 3), aD=L(120, 500), aS=U(0, .5), aR=U(60, 300), mode=0, drive=U(.1, .4),
            chorus=U(.2, .5), dlytime=C(1, 2, 3), dlymix=U(.2, .5), dlyfb=U(.3, .6), revmix=U(.15, .4)),
}

FLAVOURS = [
    ("Classic", ["Classic", "Original", "Core", "Street"]),
    ("Dark", ["Dark", "Midnight", "Shadow", "Smoked"]),
    ("Bright", ["Bright", "Chrome", "Shiny", "Crystal"]),
    ("Wide", ["Wide", "Cruisin'", "Panorama", "Boulevard"]),
    ("Dirty", ["Dirty", "Gritty", "Gangsta", "Rusty"]),
    ("Spacey", ["Spacey", "Neon", "Orbit", "Hologram"]),
    ("Vintage", ["Vintage", "Tape", "Old School", "Dusty"]),
    ("Fat", ["Fat", "Heavy", "Big Body", "Thick"]),
    ("Punchy", ["Punchy", "Snappy", "Hydraulic", "Slap"]),
    ("Smooth", ["Smooth", "Silky", "Velvet", "Buttery"]),
]

NOUNS = [
    ["Whistle", "Siren", "Slide", "Cruiser", "Glider", "Canyon Call", "Hilltop", "Sunset Whine", "Westside Cry",
     "Highway Wail", "Skyline", "Bounce Lead", "Palm Tree", "Pier Whistle", "Dre Day", "Compton Air", "Low Rider Cry",
     "Chrome Flute", "Mirage", "Hood Hymn", "Golden Hour", "Ocean Ave", "Venice", "Sunroof", "Candy Whine"],
    ["Lead", "Saw Lead", "Square Lead", "Cyber Lead", "Laser", "Stab Lead", "Melody", "Hook", "Funk Lead", "Streetlight",
     "Neon Lead", "Drive Lead", "Hyper Lead", "Ride Lead", "Skyway", "Blaze", "Reign", "Rooftop", "Coupe", "Anthem",
     "Flash", "Voltage", "Gold Chain", "Bling Lead", "Spotlight"],
    ["Sub", "Sub Boom", "808 Sub", "Trunk Rattle", "Low End", "Rumble", "Woofer", "Deep Cut", "Subwoofer", "Bass Drop",
     "Earthquake", "Basement", "Thump", "Round Bass", "Pillow Sub", "Boom", "Low Rider Sub", "Concrete", "Undertow",
     "Heavy Sub", "Seismic", "Bunker", "Tremor", "Gravity", "Pressure"],
    ["Boogie Bass", "Funk Bass", "Pluck Bass", "Moog Bass", "Slap Bass", "Walker", "Groove Bass", "Bounce Bass",
     "Pocket", "Shuffle Bass", "Bootsy", "Rubber Bass", "Squelch", "Gangsta Bass", "Cruise Bass", "Hustle", "Swagger",
     "Walking Bass", "Funkadelic", "Low Slide", "Wobble Bass", "Stomp", "Roll", "Bump", "Grease"],
    ["Talkbox", "Vocoder", "Voice", "Robot Choir", "Wah Voice", "Vowel", "Speak", "Cyber Vox", "Roger", "Formant",
     "Whisper", "Singer", "Mouth Synth", "Chant", "Cry Baby", "Droid", "Voicebox", "Human Lead", "Soul Talk", "Choir",
     "Oooh", "Aaah", "Speaker", "Intercom", "Radio Voice"],
    ["Pad", "Strings", "Atmosphere", "Wash", "Dream", "Cloud", "Ambient", "Bed", "Orchestra", "Haze", "Drift",
     "Choir Pad", "Horizon", "Aurora", "Night Drive", "Lowrider Pad", "Palm Pad", "Smog", "Velvet Pad", "Nebula",
     "Moonlight", "Tides", "Skyline Pad", "Heat Haze", "Starfield"],
    ["Keys", "E-Piano", "Rhodes", "Clav", "Organ", "Wurly", "Funk Keys", "Soul Keys", "Chords", "Stabs", "Piano Synth",
     "Bell Keys", "Glass Keys", "Dream Keys", "Sunday Keys", "Barbershop", "Lounge", "Church Keys", "Smooth Keys",
     "Jazz Keys", "Parlor", "Motown", "Coupe Keys", "Velour", "Piano Bar"],
    ["Brass", "Horns", "Trumpet", "Sax Synth", "Stab", "Fanfare", "Big Band", "Horn Section", "Blast", "Bone",
     "Brass Hit", "Pharaoh", "Parade", "Funk Horns", "Swell", "Shout", "Bullhorn", "Siren Brass", "Mariachi Synth",
     "Low Rider Horns", "Cavalry", "Royal", "Fire Brass", "Gold Horn", "Punch Brass"],
    ["Pluck", "Bell", "Marimba", "Kalimba", "Harp", "Glass", "Chime", "Zap", "Drop", "Ping", "Pizzicato", "Music Box",
     "Droplet", "Koto", "Steel Drum", "Echo Pluck", "Ice", "Spark", "Tick", "Crystal Bell", "Gem", "Tinkle", "Diamond",
     "Sparkle", "Bling Pluck"],
    ["Arp", "Sequence", "Pulse", "Run", "Chase", "Cascade", "Climb", "Orbit", "Gate", "Ripple", "Rush", "Flow",
     "Cruise Arp", "Night Run", "Strobe", "Bounce Arp", "Chain", "Cycle", "Spiral", "Motor", "Highway Arp", "Stream",
     "Pattern", "Trance Gate", "Roller"],
]

SIGNATURE = {
    (0, 0): ("2027 Whistle Lead", dict(glide=180, o1wave=.15, sub=0, cutoff=5200, reso=.2, fenvamt=.1, lfo1rate=5.5,
             lfo1pitch=.25, aA=8, aD=300, aS=.85, aR=260, drive=.25, wobble=.2, chorus=.35, revmix=.3, mode=2,
             o2level=0, uni=1)),
    (1, 0): ("Nate West Coast Synth", dict(glide=120, o1wave=2, o2wave=2, o2fine=9, o2level=.55, sub=.1, cutoff=3000,
             reso=.3, fenvamt=.3, lfo1rate=4.8, lfo1pitch=.15, aA=5, aD=400, aS=.7, aR=300, drive=.35, wobble=.3,
             chorus=.4, revmix=.25, mode=2, uni=3, unidetune=.3)),
    (3, 0): ("Sub-Boogie Bass", dict(glide=60, o1wave=2, o1oct=-1, o2wave=3, o2oct=-1, o2level=.4, sub=.8, cutoff=420,
             reso=.35, fenvamt=.55, aA=2, aD=260, aS=.5, aR=120, drive=.5, wobble=.1, chorus=0, revmix=.05, mode=1)),
    (4, 0): ("Cyber Talkbox Pad", dict(glide=20, o1wave=2, o2wave=2, o2fine=10, o2level=.6, sub=.2, ftype=2,
             cutoff=1400, reso=.65, fenvamt=.2, lfo1rate=.35, lfo1cut=.45, lfo1shape=0, aA=700, aD=900, aS=.8,
             aR=1800, drive=.3, wobble=.35, chorus=.7, revmix=.6, uni=3, mode=0)),
    (7, 0): ("LA Lowrider Brass", dict(glide=25, o1wave=2, o2wave=2, o2fine=8, o2level=.8, sub=.3, cutoff=900,
             reso=.15, fenvamt=.7, fA=60, fD=450, fS=.5, aA=60, aD=450, aS=.65, aR=220, lfo1pitch=.08, drive=.4,
             wobble=.15, chorus=.25, revmix=.3, uni=2, mode=0)),
}


def flavour(d, f, rng):
    def add(k, v):
        d[k] = d[k] + v

    if f == 1:
        d["cutoff"] *= .55; d["tone"] = -.3; d["reso"] *= .9
    elif f == 2:
        d["cutoff"] *= 1.7; d["tone"] = .3
    elif f == 3:
        d["uni"] = max(d["uni"], 3); d["unispread"] = .9; add("chorus", .25)
    elif f == 4:
        add("drive", .35); add("fdrive", .25)
    elif f == 5:
        add("revmix", .25); add("dlymix", .2); d["revsize"] = .75
    elif f == 6:
        add("wobble", .3); d["tone"] = -.2; add("drive", .15)
    elif f == 7:
        add("sub", .3); add("unidetune", .2); add("o2level", .2)
    elif f == 8:
        d["aA"] = rng.uniform(1, 3); d["aD"] = min(d["aD"], 350); add("fenvamt", .2)
    elif f == 9:
        add("aA", 20); d["reso"] *= .6; d["cutoff"] *= .8


def make_presets():
    import json, os
    trim_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "volume_trim.json")
    trim = json.load(open(trim_path)) if os.path.exists(trim_path) else None
    rng = random.Random(2027)
    presets = []
    used = set()
    defaults = {p[0]: p[5] for p in PARAMS}
    for cat in range(10):
        tpl = TEMPLATES[cat]
        for i in range(PER_CAT):
            d = dict(defaults)
            d["tone"] = 0
            for k, v in tpl.items():
                d[k] = draw(rng, v)
            fl = i % len(FLAVOURS)
            flavour(d, fl, rng)
            if (cat, i) in SIGNATURE:
                name, over = SIGNATURE[(cat, i)]
                d = dict(defaults)
                d.update(over)
            else:
                for _ in range(200):
                    adj = rng.choice(FLAVOURS[fl][1])
                    name = f"{adj} {rng.choice(NOUNS[cat])}"
                    if name not in used:
                        break
                else:
                    name = f"{name} {i}"
            used.add(name)
            if trim:
                d["volume"] = min(1.0, max(0.1, d["volume"] * trim[len(presets)]))
            vals = []
            for p in PARAMS:
                pid, _l, typ, lo, hi = p[0], p[1], p[2], p[3], p[4]
                v = float(d[pid])
                v = min(max(v, lo), hi)
                if typ in ("i", "c"):
                    v = float(round(v))
                vals.append(round(v, 4))
            presets.append((name, cat, vals))
    return presets


if __name__ == "__main__":
    ps = make_presets()
    print(len(ps), len({p[0] for p in ps}))
    for p in ps[:3] + ps[500:503]:
        print(p[0], CATEGORIES[p[1]])
