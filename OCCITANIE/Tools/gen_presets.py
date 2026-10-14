#!/usr/bin/env python3
"""Genere Source/Presets.h : 70 presets VOODOO (7 banques x 10)."""
D = dict(eng=0, cut=9000, a=.01, d=.4, s=.7, r=.5, t1=.5, t2=.5, t3=.5, t4=.3, t5=.2, drn=0, vel=.5, rev=.25, cho=0, dis=0,
         dly=0, tm=.35, fb=.35, src=0, dst=0, rate=5, amt=0, gl=0, pit=0, ft=0, mono=0, vol=.7)
KEY = dict(eng='waveform', cut='filter', a='attack', d='decay', s='sustain', r='release', t1='t1', t2='t2', t3='t3',
           t4='t4', t5='t5', drn='drone', vel='velocity', rev='reverb', cho='chorus', dis='distortion', dly='delay', tm='time',
           fb='feedback', src='lfosource', dst='destination', rate='rate', amt='amount', gl='glide', pit='pitch', ft='filtertype',
           mono='monopoly', vol='volume')
CAB, VIE, GAL, MAN, GUI, ACC, CHO, CAR = range(8)
BANKS = []
def bank(name, rows): BANKS.append((name, rows))
def p(n, **k): return (n, k)

bank("Cabrette & Cornemuse", [
 p("Capitole Cabrette", eng=CAB, t1=.5, t2=.55, t3=.55, drn=.35, a=.03, s=.9, rev=.3),
 p("Canal du Midi", eng=CAB, t1=.4, t2=.6, t3=.45, drn=.5, a=.06, s=.9, gl=.08, mono=1, rev=.45),
 p("Garonne Drone", eng=CAB, t1=.3, t2=.5, t3=.35, drn=.9, a=.2, s=1, r=1.2, cut=2500, rev=.4),
 p("Pic du Midi", eng=CAB, t1=.6, t2=.5, t3=.75, drn=.2, a=.02, s=.85, pit=12, rev=.35),
 p("Fete Votive", eng=CAB, t1=.7, t2=.55, t3=.8, drn=.3, a=.02, s=.9, cho=.3, src=0, dst=0, rate=5.5, amt=.12),
 p("Cabreta Triste", eng=CAB, t1=.2, t2=.7, t3=.3, drn=.4, a=.35, s=.9, r=1.4, rev=.6),
 p("Bal Bagpipe", eng=CAB, t1=.7, t2=.5, t3=.6, drn=.4, dis=.15, a=.02, rev=.2, vel=.7),
 p("Corbieres", eng=CAB, t1=.45, t2=.6, t3=.5, drn=.6, cut=3000, cho=.4, rev=.5, a=.08, s=.9),
 p("Pastel Bleu", eng=CAB, t1=.3, t2=.45, t3=.3, drn=.25, a=.12, r=1.1, rev=.55, src=0, dst=1, rate=.4, amt=.3),
 p("Saint Sernin", eng=CAB, t1=.4, t2=.7, t3=.4, drn=.75, pit=-12, a=.1, s=1, r=1.5, rev=.6)])
bank("Vielle & Cordes frottees", [
 p("Roue de Cordes", eng=VIE, t1=.45, t2=.6, t3=.5, t5=.3, drn=.3, a=.08, s=.95, rev=.35),
 p("Trompette Buzz", eng=VIE, t1=.5, t2=.55, t3=.6, t5=.85, drn=.35, a=.05, s=.95, rev=.3),
 p("Bourdon Albigeois", eng=VIE, t1=.3, t2=.6, t3=.35, t5=.4, drn=.8, a=.15, s=1, r=1.4, rev=.5, cut=3500),
 p("Menestrel", eng=VIE, t1=.5, t2=.55, t3=.55, t5=.3, drn=.2, a=.1, rev=.4, src=0, dst=0, rate=5.2, amt=.22),
 p("Troubadour", eng=VIE, t1=.35, t2=.65, t3=.4, t5=.3, drn=.4, a=.2, r=1.2, gl=.06, mono=1, rev=.5),
 p("Chant Cathare", eng=VIE, t1=.55, t2=.7, t3=.35, t5=.2, drn=.5, a=.3, s=1, r=1.8, rev=.7, cho=.3),
 p("Rouergue", eng=VIE, t1=.6, t2=.5, t3=.6, t5=.6, drn=.45, a=.04, s=.9, cut=6000, rev=.25),
 p("Brume du Tarn", eng=VIE, t1=.3, t2=.4, t3=.25, t4=.6, t5=.15, drn=.3, a=.5, s=1, r=2.5, rev=.8, cho=.4, cut=4500),
 p("Chanterelle", eng=VIE, t1=.7, t2=.5, t3=.7, t5=.5, drn=.15, a=.06, pit=12, rev=.35, vel=.7),
 p("Cordes sur Ciel", eng=VIE, t1=.4, t2=.6, t3=.45, t4=.7, t5=.25, drn=.3, a=.4, s=1, r=2.2, rev=.65, cho=.45)])
