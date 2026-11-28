#!/usr/bin/env python3
"""Hoenn & Seek 2.1-13: put the friend trainers' sprites into the game.

Reads the sheets in hoenn-docs/2.1-mockups/friends/ (2.1-01's placeholder
repaints, or Bryan's real designs once he saves them there under the same
names) and writes the files the build uses, in graphics/hoenn/friends/:

  <name>_front_pic.png   64x64 trainer front pic (copied as is)
  <name>_overworld.png   the 10 overworld frames of 16x32, stacked vertically
                         (2 tiles wide, so the plain png -> 4bpp rule keeps
                         every frame's tiles together; no -mwidth rule needed)

Both stay indexed PNGs with their own 16-colour palette; the build makes the
.gbapal from the PNG. Index 0 is the transparent colour.

Usage: tools/hoenn/install_friend_sprites.py   (no arguments)
"""
import os

from PIL import Image

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
SRC = os.path.join(ROOT, "hoenn-docs/2.1-mockups/friends")
OUT = os.path.join(ROOT, "graphics/hoenn/friends")
FRIENDS = ["bridgette", "aidan", "brock"]
FRAMES = 10


def indexed(path, size):
    im = Image.open(path)
    assert im.mode == "P", f"{path}: needs an indexed PNG"
    assert im.size == size, f"{path}: {im.size}, expected {size}"
    assert max(im.tobytes()) < 16, f"{path}: uses more than 16 colours"
    return im


def save16(im, path):
    """Save with exactly 16 palette entries, so the .gbapal is 32 bytes."""
    out = Image.frombytes("P", im.size, im.tobytes())
    out.putpalette(im.getpalette()[:48])
    out.save(path, bits=4)


def main():
    os.makedirs(OUT, exist_ok=True)
    for name in FRIENDS:
        pic = indexed(os.path.join(SRC, f"{name}_front_pic.png"), (64, 64))
        save16(pic, os.path.join(OUT, f"{name}_front_pic.png"))

        sheet = indexed(os.path.join(SRC, f"{name}_overworld.png"),
                        (16 * FRAMES, 32))
        tall = Image.new("P", (16, 32 * FRAMES), 0)
        tall.putpalette(sheet.getpalette()[:48])
        for f in range(FRAMES):
            tall.paste(sheet.crop((16 * f, 0, 16 * f + 16, 32)), (0, 32 * f))
        save16(tall, os.path.join(OUT, f"{name}_overworld.png"))
        print(f"{name}: front pic + {FRAMES} overworld frames")


if __name__ == "__main__":
    main()
