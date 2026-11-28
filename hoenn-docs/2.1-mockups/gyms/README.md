# 2.1-24: gym mockups (plan section 7)

`gym-mockups-ABC.png`: the three options on three gyms of different styles, waiting for Bryan's pick.

- **Violet (Falkner):** a 3-wide rug, Noctowl behind him today but off the rug.
- **Pewter (Brock):** a stone dais; Golem already stands behind him on the round stone, so Pewter is option A today.
- **Rustboro (Roxanne):** Emerald's dais with a 2-wide rug; Probopass stands to her left, so the pair is off centre.

Options (plan section 7):
- **A, Pokémon behind, centred:** the rug grows a row; leader in front, the ace straight behind on the same column.
- **B, side by side on a wider rug:** the pair in the middle two columns. A 32-pixel Pokémon overlaps its trainer a little, as in the game, and the pair sits half a tile off the room's centre line.
- **C, Pokémon on its own spot:** a cushion or pedestal beside the rug, matched by the same on the other side (Pewter: the Golem takes the right boulder's place, the left boulder stays).

These are image mockups made from the layout renders and the real overworld sprites: rugs and cushions are stretched copies of each gym's own rug, and the other gym trainers aren't drawn. The real tiles come in 2.1-25 to 27, one region per part, after Bryan picks (one option for all gyms, or one per gym style).

## Bryan's direction (Nov 28, 4:54 AM CT; still thinking, not final)
Sort the gym followers by size, one layout per group:
- **Small:** a **4-wide** rug or dais, leader and Pokémon side by side, and the gym rebuilt to match (Bryan knows this is the big job: odd-width rooms with a one-tile door can't centre a 4-wide rug, so those gyms need an even width and a two-tile door, or another fix found in 2.1-25).
- **Medium:** **5 wide**: the leader on one side, a gap, then the Pokémon.
- **Large:** the Pokémon stands **behind** the leader.
- **Brock's follower becomes ONIX** instead of Golem: the gym, the Pewter Museum weekend spot and the Mt. Moon chain step (`trainer_followers.json`). Onix is 22×29, so it's large (behind), as Golem is today.

Each sprite is a 32×32 frame; the groups below go by the biggest side of the visible pixels in its down-facing frame (small ≤ 21 px, medium 22–25, large 26+). A draft for Bryan to adjust:

| Group | Gyms (follower, visible size) |
|---|---|
| Small | Goldenrod (Miltank 20×20), Ecruteak (Gengar 21×19), Mahogany (Piloswine 19×20), Celadon (Bellossom 15×18), Lavaridge (Torkoal 19×18) |
| Medium | Azalea (Scyther 23×24), Blackthorn (Kingdra 22×24), Cerulean (Starmie 24×22), Vermilion (Raichu 24×22), Saffron (Alakazam 24×22), Mauville (Manectric 17×23), Sootopolis (Kingdra 22×24) |
| Large | Violet (Noctowl 29×22), Olivine (Steelix 32×32), Pewter (Onix 22×29), Viridian (Rhyperior 27×21), Rustboro (Probopass 25×27), Dewford (Hariyama 29×25), Petalburg (Slaking 23×27), Fortree (Altaria 30×25) |
| None | Cianwood, Cinnabar, Fuchsia, Mossdeep (no room, or the follower would give the gym's trick away) |

### Rough mockup of each group (Nov 28)
`gym_groups_mockup.png`: one gym per group, now vs. mockup, painted over the real layouts with the real sprites (rough: tiles copied and stretched, not final art).
- **Small (Goldenrod):** a 4-wide rug with Whitney and Miltank side by side; the pot row above shrinks to 4 and a pot closes the gap on the right, so the rug reads as the middle.
- **Medium (Cerulean):** the pedestal grows to 5 wide (platform widened to match): Misty, a gap, Starmie.
- **Large (Pewter):** Onix behind Brock on the dais, replacing Golem.

### Every gym, rough (Nov 28, after Bryan's "try that for all of them")
`gyms_small.png`, `gyms_medium.png`, `gyms_large.png`: all 20 gyms with a follower, now vs. mockup, same rough method. Claude's calls, for Bryan to check gym by gym:
- **Small (4 wide, side by side):** Ecruteak, Mahogany, Celadon and Lavaridge get **one more column through the middle** (the door becomes two tiles, the wall emblem sits over the middle), so a 4-wide rug is centred. Celadon is a garden, so no rug. Goldenrod is already an even width: its rug just grows and the pots around it move.
- **Medium (5 wide, leader / gap / Pokémon, the gap on the centre column):** Blackthorn's and Sootopolis's mats grow to 5; Vermilion's two pillars move out a tile; Cerulean's pedestal grows; Mauville gets one more column (it's 4 wide inside) and the rug covers the dais; Saffron's diamond tiles are already 5 wide; Azalea is a forest with no rug.
- **Large (Pokémon behind):** where the tile behind the leader is wall (Viridian, Rustboro, Dewford, Petalburg, Fortree) the **leader steps forward one tile** and the Pokémon takes their old spot; Violet's rug grows a row so Noctowl stands on it; Olivine is already this way; Pewter swaps Golem for Onix.

### Bryan's correction (Nov 28, 8:12 AM CT): beside, unless the sprite is huge
"Unless they are the super big sprites like Steelix and Onix, I prefer beside the trainer": a leader stepping in front of their Pokémon makes the platform look odd and has less aura. So **large is only Olivine (Steelix) and Pewter (Onix)**; Violet, Viridian, Rustboro, Dewford, Petalburg and Fortree move to **medium** (5 wide: leader, gap on the centre column, Pokémon), shown in `gyms_medium_2.png`; `gyms_large.png` now has only the two. Dewford is 4 wide inside, so it gets one more column like Mauville; the others' rugs or mats just grow to 5. The "steps forward" notes above are replaced by this.

| Group | Gyms |
|---|---|
| Small (4 wide, side by side) | Goldenrod, Ecruteak, Mahogany, Celadon, Lavaridge |
| Medium (5 wide, leader / gap / Pokémon) | Azalea, Blackthorn, Cerulean, Vermilion, Saffron, Mauville, Sootopolis, Violet, Viridian, Rustboro, Dewford, Petalburg, Fortree |
| Large (behind) | Olivine (Steelix), Pewter (Onix) |

### Bryan's final rule (Nov 28, 8:17 AM CT): side by side, no gap; behind only for the three biggest
"I like the original Scyther placement. I prefer them all, except the really large sprites, to be next to them without a gap in between. Steelix, Onix, Noctowl all behind, the others next to, and then the platforms redone to be 4 wide so that it is centered on the Pokémon and gym leader." This replaces the small/medium/large groups above (the earlier sheets were removed from this folder).

- **Behind (3):** Violet (Noctowl; the rug grows a row), Olivine (Steelix; no change), Pewter (Onix replaces Golem). `v3_behind.png`.
- **Side by side on a 4-wide rug centred on the pair (17):** `v3_side_1.png` to `v3_side_3.png`. Leader on the left, Pokémon on the right, the rug's middle two tiles.
  - Rooms with an odd width and a one-tile centre door get **one more column through the middle** (two-tile door, the wall emblem over the middle), so the 4-wide rug is centred in the room too: Ecruteak, Mahogany, Celadon, Lavaridge, Blackthorn, Cerulean, Vermilion, Saffron, Sootopolis, Viridian, Rustboro, Petalburg, Fortree. Most 3-wide rugs become 4 wide by themselves this way.
  - Already even: Goldenrod (the rug grows, pots re-spaced), Mauville, Dewford.
  - **Azalea unchanged** (Bryan likes Scyther where it is; a forest, no rug).

**Approved (Bryan, Nov 28, 8:21 AM CT): "I love everything else"**, with two changes, both in the sheets now:
- **Azalea:** the trees are rearranged to be symmetric around the pair (mirrored about the line between Scyther and Bugsy); the pair stays where it is.
- **Cerulean:** the pedestal is one tile narrower on each side: 2 wide, just under Misty and Starmie (the pool still gets its extra middle column).
This is the design 2.1-25 to 27 build.
- **Ecruteak (Bryan, 8:25 AM CT):** Morty's platform gets the same treatment as Misty's: 2 wide, just under Morty and Gengar. "Then it is actually perfect."
- **Red (Bryan, 8:27 AM CT):** the same rule for Red and Pikachu at the Mt. Silver summit, even if it takes rebuilding part of it. Bryan trusts the builder's calls from here on (no further check needed).
