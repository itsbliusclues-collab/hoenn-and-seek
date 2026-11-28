# ANAMARIA's box sorter: harness test (2.1-32)

Tests `src/hoenn_organizer.c` on a **test-only ROM**. Nothing here is built into the game.

1. Copy `hoenn_organizer_test.c` into `src/`, add `def_special HoennOrganizerTestRun` after
   `HoennOrganizeBoxes` in `data/specials.inc`, and put `special HoennOrganizerTestRun` in
   `Debug_EventScript_Script_7` and `special SaveGame` + `waitstate` in `Debug_EventScript_Script_8`
   (`data/scripts/debug.inc`).
2. `make hns -j4`, copy `pokehns.gba`/`.elf` to `~/emu/orgtest.gba`/`.elf`, then undo step 1 and rebuild.
3. In `~/emu`: `bash <repo>/tools/hoenn/emu/mkbase.sh orgtest.gba pt/o_base.ss`, then
   `python3 <repo>/tools/hoenn/emu/organizer_test/organizer_test.py` (every mode, skips, packing,
   an empty run), `... organizer_test.py renum` (a BY TYPE sort, then BY NUMBER puts her box names
   back), and `organizer_save_test.py` (save, reload from flash, same boxes, names and wallpapers).

`HoennOrganizerTestRun` fills all 420 slots from a seed (fill %, eggs, gaps, lots of duplicate
species), dumps every slot (species, dex number, level, egg, type, personality) before and after
sorting, using the game's own accessors. The Python side rebuilds the expected order independently
(stable sort on the same key, BY TYPE's box breaks) and compares every slot, the result code, the
multiset of Pokémon, and BY TYPE's box names and wallpapers.
