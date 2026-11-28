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
