# -*- coding: utf-8 -*-
import os, csv, itertools
from data import *
from build_catalogs import w, csvw, prods, pub, newp, TODAY, fixed, problems
from articles_data import A

byname = {p["name"]: p for p in prods}
def link_for(name):
    if name in byname: return byname[name]["url"]
    for p in prods:
        if p["name"].startswith(name) or p["full_title"].startswith(name): return p["url"]
    return "https://djbilboxbeats.gumroad.com/"
def prodrec(name):
    return byname.get(name) or next((p for p in prods if p["name"].startswith(name) or p["full_title"].startswith(name)), None)

# ===== 500 mots-clés
CATS = {
 "VST": ["vst plugins","vst3 plugins","free vst","vst for fl studio","vst bundle","vst instruments","vst effects","best free vst","vst3 download","windows vst","mac vst","producer vst"],
 "Vocoder": ["vocoder plugin","vocoder vst","free vocoder","vocoder vst3","vocoder for fl studio","vocoder effect","robot voice vocoder","vocoder autotune","vocoder synth","vocoder presets","vocoder vocal effect","talk robot plugin"],
 "Talkbox": ["talkbox plugin","talkbox vst","free talkbox","talkbox vst3","talkbox for fl studio","talkbox vocal effect","talkbox funk","talkbox g-funk","talkbox synth","talkbox software","roger troutman talkbox plugin","talkbox presets"],
 "Synth": ["synth vst","synth vst3","free synth plugin","synthesizer plugin","polyphonic synth vst","analog synth vst","synth presets","synth for fl studio","lead synth vst","pad synth vst","software synthesizer","synth plugin 2026"],
 "Oriental VST": ["oriental vst","arabic vst","oriental instruments plugin","arabic sampler vst","middle east vst","oriental melody plugin","maqam vst","oud vst","arabic scales plugin","ethnic vst","oriental trap vst","world instruments vst"],
 "G-Funk": ["g-funk vst","g-funk synth","g-funk plugin","g-funk lead synth","g-funk bass vst","g-funk loops","g-funk sample pack","g-funk drum kit","g-funk melody loops","g-funk beat maker","g-funk presets","g-funk production"],
 "West Coast": ["west coast vst","west coast sample pack","west coast drums","west coast beats","west coast hip hop loops","west coast synth","west coast drum kit","west coast melody loops","west coast producer kit","west coast bass","west coast plugin","west coast fl studio"],
 "Trap": ["trap vst","trap drum kit","trap 808 plugin","trap hi hats","trap loops","trap melody loops","trap drum loops","dark trap kit","trap beat maker","trap sample pack","trap presets","trap production tools"],
 "Beat Generator": ["beat generator vst","beat generator plugin","random beat generator","drum pattern generator","beat maker vst","drum machine vst","free drum machine","mpc vst","16 pad beat maker","groove generator","midi drum generator","beat maker fl studio"],
 "Chord Generator": ["chord generator vst","chord generator plugin","midi chord generator","chord progression generator","melody generator vst","chord generator fl studio","scale chord plugin","free chord generator","maqam chord generator","chord generator for producers","midi melody generator","harmony generator plugin"],
 "Bass Plugin": ["bass vst","bass synth vst","808 vst","808 bass plugin","sub bass plugin","free bass vst","bassline generator","bass plugin for fl studio","reese bass vst","808 slides plugin","bass synth free","bass vst3"],
 "Guitar FX": ["guitar fx plugin","guitar pedal plugin","guitar effects vst","free guitar pedal vst","fuzz pedal plugin","overdrive pedal vst","delay pedal plugin","reverb pedal vst","chorus pedal plugin","phaser pedal vst","tremolo pedal plugin","guitar pedal vst3"],
}
MODS = ["","free","download","best","2026","tutorial","presets","for beginners","windows","mac","how to use","review","vs","bundle","fl studio","royalty free"]
rows = []; sizes = [42]*8 + [41]*4
for (cat, heads), n in zip(CATS.items(), sizes):
    seen, lst = set(), []
    for m in MODS:
        for h in heads:
            kw = (h + " " + m).strip() if m not in ("free","best") or m not in h else h
            if m in ("best",): kw = f"best {h}"
            if m == "free" and "free" in h: continue
            if m == "vs": continue
            if kw not in seen: seen.add(kw); lst.append(kw)
            if len(lst) >= n: break
        if len(lst) >= n: break
    assert len(lst) == n, (cat, len(lst))
    for k in lst:
        intent = "transactional" if any(x in k for x in ("download","bundle","presets")) else "commercial" if any(x in k for x in ("best","free","review")) else "informational" if any(x in k for x in ("tutorial","how to","beginners")) else "navigational/generic"
        rows.append([cat, k, intent])
assert len(rows) == 500, len(rows)
csvw("seo/keywords_500.csv", ["categorie","mot_cle","intention"], rows)
md = ["# 500 mots-clés SEO\n"]
for cat in CATS:
    md.append(f"\n## {cat} ({sum(1 for r in rows if r[0]==cat)})\n"); md += [f"- {r[1]}" for r in rows if r[0]==cat]
