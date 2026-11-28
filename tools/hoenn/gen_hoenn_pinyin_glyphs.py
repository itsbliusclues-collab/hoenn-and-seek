#!/usr/bin/env python3
"""Hoenn & Seek 2.1-30: the toned pinyin vowels for Ryan's Mandarin.

Pinyin needs 24 toned vowels (ā á ǎ à, ē é ě è, ī í ǐ ì, ō ó ǒ ò, ū ú ǔ ù, ǖ ǘ ǚ ǜ).
Ten of them are already in the font (Emerald's French/Spanish/Italian letters:
à á è é ì í ò ó ù ú), so this draws the other 14 into the normal and small Latin
fonts and gives them charmap codes.

Where they go: glyphs 0x1C1-0x1CE, free cells just before the emoji block
(0x1D0). Glyphs 0x100+ are reached through CHAR_EXTRA_SYMBOL (F9), so the codes are
F9 C1-F9 CE. Every Latin font has these cells blank, so the other fonts just show
nothing there (Ryan's lines are message-box text, FONT_NORMAL).

How they're drawn: each one is its base letter's body (a e i o u ü, rows 5 down,
copied from the font) with a tone mark above it. Acute and grave marks are copied
from the font's own á/à (so they match the old accented letters exactly); the
macron (a line) and caron (a small v) are drawn in the same rows and style (dark
pixel with a light shadow to its right). Over ü the mark goes one row higher, above
the dots. Widths are the base letter's.

Idempotent: it rewrites the 14 cells, the 14 widths in both width tables
(src/fonts.c, upstream's file: only those numbers change) and the marked block at
the end of charmap.txt.

    tools/hoenn/gen_hoenn_pinyin_glyphs.py [--preview out.png]
"""
import argparse, os, re
from PIL import Image

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
P = lambda *a: os.path.join(ROOT, *a)

FIRST = 0x1C1
# (char, base letter code, tone): tone 1 macron, 2 acute, 3 caron, 4 grave.
GLYPHS = [("ā", "a", 1), ("ǎ", "a", 3), ("ē", "e", 1), ("ě", "e", 3),
          ("ī", "i", 1), ("ǐ", "i", 3), ("ō", "o", 1), ("ǒ", "o", 3),
          ("ū", "u", 1), ("ǔ", "u", 3), ("ǖ", "ü", 1), ("ǘ", "ü", 2), ("ǚ", "ü", 3), ("ǜ", "ü", 4)]
# Base glyphs: the body comes from these (FONT code points). For i the body is
# ì's (centred under a mark; plain i is narrower and has a dot).
BASE = {"a": 0xD5, "e": 0xD9, "i": 0x1E, "o": 0xE3, "u": 0xE9, "ü": 0xF6}
FG, SHADOW, BG = 1, 2, 3

FONTS = {
    # png, width table, the body's first row, the mark's two rows over a/e/i/o/u and over ü,
    # the mark's centre column per letter (columns of the 16-px cell)
    "normal": {"png": "graphics/fonts/latin_normal.png", "table": "gFontNormalLatinGlyphWidths",
               "body_row": 5, "rows": (3, 4), "rows_umlaut": (1, 2),
               "centre": {"a": 2, "e": 2, "i": 2, "o": 2, "u": 2, "ü": 2}},
    "small": {"png": "graphics/fonts/latin_small.png", "table": "gFontSmallLatinGlyphWidths",
              "body_row": 5, "rows": (3, 4), "rows_umlaut": (2, 3),
              "centre": {"a": 1, "e": 1, "i": 1, "o": 1, "u": 1, "ü": 1}},
}


