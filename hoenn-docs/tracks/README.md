# 2.1: two builder tracks + one reviewer (the rules)

**Status: in use since 2.1-02 (Nov 28).** Aidan approved building 2.1 on Nov 28 ("go ahead and do 2.1"). The plan is `../2.1-tracks.md` (who builds what, in which order) on top of `../2.1-plan.md` (what each part builds). This folder holds the rules that 2.1-02 set up and each track's status:

| File | What | Who edits it |
|---|---|---|
| `README.md` (this file) | The rules: IDs, files, phone, radio, ladder, docs, merging | track A's builder wrote it; changes need Aidan or the reviewer |
| `track-a.md` | Track A's parts, claims and ID use | only track A's builder (the reviewer may add a line when it merges) |
| `track-b.md` | Track B's parts, claims and ID use | only track B's builder (same) |

The words differ from 2.0 on purpose: 2.0 had **lanes** A/B (`../lanes/`, finished), 2.1 has **tracks** A/B. "Lane A" in an old note never means a 2.1 track.

**Who is who** (2.1-tracks.md section 0): track A = the account that built 2.0's lane A (Aidan's project "Pokemon Romhack"); track B = the account that built 2.0's lane B; the reviewer = the third account. **The reviewer merges every builder PR; builders never merge their own.** If you're a new session and don't know your track, ask Aidan.

## 1. IDs

### Flags and vars (`include/constants/hoenn_events_2_1.h`)
2.1 has its own SaveBlock3 arrays (`hoenn21Flags`, `hoenn21Vars`; save version 7 clears them on a 2.0 save). The split, checked at build time in `src/hoenn_flags.c`:

