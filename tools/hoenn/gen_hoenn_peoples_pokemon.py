#!/usr/bin/env python3
"""Hoenn & Seek 2.1-28: people's Pokémon (plan section 8).

HnS's Pokémon objects nearly all sit in scenery, and most houses have none. This gives
people their Pokémon, from data/hoenn/peoples_pokemon.json (its comment explains the keys):

  towns      up to N owner-pet pairs per town: a person whose graphics are in `owners`
             gets a Pokémon of a species that fits them, standing beside them and facing
             them;
  interiors  one pet in every house or shop (by name) that has no Pokémon object yet:
             beside a person who is an owner if there is one, else anywhere free, with
             the room's own species (`room_species`) or the owner's.

Every pet is one object event (~24 bytes, no code), runs HoennSeek_EventScript_PeoplesPet
(plays its cry; data/scripts/hoenn_2_1_lane_b.inc) and takes its owner's flag, so it hides
whenever they do. The script name is how re-runs find and replace them, so this is
idempotent. Placement is deterministic per map (a seed from the map's name).

Who can be an owner: listed graphics, not a trainer, standing still (no wander or walk
movement), and never moved by a script (their local ID or VAR_LAST_TALKED in an
applymovement / removeobject / setobjectxy line of their own script or their map's), since
a person who walks off in a scene would leave the pet standing there.

Where a pet can stand (all of these):
  - plain floor, not on the map's edge (the trainer followers' rules: collision 0, a floor behavior, the owner's
    elevation, no forced-move tile next to it) with nothing drawn on its top layer;
  - not on or next to a warp or a trigger, not next to a sign, not on any object or its
    wander range, not in front of any object that faces a fixed way (where you talk to it),
    not in a trainer's line of sight;
  - blocking it cuts nothing off: every tile the player could reach (flood fill from the
    warps, and from the map's edges outdoors) is still reachable, and every person and
    sign that could be talked to still can (across a counter counts);
  - the map stays within the screen checker's budget (2.1-03; or its baseline, if it was
    already over), checked with the pet added, and under 60 objects;
  - in towns, at least 6 tiles from another pet.
Beside the owner means their left or right side, then behind them, then the front
diagonals, so the tile in front of them stays free.
"""
import json, os, random, re, sys, zlib

HERE = os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, HERE)
import check_screens as CS          # noqa: E402  (budgets, map list, counting)
import gen_hoenn_followers as F     # noqa: E402  (grid, floor rules, sight lines)
import gen_hoenn_ow_mons as OW      # noqa: E402  (which metatiles draw on their top layer)

ROOT = os.path.normpath(os.path.join(HERE, "..", ".."))
P = lambda *a: os.path.join(ROOT, *a)
SCRIPT = "HoennSeek_EventScript_PeoplesPet"
MAX_OBJECTS = 60   # OBJECT_EVENT_TEMPLATES_COUNT is 64: keep room for later parts
TOWN_SPACING = 6
DIRS = F.DIRS
# The followers' floor, plus HnS's house floors and rugs, which use the decoration
# behaviors (MB_HOLDS_*_DECORATION) as plain floor. Chairs (MB_IMPASSABLE_*) are not floor.
FLOOR = F.FLOOR | {F.MB["MB_HOLDS_SMALL_DECORATION"], F.MB["MB_HOLDS_LARGE_DECORATION"]}
OPPOSITE = {"UP": "DOWN", "DOWN": "UP", "LEFT": "RIGHT", "RIGHT": "LEFT"}
STILL = re.compile(r"MOVEMENT_TYPE_(NONE|FACE_|LOOK_AROUND|WALK_IN_PLACE_|ROTATE_|WALK_SLOWLY_IN_PLACE_|JOG_IN_PLACE_)")
MOVES = re.compile(r"^\s*(applymovement|removeobject|setobjectxy|setobjectxyperm|copyobjectxytoperm|hideobjectat)\s+(\w+)")


# ---------------------------------------------------------------- inputs