bank("Flutes & Galoubet", [
 p("Galoubet Joyeux", eng=GAL, t1=.4, t3=.65, t5=.5, a=.02, s=.85, pit=12, rev=.3, vel=.7),
 p("Tambourin Air", eng=GAL, t1=.35, t3=.6, t5=.6, a=.01, d=.2, s=.5, r=.3, pit=12, rev=.25, dly=.12, tm=.3),
 p("Flute de Berger", eng=GAL, t1=.3, t3=.45, a=.08, rev=.45, src=0, dst=0, rate=5, amt=.15),
 p("Fifre Camargue", eng=GAL, t1=.5, t3=.8, t5=.5, a=.02, pit=12, rev=.3, cho=.2),
 p("Pastoral", eng=GAL, t1=.25, t3=.4, a=.12, r=.9, rev=.55),
 p("Mistral", eng=GAL, t1=.85, t2=.6, t3=.5, a=.1, s=.9, r=.8, rev=.5, src=4, dst=3, rate=3, amt=.3),
 p("Tramontane", eng=GAL, t1=.9, t2=.7, t3=.6, a=.15, s=.9, r=1, rev=.6, dly=.2, tm=.45),
 p("Flute Cathare", eng=GAL, t1=.4, t3=.35, a=.2, s=.9, r=1.4, rev=.7, cho=.25),
 p("Sifflet Occitan", eng=GAL, t1=.2, t3=.9, t5=.3, a=.02, pit=12, rev=.3, src=0, dst=0, rate=6, amt=.25),
 p("Aube Corbieres", eng=GAL, t1=.55, t2=.5, t3=.4, a=.25, s=.95, r=1.6, rev=.75, dly=.25, tm=.5)])
bank("Cordes pincees", [
 p("Mandoline Toulousaine", eng=MAN, t2=.5, t3=.7, t5=.5, rev=.3, vel=.7),
 p("Mandoline Tremolo", eng=MAN, t2=.6, t3=.6, t5=.9, rev=.35, d=.5, s=.3),
 p("Guitare Gitane", eng=GUI, t2=.5, t3=.65, rev=.3, cho=.15, vel=.7),
 p("Luth Medieval", eng=GUI, t2=.7, t3=.4, rev=.5, r=1.2),
 p("Cistre", eng=MAN, t2=.45, t3=.85, rev=.25, dly=.12, tm=.28),
 p("Guitare Nylon Soft", eng=GUI, t2=.55, t3=.3, rev=.35, cho=.2),
 p("Plectre Doux", eng=MAN, t2=.4, t3=.4, rev=.3, d=.35, s=.2),
 p("Strum du Midi", eng=GUI, t2=.5, t3=.75, rev=.25, dly=.15, tm=.22, vel=.8),
 p("Pizz Menestrel", eng=GUI, t2=.25, t3=.5, rev=.2, d=.25, s=.1),
 p("Theorbe Grave", eng=GUI, t2=.8, t3=.25, pit=-12, mono=1, cut=3500, rev=.4, r=1.5)])
bank("Accordeon & Musette", [
 p("Musette Toulouse", eng=ACC, t1=.5, t2=.5, t3=.55, t4=.6, a=.03, s=.9, cho=.2, rev=.3),
 p("Bal Musette", eng=ACC, t1=.6, t2=.45, t3=.65, t4=.8, a=.02, s=.9, cho=.15, rev=.25, vel=.7),
 p("Valse Violette", eng=ACC, t1=.45, t2=.5, t3=.5, t4=.7, a=.05, s=.85, rev=.4, src=0, dst=2, rate=2.5, amt=.2),
 p("Accordeon Cabaret", eng=ACC, t1=.4, t2=.55, t3=.5, t4=.5, a=.04, dis=.1, rev=.35, dly=.1),
 p("Java du Midi", eng=ACC, t1=.65, t2=.5, t3=.7, t4=.75, a=.02, s=.9, rev=.25),
 p("Soufflet Lent", eng=ACC, t1=.3, t2=.6, t3=.35, t4=.5, a=.3, s=1, r=1.2, rev=.5),
 p("Harmonium Chapelle", eng=ACC, t1=.25, t2=.6, t3=.3, t4=.3, a=.15, s=1, r=1.4, rev=.75, cut=4500),
 p("Orgue de Barbarie", eng=ACC, t1=.7, t2=.4, t3=.8, t4=.6, t5=.5, a=.01, s=.95, rev=.2, dis=.1),
 p("Gavotte", eng=ACC, t1=.55, t2=.5, t3=.6, t4=.65, a=.03, d=.5, s=.7, rev=.3),
 p("Cassoulet Musette", eng=ACC, t1=.5, t2=.55, t3=.45, t4=.85, a=.04, s=.9, cho=.35, rev=.4, drn=.15)])
