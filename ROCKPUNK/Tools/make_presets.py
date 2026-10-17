"""Ecrit la banque de presets d'usine de ROCKPUNK.

Un preset .rpk est l'etat APVTS en XML. Chaque fichier porte tous les
parametres, sans exception : replaceState ne touche pas ceux qui manquent, donc
un preset partiel laisserait trainer les reglages du preset precedent.

Aucun preset d'usine n'utilise la couche echantillonnee (smpOn reste a 0) :
ROCKPUNK ne livre aucun enregistrement, et ces sons doivent fonctionner sur une
installation neuve, dossier d'echantillons vide. Tout vient des deux cordes
modelisees, de l'ampli, du drive, du filtre, du LFO, du delay et de la reverb.

Les presets sont ecrits par famille plutot qu'un a un : la famille porte le
caractere commun (etage de gain, forme d'enveloppe, espace) et le preset ne
declare que son ecart a cette base. C.est ce qui rend 76 sons relisibles.

Les noms sont dans l'esprit rock'n'roll / AC-DC mais inventes : reprendre des
titres de chansons existants sur un produit vendu n'apporte rien et expose.

    python make_presets.py [dossier de sortie]
"""

import os
import sys

# ---------------------------------------------------------------------------
# Valeurs par defaut de Source/Params.h : tout preset part de la.
# ---------------------------------------------------------------------------
DEFAULTS = {
    "layerMix": 50, "masterVol": -3, "outWidth": 80, "outLimiter": 1,
    "smpOn": 0, "smpSel": 0, "smpRoot": 0, "smpLevel": 0.8,
    "smpLoop": 0, "smpAttack": 5, "smpRelease": 250,
    "laOn": 1, "laBlend": 80, "laTone": 60, "laBody": 30,
    "laSustain": 35, "laPick": 30, "laLevel": 1,
    "lbOn": 1, "lbBlend": 90, "lbTone": 25, "lbBody": 25,
    "lbSustain": 55, "lbPick": 70, "lbLevel": 1,
    "ampOn": 1, "ampGain": 6, "ampDist": 55, "ampTone": 2500, "ampLevel": 0,
    "driveAmt": 20, "driveType": 0, "driveTone": 2500,
    "delayOn": 0, "delayTime": 350, "delaySync": 0, "delayDiv": 2,
    "delayFb": 40, "delayMix": 25, "delayLP": 5000,
    "envA": 3, "envD": 600, "envS": 70, "envR": 220,
    "glideMs": 0, "monoMode": 0,
    "revMix": 18, "revSize": 60, "chorAmt": 0,
    "lfoRate": 5, "lfoAmount": 0, "lfoPitch": 0, "lfoWave": 0, "lfoDest": 1,
    "fltType": 3, "fltCutoff": 14000, "fltReso": 0.9,
    "fenvA": 5, "fenvD": 400, "fenvS": 100, "fenvR": 300,
    "velAmt": 60, "pbend": 0, "modDepth": 0, "pickShape": 1,
}

SOFT, HARD, ASYM, FUZZ = 0, 1, 2, 3             # driveType
LP, BP, HP, NOFILT = 0, 1, 2, 3                 # fltType
SINE, TRI, SQR, SH = 0, 1, 2, 3                 # lfoWave
D_PITCH, D_FILTER, D_VOL, D_DRIVE = 0, 1, 2, 3  # lfoDest
DOUX, ROND, CLAIR, TRANCHANT = 0, 1, 2, 3       # pickShape