def covered_grid(layout_id):
    """{(x, y): True} where the metatile draws on its top layer (roof edges, treetops,
    table tops): a pet there would be hidden or look like it stands on top."""
    l = F.LAYOUTS[layout_id]
    w, h = l["width"], l["height"]
    import struct
    d = struct.unpack("<%dH" % (w * h), open(P(l["blockdata_filepath"]), "rb").read()[: w * h * 2])
    (_, pcov), (_, scov) = OW.TS[l["primary_tileset"]], OW.TS[l["secondary_tileset"]]
    npri = 640 if l.get("layout_version") == "hns" else 512
    out = {}
    for y in range(h):
        for x in range(w):
            mt = d[y * w + x] & 0x3FF
            if mt < npri:
                out[x, y] = pcov[mt] if mt < len(pcov) else True
            else:
                out[x, y] = scov[mt - npri] if mt - npri < len(scov) else True
    return out


def script_bodies():
    """label -> its lines, for every map script and shared script file."""
    bodies, files = {}, []
    for folder in os.listdir(P("data/maps")):
        f = P("data/maps", folder, "scripts.inc")
        if os.path.exists(f):
            files.append(f)
    for f in os.listdir(P("data/scripts")):
        if f.endswith(".inc"):
            files.append(P("data/scripts", f))
    for f in files:
        cur = None
        for line in open(f, encoding="utf-8", errors="replace"):
            m = re.match(r"^(\w+):{1,2}", line)
            if m:
                cur = bodies.setdefault(m.group(1), [])
                continue
            if cur is not None:
                cur.append(line)
    return bodies


BODIES = script_bodies()
REFS = re.compile(r"\b(?:goto|call|goto_if\w*|call_if\w*)\b[^@\n]*?\b(\w+_EventScript_\w+|\w+_Script_\w+)")


def script_moves_talker(label, depth=0, seen=None):
    """True if the script (or what it jumps to) moves or removes the one you talk to."""
    seen = seen if seen is not None else set()
    if label in seen or depth > 4 or label not in BODIES:
        return False
    seen.add(label)
    for line in BODIES[label]:
        m = MOVES.match(line)
        if m and m.group(2) == "VAR_LAST_TALKED":
            return True
        for ref in REFS.findall(line):
            if script_moves_talker(ref, depth + 1, seen):
                return True
    return False


def moved_local_ids(folder):
    """Local IDs any line of the map's own scripts moves, hides or removes."""
    f = P("data/maps", folder, "scripts.inc")
    out = set()
    if os.path.exists(f):
        for line in open(f, encoding="utf-8", errors="replace"):
            m = MOVES.match(line)
            if m:
                out.add(m.group(2))
    return out


def never_species(cfg):
    out = {s for s in cfg["never"]}
    dist = json.load(open(P("data/hoenn/distribution.json")))
    for sp, d in dist["one_of_a_kind"].items():
        out.add(sp.replace("SPECIES_", ""))
        out |= {f.replace("SPECIES_", "") for f in d.get("family", [])}
    return out


# ---------------------------------------------------------------- the map

