# -*- coding: utf-8 -*-
import csv, json, os, html, datetime
from data import *
OUT = os.path.join(ROOT, "humpire_master")
TODAY = "2026-10-14"
prods, problems, fixed = load()
newp = new_products()
pub = [p for p in prods if p["published"]]

def w(path, text):
    full = os.path.join(OUT, path); os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "w", encoding="utf-8", newline="") as f: f.write(text)

def csvw(path, header, rows):
    full = os.path.join(OUT, path); os.makedirs(os.path.dirname(full), exist_ok=True)
    with open(full, "w", encoding="utf-8", newline="") as f:
        wr = csv.writer(f, quoting=csv.QUOTE_MINIMAL); wr.writerow(header); wr.writerows(rows)

def page_url(p): return f"{SITE}/products/{p['id']}/"
def pr(p): return f"{p['price']:.2f} {p['currency']}"
def kindword(p):
    return {"instrument":"instrument plugin","effect":"effect plugin","midi":"MIDI generator plugin","bundle":"plugin bundle","soundware":"sample pack"}[p["role"]]

def desc(p):
    parts = [p["full_title"] + "."]
    if p["facts"]: parts.append(p["facts"])
    if p["role"] != "soundware":
        parts.append(f"Formats: {p['formats']}.")
    else:
        parts.append("Royalty-free sounds for your beats.")
    if p["is_free"]: parts.append("Free download (pay what you want)." if p["pay_what_you_want"] else "Free download.")
    elif p["pay_what_you_want"]: parts.append(f"From {p['price_display'].replace('+','')}.")
    parts.append(f"By {p['brand']}.")
    return " ".join(parts)

def gcat(p):
    return "Software > Computer Software" if p["kind"] == "plugin" else "Media > Music & Sound Recordings"
def ptype(p): return f"Audio Software > {p['product_type']}" if p["kind"]=="plugin" else f"Audio Samples > {p['product_type']}"
def tags(p):
    t = [p["keyword"]] + p["genres"] + (["free"] if p["is_free"] else ["premium"])
    t += ["vst3" if p["kind"]=="plugin" else "sample pack", "fl studio", "music production", p["name"].lower()]
    out = []
    for x in t:
        if x not in out: out.append(x)
    return out

# ---- Google (payants uniquement : Merchant Center refuse le prix 0)
paid = [p for p in pub if not p["is_free"]]
csvw("catalogs/catalog_google.csv",
 ["id","title","description","link","image_link","availability","price","brand","condition","google_product_category","product_type","identifier_exists","is_bundle","custom_label_0","custom_label_1","custom_label_2"],
 [[p["sku"], p["full_title"][:150], desc(p)[:5000], page_url(p), p["image_link"], "in_stock", pr(p), p["brand"], "new", gcat(p), ptype(p), "no",
   "yes" if p["role"]=="bundle" else "no", p["kind"], p["role"], ";".join(p["genres"]) or "general"] for p in paid])

# ---- Meta (Facebook/Instagram Catalog)
csvw("catalogs/catalog_meta.csv",
 ["id","title","description","availability","condition","price","link","image_link","brand","google_product_category","fb_product_category","product_type","custom_label_0","custom_label_1"],
 [[p["sku"], p["full_title"][:200], desc(p)[:9999], "in stock", "new", pr(p), p["url"], p["image_link"], p["brand"], gcat(p),
   "Software" if p["kind"]=="plugin" else "Media > Music", ptype(p), p["kind"], ";".join(p["genres"]) or "general"] for p in pub])

# ---- Pinterest
csvw("catalogs/catalog_pinterest.csv",
 ["id","title","description","link","image_link","price","availability","google_product_category","product_type","brand","condition","product_tags"],
 [[p["sku"], p["full_title"][:500], desc(p)[:10000], page_url(p), p["image_link"], pr(p), "in stock", gcat(p), ptype(p), p["brand"], "new", ",".join(tags(p))] for p in pub])

# ---- TikTok
csvw("catalogs/catalog_tiktok.csv",
 ["sku_id","title","description","availability","condition","price","link","image_link","brand","google_product_category","product_type"],
 [[p["sku"], p["full_title"][:150], desc(p)[:9999], "in stock", "new", pr(p), p["url"], p["image_link"], p["brand"], gcat(p), ptype(p)] for p in pub])

# ---- Shopify (produits numériques)
def body_html(p):
    return f"<p>{html.escape(p['full_title'])}</p><p>{html.escape(p['facts'] or desc(p))}</p><p>{html.escape('Formats: '+p['formats']) if p['kind']=='plugin' else 'Royalty-free sounds.'}</p>"
