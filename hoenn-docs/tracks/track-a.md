# Track A: status (2.1)

**Builder:** the account that built 2.0's lane A (Aidan's project "Pokemon Romhack"). Rules: `README.md` in this folder. Plan: `../2.1-tracks.md` (order) and `../2.1-plan.md` (what each part builds).

**Owns:** systems and trainers. The shared 2.1 systems (flags, radio segments, the scaled-opponent ladder), the bag, the Seeker's Journal, swarms, Silver and his starter, the ~15 elite Hoenn trainers, the three friend trainers (Explorer Bridgette, Skydiver Aidan, Athlete Brock), the second rival (Ethan/Lyra) and Teleport to your secret base.

Only track A's builder edits this file (the reviewer may add a line when it merges).

## Parts
| Order | Part | State | Branch / PR | IDs and notes |
|---|---|---|---|---|
| A1 | **2.1-02** Foundations: 2.1 flag/var block + save v7, both tracks' ranges and files, radio segments (28's and 35b's hooks folded in), scaled-opponent ladder | PR #113 open (Nov 28) | `claude/project-thread-b7lyt3` | no flags used; `HOENN_LADDER_*` constants. Also carries the 2.1 plan docs (#103, #108) |
| A2 | 2.1-04 Key-item pocket audit (+ 80 slots if needed) | PR #114 open (Nov 28) | `v2.1-04-key-pocket` (on 2.1-02) | no flags; pocket 80 slots in SaveBlock3 (320 B), save version 8. Audit: `hoenn-docs/key-items.md` |
| A3 | 2.1-05 Seeker's Journal A | PR #121 open (Nov 28) | `v2.1-05-journal` (on 2.1-04) | heard flags: one block, `HOENN_TRACK_A_FLAGS` + 0x00–0x5F (62 used, numbers locked in `data/hoenn/journal_flags.json`); `FLAG_HOENN_GOT_SEEKERS_JOURNAL` = + 0x60; `ITEM_SEEKERS_JOURNAL` (911). Next free flag: + 0x61 |
| A4 | 2.1-06 Seeker's Journal B | PR #122 open (Nov 28) | `v2.1-06-journal-b` (on 2.1-05) | flags + 0x61–0x66 (`FLAG_HOENN_JOURNAL_DOLL_*`, `FLAG_HOENN_CALL_JOURNAL_*`), calls 100–102 (`HOENN_CALL_JOURNAL_*`), decorations 127–129 (CELEBI, MEW, JIRACHI DOLL). Next free flag: + 0x67, next call: 103, next decoration: 130 |
| A5 | 2.1-07 | next | | |
| A5 | 2.1-07 Swarms (+ the berry check) | | | |
| A6 | 2.1-08 Silver | | | joins `LADDER_RIVAL` |
| A7 | 2.1-10 Elite Hoenn trainers: pick + scaling | | | joins `LADDER_ELITE` |
| A8 | 2.1-11 Elite Hoenn trainers: teams | | | |
| A9 | 2.1-12 Friend-trainer system + Explorer Bridgette | | | waits for 2.1-01 approved (B1) and 2.1-03 (B2) |
| A10 | 2.1-13 Friend trainers' final sprites | | | waits for 2.1-01 approved |
| A11 | 2.1-14 Skydiver Aidan + Athlete Brock (+ HARD flags) | | | |
| A12 | 2.1-09 Silver's starter | | | waits for 2.1-03 (B2) |
| A13–15 | 2.1-15–17 Second rival (Ethan/Lyra) | | | waits for 2.1-03 (B2) |
| A16 | 2.1-34 Teleport to your secret base | | | |

**Order note (Nov 28):** Aidan said "go ahead and do 2.1" while 2.0's part 38 sweep is still running on another account, so 2.1-02 starts now rather than after 2.0's playtest fixes (2.1-tracks.md section 1). It touches no 2.0 content: the only 2.0 code it changes is the radio's two hook lines (now table rows) and DAMIAN's scaling (now a ladder row, same levels).

## ID plan (inside track A's ranges)
Flags, daily, weekly and vars in build order from each range's + 0x00. Calls from 100. The journal's heard flags (2.1-05) are one block; parts after it take the next free number.

## Claims on shared things
One line each, before the part that uses it.
- 2.1-02: `src/pokenav_radio.c` (one segment call before every station's broadcast, one in OAK's intro, replacing 2.0's two hook lines; a build-time check of the station numbers), `src/caps.c` (`HoennNormalLevelCap`, next to its tables), `src/hoenn_hints.c` (the condition type moved to `include/hoenn_hints.h`), `src/save.c` (version 7), `include/global.h` (the 2.1 arrays), `src/hoenn_lane_b.c` + `data/scripts/hoenn_lane_b.inc` (2.0 lane B's phone chain end now hands over to 2.1), `data/scripts/hoenn_debug.inc` (LADDERS in the debug time menu).
- 2.1-05: `src/hoenn_hints.c` (one heard-flag line where a hint line is shown, one in the field notes; the tables come from the journal generator), `src/hoenn_phone.c` (one line in `HoennTryEventCall`), `tools/hoenn/gen_hoenn_hints.py` (an NPC's `first` script), `data/hoenn/npc_hints.json` (Elm's aide's `first`, one Squirtle Squad line for the Saffron PC teacher), `data/scripts/hoenn_phone.inc` (Elm's escapee call mentions the journal), `data/scripts/hoenn_johto_legends.inc` (two heard-flag lines: the Tin Tower sage, the Ilex shrine), `src/item_use.c` + `include/constants/items.h` + `src/data/items.h` (the key item), `tools/hoenn/regen_all.sh` (`gen_hoenn_journal` after the hints).
- 2.1-06: `include/constants/hoenn_decorations.h` + `src/data/decoration/hoenn_decorations.h` (decorations 127–129), `data/maps/NewBarkTown_Lab_hns/map.json` and `data/maps/PalletTown_Lab_hns/map.json` (Elm's and Oak's object scripts swapped for wrappers that fall back to HnS's), `data/scripts/hoenn_arrival.inc` (one `call` at the top of Birch's script), `src/hoenn_hints.c` + `tools/hoenn/gen_hoenn_hints.py` (the field notes read the journal; rumor `notes` dropped), `data/hoenn/npc_hints.json` (its comment).
- 2.1-04: `include/constants/global.h` (`BAG_KEYITEMS_COUNT` 80 for HnS), `include/global.h` (`hoennKeyItems` in SaveBlock3; `struct ItemSlot` moved above it), `src/item.c` (the key pocket points at SaveBlock3; `ClearBag` clears it), `src/save.c` (version 8).
