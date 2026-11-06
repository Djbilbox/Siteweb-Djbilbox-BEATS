"""Images de l'assistant d'installation (Inno Setup) a partir du decor Flow de l'interface.
usage : python make_art.py <decor.jpg> <logo_humpire_h.png>   -> ecrit art/*.bmp
"""
import os, sys
from PIL import Image, ImageDraw, ImageFont, ImageFilter

bg = Image.open(sys.argv[1]).convert("RGB")
logo = Image.open(sys.argv[2]).convert("RGBA")
out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "art")
os.makedirs(out, exist_ok=True)
F = "C:/Windows/Fonts/"

def glow(img, xy, text, font, fill, gcol, r=10, n=2):
    layer = Image.new("RGBA", img.size, (0, 0, 0, 0))
    ImageDraw.Draw(layer).text(xy, text, font=font, fill=gcol, anchor="mm")
    b = layer.filter(ImageFilter.GaussianBlur(r))
    for _ in range(n):
        img = Image.alpha_composite(img, b)
    ImageDraw.Draw(img).text(xy, text, font=font, fill=fill, anchor="mm", stroke_width=2, stroke_fill=(20, 6, 40, 255))
    return img

# image laterale 410x797 : ciel etoile en haut (titre), voiture et feux arriere en bas
big = (410, 797)
w = int(bg.height * big[0] / big[1])
x0 = 700                      # cadre sur l'arriere de la voiture et le bandeau de feux
side = bg.crop((x0, 0, x0 + w, bg.height)).resize(big, Image.LANCZOS).convert("RGBA")
shade = Image.new("RGBA", big, (0, 0, 0, 0))
d = ImageDraw.Draw(shade)
for y in range(360):
    d.line([(0, y), (big[0], y)], fill=(8, 3, 20, int(225 * (1 - y / 360) ** 1.1)))
for y in range(big[1] - 200, big[1]):
    d.line([(0, y), (big[0], y)], fill=(8, 3, 20, int(235 * ((y - (big[1] - 200)) / 200) ** 1.3)))
side = Image.alpha_composite(side, shade)
side = glow(side, (205, 92), "NEON", ImageFont.truetype(F + "arialbi.ttf", 92), (245, 235, 255, 255), (255, 47, 180, 230), 14, 3)
side = glow(side, (205, 178), "SYNTH", ImageFont.truetype(F + "arialbi.ttf", 92), (255, 150, 220, 255), (255, 47, 180, 230), 14, 3)
side = glow(side, (205, 266), "80s  2.0", ImageFont.truetype(F + "ariblk.ttf", 56), (255, 214, 70, 255), (255, 120, 40, 200), 10, 2)
lg = logo.resize((70, int(logo.height * 70 / logo.width)), Image.LANCZOS)
side.alpha_composite(lg, (24, big[1] - lg.height - 26))
dd = ImageDraw.Draw(side)
dd.text((106, big[1] - 80), "HUMPIRE", font=ImageFont.truetype(F + "ariblk.ttf", 30), fill=(255, 255, 255))
dd.text((107, big[1] - 46), "BY DJBILBOX BEATS", font=ImageFont.truetype(F + "segoeuib.ttf", 18), fill=(220, 210, 240))
side = side.convert("RGB")
side.save(os.path.join(out, "preview_side.png"))

# tailles Inno Setup 100 % .. 250 % (meme cadrage, recadre au centre si le ratio change)
for sz in [(164, 314), (192, 386), (246, 459), (273, 556), (328, 604), (355, 700), (410, 797)]:
    r = sz[0] / sz[1]
    W, H = side.size
    cw = min(W, int(H * r)); ch = int(cw / r)
    side.crop(((W - cw) // 2, 0, (W - cw) // 2 + cw, ch)).resize(sz, Image.LANCZOS).save(os.path.join(out, f"wizard_{sz[0]}x{sz[1]}.bmp"))

# petite image d'en-tete : logo HUMPIRE sur fond violet sombre
for sz in [(55, 58), (64, 68), (83, 80), (92, 97), (110, 106), (119, 123), (138, 140)]:
    im = Image.new("RGBA", sz, (18, 10, 34, 255))
    s = int(min(sz) * 0.86)
    l = logo.resize((s, int(logo.height * s / logo.width)), Image.LANCZOS)
    im.alpha_composite(l, ((sz[0] - l.width) // 2, (sz[1] - l.height) // 2))
    im.convert("RGB").save(os.path.join(out, f"small_{sz[0]}x{sz[1]}.bmp"))
print("ok")