# ---------------------------------------------------------------------------
# Familles : l'etage de gain, la forme d'enveloppe et l'espace communs.
# ---------------------------------------------------------------------------
FAMILIES = {
    # Rythmiques : attaque seche, note courte, peu de queue.
    "crunch": dict(
        layerMix=46, pickShape=CLAIR,
        laTone=62, laBody=26, laSustain=40, laPick=58, laBlend=90,
        lbTone=36, lbBody=28, lbSustain=48, lbPick=74, lbBlend=84,
        ampGain=15, ampDist=62, ampTone=3100, driveAmt=34, driveType=HARD,
        envA=2, envD=320, envS=36, envR=180, revMix=12),

    # Solos : ca tient, ca chante, ca respire.
    "lead": dict(
        layerMix=70, pickShape=CLAIR,
        laTone=58, laBody=20, laSustain=62, laPick=45, laBlend=72,
        lbTone=52, lbBody=18, lbSustain=88, lbPick=64, lbBlend=100,
        ampGain=18, ampDist=66, ampTone=4100, driveAmt=36, driveType=ASYM,
        envA=4, envD=1400, envS=85, envR=600, revMix=24,
        delayOn=1, delaySync=1, delayDiv=2, delayFb=45, delayMix=30),

    # Gros gain : compresse, dense, agressif.
    "fuzz": dict(
        layerMix=54, pickShape=TRANCHANT,
        laTone=54, laBody=22, laSustain=48, laPick=76, laBlend=94,
        lbTone=42, lbBody=20, lbSustain=64, lbPick=90, lbBlend=96,
        ampGain=22, ampDist=88, ampTone=3400, driveAmt=58, driveType=FUZZ,
        envA=2, envD=900, envS=78, envR=380, revMix=13),

    # Clairs et mi-clairs : l'ampli sert surtout de gain propre.
    "clean": dict(
        layerMix=47, pickShape=ROND, outWidth=95,
        laTone=76, laBody=30, laSustain=54, laPick=46, laBlend=88,
        lbTone=58, lbBody=24, lbSustain=64, lbPick=58, lbBlend=74,
        ampGain=12, ampDist=12, ampTone=5200, driveAmt=4,
        envA=4, envD=950, envS=70, envR=480, revMix=26,
        delayOn=1, delaySync=1, delayDiv=2, delayFb=28, delayMix=20, delayLP=6500),

    # Cordes nues : aucune distorsion, la caisse parle.
    "bare": dict(
        layerMix=42, pickShape=DOUX, outWidth=85,
        laTone=70, laBody=58, laSustain=68, laPick=34, laBlend=100,
        lbTone=52, lbBody=48, lbSustain=76, lbPick=46, lbBlend=80,
        ampOn=1, ampGain=16, ampDist=0, ampTone=9000, driveAmt=0,
        envA=6, envD=2400, envS=86, envR=1200, revMix=34,
        delayOn=1, delaySync=1, delayDiv=4, delayFb=25, delayMix=15, delayLP=6000),

    # Nocturnes : sombre, filtre, espace.
    "night": dict(
        layerMix=56, pickShape=ROND,
        laTone=42, laBody=48, laSustain=58, laPick=28, laBlend=88,
        lbTone=24, lbBody=42, lbSustain=68, lbPick=42, lbBlend=82,
        ampGain=13, ampDist=48, ampTone=1600, driveAmt=22, driveTone=1800,
        envA=18, envD=1500, envS=72, envR=760, revMix=30,
        fltType=LP, fltCutoff=2600, fenvA=90, fenvD=1200, fenvS=70,
        delayOn=1, delaySync=1, delayDiv=2, delayFb=46, delayMix=28, delayLP=3000),

    # Textures : le LFO et le filtre sont le sujet, pas l'assaisonnement.
    "texture": dict(
        layerMix=58, pickShape=ROND, outWidth=100,
        laTone=64, laBody=34, laSustain=62, laPick=40, laBlend=88,
        lbTone=46, lbBody=30, lbSustain=74, lbPick=56, lbBlend=88,
        ampGain=12, ampDist=38, ampTone=3400, driveAmt=18,
        envA=8, envD=1600, envS=80, envR=700, revMix=32,
        lfoAmount=40, lfoRate=3.0, fltType=BP, fltCutoff=1500, fltReso=1.8,
        delayOn=1, delaySync=1, delayDiv=2, delayFb=42, delayMix=26),

    # Graves : power chords, corps profond, peu d'aigu.
    "low": dict(
        layerMix=30, pickShape=ROND,
        laTone=44, laBody=68, laSustain=54, laPick=44, laBlend=100,
        lbTone=26, lbBody=56, lbSustain=60, lbPick=58, lbBlend=76,
        ampGain=19, ampDist=74, ampTone=2100, driveAmt=42, driveType=HARD,
        envA=3, envD=520, envS=48, envR=280, revMix=14),
}

