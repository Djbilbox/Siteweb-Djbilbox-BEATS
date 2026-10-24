"""Single source of truth for GFUNK 2027: parameters and UI layout.

gen_code.py turns this into C++; gen_art.py uses PANELS to draw the artwork.
"""

# (id, label, type f|i|c, min, max, default, skew, fmt, choices)
PARAMS = [
    ("o1wave",    "WAVE",     "f", 0, 3, 2, 1, "wave", ""),
    ("o1oct",     "OCT",      "i", -2, 2, 0, 1, "oct", ""),
    ("o1level",   "LEVEL",    "f", 0, 1, 0.8, 1, "pct", ""),
    ("pw",        "PULSE W",  "f", 0.05, 0.95, 0.5, 1, "pct", ""),
    ("sub",       "SUB",      "f", 0, 1, 0.2, 1, "pct", ""),
    ("o2wave",    "WAVE",     "f", 0, 3, 2, 1, "wave", ""),
    ("o2oct",     "OCT",      "i", -2, 2, 0, 1, "oct", ""),
    ("o2semi",    "SEMI",     "i", -12, 12, 0, 1, "st", ""),
    ("o2fine",    "FINE",     "f", -100, 100, 8, 1, "ct", ""),
    ("o2level",   "LEVEL",    "f", 0, 1, 0.5, 1, "pct", ""),
    ("noise",     "NOISE",    "f", 0, 1, 0, 1, "pct", ""),
    ("uni",       "VOICES",   "i", 1, 7, 1, 1, "int", ""),
    ("unidetune", "DETUNE",   "f", 0, 1, 0.25, 1, "pct", ""),
    ("unispread", "SPREAD",   "f", 0, 1, 0.6, 1, "pct", ""),
    ("ftype",     "TYPE",     "c", 0, 3, 1, 1, "", "LP 12|LP 24|BAND|HIGH"),
    ("cutoff",    "CUTOFF",   "f", 20, 18000, 3000, 0.25, "hz", ""),
    ("reso",      "RESO",     "f", 0, 1, 0.2, 1, "pct", ""),
    ("keytrack",  "KEY TRK",  "f", 0, 1, 0.3, 1, "pct", ""),
    ("fenvamt",   "ENV AMT",  "f", -1, 1, 0.3, 1, "pct", ""),
    ("fdrive",    "DRIVE",    "f", 0, 1, 0.1, 1, "pct", ""),
    ("aA",        "ATTACK",   "f", 1, 8000, 5, 0.35, "ms", ""),
    ("aD",        "DECAY",    "f", 5, 8000, 350, 0.35, "ms", ""),
    ("aS",        "SUSTAIN",  "f", 0, 1, 0.7, 1, "pct", ""),
    ("aR",        "RELEASE",  "f", 5, 10000, 300, 0.35, "ms", ""),
    ("fA",        "ATTACK",   "f", 1, 8000, 2, 0.35, "ms", ""),
    ("fD",        "DECAY",    "f", 5, 8000, 300, 0.35, "ms", ""),
    ("fS",        "SUSTAIN",  "f", 0, 1, 0.3, 1, "pct", ""),
    ("fR",        "RELEASE",  "f", 5, 10000, 250, 0.35, "ms", ""),
    ("lfo1rate",  "RATE",     "f", 0.05, 30, 4, 0.4, "hz", ""),
    ("lfo1shape", "SHAPE",    "c", 0, 4, 0, 1, "", "SINE|TRI|SAW|SQUARE|S&H"),
    ("lfo1cut",   "TO CUT",   "f", -1, 1, 0, 1, "pct", ""),
    ("lfo1pitch", "TO PITCH", "f", 0, 1, 0, 1, "pct", ""),
    ("lfo1pwm",   "TO PWM",   "f", 0, 1, 0, 1, "pct", ""),
    ("lfo2rate",  "RATE",     "f", 0.05, 30, 0.5, 0.4, "hz", ""),
    ("lfo2amp",   "TO AMP",   "f", 0, 1, 0, 1, "pct", ""),
    ("lfo2cut",   "TO CUT",   "f", -1, 1, 0, 1, "pct", ""),
    ("glide",     "GLIDE",    "f", 0, 2000, 0, 0.4, "ms", ""),
    ("mode",      "MODE",     "c", 0, 2, 0, 1, "", "POLY|MONO|LEGATO"),
    ("arp",       "ARP",      "c", 0, 4, 0, 1, "", "OFF|UP|DOWN|UP-DN|RANDOM"),
    ("arprate",   "RATE",     "c", 0, 5, 3, 1, "", "1/4|1/8|1/8T|1/16|1/16T|1/32"),
    ("arpoct",    "OCTAVES",  "i", 1, 4, 2, 1, "int", ""),
    ("arpgate",   "GATE",     "f", 0.1, 1, 0.6, 1, "pct", ""),
    ("drive",     "TAPE",     "f", 0, 1, 0.3, 1, "pct", ""),
    ("wobble",    "WOBBLE",   "f", 0, 1, 0.2, 1, "pct", ""),
    ("phaser",    "PHASER",   "f", 0, 1, 0, 1, "pct", ""),
    ("chorus",    "CHORUS",   "f", 0, 1, 0.3, 1, "pct", ""),
    ("tone",      "TONE",     "f", -1, 1, 0, 1, "pct", ""),
    ("dlytime",   "TIME",     "c", 0, 4, 2, 1, "", "1/16|1/8T|1/8|1/8.|1/4"),
    ("dlyfb",     "FEEDBACK", "f", 0, 0.95, 0.35, 1, "pct", ""),
    ("dlymix",    "DELAY",    "f", 0, 1, 0, 1, "pct", ""),
    ("revsize",   "SIZE",     "f", 0, 1, 0.5, 1, "pct", ""),
    ("revmix",    "REVERB",   "f", 0, 1, 0.2, 1, "pct", ""),
    ("volume",    "VOLUME",   "f", 0, 1, 0.7, 1, "pct", ""),
]

