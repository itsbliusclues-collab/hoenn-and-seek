#ifndef GUARD_CONSTANTS_HOENN_EVENTS_2_1_H
#define GUARD_CONSTANTS_HOENN_EVENTS_2_1_H

// Hoenn & Seek 2.1: who owns which 2.1 flags and vars.
//
// 2.1 is built by two Claude builders in parallel, track A (systems and
// trainers) and track B (the world), with a third Claude reviewing and merging
// (hoenn-docs/2.1-tracks.md). So that their PRs never fight over numbers, 2.1
// has its own flag and var arrays (constants/hoenn_layout.h, "2.1 range") and
// this file splits them between the tracks once, in 2.1 part 02:
//
//                          track A   track B   join   (sweep 2.1-35, fixes)
//     event flags (384)      192       160      32
//     daily flags  (64)       28        28       8
//     weekly flags (64)       24        32       8    (B's chains and weekends are weekly)
//     vars         (64)       28        28       8
//
// Why those numbers (measured in part 02, Nov 28): track A's biggest user is
// the Seeker's Journal, which needs one "heard" flag per 2.0 event (~90; there
// are 41 rumors today and about twice as many events), then Silver, the three
// friends (9 flags + 3 daily, HARD wins), the second rival and the elite
// trainers: about 150 flags, 8 daily, 16 vars. Track B's are the leaders'
// weekends (no flags: the day decides), three multi-week chains + dolls, Dan's
// weekly episodes, Ryan, Anamaria and the wallpaper unlocks: about 120 flags,
// 20 weekly, 12 vars. Both get room to spare; the join tail is for the 2.1
// sweep and playtest fixes. A track that runs out takes from the join tail
// and says so in its track file (hoenn-docs/tracks/), in the same PR.
//
// How to add one: in your track's section below, "// ---- 2.1-NN: what ----",
// then
//
//     #define FLAG_HOENN_SILVER_HOENN_MET   (HOENN_TRACK_A_FLAGS + 0x00)
//     #define FLAG_HOENN_DAILY_FRIEND_BRIDGETTE (HOENN_TRACK_A_DAILY + 0x00)
//     #define VAR_HOENN_SWARM_TODAY         (HOENN_TRACK_A_VARS + 0x00)
//
// Rules (the same as 2.0's, constants/hoenn_events.h):
// - Every name starts with FLAG_HOENN_ / VAR_HOENN_ (gen_hoenn_ids.py leaves
//   those alone). Use the next free number in your track's range; keep a part's
//   flags together. Never renumber: numbers are saved.
// - Daily flags reset at midnight, weekly ones when a new week starts (Sunday).
//   A night event's daily flag also goes in sNightlyFlags (src/hoenn_flags.c).
// - Hidden items can't use 2.1 flags (their flag is a 13-bit offset ending at
//   0x20E8). 2.1 hidden items take 2.0's unused hidden-item flags: track A
//   from HOENN_LANE_A_HIDDEN + 0x00 (all 28 unused), track B from
//   HOENN_LANE_B_HIDDEN + 0x02 (26 left).
// - 2.0's own spares (HOENN_FLAGS_SPARE, 0x2380-0x23FF, HOENN_VARS_SPARE) stay
//   2.0's, for its join and playtest fixes. 2.1 doesn't take from them.
// - src/hoenn_flags.c checks at build time that the ranges fit end to end.

#include "constants/hoenn_layout.h"

// ---- The split (2.1 part 02) -----------------------------------------------
//                                                                       first   count
#define HOENN_TRACK_A_FLAGS         (HOENN_21_EVENT_FLAGS_START + 0x000) // 0x2400  192
#define HOENN_TRACK_A_FLAGS_COUNT   192
#define HOENN_TRACK_B_FLAGS         (HOENN_21_EVENT_FLAGS_START + 0x0C0) // 0x24C0  160
#define HOENN_TRACK_B_FLAGS_COUNT   160
#define HOENN_21_JOIN_FLAGS         (HOENN_21_EVENT_FLAGS_START + 0x160) // 0x2560   32
#define HOENN_21_JOIN_FLAGS_COUNT   32