# ---------------------------------------------------------------------------
# Les 76 presets : (nom, famille, ce qui le distingue).
# ---------------------------------------------------------------------------
PRESETS = [
    # ---- rythmiques crunch ------------------------------------------------
    ("Schoolyard Crunch",   "crunch", {}),
    ("Dirty Alley Riff",    "crunch", dict(ampGain=17, ampDist=72, driveType=FUZZ,
                                           driveTone=3400, pickShape=TRANCHANT,
                                           fltType=HP, fltCutoff=180)),
    ("Bar Fight Boogie",    "crunch", dict(ampGain=15, ampDist=60, laPick=62, lbPick=74)),
    ("Blacktop Shuffle",    "crunch", dict(ampGain=9, ampDist=35, ampTone=3600, driveAmt=18,
                                           lfoDest=D_VOL, lfoAmount=22, lfoRate=5.5,
                                           lfoWave=TRI)),
    ("Stadium Stomp",       "crunch", dict(outWidth=100, envD=520, envS=50, revMix=26,
                                           delayOn=1, delaySync=1, delayDiv=4,
                                           delayFb=32, delayMix=30, delayLP=4200)),
    ("Highway Dust",        "crunch", dict(ampTone=2700, laBody=38, envD=380)),
    ("Loud And Guilty",     "crunch", dict(ampGain=18, ampDist=70, driveAmt=44)),
    ("Steel Toe Boots",     "crunch", dict(laBody=44, lbBody=40, ampTone=2400, envD=260)),
    ("Rattlecan Red",       "crunch", dict(pickShape=TRANCHANT, ampTone=3800, envD=240)),
    ("Curfew Breaker",      "crunch", dict(ampGain=16, driveType=ASYM, driveAmt=40, envS=44)),
    ("Two Bar Warning",     "crunch", dict(delayOn=1, delaySync=1, delayDiv=0,
                                           delayFb=24, delayMix=16, envD=300)),
    ("Half Past Loud",      "crunch", dict(ampGain=17, ampTone=3400, laBlend=96, lbBlend=92)),
    ("Smokestack Strut",    "crunch", dict(ampTone=2900, laPick=52, envD=420, revMix=18)),
    ("Rust Belt Anthem",    "crunch", dict(outWidth=100, ampGain=16, envD=600, envS=56,
                                           revMix=24, delayOn=1, delaySync=1, delayDiv=4,
                                           delayFb=30, delayMix=24)),

    # ---- solos ------------------------------------------------------------
    ("Leather Jacket Lead", "lead",   dict(lfoPitch=14, lfoRate=5.2)),
    ("The Long Encore",     "lead",   dict(envD=2200, envS=90, envR=900, lfoPitch=20,
                                           lfoRate=4.6, modDepth=10, delayDiv=4,
                                           delayFb=60, delayMix=40, delayLP=4000, revMix=30)),
    ("Last Call",           "lead",   dict(envA=35, envD=1800, envS=88, envR=1100,
                                           ampGain=13, ampDist=52, driveAmt=22,
                                           pickShape=DOUX, lfoPitch=26, lfoRate=4.2,
                                           delayDiv=4, delayFb=55, delayMix=38, revMix=34)),
    ("Screaming Treble",    "lead",   dict(laTone=78, lbTone=72, ampTone=6500,
                                           driveTone=5200, pickShape=TRANCHANT,
                                           fltType=HP, fltCutoff=320)),
    ("Third Encore",        "lead",   dict(envR=1000, delayFb=58, delayMix=36, revMix=32)),
    ("Pocket Knife Lead",   "lead",   dict(ampGain=20, ampDist=72, lbSustain=92,
                                           pickShape=TRANCHANT)),
    ("Gutter Star",         "lead",   dict(ampTone=3600, laBody=26, lfoPitch=10)),
    ("Barbed Wire Bend",    "lead",   dict(pbend=0.5, lfoPitch=18, lfoRate=6.0, glideMs=45)),
    ("Nine Volt Heart",     "lead",   dict(ampGain=16, ampDist=58, envD=1800, envS=88,
                                           revMix=30, chorAmt=22)),
    ("Siren Lead",          "lead",   dict(glideMs=140, monoMode=1, lfoPitch=22, lfoRate=5.6)),
    ("Empty Stage",         "lead",   dict(envA=40, envR=1400, delayDiv=4, delayFb=62,
                                           delayMix=42, revMix=40, revSize=80)),
    ("Backstage Pass",      "lead",   dict(ampTone=4600, lbPick=72, delayDiv=1,
                                           delayFb=48, delayMix=32)),

    # ---- gros gain --------------------------------------------------------
    ("Amp On Fire",         "fuzz",   dict(ampGain=24, ampDist=95, driveAmt=65,
                                           fltType=BP, fltCutoff=1800, fltReso=2.2)),
    ("Six String Riot",     "fuzz",   dict(ampGain=22, ampDist=85, lbSustain=82,
                                           fltType=LP, fltCutoff=6000, fenvA=1,
                                           fenvD=500, fenvS=70, outWidth=90)),
    ("Whiskey Throttle",    "fuzz",   dict(driveType=ASYM, ampGain=20, ampDist=80,
                                           ampTone=2800, driveAmt=55)),
    ("Gravel And Gasoline", "fuzz",   dict(driveType=ASYM, driveTone=2100, ampTone=2600,
                                           laBody=55, layerMix=44)),
    ("Broken Bottle Blues", "fuzz",   dict(ampTone=2900, envD=700, envS=62, revMix=18)),
    ("Iron Lung Crunch",    "fuzz",   dict(laBody=50, lbBody=44, ampTone=2300)),
    ("Wrecking Yard",       "fuzz",   dict(ampGain=23, driveAmt=68, fltType=HP, fltCutoff=220)),
    ("Bad Ignition",        "fuzz",   dict(lfoDest=D_DRIVE, lfoAmount=45, lfoRate=7.5,
                                           lfoWave=SH)),
    ("Full Throttle Choir", "fuzz",   dict(outWidth=100, chorAmt=30, revMix=22, envS=84)),
    ("Sweat And Sparks",    "fuzz",   dict(ampTone=3800, pickShape=TRANCHANT, envD=620)),

    # ---- clairs / mi-clairs ----------------------------------------------
    ("Clean Getaway",       "clean",  dict(ampGain=13, ampDist=8, fltType=LP, fltCutoff=7000,
                                           fenvA=90, fenvD=1200, fenvS=80, chorAmt=30)),
    ("Chrome Bumper",       "clean",  dict(outWidth=100, ampGain=12, ampDist=15, ampTone=4800,
                                           chorAmt=45, lfoDest=D_FILTER, lfoAmount=30,
                                           lfoRate=0.9, fltType=LP, fltCutoff=5200,
                                           delayDiv=3, delayFb=30, delayMix=22)),
    ("First Gear",          "clean",  dict(ampGain=11, ampDist=18, ampTone=4000, driveAmt=10,
                                           pickShape=CLAIR, delayOn=0)),
    ("Train Yard Twang",    "clean",  dict(laTone=84, lbTone=74, laPick=78, lbPick=95,
                                           pickShape=TRANCHANT, ampTone=5600,
                                           delayDiv=0, delayFb=22, delayMix=18, delayLP=7000)),
    ("Back Row Boogie",     "clean",  dict(ampGain=10, ampDist=25, ampTone=4400, driveAmt=14,
                                           delayDiv=0, delayFb=25, delayMix=18)),
    ("Cold Start Clean",    "clean",  dict(ampGain=10, ampDist=6, revMix=20, delayOn=0)),
    ("Lipstick Pickup",     "clean",  dict(laTone=82, ampTone=6000, chorAmt=35, revMix=30)),
    ("Motel Neon",          "clean",  dict(chorAmt=40, delayDiv=3, delayFb=38, delayMix=28,
                                           revMix=32)),
    ("Paper Cup Whiskey",   "clean",  dict(ampDist=22, ampTone=4200, envD=700, revMix=22)),
    ("Alley Cat Strut",     "clean",  dict(pickShape=CLAIR, laPick=70, ampTone=5000,
                                           delayDiv=0, delayMix=16)),
    ("Cellar Door",         "clean",  dict(ampTone=3400, laBody=44, revMix=34, revSize=78)),
    ("Closing Time Chords", "clean",  dict(envA=20, envD=1600, envR=900, chorAmt=25,
                                           revMix=36, delayDiv=4, delayFb=34, delayMix=26)),

    # ---- cordes nues ------------------------------------------------------
    ("Sunday Sermon",       "bare",   {}),
    ("Porch Light",         "bare",   dict(layerMix=38, outWidth=70, laTone=64, laBody=56,
                                           lbBlend=70, delayOn=0, revMix=30)),
    ("Open Road Steel",     "bare",   dict(layerMix=46, outWidth=95, laTone=80, lbTone=68,
                                           pickShape=CLAIR, ampGain=12, driveAmt=5,
                                           delayDiv=4, delayFb=30, delayMix=20,
                                           delayLP=7000, revMix=36)),
    ("Double Denim",        "bare",   dict(layerMix=58, outWidth=90, ampGain=10, ampDist=10,
                                           ampTone=5000, driveAmt=6, pickShape=ROND,
                                           delayOn=0, revMix=24)),
    ("Ashtray Ballad",      "bare",   dict(envA=25, envR=1600, revMix=38, revSize=78)),
    ("Slow Burn Sunday",    "bare",   dict(laBody=64, envD=2800, envR=1500, revMix=36)),
    ("Tin Roof Rumble",     "bare",   dict(layerMix=32, laBody=70, laTone=58, revMix=30)),
    ("Velvet Bruise",       "bare",   dict(pickShape=DOUX, laTone=62, chorAmt=28, revMix=38)),

    # ---- nocturnes --------------------------------------------------------
    ("Midnight Mechanic",   "night",  {}),
    ("Neon Garage",         "night",  dict(ampGain=13, ampDist=55, ampTone=3000, driveAmt=26,
                                           driveType=HARD, pickShape=CLAIR,
                                           lfoDest=D_FILTER, lfoAmount=45, lfoRate=3.4,
                                           lfoWave=SH, fltType=BP, fltCutoff=1400,
                                           fltReso=1.8, delayDiv=1, delayFb=42,
                                           delayMix=28, delayLP=4400, revMix=24)),
    ("Nightshift Riff",     "night",  dict(fltCutoff=3200, ampDist=56, envD=900, envS=60)),
    ("Cheap Hotel",         "night",  dict(fltCutoff=1800, revMix=34, delayFb=52)),
    ("Static Highway",      "night",  dict(lfoDest=D_DRIVE, lfoAmount=32, lfoRate=2.2,
                                           lfoWave=SH, fltCutoff=3000)),
    ("Last Bus Home",       "night",  dict(envA=30, envR=1200, delayDiv=4, delayFb=54,
                                           delayMix=32, revMix=36)),
    ("Diesel Sunday",       "night",  dict(ampGain=15, ampTone=1900, laBody=56,
                                           fltCutoff=2200)),
    ("Amp In The Rain",     "night",  dict(chorAmt=30, revMix=38, revSize=82, fltCutoff=2400)),

    # ---- textures ---------------------------------------------------------
    ("Feedback Chapel",     "texture", dict(envA=60, envR=1800, revMix=44, revSize=88,
                                            lfoDest=D_FILTER, lfoAmount=55, lfoRate=1.2)),
    ("Warehouse Echo",      "texture", dict(delayDiv=4, delayFb=62, delayMix=40,
                                            revMix=36, lfoAmount=30, lfoRate=0.8)),
    ("Dust Devil",          "texture", dict(lfoWave=SH, lfoAmount=60, lfoRate=6.5,
                                            fltCutoff=1200, fltReso=2.4)),
    ("Overdrive Sunset",    "texture", dict(lfoDest=D_DRIVE, lfoAmount=40, lfoRate=1.6,
                                            lfoWave=SINE, ampGain=17, ampDist=62,
                                            fltType=NOFILT)),
    ("Rooftop Riot",        "texture", dict(lfoDest=D_VOL, lfoAmount=55, lfoRate=7.0,
                                            lfoWave=SQR, fltType=NOFILT, ampGain=16,
                                            ampDist=60)),
    ("Chrome And Bone",     "texture", dict(chorAmt=50, fltType=LP, fltCutoff=4200,
                                            fenvA=120, fenvD=1400, fenvS=65, revMix=34)),
    ("Dead Battery",        "texture", dict(glideMs=260, monoMode=1, lfoPitch=16,
                                            fltCutoff=1100, revMix=30)),
    ("Nightshift Wah",      "texture", dict(lfoDest=D_FILTER, lfoAmount=70, lfoRate=2.6,
                                            lfoWave=TRI, fltType=BP, fltCutoff=900,
                                            fltReso=3.0)),

    # ---- graves -----------------------------------------------------------
    ("Rusty Thunder",       "low",    dict(fltType=LP, fltCutoff=3200, fenvD=900, fenvS=55)),
    ("Iron Ballast",        "low",    dict(laBody=74, ampTone=1900, envD=620)),
    ("Anvil Chorus",        "low",    dict(chorAmt=26, outWidth=95, ampTone=2300)),
    ("Bottom Feeder",       "low",    dict(layerMix=24, laBody=76, ampGain=20, ampDist=78)),
]