class Room:
    def __init__(self, folder, m):
        self.folder, self.m = folder, m
        self.cells = F.grid(m["layout"])
        self.cov = covered_grid(m["layout"])
        self.outdoor = m["map_type"] not in ("MAP_TYPE_INDOOR",)
        self.objs = [o for o in m.get("object_events", []) if o.get("type") != "clone" and "graphics_id" in o]
        self.blocked = self._blocked()
        self.before = self.reach([])
        self.talk_before = self.talkable(self.before)

    def _blocked(self):
        m, b = self.m, set()
        for w in m.get("warp_events", []) + m.get("coord_events", []):
            for dx in (-1, 0, 1):
                for dy in (-1, 0, 1):
                    b.add((w["x"] + dx, w["y"] + dy))
        for e in m.get("bg_events", []):
            b.add((e["x"], e["y"]))
            for dx, dy in DIRS.values():
                b.add((e["x"] + dx, e["y"] + dy))
        for o in self.objs:
            x, y = int(o["x"]), int(o["y"])
            rx = ry = 0  # a range only matters for people who walk about
            if not STILL.match(o.get("movement_type", "")):
                rx = int(o.get("movement_range_x", 0) or 0) + 1
                ry = int(o.get("movement_range_y", 0) or 0) + 1
            for xx in range(x - rx, x + rx + 1):
                for yy in range(y - ry, y + ry + 1):
                    b.add((xx, yy))
            if STILL.match(o.get("movement_type", "")) and "LOOK_AROUND" not in o["movement_type"] \
                    and "ROTATE" not in o["movement_type"]:
                fx, fy = DIRS[F.facing(o)]
                b.add((x + fx, y + fy))  # where you stand to talk to them
        b |= F.sight_tiles(self.objs)
        return b

    def reach(self, extra):
        """Tiles the player can get to (objects' tiles are stops, not walls)."""
        stops = {(int(o["x"]), int(o["y"])) for o in self.objs + extra}
        seeds = [(w["x"], w["y"]) for w in self.m.get("warp_events", [])]
        if self.outdoor:
            seeds += [xy for xy in self.cells if (xy[0] in (0, self.w - 1) or xy[1] in (0, self.h - 1))
                      and not self.cells[xy][0]]
        seen, todo = set(), list(seeds)
        while todo:
            xy = todo.pop()
            if xy in seen or xy not in self.cells:
                continue
            seen.add(xy)
            if xy in stops:
                continue
            x, y = xy
            for dx, dy in DIRS.values():
                n = (x + dx, y + dy)
                if n in self.cells and not self.cells[n][0] and n not in seen:
                    todo.append(n)
        return seen

    @property
    def w(self):
        return F.LAYOUTS[self.m["layout"]]["width"]

    @property
    def h(self):
        return F.LAYOUTS[self.m["layout"]]["height"]

    def talkable(self, reach, extra_stops=()):
        """Objects and signs the player can talk to: a reachable free neighbour, or one
        two tiles away across a counter."""
        stops = {(int(o["x"]), int(o["y"])) for o in self.objs} | set(extra_stops)
        free = reach - stops
        out = set()
        targets = [("o%d" % i, int(o["x"]), int(o["y"])) for i, o in enumerate(self.objs)]
        targets += [("b%d" % i, e["x"], e["y"]) for i, e in enumerate(self.m.get("bg_events", []))]
        for key, x, y in targets:
            for dx, dy in DIRS.values():
                if (x + dx, y + dy) in free:
                    out.add(key)
                    break
                mid = self.cells.get((x + dx, y + dy))
                if mid and mid[2] == F.MB["MB_COUNTER"] and (x + 2 * dx, y + 2 * dy) in free:
                    out.add(key)
                    break
        return out

    def floor_ok(self, xy, elev):
        c = self.cells.get(xy)
        if c is None or c[0] or c[2] not in FLOOR or self.cov.get(xy, True):
            return False
        if not (1 <= xy[0] < self.w - 1 and 1 <= xy[1] < self.h - 1):
            return False  # never on the map's edge (outdoors that's where the next map starts)
        if elev and c[1] not in (0, elev):
            return False
        x, y = xy
        if any(self.cells.get((x + a, y + b), (0, 0, 0))[2] in F.FORCED for a, b in DIRS.values()):
            return False
        return xy not in self.blocked

    def cuts_nothing(self, xy):
        pet = [{"x": xy[0], "y": xy[1]}]
        after = self.reach(pet)
        if self.before - after:
            return False
        return self.talk_before <= self.talkable(after, extra_stops=[xy])


def screen_counts(m):
    res = [CS.count_map(m, s) for s in ("day", "night")]
    return max(r[0] for r in res), max(r[1] for r in res)


# ---------------------------------------------------------------- placing

def pet_object(species, xy, elev, flag, movement):
    return {
        "graphics_id": "OBJ_EVENT_GFX_MON_BASE+SPECIES_" + species,
        "x": xy[0], "y": xy[1], "elevation": elev,
        "movement_type": movement, "movement_range_x": 0, "movement_range_y": 0,
        "trainer_type": "TRAINER_TYPE_NONE", "trainer_sight_or_berry_tree_id": "0",
        "script": SCRIPT, "flag": flag,
    }