| Resource | Track A | Track B | Join (2.1-35 on) |
|---|---|---|---|
| Event flags 0x2400–0x255F | `HOENN_TRACK_A_FLAGS` 0x2400, 192 | `HOENN_TRACK_B_FLAGS` 0x24C0, 160 | `HOENN_21_JOIN_FLAGS` 0x2560, 32 |
| Daily flags 0x2580–0x25BF | `HOENN_TRACK_A_DAILY` 0x2580, 28 | `HOENN_TRACK_B_DAILY` 0x259C, 28 | `HOENN_21_JOIN_DAILY` 0x25B8, 8 |
| Weekly flags 0x25C0–0x25FF | `HOENN_TRACK_A_WEEKLY` 0x25C0, 24 | `HOENN_TRACK_B_WEEKLY` 0x25D8, 32 | `HOENN_21_JOIN_WEEKLY` 0x25F8, 8 |
| Vars 0x7100–0x713F | `HOENN_TRACK_A_VARS` 0x7100, 28 | `HOENN_TRACK_B_VARS` 0x711C, 28 | `HOENN_21_JOIN_VARS` 0x7138, 8 |
| PokéGear event call IDs (`constants/hoenn_phone.h`) | 100–129 | 130–159 | — |
| Hidden-item flags (they can't use 2.1 flags: 13-bit offset) | 2.0 lane A's unused `HOENN_LANE_A_HIDDEN` + 0x00 on (28) | 2.0 lane B's unused `HOENN_LANE_B_HIDDEN` + 0x02 on (26) | — |

- **Measured in 2.1-02** (`2.1-tracks.md` section 4 asked for it): track A needs ~150 flags (the journal's ~90 heard flags for 2.0's events, Silver, the friends, the second rival, the elite trainers), 8 daily, 16 vars; track B ~120 flags, ~20 weekly, ~12 vars. So the proposed split stands.
- **2.0's own spares stay 2.0's.** 2.0 still has ~240 unreserved flags (`HOENN_FLAGS_SPARE` + 0x2380–0x23FF), 4 daily, 3 weekly and 16 spare vars, plus whatever its lanes left unused. They're for 2.0's join and playtest fixes (and 2.2). 2.1 parts don't take from them.
- Define each one in **your track's section** of `hoenn_events_2_1.h`, under `// ---- 2.1-NN: what ----`, as `BASE + n`, in build order. Names start `FLAG_HOENN_` / `VAR_HOENN_`. Never renumber (numbers are saved).
- A night event's daily flag goes in `sNightlyFlags` (`src/hoenn_flags.c`), as in 2.0.
- If a track runs out, it takes from the join tail and writes the claim in its track file in the same PR.

### Things a range can't reserve (unchanged from 2.0)
- **Key items:** both append after the last one; on a conflict keep both, in merge order. Never hard-code an item number in tests (`pt.resolve`).
- **New maps:** both append to `gMapGroup_SpecialArea2_Hns`; numbers follow merge order. Never hard-code a map number.
- **Trainer IDs:** a new trainer shifts the Hoenn IDs after it. 2.1 needs several (the three friends, the second rival's teams, Silver's tiers): **say so in your track file before starting**, prefer renaming Emerald's unused slots (like DAMIAN = `TRAINER_LEROY`), and the other track re-runs the generators when it next syncs.
- **Decoration IDs** (dolls, 127–255 free): append, merge order.

## 2. Files: one per track, small hooks

| | Track A | Track B |
|---|---|---|
| C | `src/hoenn_2_1_a.c` | `src/hoenn_2_1_b.c` |
| Scripts and texts | `data/scripts/hoenn_2_1_a.inc` | `data/scripts/hoenn_2_1_b.inc` |
| Declarations | track A's section of `include/hoenn_2_1.h` | track B's section |
| Specials | track A's block at the end of `data/specials.inc` | track B's block after it |
| Radio rows | `"track_a"` in `data/hoenn/radio_segments.json` | `"track_b"` |
| Checklist | "Track A (2.1)" in `playtest-checklist.md` | "Track B (2.1)" |
| CLAUDE.md | "Track A sizes", "Track A notes" | "Track B sizes", "Track B notes" |

A big system may have its own file (the Seeker's Journal, the swarms, Anamaria's sorting, the people's Pokémon generator); it's still that track's file, named in the part's notes.

**Shared code a part has to touch** (a 2.0 file, an HnS file, the other track's map): one line in your track file *before* the part ("Claims on shared things"), and keep the edit to a hook line.

## 3. The shared systems 2.1-02 built (use them, don't add hooks)
- **Phone chain:** `GetDueEventCall` → … → `HoennLaneBDueCall` (2.0) → `Hoenn21ADueCall` (`src/hoenn_2_1_a.c`) → `Hoenn21BDueCall` (`src/hoenn_2_1_b.c`). The script switch falls through the same way: `HoennSeek_EventScript_PhoneEventLaneB` → `…PhoneEvent21A` → `…PhoneEvent21B`. Add an `if` to your due function and a `case` to your script switch. The phone rule (every new quest, area or battle gets a Brendan/May line or event call that says who told the caller) still applies.
- **Radio:** announcements are rows in `data/hoenn/radio_segments.json` (fixed lines, or a C builder), played before a station's broadcast by `src/hoenn_radio.c`. Never add a hook line to `src/pokenav_radio.c`. A row's `"heard"` flag marks a journal rumor as heard.
- **Scaled-opponent ladder:** `HoennScaledAce(LADDER_PROGRESS / LADDER_RIVAL / LADDER_ELITE)` in `src/hoenn_scaling.c`. A trainer joins a ladder by one row in `sLadderTrainers` (track A owns that table; track B asks in its track file). Scripts read a level with `specialvar VAR_RESULT, HoennScaledAceFor` (`VAR_0x8004` = `HOENN_LADDER_*`). The debug menu shows all three: Utilities → "H&S time & weather…" → LADDERS.
- **Hint conditions** (`include/hoenn_hints.h`: `struct HoennHintCond`, `HoennHintCondsHold`) are shared by hints and radio rows; the journal (2.1-05) uses them too.

## 4. Generated files: never hand-merge
Unchanged from 2.0 (`../lanes/README.md` section 3.4): on a conflict in a generated file, take `hoenn`'s side, keep both sides of the hand-written inputs, run `tools/hoenn/regen_all.sh`, rebuild. 2.1-02 added `gen_hoenn_radio` to it. Every new hint NPC and journal entry needs a `"key"`. Track B's people's Pokémon (2.1-28/29) touch almost every town's `map.json`: track A re-runs regen after syncing instead of resolving those files.

## 5. Claiming, syncing, merging (as in 2.0)
- **Claim:** push the branch `v2.1-NN-<slug>` and add a line to your track file.
- **Draft until synced:** `git fetch origin hoenn && git merge origin/hoenn` (merge, never rebase or force-push), `regen_all.sh`, rebuild release + debug, re-measure, re-run one smoke test, then mark the PR ready.
- **Merge order** (the reviewer): first the PR the other track waits for (2.1-02, 2.1-03, 2.1-01's approval, 2.1-14), then review fixes (`2.1-NNr`), then the older one.
- **Builders don't wait for a merge** to start their next part: branch it off `hoenn` and merge your previous branch in if it isn't merged yet.

## 6. Every part ends with (README.md's list, for 2.1)
- release + debug build; a row in your track's size table in `CLAUDE.md` (whole ROM, measured after merging `hoenn`)
- the debug `.ups` in `/mnt/project-files/hoenn-2.1/part-NN/` (base ROM never committed)
- emulator tests with a labeled screenshot grid in the same folder
- an edge-case pass (wrong time/day/weather, coming back after, caught vs. knocked out vs. ran, full party and boxes, save and reload mid-chain, every text box read)
- its journal entry (from 2.1-05 on; before that, a rumor), hint/rumor lines, phone line, and the screen checker (from 2.1-03 on) for parts that add objects
- `CLAUDE.md` notes in your track's section, your track file's status, untested items in your checklist section
- a PR into `hoenn`. Don't pause at approval gates (art, scene text, design): decide, record it in the docs and the PR, keep going (Aidan, Nov 28).

## 7. Budget
2.1 has ~250 KB of release space planned (`2.1-plan.md` section 19). Soft budget **110 KB per track**, ~30 KB for the join. A part over 15 KB says why in its PR. The debug build stays under 32 MB (33,554,432 B).