# ---------------------------------------------------------------------------
# Volumes de sortie, cales au banc par Tools/balance_presets.py.
# C'est le seul tableau que ce script-la modifie : les reglages musicaux
# ci-dessus lui restent hors de portee.
# ---------------------------------------------------------------------------
MASTER = {
    "01": -7.1,
    "02": -9.5,
    "03": -6.6,
    "04": 1.1,
    "05": -6.3,
    "06": -6,
    "07": -9.4,
    "09": -5.7,
    "10": -7.2,
    "11": -6.6,
    "12": -8.4,
    "13": -7.2,
    "14": -9.6,
    "15": -8,
    "16": -8.4,
    "17": -3.8,
    "18": -12.9,
    "19": -12.5,
    "20": -13.5,
    "21": -12.5,
    "22": -11.5,
    "23": -9.1,
    "24": -12.2,
    "25": -14.1,
    "26": -14.7,
    "27": -10,
    "28": -16.7,
    "29": -13.3,
    "30": -14.2,
    "31": -16.1,
    "32": -15.6,
    "33": -19.2,
    "34": -16.3,
    "35": -14.7,
    "36": -19.2,
    "37": 3.7,
    "38": 4.3,
    "39": -0.4,
    "41": -0.4,
    "43": -0,
    "44": 1.7,
    "48": -2.6,
    "49": 2.9,
    "51": 0.1,
    "52": -0.2,
    "57": 5.6,
    "58": 6,
    "59": -0.2,
    "60": -0.4,
    "61": -0.4,
    "62": -0.2,
    "64": -2.2,
    "65": 6,
    "66": 6,
    "67": -0.5,
    "68": -9.2,
    "69": -7.1,
    "72": -7.8,
    "73": -4.9,
    "74": -9.4,
    "75": -9.1,
    "76": -9.5,
}