PARAM_INDEX = {p[0]: i for i, p in enumerate(PARAMS)}

CELL_H = 80
TITLE_H = 38

W, H = 1600, 812

# name, title, rect(x,y,w,h), columns, rows of param ids (None = empty cell)
PANELS = [
    ("osc", "OSCILLATORS", (14, 74, 410, 300), 5, [
        ["o1wave", "o1oct", "o1level", "pw", "sub"],
        ["o2wave", "o2oct", "o2semi", "o2fine", "o2level"],
        ["noise", "uni", "unidetune", "unispread", None]]),
    ("screen", "", (434, 74, 700, 300), 1, []),
    ("hero", "", (1144, 74, 442, 300), 1, []),
    ("env", "ENVELOPES", (14, 384, 304, 198), 4, [
        ["aA", "aD", "aS", "aR"],
        ["fA", "fD", "fS", "fR"]]),
    ("mod", "MODULATION", (328, 384, 304, 198), 4, [
        ["lfo1rate", "lfo1shape", "lfo1cut", "lfo1pitch"],
        ["lfo1pwm", "lfo2rate", "lfo2amp", "lfo2cut"]]),
    ("filter", "FILTER", (642, 384, 236, 198), 3, [
        ["ftype", "cutoff", "reso"],
        ["keytrack", "fenvamt", "fdrive"]]),
    ("play", "PLAY / ARP", (888, 384, 236, 198), 3, [
        ["glide", "mode", "arp"],
        ["arprate", "arpoct", "arpgate"]]),
    ("fx", "TAPE / FX / MASTER", (1134, 384, 452, 198), 6, [
        ["drive", "wobble", "phaser", "chorus", "tone", "volume"],
        ["dlytime", "dlyfb", "dlymix", "revsize", "revmix", None]]),
    ("keys", "", (14, 592, 1572, 206), 1, []),
]

HEADER_RECT = (14, 6, 1572, 62)
LCD_RECT = (520, 12, 560, 46)          # preset display in header
BTN_Y, BTN_H = 15, 40                  # header buttons: prev next rand fav
BTN_X = [1100, 1146, 1192, 1238, 1284]
METER_RECT = (1338, 18, 234, 34)
SCREEN_RECT = (434, 74, 700, 300)
SCREEN_INNER = (452, 90, 664, 236)
TABS_RECT = (452, 334, 664, 28)
HERO_RECT = (1144, 74, 442, 300)
KEYS_RECT = (14, 592, 1572, 206)
KEYBOARD_RECT = (130, 638, 1440, 146)
WHEELS_RECT = (34, 638, 84, 146)


def layout_items():
    """Yield dicts with component geometry for every control."""
    items = []
    for name, title, (px, py, pw, ph), cols, rows in PANELS:
        x0, y0 = px + 16, py + TITLE_H
        cw = (pw - 32) / cols
        for r, row in enumerate(rows):
            for c, pid in enumerate(row):
                if pid is None:
                    continue
                p = PARAMS[PARAM_INDEX[pid]]
                cx = x0 + c * cw
                cy = y0 + r * CELL_H
                kind = "combo" if p[2] == "c" else "knob"
                it = dict(id=pid, param=PARAM_INDEX[pid], kind=kind,
                          lx=int(cx) + 2, ly=int(cy), lw=int(cw) - 4, lh=13)
                if kind == "knob":
                    it.update(x=int(cx + cw / 2 - 32), y=int(cy + 14), w=64, h=64)
                else:
                    it.update(x=int(cx) + 4, y=int(cy + 28), w=int(cw) - 8, h=26)
                items.append(it)
    return items