w("seo/KEYWORDS_500.md", "\n".join(md) + "\n")

# ===== Contenu social (produits catalogue publiés)
pool = [p for p in pub if p["name"] not in ("DJBILBOX BEATS Complete Paid Packs, Kits & Loops",)]
def st(p): return p["name"] + (" — " + p["subtitle"].split("(")[0].strip() if p["subtitle"] else "")
def cyc(n): return [pool[i % len(pool)] for i in range(n)]
def price_txt(p):
    return "free" if p["is_free"] else f"{p['price']:.0f} {'€' if p['currency']=='EUR' else '$'}"
def hook(p):
    return p["facts"].split(".")[0] if p["facts"] else p["subtitle"].split("(")[0].strip()

YT = ["{n}: {h} (Full Walkthrough)","I Made a Beat With {n} in 10 Minutes","{n} Review: Is It Worth It?","{n} — Sound Demo, No Talking","How to Use {n} in FL Studio","{n}: 5 Tips You Need to Know","Free Download: {n} ({k})","{n} vs Stock Plugins: Quick Comparison","Beat From Scratch Using Only {n}","{n} Tutorial for Beginners"]
yt = []
for i in range(100):
    p = pool[i % len(pool)]; t = YT[(i // len(pool) + i) % len(YT)]
    yt.append(t.format(n=p["name"], h=hook(p), k=p["keyword"]))
# nouveaux synthés dans les titres
for j, n in enumerate(newp):
    yt[10*j] = f"{n['name']}: {n['sub'].split('—')[0].strip()} — Sound Demo"; yt[10*j+5] = f"I Built a Beat Using Only {n['name']} Presets"
SH = ["Hook: stop scrolling — {h}.","Hook: this plugin is {pr}.","Hook: one knob changes everything in {n}.","Hook: 15 seconds, one beat, only {n}."]
shorts = []
for i in range(100):
    p = pool[(i*3) % len(pool)]; s = SH[i % 4].format(h=hook(p), pr=("free" if p["is_free"] else "made for "+(p["genres"][0] if p["genres"] else "producers")), n=p["name"])
    shorts.append(f"{i+1}. **{p['name']} — #{(i%7)+1} short** · {s} Demo: play 2 presets / one loop (≤45 s). CTA: link in description ({p['url']}). Tags: #{p['keyword'].replace(' ','')} #flstudio #beatmaking")
w("social/YOUTUBE_100_TITLES.md", "# 100 titres YouTube\n\n" + "\n".join(f"{i+1}. {t}" for i,t in enumerate(yt)) + "\n")
w("social/YOUTUBE_100_SHORTS.md", "# 100 shorts YouTube\n\n" + "\n".join(shorts) + "\n")

SUBS = ["r/FL_Studio","r/WeAreTheMusicMakers","r/edmproduction","r/Beatmakers"]
RF = [("I made a {k}: feedback welcome","I'm the developer of {n} ({h}). {pr} Happy to answer questions about how it works. Link in comments if rules allow: {u}"),
      ("Question: how do you approach {k}?","Working on {n}, I noticed most producers handle this differently. How do you do it? Disclosure: I make {n}. {u}"),
      ("Tip: getting more out of {k}","Short workflow tip from my own use of {n}: start from a preset, change one parameter at a time, save. Disclosure: I'm the author. {u}"),
      ("{n} — {pr_short}","Sharing {n} ({h}). Disclosure: my own product. {u}"),
      ("Dev log: what I learned building {k}","Notes from building {n}. Tell me what you'd want added. {u}")]
reddit = []
for i in range(100):
    p = pool[i % len(pool)]; t, b = RF[i % 5]
    d = dict(n=p["name"], h=hook(p), k=p["keyword"], u=p["url"], pr=("It is free." if p["is_free"] else f"Price: {price_txt(p)}."), pr_short=("free" if p["is_free"] else price_txt(p)))
    reddit.append(f"### {i+1}. {SUBS[i%4]}\n**Titre :** {t.format(**d)}\n\n{b.format(**d)}\n")
w("social/REDDIT_100.md", "# 100 publications Reddit\nPublier à la main, un seul post par sub et par produit, règles de chaque sub relues avant.\n\n" + "\n".join(reddit))

FB = ["New in the catalogue: {n}. {h}. {pr} {u}","Producers: {n} — {h}. {pr} {u}","Quick demo of {n} coming through. {h}. {u}","{n}: {h}. {pr} Tell me what you make with it. {u}"]
fb = []
for i in range(100):
    p = pool[i % len(pool)]
    fb.append(f"{i+1}. " + FB[i % 4].format(n=p["name"], h=hook(p), u=p["url"], pr=("Free." if p["is_free"] else price_txt(p).capitalize()+".")))
w("social/FACEBOOK_100.md", "# 100 publications Facebook\n\n" + "\n\n".join(fb) + "\n")
X = ["{n}: {h}. {u} #{k}","{pr_s} {n} for your next beat. {u} #flstudio #{k}","{h}. Try {n}. {u} #beatmaking"]
xs = []
for i in range(100):
    p = pool[i % len(pool)]
    t = X[i % 3].format(n=p["name"], h=hook(p), u=p["url"], k=p["keyword"].replace(" ",""), pr_s=("Free:" if p["is_free"] else "New:"))
    if len(t) > 280: t = t[:277] + "..."
    xs.append(f"{i+1}. {t}")
w("social/X_100.md", "# 100 publications X (≤280 caractères)\n\n" + "\n".join(xs) + "\n")
ig = []
for i in range(50):
    p = pool[i % len(pool)]
    ig.append(f"{i+1}. {p['name']} — {hook(p)}. {'Free download.' if p['is_free'] else price_txt(p).capitalize()+'.'} Link in bio.\n#{p['keyword'].replace(' ','')} #beatmaker #flstudio #vst #{(p['genres'] or ['producer'])[0].replace(' ','').replace('-','')}")
w("social/INSTAGRAM_50.md", "# 50 publications Instagram\n\n" + "\n\n".join(ig) + "\n")

# ===== 50 articles
arts = ["# 50 articles SEO\n"]
for i, (title, kw, pname, intro, steps, faq) in enumerate(A):
    p = prodrec(pname); u = p["url"] if p else "https://djbilboxbeats.gumroad.com/"
    nm = p["name"] if p else pname
    fact = (p["facts"] or p["subtitle"]) if p else ""
    body = [f"\n---\n## Article {i+1} — {title}\n", f"**Mot-clé :** {kw} · **Slug :** /blog/{slugify(title)}/ · **Meta :** {title[:60]} — practical steps for producers.\n", f"# {title}\n", intro + "\n"]
    for h, t in steps: body.append(f"### {h}\n{t}\n")
    body.append(f"### Where {nm} fits\n{nm}: {fact} [Get {nm}]({u}). Judge the sound yourself with the free demo or preview where available.\n")
    body.append("### FAQ\n" + "\n".join(f"**{q}** {a}\n" for q, a in faq))
    body.append(f"### Summary\nApply the steps above to your next session and save the result as a template. Keyword focus: {kw}.\n")
    arts.append("\n".join(body))
w("seo/ARTICLES_50.md", "\n".join(arts))

# ===== emails
em = ["# email-launch-sequence\n", "Remplacer {name}. Codes de remise : à créer dans Gumroad avant envoi (WELCOME20, BUNDLE15 ne sont pas encore créés).\n"]
F = ["MINI MPC HUMPIRE","THUGLIFE","DJBILBOX PRO BUNDLE"]
fl = lambda n: byname[n]["url"]
blocks = {
"Acquisition": [("Your free drum machine is inside","Hi {name}, here is your download: "+fl("MINI MPC HUMPIRE")+". 64 pads, 1,000 grooves, VST3. Reply with your genre and I'll point you to the right kit."),
                ("3 free tools producers actually use","MPC 2026, NEON SYNTH 80s and ORIENTAL INSTRUMENT Free Demo are free. Start here: "+fl("MPC 2026")+".")],
"Relance": [("Did you open it yet?","Quick reminder: your plugin is a rescan away. Copy the VST3 to your plugin folder and rescan in your DAW."),
            ("One tip to start faster","Load a preset, change one knob, save it as yours. Need help? Just reply.")],
"Upsell": [("Ready for more?","If MINI MPC fits your workflow, MPC 2026 and BLOCK GORILLA add pads and beat generation: "+fl("BLOCK GORILLA")+"."),
           ("Try THUGLIFE free for 30 days","The G-Funk synth has a 30-day demo: "+fl("THUGLIFE Free Demo")+". Full version: "+fl("THUGLIFE")+".")],
"Cross sell": [("Bass to match your leads","ALIEN 808 ("+fl("ALIEN 808")+") and CAMEL PILOT ("+fl("CAMEL PILOT")+") pair with synth leads."),
               ("Oriental? Add the chords","CASBAH CHORDS ("+fl("CASBAH CHORDS")+") gives maqam-based progressions.")],
"Bundle": [("Stop buying one plugin at a time","DJBILBOX PRO BUNDLE: 8 VST plugins, one payment: "+fl("DJBILBOX PRO BUNDLE")+"."),
           ("The complete collection","HUMPIRE VST SUITE (7 plugins) "+fl("HUMPIRE VST SUITE")+" or the 25-plugin collection "+fl("HUMPIRE VST & Effects")+".")],
"Abandon panier": [("You left something behind","Your cart is waiting: {link}. VST3 on Windows and macOS where stated on the page."),
                   ("Still deciding?","A free demo exists for THUGLIFE (30 days) and ORIENTAL INSTRUMENT. Questions? Reply."),
                   ("Last reminder","This is the last note about your cart: {link}. Unsubscribe anytime.")],
}
for k, v in blocks.items():
    em.append(f"\n## {k}\n")
    for j, (s, b) in enumerate(v): em.append(f"### Email {j+1}\n**Objet :** {s}\n\n{b}\n\n_Désinscription en pied d'email._\n")
w("email-launch-sequence.md", "\n".join(em))
print("content ok", len(arts)-1)