#define HOENN_TRACK_A_DAILY         (HOENN_21_DAILY_FLAGS_START + 0x00)  // 0x2580   28
#define HOENN_TRACK_A_DAILY_COUNT   28
#define HOENN_TRACK_B_DAILY         (HOENN_21_DAILY_FLAGS_START + 0x1C)  // 0x259C   28
#define HOENN_TRACK_B_DAILY_COUNT   28
#define HOENN_21_JOIN_DAILY         (HOENN_21_DAILY_FLAGS_START + 0x38)  // 0x25B8    8
#define HOENN_21_JOIN_DAILY_COUNT   8

#define HOENN_TRACK_A_WEEKLY        (HOENN_21_WEEKLY_FLAGS_START + 0x00) // 0x25C0   24
#define HOENN_TRACK_A_WEEKLY_COUNT  24
#define HOENN_TRACK_B_WEEKLY        (HOENN_21_WEEKLY_FLAGS_START + 0x18) // 0x25D8   32
#define HOENN_TRACK_B_WEEKLY_COUNT  32
#define HOENN_21_JOIN_WEEKLY        (HOENN_21_WEEKLY_FLAGS_START + 0x38) // 0x25F8    8
#define HOENN_21_JOIN_WEEKLY_COUNT  8

#define HOENN_TRACK_A_VARS          (HOENN_21_VARS_START + 0x00)         // 0x7100   28
#define HOENN_TRACK_A_VARS_COUNT    28
#define HOENN_TRACK_B_VARS          (HOENN_21_VARS_START + 0x1C)         // 0x711C   28
#define HOENN_TRACK_B_VARS_COUNT    28
#define HOENN_21_JOIN_VARS          (HOENN_21_VARS_START + 0x38)         // 0x7138    8
#define HOENN_21_JOIN_VARS_COUNT    8

// ============================================================================
// ==== Track A: systems and trainers (hoenn-docs/tracks/track-a.md) ==========
// ============================================================================
// Parts 2.1-02, 04-17, 34. Sections in build order, "// ---- 2.1-NN: ... ----".

// ---- 2.1-02: the scaled-opponent ladder (src/hoenn_scaling.c) --------------
// HoennScaledAce(ladder): the ace level an opponent who follows your progress
// fields. Scripts pass these to `specialvar VAR_RESULT, HoennScaledAceFor`
// with the ladder in VAR_0x8004; C code uses the enum in include/hoenn.h,
// which src/hoenn_scaling.c checks matches these.
#define HOENN_LADDER_PROGRESS  0 // DAMIAN: your progress cap; 85 after Wallace
#define HOENN_LADDER_RIVAL     1 // Silver, the second rival, the three friends: progress cap, post-game up to 90
#define HOENN_LADDER_ELITE     2 // the ~15 elite Hoenn trainers: the next leader's ace - 1
#define HOENN_LADDER_COUNT     3

// ---- 2.1-05: the SEEKER'S JOURNAL (src/hoenn_journal.c) -------------------
// Its heard flags are generated (one per event, tools/hoenn/gen_hoenn_journal.py
// from data/hoenn/journal.json): FLAG_HOENN_HEARD_<KEY> in the block
// HOENN_TRACK_A_FLAGS + 0x00..0x5F. Later parts take track A's flags from 0x60.
#include "constants/hoenn_journal.h"
#define FLAG_HOENN_GOT_SEEKERS_JOURNAL (HOENN_TRACK_A_FLAGS + 0x60) // Elm's aide gave it

// (track A's parts add their sections here, above track B's banner)

// ============================================================================
// ==== Track B: the world and its people (hoenn-docs/tracks/track-b.md) ======
// ============================================================================
// Parts 2.1-01, 03, 18-33. Sections in build order, "// ---- 2.1-NN: ... ----".

// (track B's parts add their sections here)

// ============================================================================
// ==== The join: 2.1-35 sweep, 2.1-36 pre-pass fixes, playtest fixes =========
// ============================================================================
// Uses HOENN_21_JOIN_*. Also the overflow for either track (claim it in your
// track file first).

#endif // GUARD_CONSTANTS_HOENN_EVENTS_2_1_H
