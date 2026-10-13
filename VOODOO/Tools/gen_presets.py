#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets VOODOO (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.01, d=.4, s=.7, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, rev=.25, cho=0, dis=0,
         dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', rev='reverb', cho='chorus', dis='distortion', dly='delay', tm='time', fb='feedback',
           src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide', pit='pitch', ft='filtertype',
           mono='monopoly', vol='volume')
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Flutes", [
 p("Mystic Reed", eng=0, t1=.4, t3=.45, a=.08, rev=.4, src=0, dst=0, rate=5.2, amt=.12),
 p("Moon Whistle", eng=0, t1=.3, t3=.7, a=.12, r=1.2, rev=.55, dly=.2, tm=.45, pit=12),
 p("Shaman Breath", eng=2, t1=.8, t2=.7, a=.15, r=.9, rev=.45, src=4, dst=3, rate=2, amt=.3),
 p("Jungle Dawn", eng=1, t1=.6, t2=.5, a=.05, r=.8, rev=.5, cho=.25),
 p("Andean Mist", eng=1, t1=.75, t2=.65, a=.1, d=.6, s=.8, r=1.5, rev=.7, dly=.25, tm=.5),
 p("Quena Ghost", eng=2, t1=.55, t2=.4, a=.06, rev=.6, cut=6000, src=0, dst=0, rate=4.5, amt=.2),
 p("Bamboo Spirit", eng=0, t1=.25, t3=.6, a=.02, d=.2, s=.55, r=.4, rev=.3, cho=.3),
 p("Silver Vibrato", eng=0, t1=.35, t3=.55, a=.1, rev=.35, src=0, dst=0, rate=6, amt=.3),
 p("Desert Ney", eng=2, t1=.7, t2=.55, t3=.45, a=.07, r=.7, rev=.5, gl=.12, src=0, dst=0, rate=5, amt=.18),
 p("Temple Whisper", eng=1, t1=.9, t2=.8, a=.25, s=.9, r=2.5, rev=.8, cut=7000, src=3, dst=2, rate=3, amt=.2)])
bank("Raga & Strings", [
 p("Raga Dusk", eng=3, t2=.7, t3=.55, t4=.6, t5=.4, rev=.4, dly=.2, tm=.4, r=1.5),
 p("Sitar Buzz", eng=3, t2=.55, t3=.7, t4=.5, t5=.8, rev=.3, cut=11000),
 p("Oud Nights", eng=4, t2=.6, t3=.4, rev=.4, cho=.2, r=1.2),
 p("Gypsy Oud", eng=4, t2=.45, t3=.65, rev=.25, dly=.15, tm=.28, gl=.04),
 p("Tanpura Drone", eng=6, t1=.3, t2=.5, t3=.3, t4=.5, a=1.2, s=1, r=3, rev=.6, cho=.35, mono=0),
 p("Veena Glow", eng=3, t2=.8, t3=.4, t4=.7, t5=.2, rev=.55, r=2),
 p("Erhu Tears", eng=6, t1=.5, t2=.55, t3=.5, t4=.35, a=.18, rev=.5, src=0, dst=0, rate=5.5, amt=.22),
 p("Sarangi Moan", eng=6, t1=.35, t2=.7, t3=.35, t4=.5, a=.25, r=1.2, gl=.1, rev=.45, src=0, dst=0, rate=5, amt=.25),
 p("Bazaar Pluck", eng=4, t2=.3, t3=.75, rev=.2, d=.25, s=.2, r=.3),
 p("Maqam Sunrise", eng=3, t2=.65, t3=.6, t4=.55, t5=.5, rev=.5, dly=.3, tm=.42, fb=.45)])
bank("Afrique", [
 p("Mbira Rain", eng=5, t2=.55, t3=.5, rev=.45, dly=.2, tm=.3, d=1, s=.0, r=.8),
 p("Kalimba Fire", eng=5, t2=.4, t3=.75, t5=.5, rev=.3, cho=.2, d=.8, s=0),
 p("Savanna Thumb", eng=5, t2=.35, t3=.3, rev=.35, pit=-12, d=.9, s=0, r=.6),
 p("Kora Dreams", eng=4, t2=.8, t3=.55, rev=.5, cho=.45, r=2),
 p("Balafon Sun", eng=5, t2=.7, t3=.65, t5=.6, rev=.35, d=1.2, s=0, r=1),
 p("Sanza Bells", eng=5, t2=.85, t3=.8, rev=.6, dly=.3, tm=.38, pit=12, d=1.5, s=0, r=1.4),
 p("Tribal Marimba", eng=5, t2=.3, t3=.4, t5=.7, rev=.25, d=.5, s=0, r=.4),
 p("Griot Pluck", eng=4, t2=.5, t3=.7, rev=.3, dly=.18, tm=.25, d=.3, s=.15),
 p("Zulu Chime", eng=5, t2=.9, t3=.85, rev=.7, cho=.3, pit=12, d=2, s=0, r=2),
 p("Nile Echo", eng=5, t2=.6, t3=.55, rev=.5, dly=.5, tm=.5, fb=.55, d=1, s=0)])
