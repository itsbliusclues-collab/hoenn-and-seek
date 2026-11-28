#!/usr/bin/env python3
"""Hoenn & Seek 2.1-04: which key items can a player hold at once?

The key-item pocket has BAG_KEYITEMS_COUNT slots (include/constants/global.h).
Every key item takes one slot, whatever the quantity, so a full pocket means
the next key item a script gives is lost (giveitem's "no room" text).

This lists every item whose pocket is POCKET_KEY_ITEMS in the HnS build
(src/data/items.h, read through the preprocessor so HnS's #ifs apply), then
looks for every way to get one and every way to lose one:

  gets   giveitem / additem / finditem / giveitem_msg in a script the HnS build
         assembles (data/scripts/*.inc, and data/maps/<map>/scripts.inc for maps
         in the build), `setvar VAR_0x8000, ITEM_X` (the callstd item helpers),
         `.2byte ITEM_X` (mart lists), item balls and hidden items in map.json,
         and AddBagItem(ITEM_X, ...) in C (src/debug.c excluded).
  loses  removeitem in scripts, RemoveBagItem(ITEM_X, ...) in C.

The "most held at once" is an upper bound: every item that can be got, minus the
ones a script always takes back before the story can go on (the table in
TAKEN_BACK says which, and why). Items listed in UNREACHABLE are given only by
scripts the post-Emerald Hoenn state never runs (the reason is next to each).

Exit status 1 when the bound comes within MARGIN of the pocket's size.
regen_all.sh runs it (report: hoenn-docs/key-items.md).

Usage: key_item_audit.py [--report FILE]
"""
import json, os, re, subprocess, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
P = lambda *a: os.path.join(ROOT, *a)

# Given, then always taken back by the story before another key item can be
# got, so two of a pair never sit in the bag together. Checked by hand (2.1-04).
TAKEN_BACK = {
    "ITEM_MYSTERY_EGG": "Elm takes it (New Bark lab) before the story goes on",
    "ITEM_SECRET_POTION": "Jasmine takes it for Amphy before Olivine's gym opens",
    "ITEM_MACHINE_PART": "returned to the Power Plant before the train runs",
    "ITEM_METEORITE": "Tate & Liza's gym waits until it's delivered (2.0 part 13b)",
    "ITEM_MACH_BIKE": "the Mauville bike shop swaps it for the ACRO BIKE: one of the two",
}

# Given only by Emerald story scenes that the post-Emerald state (parts 10-12's
# newgame overrides) never plays; their scripts stay assembled, so the scan
# finds them.
UNREACHABLE = {
    "ITEM_LETTER": "Mr. Stone's first meeting (Devon 3F); he gives a Lucky Egg instead (part 11)",
    "ITEM_DEVON_PARTS": "the Rusturf Tunnel grunt, hidden in the post-Emerald state",
    "ITEM_MAGMA_EMBLEM": "the Mt. Pyre old lady's scene after Archie takes the orbs, already past",
}


def key_items():
    cmd = ["arm-none-eabi-cpp", "-iquote", "include", "-Wno-trigraphs", "-DMODERN=1", "-DTESTING=0",
           "-DPOKEMON_HNS", "-std=gnu17", "src/item.c"]
    s = subprocess.run(cmd, cwd=ROOT, capture_output=True, text=True, check=True).stdout
    body = s[s.index("gItemsInfo[]"):]
    out = []
    for m in re.finditer(r"\[(ITEM_\w+)\]\s*=\s*\{(.*?)\n\s*\},", body, re.S):
        pm = re.search(r"\.pocket\s*=\s*(\w+)", m.group(2))
        if pm and pm.group(1) == "POCKET_KEY_ITEMS":
            out.append(m.group(1))
    return out


def built_maps():
    kept = set(json.load(open(P("data/hoenn/maps.json")))["items"])
    for folder in sorted(os.listdir(P("data/maps"))):
        path = P("data/maps", folder, "map.json")
        if not os.path.exists(path):
            continue
        m = json.load(open(path))
        if m.get("game_version") == "hns" or (m.get("game_version") == "emerald" and folder in kept):
            yield folder, m


GIVE = re.compile(r"^\s*(?:giveitem|additem|finditem|giveitem_msg)\s+(?:[^,\n]*,\s*)?(ITEM_\w+)", re.M)
GIVE_FIRST = re.compile(r"^\s*(?:giveitem|additem|finditem)\s+(ITEM_\w+)", re.M)
GIVE_MSG = re.compile(r"^\s*giveitem_msg\s+\w+\s*,\s*(ITEM_\w+)", re.M)
SETVAR = re.compile(r"^\s*setvar\s+VAR_0x8000\s*,\s*(ITEM_\w+)", re.M)
MART = re.compile(r"^\s*\.2byte\s+(ITEM_\w+)", re.M)
REMOVE = re.compile(r"^\s*removeitem\s+(ITEM_\w+)", re.M)
C_ADD = re.compile(r"Add(?:Bag|PC)Item\(\s*(ITEM_\w+)")
C_REMOVE = re.compile(r"RemoveBagItem\(\s*(ITEM_\w+)")