def beside(owner):
    """Candidate tiles around an owner, best first: sides, behind, front diagonals."""
    fx, fy = DIRS[F.facing(owner)]
    sx, sy = -fy, fx
    x, y = int(owner["x"]), int(owner["y"])
    order = [(sx, sy), (-sx, -sy), (-fx, -fy), (sx - fx, sy - fy), (-sx - fx, -sy - fy),
             (sx + fx, sy + fy), (-sx + fx, -sy + fy)]
    return [(x + dx, y + dy) for dx, dy in order]


def toward(frm, to):
    dx, dy = to[0] - frm[0], to[1] - frm[1]
    if abs(dx) >= abs(dy) and dx:
        return "RIGHT" if dx > 0 else "LEFT"
    return "DOWN" if dy > 0 else "UP"


class Placer:
    def __init__(self, folder, m, cfg, never, baseline):
        self.folder, self.m, self.cfg, self.never = folder, m, cfg, never
        self.room = Room(folder, m)
        self.rng = random.Random(zlib.crc32(folder.encode()))
        b = baseline.get(folder, [0, 0])
        self.allow = (max(CS.OBJECT_BUDGET, b[0]), max(CS.PALETTE_BUDGET, b[1]))
        self.moved = moved_local_ids(folder)
        self.added = []
        self.used = set()

    def owners(self):
        out = []
        for o in self.room.objs:
            if o["graphics_id"] not in self.cfg["owners"]:
                continue
            if o.get("trainer_type", "TRAINER_TYPE_NONE") not in ("TRAINER_TYPE_NONE", "0"):
                continue
            if not STILL.match(o.get("movement_type", "")):
                continue
            if o.get("local_id") in self.moved or script_moves_talker(o.get("script", "")):
                continue
            out.append(o)
        return out

    def species_for(self, choices):
        """Species to try for one owner, best first: theirs not used yet on this map (in a
        random order), theirs already used, then (towns only) the species the map already
        shows, whose palette is often loaded anyway. The screen budget decides."""
        choices = [s for s in choices if s not in self.never]
        fresh = [s for s in choices if s not in self.used]
        self.rng.shuffle(fresh)
        out = fresh + [s for s in choices if s in self.used]
        if self.room.outdoor:
            local = sorted({o["graphics_id"].split("SPECIES_")[1] for o in self.room.objs
                            if "MON_BASE+SPECIES_" in o["graphics_id"] and o.get("script") != SCRIPT
                            and "+" not in o["graphics_id"].split("SPECIES_")[1]})
            out += [s for s in local if s not in out and s not in self.never and (s in self.cfg["local_ok"] or s in OWNER_SPECIES)]
        return out

    def try_place(self, species, xy, elev, flag, movement):
        r = self.room
        if not r.floor_ok(xy, elev):
            return False
        if any(abs(xy[0] - p["x"]) < TOWN_SPACING and abs(xy[1] - p["y"]) < TOWN_SPACING
               for p in self.added) and r.outdoor:
            return False
        if not r.cuts_nothing(xy):
            return False
        if len(self.m["object_events"]) + 1 > MAX_OBJECTS:
            return False
        pet = pet_object(species, xy, elev, flag, movement)
        self.m["object_events"].append(pet)
        o, p = screen_counts(self.m)
        if o > self.allow[0] or p > self.allow[1]:
            self.m["object_events"].pop()
            return False
        self.added.append(pet)
        self.used.add(species)
        r.blocked |= {(xy[0] + a, xy[1] + b) for a in (-1, 0, 1) for b in (-1, 0, 1)}
        return True

    def with_owner(self, owner, species_list):
        elev = int(owner.get("elevation", 0))
        ox, oy = int(owner["x"]), int(owner["y"])
        for species in species_list:
            for xy in beside(owner):
                face = toward(xy, (ox, oy))
                movement = self.rng.choice(["MOVEMENT_TYPE_FACE_" + face, "MOVEMENT_TYPE_FACE_" + face,
                                            "MOVEMENT_TYPE_WALK_IN_PLACE_" + face, "MOVEMENT_TYPE_LOOK_AROUND"])
                if self.try_place(species, xy, elev, owner.get("flag", "0"), movement):
                    return True
        return False

    def town(self, pairs):
        owners = self.owners()
        self.rng.shuffle(owners)
        for owner in owners:
            if len(self.added) >= pairs:
                break
            self.with_owner(owner, self.species_for(self.cfg["owners"][owner["graphics_id"]]))

    def interior(self):
        rs = self.cfg["interiors"]["room_species"].get(self.folder)
        owners = self.owners()
        self.rng.shuffle(owners)
        for owner in owners:
            if self.with_owner(owner, [rs] if rs else self.species_for(self.cfg["owners"][owner["graphics_id"]])):
                return
        # Nobody to sit beside: anywhere free in the room, facing down or looking around.
        fb = self.cfg["interiors"]["fallback"]
        kind = next((k for k in fb if k and k in self.folder), "")
        sp = rs or self.rng.choice([s for s in fb[kind] if s not in self.never])
        elev = max((int(o.get("elevation", 0)) for o in self.room.objs), default=3) or 3
        tiles = sorted(xy for xy in self.room.cells)
        self.rng.shuffle(tiles)
        for xy in tiles:
            mv = self.rng.choice(["MOVEMENT_TYPE_FACE_DOWN", "MOVEMENT_TYPE_LOOK_AROUND",
                                  "MOVEMENT_TYPE_WALK_IN_PLACE_DOWN"])
            if self.try_place(sp, xy, elev, "0", mv):
                return