bank("Choeurs & Pads", [
 p("Choeur Cathare", eng=CHO, t1=.3, t2=.6, t3=.45, t4=.6, a=.9, s=1, r=2.5, rev=.6, cho=.35),
 p("Montsegur", eng=CHO, t1=.2, t2=.7, t3=.3, t4=.8, a=1.4, s=1, r=3.2, rev=.8, cut=4500),
 p("Carcassonne Pad", eng=CHO, t1=.5, t2=.5, t3=.5, t4=.7, a=1, s=1, r=2.8, rev=.65, cho=.4, drn=.25),
 p("Cite Medievale", eng=CHO, t1=.4, t2=.65, t3=.4, t4=.6, a=.7, s=1, r=2.4, rev=.7, drn=.35),
 p("Voix d Oc", eng=CHO, t1=.65, t2=.6, t3=.55, t4=.5, a=.5, s=1, r=2, rev=.55, src=0, dst=3, rate=.5, amt=.4),
 p("Plainchant Albi", eng=CHO, t1=.15, t2=.75, t3=.3, t4=.4, a=.6, s=1, r=2.2, rev=.8, drn=.4, cut=5000),
 p("Lune d Oc", eng=CHO, t1=.75, t2=.4, t3=.6, t4=.8, a=1.6, s=1, r=3.5, rev=.7, cho=.45, pit=12),
 p("Abbaye Moissac", eng=CHO, t1=.35, t2=.8, t3=.35, t4=.5, a=1.1, s=1, r=3, rev=.9, dly=.3, tm=.55, fb=.5),
 p("Brume Ariege", eng=VIE, t1=.3, t2=.4, t3=.25, t4=.9, t5=.1, a=2, s=1, r=4, rev=.85, cho=.5, cut=4000, drn=.3),
 p("Crypte Saint Sernin", eng=CHO, t1=.1, t2=.85, t3=.2, t4=.6, t5=.2, a=1.5, s=1, r=4, rev=.9, dis=.15, cut=3000, pit=-12)])
bank("Carillons & Textures", [
 p("Carillon Capitole", eng=CAR, t2=.55, t3=.6, rev=.45, d=1.2, s=0, r=1, dly=.2, tm=.35),
 p("Cloches Saint Etienne", eng=CAR, t2=.75, t3=.5, rev=.6, d=1.8, s=0, r=1.8, pit=-12),
 p("Angelus du Soir", eng=CAR, t2=.8, t3=.7, rev=.7, d=2, s=0, r=2, dly=.3, tm=.5, fb=.5),
 p("Clocher d Albi", eng=CAR, t2=.65, t3=.45, t5=.4, rev=.5, d=1.5, s=0, r=1.4, pit=-5),
 p("Glockenspiel Rose", eng=CAR, t2=.4, t3=.85, t5=.6, rev=.35, d=.7, s=0, r=.6, pit=12, vel=.7),
 p("Cristaux Ariege", eng=CAR, t2=.9, t3=.9, rev=.75, cho=.3, d=2.2, s=0, r=2.2, pit=12),
 p("Rosee Garonne", eng=CAR, t2=.6, t3=.7, t5=.7, rev=.6, dly=.4, tm=.4, fb=.6, d=1, s=0),
 p("Vent d Autan", eng=GAL, t1=1, t2=.6, t3=.7, a=1, s=1, r=2.5, rev=.85, cut=6000, src=0, dst=1, rate=.25, amt=.6),
 p("Orage Cevenol", eng=VIE, t1=.2, t2=.3, t3=.2, t4=.9, t5=.9, a=.6, s=1, r=2, rev=.7, dis=.3, cut=2500, src=3, dst=1, rate=.3, amt=.5),
 p("Etoiles d Oc", eng=CAR, t2=.85, t3=.8, t5=.8, rev=.8, cho=.4, dly=.45, tm=.45, fb=.65, d=2, s=0, r=2.5, pit=12)])

def emit():
    out = ['// Genere par Tools/gen_presets.py - ne pas editer a la main', '#pragma once', '#include <vector>',
           '#include <utility>', 'namespace Occ {', 'struct PresetDef { const char* bank; const char* name; std::vector<std::pair<const char*, float>> v; };',
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