def scan():
    gets, loses = {}, {}
    def note(d, item, where):
        d.setdefault(item, set()).add(where)

    scripts = [P("data/scripts", f) for f in sorted(os.listdir(P("data/scripts"))) if f.endswith(".inc")]
    maps = list(built_maps())
    scripts += [P("data/maps", f, "scripts.inc") for f, _ in maps if os.path.exists(P("data/maps", f, "scripts.inc"))]
    for path in scripts:
        if os.path.basename(path) == "debug.inc":
            continue
        t = open(path).read()
        where = os.path.relpath(path, ROOT)
        for rx in (GIVE_FIRST, GIVE_MSG, SETVAR, MART):
            for i in rx.findall(t):
                note(gets, i, where)
        for i in REMOVE.findall(t):
            note(loses, i, where)
    for folder, m in maps:
        for o in m.get("object_events") or []:
            v = str(o.get("trainer_sight_or_berry_tree_id", ""))
            if v.startswith("ITEM_") and "Item" in str(o.get("script", "")):
                note(gets, v, f"data/maps/{folder} (item ball)")
        for b in m.get("bg_events") or []:
            if b.get("type") == "hidden_item" and str(b.get("item", "")).startswith("ITEM_"):
                note(gets, b["item"], f"data/maps/{folder} (hidden)")
    for f in sorted(os.listdir(P("src"))):
        if not f.endswith(".c") or f == "debug.c":
            continue
        t = open(P("src", f)).read()
        for i in C_ADD.findall(t):
            note(gets, i, "src/" + f)
        for i in C_REMOVE.findall(t):
            note(loses, i, "src/" + f)
    return gets, loses


# The check fails when the bound comes within this many slots of the pocket's
# size: a part that adds key items has to grow the pocket first.
MARGIN = 5


def capacity():
    """SaveBlock1's slots + Hoenn & Seek's extra ones (include/hoenn_bag.h)."""
    t = open(P("include/constants/global.h")).read()
    base = int(re.findall(r"#define BAG_KEYITEMS_COUNT (\d+)", t)[-1])
    extra = re.search(r"#define HOENN_KEY_ITEMS_EXTRA\s+(\d+)", open(P("include/constants/hoenn_layout.h")).read())
    return base + (int(extra.group(1)) if extra else 0)


def main(argv):
    keys = key_items()
    gets, loses = scan()
    got = [k for k in keys if k in gets and k not in UNREACHABLE]
    never = [k for k in keys if k not in gets]
    taken = [k for k in got if k in TAKEN_BACK]
    held = len(got) - len(taken)
    cap = capacity()
    print(f"key items: {len(keys)} in the table, {len(got)} can be got, {len(taken)} always taken back: "
          f"at most {held} held at once (pocket {cap})")
    if "--report" in argv:
        path = argv[argv.index("--report") + 1]
        L = ["# Key-item pocket audit", "",
             "Generated by `tools/hoenn/key_item_audit.py --report` (2.1-04; its header explains the scan).", "",
             f"- Key items in the item table (HnS build): **{len(keys)}**",
             f"- Can be got in the game: **{len(got)}**",
             f"- Always taken back by the story before the next one: **{len(taken)}**",
             f"- **Most held at once (upper bound): {held}**; pocket size: **{cap}** slots", "",
             "Decision (2.1-04): the pocket grows from 60 to 80 slots (`HOENN_KEY_ITEMS_EXTRA`: the 20 new slots live at the end of SaveBlock3, `include/hoenn_bag.h`), "
             "because the bound is past 50 and 2.1 adds more (the Seeker's Journal, lane A's chains).", "",
             "## Can be got", "", "| Item | Given by | Taken back by |", "|---|---|---|"]
        for k in got:
            src = ", ".join(sorted(gets[k]))
            back = TAKEN_BACK.get(k, ", ".join(sorted(loses.get(k, []))) + (" (not always)" if k in loses else ""))
            L.append(f"| {k[5:]} | {src} | {back} |")
        L += ["", "## Given only by scenes the post-Emerald state skips", "", "| Item | Why |", "|---|---|"]
        L += [f"| {k[5:]} | {UNREACHABLE[k]} |" for k in keys if k in UNREACHABLE]
        L += ["", "## In the table, never given", "", ", ".join(k[5:] for k in never), ""]
        text = "\n".join(L) + "\n"
        if not os.path.exists(path) or open(path).read() != text:
            open(path, "w").write(text)
    if held > cap - MARGIN:
        print(f"key_item_audit: up to {held} key items, only {cap} slots (keep {MARGIN} spare): grow the pocket "
              "(HOENN_KEY_ITEMS_EXTRA, SaveBlock3)", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