# Ce qui separe deux sons a l'oreille, dans l'ordre : la brillance des cordes,
# leur corps, la durete du mediator, la longueur de la note et le dosage entre
# les deux cordes. Un preset qui ne change que le gain de l'ampli reste le meme
# son en plus fort.
#
# Chaque preset recoit donc un ecart propre a son rang dans sa famille, reparti
# sur ces cinq axes. Sans cela quatorze rythmiques crunch se ressemblaient
# toutes : elles partageaient la base de famille et ne differaient que par
# quelques decibels.
SPREAD = {
    "laTone":   28.0,
    "lbTone":   24.0,
    "laBody":   26.0,
    "lbBody":   20.0,
    "laPick":   26.0,
    "lbPick":   22.0,
    "layerMix": 22.0,
    "envD":     0.55,     # relatif, pas absolu : les decroissances vont de 240 a 2800 ms
    "ampTone":  0.35,     # relatif aussi
}


def variation(rank, count):
    """Position du preset dans sa famille, de -1 a +1."""
    if count < 2:
        return 0.0
    return 2.0 * rank / (count - 1) - 1.0


def spread_values(values, rank, count):
    """Ecarte le preset des autres de sa famille sur les axes audibles."""
    v = variation(rank, count)

    # Les axes ne bougent pas tous dans le meme sens : un preset brillant est
    # aussi plus sec et plus claquant, ce qui fait des caracteres tranches
    # plutot qu'un degrade uniforme.
    directions = {
        "laTone": 1.0, "lbTone": 1.0, "laPick": 1.0, "lbPick": 0.8,
        "laBody": -1.0, "lbBody": -0.8, "layerMix": 0.6,
        "envD": -1.0, "ampTone": 1.0,
    }

    for key, amount in SPREAD.items():
        if key not in values:
            continue
        d = directions.get(key, 1.0) * v
        if key in ("envD", "ampTone"):
            values[key] = round(values[key] * (1.0 + amount * d), 1)
        else:
            values[key] = round(min(100.0, max(0.0, values[key] + amount * d)), 1)

    return values


