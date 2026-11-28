#!/usr/bin/env python3
"""Hoenn & Seek 2.1-01: mockups of the three friend trainers' sprites.

Bryan's briefs (hoenn-docs/2.1-plan.md section 10):
  EXPLORER BRIDGETTE  pale-skinned, tall, explorer gear
  SKYDIVER AIDAN      athletic, in a wing suit
  ATHLETE BROCK       athletic, shirtless, baggy trousers gathered tight
                      above the ankle

Each friend gets a trainer front pic (64x64, 16 colours) and an overworld
sheet (HnS's 10 frames of 16x32). These mockups are repaints of existing
sprites in the build, so they match the game's style and scale from the start:

  Bridgette  front pic: Emerald's POKEMON RANGER (F), overworld: HnS's PICNICKER
  Aidan      front pic: Emerald's POKEMON RANGER (M), overworld: HnS's COOLTRAINER (M)
  Brock      front pic: FRLG's BLACK BELT, overworld: HnS's BLACK BELT

A repaint is palette changes, index swaps inside rectangles (to split
colours one palette entry shares, e.g. a gi top from its trousers) and a few
drawn pixels (Aidan's wing webbing and goggles). Once Bryan approves a look,
the final sprites are drawn on top of these in their own part.

Usage: tools/hoenn/draw_friend_mockups.py OUT_DIR
Writes each sprite as an indexed PNG plus friends_mockup.png, a sheet with
HnS's SILVER and LEADER BROCK beside them for scale.
"""
import os
import sys

from PIL import Image, ImageDraw

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
PICS = os.path.join(ROOT, "graphics/trainers/front_pics")
OW = os.path.join(ROOT, "graphics/object_events/pics/people")


def load(path):
    im = Image.open(path)
    assert im.mode == "P", path
    return im.copy()


def set_pal(im, colours):
    """colours: {index: (r, g, b)}"""
    pal = im.getpalette()[:48]
    for i, c in colours.items():
        pal[3 * i:3 * i + 3] = list(c)
    im.putpalette(pal + [0] * (768 - len(pal)))


def swap(im, box, mapping):
    """Inside box (x0, y0, x1, y1), exclusive ends: index a -> b."""
    px = im.load()
    x0, y0, x1, y1 = box
    for y in range(y0, min(y1, im.height)):
        for x in range(x0, min(x1, im.width)):
            if px[x, y] in mapping:
                px[x, y] = mapping[px[x, y]]


def fill_bg(im, pts, idx, outline=None):
    """Fill the background pixels (index 0) inside a polygon with idx; the
    polygon's edge pixels that are background get outline (if given)."""
    mask = Image.new("L", im.size, 0)
    d = ImageDraw.Draw(mask)
    d.polygon(pts, fill=1, outline=2 if outline is not None else 1)
    px, m = im.load(), mask.load()
    for y in range(im.height):
        for x in range(im.width):
            if px[x, y] == 0 and m[x, y]:
                px[x, y] = outline if (m[x, y] == 2 and outline is not None) else idx


# --------------------------------------------------------------- Bridgette
def bridgette_pic():
    im = load(os.path.join(PICS, "pokemon_ranger_f.png"))
    set_pal(im, {
        1: (255, 238, 230), 2: (246, 211, 196), 3: (214, 168, 150),   # pale skin
        4: (115, 74, 49), 5: (66, 41, 33),                            # dark brown hair
        6: (140, 148, 98), 7: (98, 106, 66), 8: (57, 66, 41),         # olive shirt/shorts
        11: (230, 205, 148), 12: (189, 156, 98), 13: (131, 106, 66),  # khaki jacket, hat
    })
    return im


def bridgette_ow():
    im = load(os.path.join(OW, "picnicker_hns.png"))
    set_pal(im, {
        1: (255, 238, 230), 2: (246, 211, 196), 3: (214, 168, 150),
        6: (115, 74, 49), 7: (66, 41, 33),                            # hair
        8: (230, 205, 148), 9: (189, 156, 98), 10: (115, 90, 49),     # khaki
    })
    return im


# ------------------------------------------------------------------- Aidan
SUIT = {6: (123, 189, 238), 7: (57, 123, 205), 8: (32, 57, 115)}      # sky-blue wing suit
ACCENT = {11: (255, 238, 98), 12: (238, 164, 32), 13: (148, 90, 16)}  # yellow panels


def aidan_pic():
    im = load(os.path.join(PICS, "pokemon_ranger_m.png"))
    set_pal(im, {4: (115, 74, 49), 5: (66, 41, 33), **SUIT, **ACCENT,
                 9: (230, 230, 238), 14: (255, 255, 255)})
    # Cap -> helmet: the accent colours above the face become white/grey.
    swap(im, (20, 0, 44, 16), {11: 14, 12: 9, 13: 6})
    # Goggles across the helmet's brim (yellow lenses in a dark frame).
    px = im.load()
    for x in range(26, 41):
        if px[x, 11] != 0:
            px[x, 11] = 15
        if px[x, 12] != 0:
            px[x, 12] = 10 if x not in (26, 33, 40) else 15
        if px[x, 13] != 0:
            px[x, 13] = 15
    # Webbing: from the outstretched left arm down to the hip, and a small
    # one between the right elbow and the thigh.
    fill_bg(im, [(14, 31), (26, 31), (25, 43)], 7, outline=15)
    fill_bg(im, [(43, 37), (50, 35), (45, 46)], 7, outline=15)
    return im