def cell(g):
    return (g % 16) * 16, (g // 16) * 16


def mark_pixels(font, tone, c, top):
    """(x, y) of the mark's dark pixels for centre column c, rows top/top+1."""
    if tone == 1:  # macron: a line as wide as the mark area
        return [(c - 1, top + 1), (c, top + 1), (c + 1, top + 1)] if font == "normal" or c >= 1 \
            else [(c, top + 1), (c + 1, top + 1)]
    if tone == 2:  # acute, as in the font's á
        return [(c, top), (c + 1, top), (c - 1, top + 1)] if font == "normal" else [(c + 1, top), (c, top + 1)]
    if tone == 3:  # caron: a small v
        return [(c - 1, top), (c + 1, top), (c, top + 1)]
    return [(c - 1, top), (c, top), (c + 1, top + 1)] if font == "normal" else [(c - 1, top), (c, top + 1)]


def draw(font, px):
    f = FONTS[font]
    for k, (_, base, tone) in enumerate(GLYPHS):
        gx, gy = cell(FIRST + k)
        bx, by = cell(BASE[base])
        # clear the cell to the font's in-glyph background, keeping the cell's
        # outside colour (index 0) where the font has it
        for y in range(16):
            for x in range(16):
                px[gx + x, gy + y] = px[bx + x, by + y] if px[bx + x, by + y] == 0 else BG
        first = f["body_row"] if base != "ü" else f["rows"][0]  # ü keeps its dots
        if base == "i":
            first = f["body_row"]
        for y in range(first, 16):
            for x in range(16):
                px[gx + x, gy + y] = px[bx + x, by + y]
        top = (f["rows_umlaut"] if base == "ü" else f["rows"])[0]
        c = f["centre"][base]
        dark = mark_pixels(font, tone, c, top)
        for x, y in dark:
            px[gx + x, gy + y] = FG
        for x, y in dark:
            if (x + 1, y) not in dark and px[gx + x + 1, gy + y] == BG:
                px[gx + x + 1, gy + y] = SHADOW


def widths_update(src, table, widths):
    start = src.index(table + "[] = {")
    open_ = src.index("{", start) + 1
    close = src.index("};", open_)
    body = src[open_:close]
    nums = list(re.finditer(r"\d+", body))
    out, last = [], 0
    for i, m in enumerate(nums):
        if FIRST <= i < FIRST + len(GLYPHS):
            out.append(body[last:m.start()])
            out.append("%d" % widths[i - FIRST] if len(m.group()) == 1 else "%2d" % widths[i - FIRST])
            last = m.end()
    out.append(body[last:])
    return src[:open_] + "".join(out) + src[close:]


def width_of(src, table, code):
    body = src[src.index(table + "[] = {"):]
    body = body[body.index("{") + 1:body.index("};")]
    return [int(v) for v in re.findall(r"\d+", body)][code]


BEGIN = "@ Hoenn & Seek 2.1-30: toned pinyin vowels (tools/hoenn/gen_hoenn_pinyin_glyphs.py)"
END = "@ end of Hoenn & Seek's pinyin vowels"


def charmap_update(text):
    block = [BEGIN] + ["'%s' = F9 %02X" % (ch, (FIRST + k) & 0xFF) for k, (ch, _, _) in enumerate(GLYPHS)] + [END]
    if BEGIN in text:
        text = text[:text.index(BEGIN)] + text[text.index(END) + len(END) + 1:]
    return text.rstrip("\n") + "\n\n" + "\n".join(block) + "\n"


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--preview", help="also write a preview PNG of both fonts' new glyphs")
    args = ap.parse_args()
    src = open(P("src/fonts.c")).read()
    previews = []
    for font, f in FONTS.items():
        im = Image.open(P(f["png"]))
        draw(font, im.load())
        im.save(P(f["png"]))
        widths = [width_of(src, f["table"], BASE[b]) for _, b, _ in GLYPHS]
        src = widths_update(src, f["table"], widths)
        previews.append(im)
    open(P("src/fonts.c"), "w").write(src)
    cm = open(P("charmap.txt"), encoding="utf-8").read()
    open(P("charmap.txt"), "w", encoding="utf-8").write(charmap_update(cm))
    if args.preview:
        out = Image.new("RGB", (16 * len(GLYPHS) * 4, 16 * 2 * 4), "white")
        for r, im in enumerate(previews):
            rgb = im.convert("RGB")
            for k in range(len(GLYPHS)):
                x, y = cell(FIRST + k)
                out.paste(rgb.crop((x, y, x + 16, y + 16)).resize((64, 64), Image.NEAREST), (k * 64, r * 64))
        out.save(args.preview)


if __name__ == "__main__":
    main()
