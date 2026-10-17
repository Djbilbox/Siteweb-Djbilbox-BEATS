"""Fabrique l'icone de ROCKPUNK (Assets/ROCKPUNK.ico + icon_source.png).

Un eclair plein, sans lettrage : a 16 px une abreviation devient une tache, une
silhouette reste lisible. Palette de la facade : noir profond, orange neon.
"""

import os

from PIL import Image, ImageDraw, ImageFilter

SIZE = 1024
BG_OUTER = (10, 9, 12, 255)
BG_INNER = (24, 20, 26, 255)
ORANGE = (255, 122, 24, 255)
ORANGE_HOT = (255, 196, 96, 255)

# Eclair, en fractions du cote, dessine une fois puis mis a l'echelle.
BOLT = [
    (0.575, 0.085), (0.290, 0.545), (0.455, 0.545),
    (0.375, 0.915), (0.720, 0.430), (0.545, 0.430),
]


def rounded(size, radius, fill):
    img = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    ImageDraw.Draw(img).rounded_rectangle([0, 0, size - 1, size - 1], radius, fill=fill)
    return img


def build():
    img = rounded(SIZE, int(SIZE * 0.22), BG_OUTER)

    inset = int(SIZE * 0.045)
    inner = rounded(SIZE - 2 * inset, int(SIZE * 0.185), BG_INNER)
    img.paste(inner, (inset, inset), inner)

    points = [(x * SIZE, y * SIZE) for x, y in BOLT]

    # Halo : le meme eclair, epaissi et flouté, sous le trait net.
    glow = Image.new("RGBA", (SIZE, SIZE), (0, 0, 0, 0))
    ImageDraw.Draw(glow).polygon(points, fill=(255, 110, 20, 190))
    glow = glow.filter(ImageFilter.GaussianBlur(SIZE * 0.045))
    img = Image.alpha_composite(img, glow)

    draw = ImageDraw.Draw(img)
    draw.polygon(points, fill=ORANGE)

    # Liseré clair sur le contour de l'eclair. Trace ferme sur le polygone
    # entier : suivre seulement quelques sommets barrait la forme d'une diagonale
    # qui se lisait comme une rayure, pas comme un reflet.
    draw.line(points + [points[0]], fill=ORANGE_HOT,
              width=max(2, SIZE // 200), joint="curve")

    return img


def main():
    here = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    assets = os.path.join(here, "Assets")
    os.makedirs(assets, exist_ok=True)

    img = build()

    png = os.path.join(assets, "icon_source.png")
    img.save(png)

    ico = os.path.join(assets, "ROCKPUNK.ico")
    img.save(ico, sizes=[(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)])

    print("ecrit : %s\n       %s" % (png, ico))


if __name__ == "__main__":
    main()