def settings_for(index, family, overrides, rank=0, count=1):
    values = dict(DEFAULTS)
    values.update(FAMILIES[family])
    spread_values(values, rank, count)

    unknown = set(overrides) - set(DEFAULTS)
    if unknown:
        raise KeyError("parametre inconnu : %s" % ", ".join(sorted(unknown)))
    values.update(overrides)

    if values["smpOn"]:
        raise ValueError("un preset d'usine ne doit exiger aucun echantillon")

    values["masterVol"] = MASTER.get("%02d" % index, values["masterVol"])
    return values


def write_preset(directory, index, name, values):
    lines = ['<?xml version="1.0" encoding="UTF-8"?>', "<ROCKPUNK_GUITAR_STATE>"]
    for key in DEFAULTS:                       # ordre stable, lisible en diff
        v = values[key]
        text = ("%g" % v) if isinstance(v, float) else str(v)
        lines.append('  <PARAM id="%s" value="%s"/>' % (key, text))
    lines.append("</ROCKPUNK_GUITAR_STATE>")
    lines.append("")

    with open(os.path.join(directory, "%02d %s.rpk" % (index, name)),
              "w", encoding="utf-8") as f:
        f.write("\n".join(lines))


def main():
    directory = sys.argv[1] if len(sys.argv) > 1 else "Presets"
    os.makedirs(directory, exist_ok=True)

    # Les anciens fichiers sont retires d'abord : la banque a ete renumerotee,
    # et sans cela les presets de l'ancienne serie resteraient a cote des neufs.
    for stale in os.listdir(directory):
        if stale.endswith(".rpk"):
            os.remove(os.path.join(directory, stale))

    totals = {}
    for _, family, _ in PRESETS:
        totals[family] = totals.get(family, 0) + 1

    names = set()
    seen = {}
    for i, (name, family, overrides) in enumerate(PRESETS, start=1):
        if name in names:
            raise ValueError("nom en double : %s" % name)
        names.add(name)

        rank = seen.get(family, 0)
        seen[family] = rank + 1
        write_preset(directory, i, name,
                     settings_for(i, family, overrides, rank, totals[family]))

    counts = {}
    for _, family, _ in PRESETS:
        counts[family] = counts.get(family, 0) + 1

    print("%d presets ecrits dans %s" % (len(PRESETS), os.path.abspath(directory)))
    print("   " + ", ".join("%s %d" % (k, v) for k, v in sorted(counts.items())))


if __name__ == "__main__":
    main()