def aidan_ow():
    im = load(os.path.join(OW, "cooltrainer_m_hns.png"))
    set_pal(im, {8: (164, 115, 82), 9: (115, 74, 49), 10: (66, 41, 33),  # brown hair
                 11: (123, 189, 238), 12: (57, 123, 205), 13: (32, 57, 115),
                 5: (255, 238, 98)})
    # Goggles: a yellow band across the hair, front-facing frames only
    # (frames 0, 3, 4, 9 face down).
    px = im.load()
    for f in (0, 3, 4, 9):
        for x in range(4, 12):
            if px[f * 16 + x, 15] not in (0, 15):
                px[f * 16 + x, 15] = 5
    return im


# ------------------------------------------------------------------- Brock
def brock_pic():
    im = load(os.path.join(PICS, "black_belt_frlg.png"))
    set_pal(im, {
        1: (238, 189, 156), 2: (214, 156, 115), 3: (172, 115, 82), 4: (106, 66, 41),  # tanned skin
        10: (106, 115, 82), 11: (74, 82, 57),                           # olive trousers
        12: (255, 205, 65), 13: (205, 139, 32),                         # headband: gold
    })
    # Gi top (above the belt, row 39) -> bare torso; gi below -> baggy trousers.
    swap(im, (0, 20, 64, 39), {14: 1, 9: 2, 5: 3, 6: 4})
    swap(im, (0, 42, 64, 64), {14: 10, 9: 10, 5: 11, 6: 11})
    # Gathered above the ankle: a dark band where each trouser leg ends.
    px = im.load()
    for y in (55, 56):
        for x in range(64):
            if px[x, y] in (10, 11):
                px[x, y] = 8
    return im


def brock_ow():
    im = load(os.path.join(OW, "black_belt_hns.png"))
    set_pal(im, {8: (106, 115, 82), 9: (74, 82, 57), 10: (41, 49, 33),
                 1: (246, 205, 172), 2: (222, 164, 123), 3: (180, 123, 90), 4: (106, 66, 41)})
    swap(im, (0, 22, 160, 26), {14: 1, 11: 2, 12: 3, 13: 4})   # bare torso
    swap(im, (0, 27, 160, 32), {14: 8, 11: 8, 12: 9, 13: 9})   # trousers
    px = im.load()
    for x in range(160):                                       # ankle band
        if px[x, 28] in (8, 9):
            px[x, 28] = 10
    return im


# ------------------------------------------------------------------- sheet
def on_white(im):
    rgba = im.convert("RGBA")
    bg = im.getpalette()[:3]
    px = rgba.load()
    for y in range(rgba.height):
        for x in range(rgba.width):
            if px[x, y][:3] == tuple(bg):
                px[x, y] = (255, 255, 255, 255)
    return rgba.convert("RGB")


def sheet(friends, out):
    S = 3
    refs = [("SILVER (HnS)", load(os.path.join(PICS, "silver_hns.png"))),
            ("LEADER BROCK (HnS)", load(os.path.join(PICS, "leader_brock_hns.png")))]
    pics = refs + [(n, p) for n, p, _ in friends]
    W = 64 * S + 16
    H = 64 * S + 24 + 32 * S * 1 + 40
    img = Image.new("RGB", (W * len(pics), H), "white")
    d = ImageDraw.Draw(img)
    for i, (name, p) in enumerate(pics):
        x = i * W + 8
        d.text((x, 4), name, fill="black")
        img.paste(on_white(p).resize((64 * S, 64 * S), Image.NEAREST), (x, 20))
    # Overworld frames under each friend: down, up, left, then one walk frame each way.
    owref = load(os.path.join(OW, "cooltrainer_m_hns.png"))
    rows = [("overworld (HnS COOLTRAINER for scale)", owref, 0)] + [(n, o, 2 + k) for k, (n, _, o) in enumerate(friends)]
    for name, o, col in rows:
        x = col * W + 8
        y = 64 * S + 28
        frames = on_white(o)
        for j, f in enumerate((0, 1, 2, 3, 7)):
            fr = frames.crop((f * 16, 0, f * 16 + 16, 32)).resize((16 * 2, 32 * 2), Image.NEAREST)
            img.paste(fr, (x + j * 36, y))
    d.text((8, H - 16), "overworld frames: down, up, left, walk down, walk left (2x); front pics 3x", fill="black")
    img.save(out)


def main():
    out = sys.argv[1] if len(sys.argv) > 1 else "."
    os.makedirs(out, exist_ok=True)
    friends = [("EXPLORER BRIDGETTE", bridgette_pic(), bridgette_ow()),
               ("SKYDIVER AIDAN", aidan_pic(), aidan_ow()),
               ("ATHLETE BROCK", brock_pic(), brock_ow())]
    for (name, pic, ow), key in zip(friends, ("bridgette", "aidan", "brock")):
        pic.save(os.path.join(out, f"{key}_front_pic.png"))
        ow.save(os.path.join(out, f"{key}_overworld.png"))
    sheet(friends, os.path.join(out, "friends_mockup.png"))
    print("wrote", out)


if __name__ == "__main__":
    main()
