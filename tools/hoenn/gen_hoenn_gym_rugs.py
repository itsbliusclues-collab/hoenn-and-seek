#!/usr/bin/env python3
"""Hoenn & Seek (2.1-25 to 27): gym leaders' rooms redrawn so the leader and their Pokémon
stand side by side, centred, on a rug made for the pair (Bryan's design, PR #142,
hoenn-docs/2.1-mockups/gyms/README.md).

Reads data/hoenn/gym_rugs.json. Each entry names a map and a list of ops, applied in order:

  Block ops (whole 16x16 blocks, collision and elevation included):
    insert_col {x, rows?}   a copy of column x goes in at x+1 and the rest moves right. With
                            "rows": [y0, y1] only those rows move (the map keeps its width and
                            loses those rows' last block), for a room that floats in a void.
                            Events right of x (in those rows) move with it; warps and coord
                            events on column x get a copy on x+1 (a door gets two tiles, a row
                            of triggers stays unbroken). Objects never get copies.
    widen {}                one more column on the right, a copy of the last column, for a room
                            whose walls are the map's edges (then a partial insert_col has a
                            column to lose).
    add_event {kind, index, at: [x, y], fields?}  append a copy of an event, with some fields
                            changed (Celadon's fourth cut tree, with its own flag).
    move_event {kind, index, x}  move an event to another column.
    mirror {x0, x1, y0, y1, axis}  block (x, y) is copied to (axis - x, y) for x0 <= x <= x1
                            (the art isn't flipped: trees stay trees).
    put {src: [x, y, w, h], dst: [x, y]}  copy blocks.
    set {at: [x, y], collision?, elevation?}  change one block's collision or elevation.

  Pixel ops, on the map drawn after the block ops ("ref"; "from": "orig" reads the map as it
  was before any op, in the coordinates of the original layout):
    orn {x, y, h?}          the wall ornament on column x was copied by insert_col: fill x and
                            x+1 with the wall of column x-1, then draw the ornament once, half a
                            block right, so it sits over the room's new middle.
    nine {src, dst, b, mode?}  9-slice a rug: the b-pixel border keeps its size and the middle
                            is stretched ("scale", the default) or tiled ("repeat"). src/dst are
                            [x, y, w, h] in blocks, or in pixels with "px": true.
    paste {src, dst}        copy pixels ([x, y, w, h] and [x, y], blocks or "px").

Both layers of each block are edited as palette indices (a pixel keeps its palette), so the
result is re-tiled exactly: every changed block gets a metatile with the same attributes as
the block it replaces (behavior, layer type), made of existing tiles where one matches (with
flips) and new tiles otherwise. New tiles go into blank tiles of the secondary tileset that no
metatile uses; new metatiles go into blank metatile slots that no layout using that tileset
draws, then into new slots after the tileset's last metatile (its metatile and attribute files
grow; up to 384). Every 8x8 tile of a layer must keep one palette (the generator stops if an op mixes two).

Undo: data/hoenn/gym_rug_slots.json records what each run changed (the tileset slots and their
old contents, the events it moved or added) and data/hoenn/gym_originals/<LAYOUT>.bin keeps each
layout's blocks from before the first run, so a re-run starts from the original: re-runs give
the same result. To change a gym, edit the JSON and re-run; to drop one, delete its entry and
re-run (its layout and events go back).

Scripts aren't touched: a gym whose scripts name coordinates right of an inserted column
(Blackthorn's bridges) was edited by hand once; the JSON entry says so. The followers are placed
by gen_hoenn_followers.py ("at" in trainer_followers.json), which runs later.
"""
import json, os, re, shutil, struct, sys

ROOT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..")
P = lambda *a: os.path.join(ROOT, *a)
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
import render_layout as R  # tileset sources, palettes, tile sheets