csvw("catalogs/catalog_shopify.csv",
 ["Handle","Title","Body (HTML)","Vendor","Product Category","Type","Tags","Published","Option1 Name","Option1 Value","Variant SKU","Variant Grams","Variant Inventory Tracker","Variant Inventory Policy","Variant Fulfillment Service","Variant Price","Variant Compare At Price","Variant Requires Shipping","Variant Taxable","Image Src","Image Position","Image Alt Text","Gift Card","SEO Title","SEO Description","Status"],
 [[p["id"], p["full_title"], body_html(p), p["brand"], "Software", p["product_type"], ", ".join(tags(p)), "TRUE", "Title", "Default Title", p["sku"], 0, "", "deny", "manual",
   f"{p['price']:.2f}", "", "FALSE", "FALSE", p["image_link"], 1 if p["image_link"] else "", p["name"], "FALSE",
   p["full_title"][:70], desc(p)[:320], "active"] for p in pub])

# ---- KVR (plugins uniquement ; soundware hors périmètre KVR)
def kvr_cat(p):
    return {"instrument":"Instrument","effect":"Effect","midi":"MIDI Tool","bundle":"Bundle"}[p["role"]]
kvr_rows = []
for p in pub:
    if p["kind"] != "plugin": continue
    kvr_rows.append([p["name"], "HUMPIRE / DJBILBOX BEATS", kvr_cat(p), p["product_type"], p["formats"], "see product page",
                     "Free" if p["is_free"] else f"{p['price']:.2f} {p['currency']}", "Freeware (demo/free version)" if p["is_free"] else "Commercial",
                     p["url"], (p["facts"] or p["subtitle"])[:200], ", ".join(tags(p)), "listed-in-catalogue; KVR status not verified"])
for n in newp:
    kvr_rows.append([n["name"], "HUMPIRE / DJBILBOX BEATS", "Instrument", n["cat"], "VST3, Standalone", "Windows (build scripts); macOS/Linux build via CMake",
                     "TBD", "TBD", "TBD", n["sub"], ", ".join(n["tags"]), "NEW - not in Gumroad catalogue"])
csvw("catalogs/catalog_kvr.csv",
 ["product_name","developer","type","category","formats","platforms","price","license","url","short_description","tags","note"], kvr_rows)

# ---- Master DB
db = dict(generated=TODAY, brands=["HUMPIRE Plugins","DJBILBOX BEATS"], site=SITE,
          counts=dict(catalogue_rows=len(prods), published=len(pub), unpublished=len(prods)-len(pub), paid=len(paid), free=len([p for p in pub if p["is_free"]]), new_products_repo=len(newp)),
          corrections=fixed, products=[dict(p, description=desc(p), tags=tags(p), site_page=page_url(p), google_category=gcat(p)) for p in prods],
          new_products=newp)
w("MASTER_PRODUCT_DATABASE.json", json.dumps(db, ensure_ascii=False, indent=1))

# ---- Site SEO : pages produits + sitemap + robots + schemas
org = {"@context":"https://schema.org","@type":"Organization","@id":SITE+"/#organization","name":"HUMPIRE Plugins","alternateName":["DJBILBOX BEATS"],
       "url":SITE,"founder":{"@type":"Person","name":"DJBILBOX"},
       "sameAs":["https://djbilboxbeats.gumroad.com/"],
       "makesOffer":[{"@type":"Offer","url":"https://djbilboxbeats.gumroad.com/"}],
       "description":"VST3 plugins (synths, effects, drum machines, guitar pedals) and royalty-free sample packs for Hip-Hop, G-Funk, West Coast, Trap and Oriental production."}
w("site_seo/organization-schema.json", json.dumps(org, ensure_ascii=False, indent=1))

def prod_schema(p):
    d = {"@context":"https://schema.org","@type":["Product","SoftwareApplication"] if p["kind"]=="plugin" else "Product",
         "@id":page_url(p)+"#product","name":p["full_title"],"sku":p["sku"],"description":desc(p),"url":page_url(p),
         "brand":{"@type":"Brand","name":p["brand"]},"category":p["product_type"],
         "offers":{"@type":"Offer","url":p["url"],"price":f"{p['price']:.2f}","priceCurrency":p["currency"],"availability":"https://schema.org/InStock","itemCondition":"https://schema.org/NewCondition","seller":{"@id":SITE+"/#organization"}}}
    if p["kind"]=="plugin": d["applicationCategory"]="MultimediaApplication"
    if p["image_link"]: d["image"]=p["image_link"]
    return d
