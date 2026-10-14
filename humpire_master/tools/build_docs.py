# -*- coding: utf-8 -*-
import os
from data import *
from newlong import *
from build_catalogs import w, desc, prods, pub, newp, TODAY

REQ = "Windows 64-bit; VST3-compatible host (e.g. FL Studio) or standalone application. No sample library to install (synthesis only)."
CATN = {"VOODOO":"Instruments > Synthesizers > World / Physical modeling","WARZONE808":"Instruments > Synthesizers > Bass / 808",
        "FUNKYKOPS":"Instruments > Synthesizers > Funk / Disco","OCCITANIE":"Instruments > Synthesizers > World / Folk"}

# ===== KVR
s = ["# KVR_IMPORT_READY\n", f"Généré {TODAY}. Fiches à copier-coller dans le formulaire développeur KVR (soumission faite par un humain : la page KVR interdit les envois par agent IA).",
     "Périmètre : les 4 synthés présents dans le repo et absents du catalogue Gumroad. Statut KVR des autres produits non vérifiable (accès réseau bloqué) : leurs données sont dans `catalogs/catalog_kvr.csv`.\n",
     "Champs prix / URL de téléchargement : non fournis par les sources du repo -> à saisir dans le formulaire.\n"]
for n in newp:
    nm = n["name"]
    s += [f"\n---\n## {nm}\n",
      f"**Nom :** {nm}", f"**Développeur :** HUMPIRE / DJBILBOX BEATS", f"**Catégorie :** {CATN[nm]}", f"**Type :** Instrument (VST3, Standalone)",
      f"**Version :** 1.0.0", f"**Description courte :** {SHORT[nm]}", f"\n**Description longue :**\n\n{LONG[nm]}",
      f"\n**Nouveautés (Whats new) :** Initial release 1.0.0. {n['presets']} factory presets, {n['voices']} voices, VST3 and standalone builds.",
      f"\n**Tags :** {', '.join(n['tags'] + ['vst3','synthesizer','juce','fl studio'])}",
      "\n**Features :**"] + [f"- {f}" for f in FEATURES[nm]] + [
      f"\n**Moteurs / sources sonores :** {n['engines']}", f"**Banques de presets :** {n['banks']}",
      f"**Contrôles :** {n['controls']}", f"**Système requis :** {REQ}", "**Formats :** VST3, Standalone", "**Licence / prix :** à saisir (non défini dans le repo)"]
w("KVR_IMPORT_READY.md", "\n".join(s) + "\n")

# ===== marketplaces
def product_block(p):
    price = "Free" if p["is_free"] else f"{p['price']:.2f} {p['currency']}"
    out = [f"### {p['name']}", f"- Titre : {p['full_title']}", f"- Type : {p['product_type']}", f"- Prix : {price}" + (" (pay what you want)" if p["pay_what_you_want"] and p["is_free"] else ""),
           f"- Formats : {p['formats']}", f"- Genres : {', '.join(p['genres']) or 'general'}", f"- Lien source : {p['url']}", f"- Description : {desc(p)}", ""]
    return "\n".join(out)
def newblock(n):
    nm = n["name"]
    return "\n".join([f"### {nm}", f"- Titre : {nm} — {n['sub']}", "- Type : Instrument synth", "- Prix : à définir", "- Formats : VST3, Standalone", "- Version : 1.0.0",
        f"- Court : {SHORT[nm]}", f"- Long : {LONG[nm]}", "- Features : " + "; ".join(FEATURES[nm]), f"- Requis : {REQ}", ""])

def sheet(fname, title, intro, plug_filter, snd_filter, include_new):
    s = [f"# {title}\n", intro, ""]
    if include_new:
        s.append("## Nouveaux produits\n"); s += [newblock(n) for n in newp]
    pl = [p for p in pub if plug_filter(p)]; sd = [p for p in pub if snd_filter(p)]
    if pl: s.append("## Plugins catalogue\n"); s += [product_block(p) for p in pl]
    if sd: s.append("## Soundware catalogue\n"); s += [product_block(p) for p in sd]
    w(fname, "\n".join(s) + "\n")

paidplug = lambda p: p["kind"]=="plugin" and not p["is_free"] and p["role"]!="bundle"
sheet("PLUGIN_BOUTIQUE_READY.md", "PLUGIN_BOUTIQUE_READY", "Fiches vendeur Plugin Boutique : plugins payants (+ 4 nouveaux synthés). Prix des 4 nouveaux à fixer avant soumission. Plugin Boutique n'a pas de voie de soumission confirmée (voir `djbilbox_campaign/prospection/sources_et_regles.md`) : fiches prêtes si un accès vendeur est obtenu.", paidplug, lambda p: False, True)
sheet("ADSR_READY.md", "ADSR_READY", "Fiches ADSR Sounds : plugins payants et soundware payant + nouveaux synthés.", paidplug, lambda p: p["kind"]=="soundware" and not p["is_free"], True)
sheet("VSTBUZZ_READY.md", "VSTBUZZ_READY", "Fiches VSTBuzz : plugins payants et bundles + nouveaux synthés (offres promo : prix à définir).", lambda p: p["kind"]=="plugin" and not p["is_free"], lambda p: False, True)
sheet("LOOPMASTERS_READY.md", "LOOPMASTERS_READY", "Fiches soundware (loops/kits) pour Loopmasters. Le nombre de fichiers / BPM / tonalités n'est pas dans les sources : à lire dans les ZIP avant dépôt.", lambda p: False, lambda p: p["kind"]=="soundware", False)
sheet("PRODUCER_LOOPS_READY.md", "PRODUCER_LOOPS_READY", "Fiches soundware pour Producer Loops. Même limite : contenus chiffrés non présents dans les sources.", lambda p: False, lambda p: p["kind"]=="soundware", False)
print("docs1 ok")