OWNER_SPECIES = set()


def is_pokemon(o):
    g = o.get("graphics_id", "")
    return "MON_BASE" in g or "GFX_SPECIES" in g


def wanted(folder, m, cfg):
    """'town', 'interior' or None."""
    if folder in cfg["skip"] or folder.endswith("_hoenn"):
        return None
    region = "hoenn" if m.get("game_version") == "emerald" else "johto_kanto"
    if region not in cfg["regions"]:
        return None
    if m["map_type"] in cfg["towns"]["types"]:
        return "town"
    if m["map_type"] == "MAP_TYPE_INDOOR":
        ic = cfg["interiors"]
        if any(s in folder for s in ic["include"]) and not any(s in folder for s in ic["exclude"]):
            others = [o for o in m.get("object_events", []) if o.get("script") != SCRIPT]
            if not any(is_pokemon(o) for o in others):
                return "interior"
    return None


def main():
    cfg = json.load(open(P("data/hoenn/peoples_pokemon.json")))
    never = never_species(cfg)
    OWNER_SPECIES.update(s for v in cfg["owners"].values() for s in v)
    for sp in {s for v in cfg["owners"].values() for s in v} | set(cfg["interiors"]["room_species"].values()):
        if sp in never:
            sys.exit("peoples_pokemon.json: %s is one-of-a-kind or never a pet" % sp)
    baseline = json.load(open(CS.BASELINE))["maps"] if os.path.exists(CS.BASELINE) else {}
    towns = rooms = pets = 0
    missed = []
    for folder, m in CS.hns_maps():
        path = P("data/maps", folder, "map.json")
        text = open(path).read()
        old = m.get("object_events", [])
        m["object_events"] = [o for o in old if o.get("script") != SCRIPT]
        kind = wanted(folder, m, cfg)
        if kind and m["layout"] in F.LAYOUTS:
            pl = Placer(folder, m, cfg, never, baseline)
            if kind == "town":
                pl.town(cfg["towns"]["pairs"].get(folder, cfg["towns"]["pairs_default"]))
                towns += bool(pl.added)
            else:
                pl.interior()
                rooms += bool(pl.added)
                if not pl.added:
                    missed.append(folder)
            pets += len(pl.added)
        if len(m["object_events"]) != len(old) or any(a != b for a, b in zip(m["object_events"], old)):
            new = json.dumps(m, indent=2, ensure_ascii=False) + "\n"
            if new != text:
                open(path, "w").write(new)
    print(f"{pets} people's Pokémon: {towns} towns, {rooms} rooms")
    if missed:
        print("no room for a pet in: " + ", ".join(missed))


if __name__ == "__main__":
    main()
    # 2.1-03: stop if this run's objects crowd a screen past the sprite budget.
    sys.exit(CS.main(["--quiet"]))
