#!/usr/bin/env python3
"""Key-item pocket audit (Hoenn & Seek 2.1-04).

    tools/hoenn/key_item_audit.py            print the counts
    tools/hoenn/key_item_audit.py --write    also rewrite hoenn-docs/key-items.md

Lists every key item in src/data/items.h (pocket POCKET_KEY_ITEMS) and, for
each, the HnS-built scripts that give it (giveitem / additem / finditem, or an
item ball in a map.json) and the ones that take it back (removeitem). "Built"
means a map whose game_version is hns, or an emerald map that Hoenn & Seek
keeps (data/hoenn/maps.json), plus data/scripts/*.inc except FRLG and debug
scripts. It's a text scan, so it over-counts a little (a give behind a branch
the player never takes still counts); that's the safe side for sizing the
pocket. Items given from C code, or through a var, wouldn't show up.

Re-run it when a part adds key items; the pocket is BAG_KEYITEMS_COUNT
(include/constants/global.h, 80 since 2.1-04)."""
import collections, glob, json, os, re, sys

ROOT = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
os.chdir(ROOT)
GIVE = re.compile(r"(giveitem|additem|finditem|giveitem_msg)\b")
POCKET = 80

def key_items():
    src = open("src/data/items.h").read()
    keys = set()
    for m in re.finditer(r"\[(ITEM_\w+)\]\s*=\s*\{(.*?)\n    \},", src, re.S):
        if "POCKET_KEY_ITEMS" in m.group(2):
            keys.add(m.group(1))
    return sorted(keys)

def built(path):
    if path.startswith("data/maps/"):
        name = path.split("/")[2]
        try:
            gv = json.load(open(f"data/maps/{name}/map.json")).get("game_version")
        except (OSError, ValueError):
            return False
        return gv == "hns" or (gv == "emerald" and name in KEPT)
    base = os.path.basename(path)
    return "frlg" not in base and base not in ("debug.inc", "hoenn_debug.inc")

def short(path):
    return path.split("/")[2] if path.startswith("data/maps/") else os.path.basename(path)

KEPT = set(json.load(open("data/hoenn/maps.json"))["items"])

def scan(keys):
    give, take = collections.defaultdict(set), collections.defaultdict(set)
    pats = {k: re.compile(r"\b%s\b" % k) for k in keys}
    for f in glob.glob("data/**/*.inc", recursive=True) + glob.glob("data/**/*.s", recursive=True):
        if not built(f):
            continue
        for line in open(f, errors="ignore"):
            l = line.strip()
            if not (GIVE.match(l) or l.startswith("removeitem")):
                continue
            for k, p in pats.items():
                if p.search(l):
                    (take if l.startswith("removeitem") else give)[k].add(short(f))
    for f in glob.glob("data/maps/*/map.json"):
        if not built(f):
            continue
        t = open(f).read()
        for k in keys:
            if f'"{k}"' in t:
                give[k].add(short(f))
    return give, take

def main():
    keys = key_items()
    give, take = scan(keys)
    got = [k for k in keys if give[k]]
    print(f"key items in the item table: {len(keys)}")
    print(f"given by a built script: {len(got)} (pocket: {POCKET})")
    print(f"of those, taken back by some script: {sum(1 for k in got if take[k])}")
    if "--write" not in sys.argv:
        return
    rows = "\n".join(f"| `{k[5:]}` | {', '.join(sorted(give[k])[:3])}{' …' if len(give[k]) > 3 else ''} | {', '.join(sorted(take[k]))} |" for k in got)
    never = ", ".join(f"`{k[5:]}`" for k in keys if k not in got)
    doc = open("hoenn-docs/key-items.md").read()
    start, end = "<!-- generated: key_item_audit.py -->", "<!-- end generated -->"
    i, j = doc.index(start) + len(start), doc.index(end)
    body = (f"\n{len(got)} of {len(keys)} key items are given by a built script.\n\n"
            "| Item | Given by | Taken back by |\n|---|---|---|\n" + rows +
            f"\n\nNever given (debug, other games, or unused): {never}.\n")
    open("hoenn-docs/key-items.md", "w").write(doc[:i] + body + doc[j:])

if __name__ == "__main__":
    main()