w("site_seo/product-schema.json", json.dumps([prod_schema(p) for p in pub], ensure_ascii=False, indent=1))
w("site_seo/schema.json", json.dumps({"@context":"https://schema.org","@graph":[org,
   {"@type":"WebSite","@id":SITE+"/#website","url":SITE,"name":"HUMPIRE Plugins","publisher":{"@id":SITE+"/#organization"},"inLanguage":"en"},
   {"@type":"ItemList","@id":SITE+"/products/#list","name":"HUMPIRE & DJBILBOX BEATS catalogue","numberOfItems":len(pub),
    "itemListElement":[{"@type":"ListItem","position":i+1,"url":page_url(p),"name":p["full_title"]} for i,p in enumerate(pub)]}]}, ensure_ascii=False, indent=1))

CSS = "body{font-family:system-ui,sans-serif;max-width:760px;margin:0 auto;padding:16px;line-height:1.55;background:#0f0f12;color:#eee}a{color:#ffb347}.btn{display:inline-block;background:#ffb347;color:#111;padding:10px 18px;border-radius:6px;text-decoration:none;font-weight:700}small{opacity:.7}li{margin:4px 0}"
for p in pub:
    ttl = f"{p['full_title']} | HUMPIRE Plugins"[:70]
    ld = json.dumps(prod_schema(p), ensure_ascii=False)
    price_line = "Free download" if p["is_free"] and not p["pay_what_you_want"] else ("Free / pay what you want" if p["is_free"] else f"{p['price']:.2f} {p['currency']}")
    page = f"""<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>{html.escape(ttl)}</title><meta name="description" content="{html.escape(desc(p)[:155])}">
<link rel="canonical" href="{page_url(p)}"><meta property="og:type" content="product"><meta property="og:title" content="{html.escape(p['full_title'])}">
<meta property="og:description" content="{html.escape(desc(p)[:200])}"><meta property="og:url" content="{page_url(p)}">
<style>{CSS}</style><script type="application/ld+json">{ld}</script></head><body>
<nav><a href="/">HUMPIRE Plugins</a> › <a href="/products/">Catalogue</a></nav>
<h1>{html.escape(p['full_title'])}</h1><p>{html.escape(desc(p))}</p>
<ul><li>Type: {html.escape(p['product_type'])}</li><li>Brand: {p['brand']}</li><li>Price: {price_line}</li><li>Genres: {html.escape(', '.join(p['genres']) or 'general')}</li></ul>
<p><a class="btn" href="{p['url']}" rel="noopener">Get it on Gumroad</a></p></body></html>"""
    w(f"site_seo/products/{p['id']}/index.html", page)
groups = {}
for p in pub: groups.setdefault(p["role"], []).append(p)
lbl = {"instrument":"Instruments","effect":"Effects","midi":"MIDI generators","bundle":"Bundles","soundware":"Sample packs & kits"}
li = "".join(f"<h2>{lbl[r]}</h2><ul>"+"".join(f'<li><a href="/products/{p["id"]}/">{html.escape(p["full_title"])}</a></li>' for p in ps)+"</ul>" for r, ps in groups.items())
w("site_seo/products/index.html", f"""<!doctype html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>VST3 Plugins & Sample Packs Catalogue | HUMPIRE Plugins</title><meta name="description" content="Full catalogue of HUMPIRE VST3 plugins and DJBILBOX BEATS sample packs: synths, effects, drum machines, guitar pedals, G-Funk, West Coast, Trap and Oriental.">
<link rel="canonical" href="{SITE}/products/"><style>{CSS}</style></head><body><h1>HUMPIRE Plugins — Catalogue</h1>{li}</body></html>""")
urls = [SITE+"/", SITE+"/products/"] + [page_url(p) for p in pub]
import re as _re
_ex = open(os.path.join(OUT, "data", "site_sitemap_existing.xml"), encoding="utf-8").read()
_have = set(_re.findall(r"<loc>([^<]+)</loc>", _ex))
_new = "".join(f"  <url><loc>{u}</loc><lastmod>{TODAY}</lastmod><changefreq>monthly</changefreq><priority>0.6</priority></url>\n" for u in urls[1:] if u not in _have)
w("site_seo/sitemap.xml", _ex.replace("</urlset>", _new + "</urlset>"))

print("catalogs ok", len(pub), len(paid), len(urls))