CFG = "data/hoenn/gym_rugs.json"
SLOTS = "data/hoenn/gym_rug_slots.json"
ORIG = "data/hoenn/gym_originals"
NUM_PRIMARY = 640        # HnS primary tilesets: 640 tiles, 640 metatiles, 7 palettes
NUM_PRIMARY_PALS = 7
NUM_SECONDARY_METATILES = 384
FOLLOWER = "HoennSeek_EventScript_TrainerFollower"


def load(path):
    return json.load(open(P(path)))


def save(path, data):
    with open(P(path), "w") as fh:
        json.dump(data, fh, indent=2, ensure_ascii=False)
        fh.write("\n")


def sheet_path(tiles):
    return re.sub(r"\.4bpp(\.\w+)*$", ".png", tiles)


class Tileset:
    """A secondary tileset: its sheet, metatiles and attributes, plus the primary it is drawn with."""

    def __init__(self, name, prim, sec):
        from PIL import Image
        self.name = name
        self.sheet_file = sheet_path(sec["tiles"])
        self.sheet = Image.open(P(self.sheet_file))
        assert self.sheet.mode == "P", self.sheet_file
        self.mt_file = sec["metatiles"]
        self.attr_file = re.sub(r"metatiles\.bin$", "metatile_attributes.bin", sec["metatiles"])
        d = open(P(self.mt_file), "rb").read()
        self.mt = list(struct.unpack("<%dH" % (len(d) // 2), d))
        self.attr = list(struct.unpack("<%dH" % (len(open(P(self.attr_file), "rb").read()) // 2),
                                       open(P(self.attr_file), "rb").read()))
        self.prim_tiles = R.read_tiles(prim["tiles"])[:NUM_PRIMARY]
        self.prim_tiles += [[[0] * 8 for _ in range(8)]] * (NUM_PRIMARY - len(self.prim_tiles))
        d = open(P(prim["metatiles"]), "rb").read()
        self.prim_mt = list(struct.unpack("<%dH" % (len(d) // 2), d))
        a = open(P(re.sub(r"metatiles\.bin$", "metatile_attributes.bin", prim["metatiles"])), "rb").read()
        self.prim_attr = list(struct.unpack("<%dH" % (len(a) // 2), a))
        self.pals = [R.read_pal(f) for f in prim["pals"][:NUM_PRIMARY_PALS]] + \
                    [R.read_pal(f) for f in sec["pals"][NUM_PRIMARY_PALS:]]
        self.sheet_w = self.sheet.size[0] // 8
        self.n_sheet = self.sheet_w * (self.sheet.size[1] // 8)
        self.changed = False
        self.sheet_changed = False

    # --- tiles ---
    def sheet_tile(self, i):
        tx, ty = i % self.sheet_w, i // self.sheet_w
        return [[self.sheet.getpixel((tx * 8 + x, ty * 8 + y)) & 15 for x in range(8)] for y in range(8)]

    def put_sheet_tile(self, i, tile):
        tx, ty = i % self.sheet_w, i // self.sheet_w
        for y in range(8):
            for x in range(8):
                self.sheet.putpixel((tx * 8 + x, ty * 8 + y), tile[y][x])
        self.sheet_changed = True

    def tile(self, t):
        return self.prim_tiles[t] if t < NUM_PRIMARY else self.sheet_tile(t - NUM_PRIMARY)

    # --- metatiles ---
    def count(self):
        return len(self.mt) // 8

    def entries(self, mid):
        if mid < NUM_PRIMARY:
            return list(self.prim_mt[mid * 8:mid * 8 + 8])
        i = mid - NUM_PRIMARY
        return list(self.mt[i * 8:i * 8 + 8])

    def attribute(self, mid):
        return self.prim_attr[mid] if mid < NUM_PRIMARY else self.attr[mid - NUM_PRIMARY]

    def set_metatile(self, mid, ents, attr):
        i = mid - NUM_PRIMARY
        if ents is None:  # undo of an appended slot: drop it (and anything after it)
            del self.mt[i * 8:], self.attr[i:]
            self.changed = True
            return
        while self.count() <= i:  # a slot past the end: the tileset grows (up to 384 metatiles)
            self.mt += [0] * 8
            self.attr.append(0)
        self.mt[i * 8:i * 8 + 8] = ents
        self.attr[i] = attr
        self.changed = True

    def save(self):
        if self.sheet_changed:
            self.sheet.save(P(self.sheet_file))
        if not self.changed:
            return
        open(P(self.mt_file), "wb").write(struct.pack("<%dH" % len(self.mt), *self.mt))
        open(P(self.attr_file), "wb").write(struct.pack("<%dH" % len(self.attr), *self.attr))


def layout_blocks(lay, path=None):
    d = open(P(path or lay["blockdata_filepath"]), "rb").read()
    return list(struct.unpack("<%dH" % (len(d) // 2), d))


def used_metatiles(layouts, secondary, skip=()):
    """Metatile IDs drawn by any layout that uses this secondary tileset (its blocks and border)."""
    used = set()
    for l in layouts:
        if l.get("secondary_tileset") != secondary or l["id"] in skip:
            continue
        used |= {b & 0x3FF for b in layout_blocks(l)}
        if l.get("border_filepath") and os.path.exists(P(l["border_filepath"])):
            used |= {b & 0x3FF for b in layout_blocks(l, l["border_filepath"])}
    return used


class Canvas:
    """Both layers of a layout as palette-indexed pixels: [layer][y][x] = (palette << 4) | index."""

    def __init__(self, ts, blocks, w, h):
        self.w, self.h = w, h
        self.px = [[[0] * (w * 16) for _ in range(h * 16)] for _ in range(2)]
        for by in range(h):
            for bx in range(w):
                self.draw_block(ts, bx, by, blocks[by * w + bx] & 0x3FF)

    def draw_block(self, ts, bx, by, mid):
        ents = ts.entries(mid)
        for k, e in enumerate(ents):
            layer, q = k // 4, k % 4
            t, hf, vf, pal = e & 0x3FF, (e >> 10) & 1, (e >> 11) & 1, e >> 12
            tile = ts.tile(t)
            for y in range(8):
                for x in range(8):
                    c = tile[7 - y if vf else y][7 - x if hf else x]
                    self.px[layer][by * 16 + (q // 2) * 8 + y][bx * 16 + (q % 2) * 8 + x] = (pal << 4) | c

    def crop(self, x, y, w, h):
        return [[row[x:x + w] for row in self.px[layer][y:y + h]] for layer in range(2)]

    def paste(self, img, x, y):
        for layer in range(2):
            for j, row in enumerate(img[layer]):
                self.px[layer][y + j][x:x + len(row)] = row

    def block(self, bx, by):
        return [[row[bx * 16:bx * 16 + 16] for row in self.px[layer][by * 16:by * 16 + 16]] for layer in range(2)]


def resample(img, w, h, mode):
    """Stretch (nearest) or tile a [layer][y][x] image to w x h."""
    sh, sw = len(img[0]), len(img[0][0]) if img[0] else 0
    out = []
    for layer in range(2):
        rows = []
        for y in range(h):
            sy = (y * sh // h) if mode == "scale" else y % sh
            src = img[layer][sy]
            rows.append([src[(x * sw // w) if mode == "scale" else x % sw] for x in range(w)])
        out.append(rows)
    return out


def nine(img, w, h, b, mode):
    """9-slice: the b-pixel border keeps its size, the middle is stretched or tiled."""
    sh, sw = len(img[0]), len(img[0][0])
    out = [[[0] * w for _ in range(h)] for _ in range(2)]
    xs = [(0, b, 0, b), (b, sw - b, b, w - b), (sw - b, sw, w - b, w)]
    ys = [(0, b, 0, b), (b, sh - b, b, h - b), (sh - b, sh, h - b, h)]
    for sx0, sx1, dx0, dx1 in xs:
        for sy0, sy1, dy0, dy1 in ys:
            if sx1 <= sx0 or sy1 <= sy0 or dx1 <= dx0 or dy1 <= dy0:
                continue
            part = [[row[sx0:sx1] for row in img[layer][sy0:sy1]] for layer in range(2)]
            part = resample(part, dx1 - dx0, dy1 - dy0, mode)
            for layer in range(2):
                for j in range(dy1 - dy0):
                    out[layer][dy0 + j][dx0:dx1] = part[layer][j]
    return out


def box(op, key):
    v = op[key]
    return [c if op.get("px") else c * 16 for c in v]


class Tiler:
    """Turns edited blocks back into metatiles, reusing tiles and metatiles where they match."""

    def __init__(self, ts, free_tiles, free_mts, record):
        self.ts, self.free_tiles, self.free_mts, self.record = ts, free_tiles, free_mts, record
        self.tile_index = {}  # 64 indices -> (tile, hflip, vflip)
        for t in list(range(NUM_PRIMARY)) + [NUM_PRIMARY + i for i in range(ts.n_sheet)]:
            if t >= NUM_PRIMARY and t - NUM_PRIMARY in free_tiles:
                continue
            tile = ts.tile(t)
            for hf in (0, 1):
                for vf in (0, 1):
                    key = tuple(tile[7 - y if vf else y][7 - x if hf else x] for y in range(8) for x in range(8))
                    self.tile_index.setdefault(key, (t, hf, vf))
        self.mt_index = {}
        for mid in list(range(len(ts.prim_mt) // 8)) + [NUM_PRIMARY + i for i in range(ts.count())]:
            if mid >= NUM_PRIMARY and mid in free_mts:
                continue
            self.mt_index.setdefault((tuple(ts.entries(mid)), ts.attribute(mid)), mid)

    def tile_entry(self, pix, where):
        pals = {p >> 4 for row in pix for p in row if p & 15}
        if len(pals) > 1:
            pix = self.one_palette(pix, pals, where)
            pals = {p >> 4 for row in pix for p in row if p & 15}
        pal = pals.pop() if pals else 0
        key = tuple(p & 15 for row in pix for p in row)
        if key not in self.tile_index:
            if not self.free_tiles:
                sys.exit(f"gen_hoenn_gym_rugs: {self.ts.name} has no free tiles left")
            t = self.free_tiles.pop(0)
            self.record["tiles"].append({"slot": t, "old": self.ts.sheet_tile(t)})
            self.ts.put_sheet_tile(t, [[p & 15 for p in row] for row in pix])
            self.tile_index[key] = (NUM_PRIMARY + t, 0, 0)
        t, hf, vf = self.tile_index[key]
        return (pal << 12) | (vf << 11) | (hf << 10) | t

    def one_palette(self, pix, pals, where):
        """A tile whose pixels come from two palettes: redraw it in one of them, if that palette has
        every colour it needs, or one within a shade (rooms often load the same colours twice)."""
        rgb = lambda p: self.ts.pals[p >> 4][p & 15]
        for target in sorted(pals, key=lambda q: -sum(1 for row in pix for p in row if p & 15 and p >> 4 == q)):
            pal = self.ts.pals[target]
            out, ok = [], True
            for row in pix:
                r = []
                for p in row:
                    if not p & 15 or p >> 4 == target:
                        r.append(p)
                        continue
                    # the same colour, or one a shade off (Ecruteak's floor is drawn in two palettes
                    # whose greys differ by 8): within 16 on every channel
                    hit = sorted((max(abs(a - b) for a, b in zip(pal[i], rgb(p))), i) for i in range(1, 16))
                    hit = [i for d, i in hit if d <= 16]
                    if not hit:
                        ok = False
                        break
                    r.append((target << 4) | hit[0])
                if not ok:
                    break
                out.append(r)
            if ok:
                return out
        sys.exit(f"gen_hoenn_gym_rugs: {where} mixes palettes {sorted(pals)} in one 8x8 tile")

    def metatile(self, img, attr, where):
        ents = []
        for layer in range(2):
            for q in range(4):
                ox, oy = (q % 2) * 8, (q // 2) * 8
                ents.append(self.tile_entry([row[ox:ox + 8] for row in img[layer][oy:oy + 8]], where))
        key = (tuple(ents), attr)
        if key not in self.mt_index:
            if not self.free_mts:
                sys.exit(f"gen_hoenn_gym_rugs: {self.ts.name} has no free metatile slots left")
            mid = self.free_mts.pop(0)
            grown = mid - NUM_PRIMARY >= self.ts.count()
            self.record["metatiles"].append({"slot": mid, "old": None if grown else self.ts.entries(mid),
                                             "old_attr": 0 if grown else self.ts.attribute(mid)})
            self.ts.set_metatile(mid, ents, attr)
            self.mt_index[key] = mid
        return self.mt_index[key]


def undo(slots, layouts_by_id, tilesets):
    """Put back everything a previous run changed (tilesets last-in first-out, layouts, events)."""
    for name, rec in slots.get("tilesets", {}).items():
        ts = tilesets(name)
        for m in reversed(rec["metatiles"]):
            ts.set_metatile(m["slot"], m["old"], m["old_attr"])
        for t in reversed(rec["tiles"]):
            ts.put_sheet_tile(t["slot"], t["old"])
    for lid, rec in slots.get("layouts", {}).items():
        lay = layouts_by_id[lid]
        shutil.copyfile(P(ORIG, lid + ".bin"), P(lay["blockdata_filepath"]))
        lay["width"], lay["height"] = rec["width"], rec["height"]
    for mapname, rec in slots.get("maps", {}).items():
        path = f"data/maps/{mapname}/map.json"
        m = load(path)
        for kind, added in rec["added"].items():  # events this tool added: drop them
            if isinstance(added, int):  # older records: a count of events at the end of the list
                m[kind] = m[kind][:len(m[kind]) - added]
                continue
            for e in added:  # by content: other generators append after these (the followers)
                m[kind].remove(next(x for x in m[kind] if x == e))
        for kind, i, x in rec["moved"]:
            if i < len(m[kind]):
                m[kind][i]["x"] = x
        save(path, m)


def main():
    cfg = load(CFG)
    slots = load(SLOTS) if os.path.exists(P(SLOTS)) else {}
    lfile = load("data/layouts/layouts.json")
    layouts = [l for l in lfile["layouts"] if "id" in l]
    by_id = {l["id"]: l for l in layouts}
    sets = R.tileset_sources()
    cache = {}

    def tileset(name, primary=None):
        if name not in cache:
            prim = primary or next(l["primary_tileset"] for l in layouts if l["secondary_tileset"] == name)
            cache[name] = Tileset(name, sets[prim], sets[name])
        return cache[name]

    undo(slots, by_id, tileset)
    new_slots = {"tilesets": {}, "layouts": {}, "maps": {}}
    gyms = cfg["gyms"]
    gym_layouts = {load(f"data/maps/{g['map']}/map.json")["layout"] for g in gyms}

    for g in gyms:
        path = f"data/maps/{g['map']}/map.json"
        m = load(path)
        lay = by_id[m["layout"]]
        lid = lay["id"]
        os.makedirs(P(ORIG), exist_ok=True)
        if not os.path.exists(P(ORIG, lid + ".bin")):
            shutil.copyfile(P(lay["blockdata_filepath"]), P(ORIG, lid + ".bin"))
        new_slots["layouts"][lid] = {"width": lay["width"], "height": lay["height"]}
        ts = tileset(lay["secondary_tileset"], lay["primary_tileset"])
        rec = new_slots["tilesets"].setdefault(ts.name, {"metatiles": [], "tiles": []})
        mrec = new_slots["maps"].setdefault(g["map"], {"moved": [], "added": {}})

        W, H = lay["width"], lay["height"]
        blocks = layout_blocks(lay, f"{ORIG}/{lid}.bin")
        orig_canvas = Canvas(ts, blocks, W, H)

        # --- block ops ---
        moved = {}  # (kind, index) -> original x
        for op in g["ops"]:
            kind = op["op"]
            if kind == "insert_col":
                x = op["x"]
                y0, y1 = op.get("rows", [0, H - 1])
                full = "rows" not in op
                nw = W + 1 if full else W
                nb = []
                for y in range(H):
                    row = blocks[y * W:(y + 1) * W]
                    if y0 <= y <= y1:
                        row = row[:x + 1] + [row[x]] + row[x + 1:]
                    if len(row) > nw:
                        row = row[:nw]
                    nb += row
                blocks, W = nb, nw
                for ek in ("object_events", "warp_events", "coord_events", "bg_events"):
                    events = m.get(ek, [])
                    copies = []
                    for i, e in enumerate(events):
                        if not (y0 <= e["y"] <= y1) or e.get("script") == FOLLOWER:
                            continue  # trainer followers are placed by gen_hoenn_followers.py
                        if e["x"] > x:
                            moved.setdefault((ek, i), e["x"])
                            e["x"] += 1
                        elif e["x"] == x and ek in ("warp_events", "coord_events"):
                            c = dict(e)
                            c["x"] = x + 1
                            copies.append(c)
                    events += copies
                    mrec["added"].setdefault(ek, []).extend(copies)
            elif kind == "widen":
                # one more column on the right, a copy of the last one (a room whose walls are the
                # map's edges, so a partial insert_col afterwards has a column to drop)
                blocks = [b for y in range(H) for b in blocks[y * W:(y + 1) * W] + [blocks[y * W + W - 1]]]
                W += 1
            elif kind == "add_event":
                # a copy of an event at another x (a third cut tree where insert_col left a gap)
                ek = op["kind"]
                c = dict(m[ek][op["index"]])
                c["x"], c["y"] = op["at"]
                c.update(op.get("fields", {}))
                m[ek].append(c)
                mrec["added"].setdefault(ek, []).append(c)
            elif kind == "move_event":
                # an event to another column (Celadon's twins, centred on the wider path)
                e = m[op["kind"]][op["index"]]
                moved.setdefault((op["kind"], op["index"]), e["x"])
                e["x"] = op["x"]
            elif kind == "mirror":
                for y in range(op["y0"], op["y1"] + 1):
                    for x in range(op["x0"], op["x1"] + 1):
                        blocks[y * W + op["axis"] - x] = blocks[y * W + x]
            elif kind == "put":
                sx, sy, w, h = op["src"]
                dx, dy = op["dst"]
                src = [blocks[(sy + j) * W + sx + i] for j in range(h) for i in range(w)]
                for j in range(h):
                    for i in range(w):
                        blocks[(dy + j) * W + dx + i] = src[j * w + i]
            elif kind == "set":
                x, y = op["at"]
                b = blocks[y * W + x]
                if "collision" in op:
                    b = (b & ~0x0C00) | (op["collision"] << 10)
                if "elevation" in op:
                    b = (b & 0x0FFF) | (op["elevation"] << 12)
                blocks[y * W + x] = b
        mrec["moved"] = [[k, i, x] for (k, i), x in sorted(moved.items())]

        # --- pixel ops ---
        ref = Canvas(ts, blocks, W, H)
        cur = Canvas(ts, blocks, W, H)
        pixel_ops = [op for op in g["ops"] if op["op"] in ("orn", "nine", "paste")]
        for op in pixel_ops:
            src_canvas = orig_canvas if op.get("from") == "orig" else ref
            if op["op"] == "orn":
                x, y, h = op["x"], op["y"], op.get("h", 1)
                wall = ref.crop((x - 1) * 16, y * 16, 16, h * 16)
                piece = orig_canvas.crop(x * 16, y * 16, 16, h * 16)
                cur.paste(wall, x * 16, y * 16)
                cur.paste(wall, (x + 1) * 16, y * 16)
                cur.paste(piece, x * 16 + 8, y * 16)
            elif op["op"] == "nine":
                sx, sy, sw, sh = box(op, "src")
                dx, dy, dw, dh = box(op, "dst")
                cur.paste(nine(src_canvas.crop(sx, sy, sw, sh), dw, dh, op["b"], op.get("mode", "scale")), dx, dy)
            elif op["op"] == "paste":
                sx, sy, sw, sh = box(op, "src")
                d = op["dst"]
                dx, dy = (d if op.get("px") else [c * 16 for c in d])
                cur.paste(src_canvas.crop(sx, sy, sw, sh), dx, dy)

        # --- re-tile the blocks whose pixels changed ---
        if pixel_ops:
            # Every layout on this tileset as it is on disk now (undo put the gyms back, and gyms
            # done earlier in this run only use metatiles that are no longer blank), plus this one.
            used = used_metatiles(layouts, ts.name) | {b & 0x3FF for b in blocks}
            referenced = {e & 0x3FF for e in ts.mt}
            free_mts = [NUM_PRIMARY + i for i in range(ts.count())
                        if NUM_PRIMARY + i not in used and all(e & 0x3FF == 0 for e in ts.entries(NUM_PRIMARY + i))
                        and NUM_PRIMARY + i not in cfg.get("reserved_metatiles", {}).get(ts.name, [])]
            # then slots past the end of the tileset's metatile list (Vermilion's has 78 of 384)
            free_mts += [NUM_PRIMARY + i for i in range(ts.count(), NUM_SECONDARY_METATILES)]
            free_tiles = [i for i in range(ts.n_sheet)
                          if NUM_PRIMARY + i not in referenced and not any(any(r) for r in ts.sheet_tile(i))]
            free_tiles.reverse()  # take from the end of the sheet
            tiler = Tiler(ts, free_tiles, free_mts, rec)
            for by in range(H):
                for bx in range(W):
                    img = cur.block(bx, by)
                    if img == ref.block(bx, by):
                        continue
                    b = blocks[by * W + bx]
                    mid = tiler.metatile(img, ts.attribute(b & 0x3FF), f"{g['map']} ({bx},{by})")
                    blocks[by * W + bx] = (b & 0xFC00) | mid

        open(P(lay["blockdata_filepath"]), "wb").write(struct.pack("<%dH" % len(blocks), *blocks))
        lay["width"], lay["height"] = W, H
        save(path, m)
        print(f"{g['map']}: {W}x{H}, {len(rec['metatiles'])} new metatiles and {len(rec['tiles'])} new tiles in {ts.name} so far")

    for ts in cache.values():
        ts.save()
    new_slots["tilesets"] = {k: v for k, v in new_slots["tilesets"].items() if v["metatiles"] or v["tiles"]}
    save("data/layouts/layouts.json", lfile)
    save(SLOTS, new_slots)


def touched_files():
    """Every file a run may write, so a run that stops half-way can put them all back (otherwise a
    second run would undo from a record that doesn't match the files)."""
    cfg = load(CFG)
    slots = load(SLOTS) if os.path.exists(P(SLOTS)) else {}
    layouts = {l["id"]: l for l in load("data/layouts/layouts.json")["layouts"] if "id" in l}
    sets = R.tileset_sources()
    maps = {g["map"] for g in cfg["gyms"]} | set(slots.get("maps", {}))
    files = {CFG, SLOTS, "data/layouts/layouts.json"}
    names = set(slots.get("tilesets", {}))
    for mp in maps:
        files.add(f"data/maps/{mp}/map.json")
        lay = layouts[load(f"data/maps/{mp}/map.json")["layout"]]
        files.add(lay["blockdata_filepath"])
        names.add(lay["secondary_tileset"])
    for n in names:
        src = sets[n]
        files |= {sheet_path(src["tiles"]), src["metatiles"],
                  re.sub(r"metatiles\.bin$", "metatile_attributes.bin", src["metatiles"])}
    return {f: open(P(f), "rb").read() for f in files if os.path.exists(P(f))}


if __name__ == "__main__":
    before = touched_files()
    try:
        main()
    except SystemExit as e:
        if e.code:  # an error: leave every file as it was
            for f, data in before.items():
                open(P(f), "wb").write(data)
        raise