bank("Amerique Latine", [
 p("Charango Sun", eng=3, t2=.35, t3=.85, t4=.3, t5=.2, rev=.25, d=.3, s=.1, pit=12),
 p("Zampona Rain", eng=1, t1=.7, t2=.6, a=.04, rev=.6, cho=.25),
 p("Quena Mountain", eng=2, t1=.5, t2=.45, a=.05, rev=.65, dly=.2, tm=.45, src=0, dst=0, rate=5, amt=.2),
 p("Cumbia Pluck", eng=4, t2=.3, t3=.65, rev=.2, cho=.15, d=.3, s=.25, r=.3),
 p("Machu Dawn", eng=7, t1=.4, t2=.4, t3=.4, t4=.6, a=1.4, s=1, r=3, rev=.7, cho=.4),
 p("Tango Bow", eng=6, t1=.45, t2=.6, t3=.55, a=.12, rev=.35, src=0, dst=0, rate=5.5, amt=.3),
 p("Rio Marimba", eng=5, t2=.35, t3=.45, t5=.6, rev=.25, d=.6, s=0, r=.5),
 p("Cantina Oud", eng=4, t2=.4, t3=.6, rev=.3, dis=.2, dly=.15, tm=.22),
 p("Andes Echo", eng=1, t1=.65, t2=.55, a=.08, rev=.55, dly=.55, tm=.55, fb=.6),
 p("Amazon Mist", eng=7, t1=.65, t2=.3, t3=.3, t4=.7, t5=.5, a=2, s=1, r=4, rev=.85, cut=5000)])
bank("Pads Mystiques", [
 p("Voodoo Choir", eng=7, t1=.3, t2=.6, t3=.45, t4=.6, a=.9, s=1, r=2.5, rev=.6, cho=.35),
 p("Swamp Spirits", eng=7, t1=.15, t2=.7, t3=.25, t4=.8, a=1.5, s=1, r=3, rev=.7, cut=3500, src=1, dst=1, rate=.3, amt=.35),
 p("Ghost Priestess", eng=7, t1=.75, t2=.6, t3=.5, t4=.5, a=1.2, s=1, r=3.5, rev=.8, dly=.25, tm=.5),
 p("Candle Ritual", eng=7, t1=.5, t2=.45, t3=.4, t4=.55, a=.8, s=1, r=2.5, rev=.65, cho=.3, src=4, dst=3, rate=1.2, amt=.25),
 p("Bone Orchard", eng=7, t1=.2, t2=.5, t3=.3, t4=.9, a=2, s=1, r=4, rev=.75, dis=.15),
 p("Mystic Dawn", eng=7, t1=.6, t2=.4, t3=.7, t4=.65, a=1.8, s=1, r=3, rev=.7, cho=.5, pit=12),
 p("Moon Chant", eng=7, t1=.55, t2=.8, t3=.4, t4=.4, a=.6, s=1, r=2, rev=.55, src=0, dst=3, rate=.4, amt=.5),
 p("Veil of Smoke", eng=6, t1=.3, t2=.3, t3=.3, t4=.8, a=2.5, s=1, r=5, rev=.9, cho=.5, cut=4500),
 p("Hollow Temple", eng=7, t1=.35, t2=.9, t3=.35, t4=.5, a=1, s=1, r=3, rev=.9, dly=.4, tm=.6, fb=.5),
 p("Obsidian Hymn", eng=7, t1=.1, t2=.75, t3=.2, t4=.9, t5=.4, a=1.6, s=1, r=4, rev=.8, dis=.25, cut=3000)])
bank("Bass & Plucks", [
 p("Hex Bass", eng=4, t2=.7, t3=.25, pit=-12, mono=1, gl=.08, cut=2500, rev=.1, d=.5, s=.4),
 p("Root Doctor", eng=3, t2=.5, t3=.3, t5=.5, pit=-12, mono=1, gl=.05, cut=3500, rev=.1),
 p("Gris-Gris Sub", eng=6, t1=.2, t2=.3, t3=.15, pit=-12, mono=1, gl=.1, cut=1800, a=.02, rev=.05, ft=0),
 p("Cursed Pluck", eng=3, t2=.3, t3=.8, t5=.7, rev=.25, d=.2, s=.1, r=.2, dis=.15),
 p("Zombie Walk", eng=4, t2=.35, t3=.3, pit=-12, mono=1, gl=.15, cut=2200, dis=.3, rev=.1),
 p("Crossroads", eng=4, t2=.55, t3=.45, pit=-12, mono=1, rev=.2, dly=.2, tm=.3, cut=4000),
 p("Bayou Thump", eng=5, t2=.4, t3=.2, pit=-24, mono=1, cut=2000, d=.5, s=0, r=.3, rev=.1),
 p("Mojo Pluck", eng=3, t2=.4, t3=.6, t4=.4, t5=.5, rev=.3, d=.3, s=.1, pit=0),
 p("Skull Drone", eng=6, t1=.15, t2=.4, t3=.2, t4=.7, pit=-24, mono=1, a=.6, s=1, cut=1200, dis=.2, rev=.3),
 p("Midnight Mojo", eng=4, t2=.6, t3=.35, pit=-12, mono=1, gl=.06, cut=3000, cho=.2, rev=.25)])
bank("Textures", [
 p("Fog Over Bayou", eng=1, t1=1, t2=.3, t3=.3, a=1.5, s=1, r=4, rev=.9, cut=4000, src=1, dst=1, rate=.2, amt=.5),
 p("Whispers in Smoke", eng=2, t1=1, t2=.9, a=1, s=1, r=3, rev=.85, dly=.4, tm=.6, src=4, dst=3, rate=3, amt=.6),
 p("Cauldron", eng=7, t1=.1, t2=.9, t3=.2, t4=1, t5=1, a=.5, s=1, r=2, dis=.4, rev=.6, cut=2500, src=0, dst=1, rate=.5, amt=.6),
 p("Rain Dance", eng=5, t2=.5, t3=.9, t5=1, rev=.5, dly=.35, tm=.18, fb=.6, d=.8, s=0, src=4, dst=0, rate=8, amt=.1),
 p("Ritual Fire", eng=3, t2=.6, t3=.9, t4=.9, t5=1, dis=.4, rev=.4, dly=.3, tm=.33, r=1.5),
 p("Spirit Wind", eng=2, t1=1, t2=.5, t3=.8, a=1.2, s=1, r=3, rev=.8, ft=2, cut=6000, src=0, dst=1, rate=.25, amt=.7),
 p("Dark Bazaar", eng=4, t2=.6, t3=.5, t5=.6, dis=.3, rev=.5, dly=.35, tm=.37, fb=.5, cho=.3),
 p("Jinn Voices", eng=7, t1=.9, t2=.8, t3=.6, t4=.9, t5=.6, a=.7, s=1, r=2.5, rev=.7, src=3, dst=3, rate=.8, amt=.6),
 p("Ancient Tape", eng=6, t1=.4, t2=.4, t3=.3, t4=.6, a=.3, s=1, r=1.5, cho=.7, dis=.35, rev=.4, cut=5000, src=0, dst=0, rate=.7, amt=.25),
 p("Voodoo Doll", eng=5, t2=.7, t3=.7, t4=.5, t5=.9, rev=.6, dly=.45, tm=.4, fb=.65, cho=.4, pit=12, d=1.5, s=0, r=2)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Voodoo {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
           'inline const std::vector<PresetDef>& presets() { static const std::vector<PresetDef> P = {']
    n = 0
    for b, rows in BANKS:
        assert len(rows) == 10, b
        for name, k in rows:
            kv = dict(D); kv.update(k)
            items = ', '.join('{"%s", %sf}' % (KEY[x], repr(float(kv[x]))) for x in KEY)
            out.append('  {"%s", "%s", {%s}},' % (b, name, items)); n += 1
    out.append('}; return P; } }')
    assert n == 70
    return '\n'.join(out) + '\n'

if __name__ == '__main__':
    import sys
    open(sys.argv[1], 'w').write(emit())
