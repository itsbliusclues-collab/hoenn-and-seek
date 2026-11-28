#ifndef GUARD_CONSTANTS_HOENN_EVENTS_2_1_H
#define GUARD_CONSTANTS_HOENN_EVENTS_2_1_H

// Hoenn & Seek 2.1: who owns which 2.1 flags and vars (2.1-02).
//
// 2.0's event flags (constants/hoenn_events.h) are nearly used up, so 2.1 has
// its own range (constants/hoenn_layout.h: flags 0x2400-0x26FF, vars
// 0x7100-0x713F), split between the two builder lanes like 2.0's lane setup
// (hoenn-docs/lanes/2.1-lanes.md). The rules are 2.0's:
// - Names start with FLAG_HOENN_ / VAR_HOENN_ (gen_hoenn_ids.py skips them).
// - Define a part's flags in its lane's section below, as LANE_START + n,
//   kept together under a "// 2.1-NN: ..." comment. Never move a block:
//   numbers are saved.
// - Daily flags reset with the 2.0 ones at midnight; a night event's daily flag
//   also goes in sNightlyFlags (src/hoenn_flags.c). Weekly flags reset when a
//   new week starts.
// - 2.1 hidden items need a flag below 0x20E9, which this range isn't: take
//   them from the top of your 2.0 lane's hidden-item range, counting down
//   (HOENN_LANE_A_HIDDEN / HOENN_LANE_B_HIDDEN in constants/hoenn_events.h).
// - If a lane runs out, it takes from HOENN_21_FLAGS_SPARE / HOENN_21_VARS_SPARE
//   and says so in its lane status file.
// - src/hoenn_flags.c checks at build time that the blocks are in order and fit.

#include "constants/hoenn_layout.h"

// ---- Event flags (0x2400-0x267F) ---------------------------------------
#define HOENN_21_FLAGS_SHARED       (HOENN_21_FLAGS_START + 0x000) //  64  2.1-02, the sweep, anything both lanes use
#define HOENN_21_LANE_A_FLAGS       (HOENN_21_FLAGS_START + 0x040) // 192  lane A
#define HOENN_21_LANE_A_FLAGS_COUNT 0xC0
#define HOENN_21_LANE_B_FLAGS       (HOENN_21_FLAGS_START + 0x100) // 192  lane B
#define HOENN_21_LANE_B_FLAGS_COUNT 0xC0
#define HOENN_21_JOURNAL_FLAGS      (HOENN_21_FLAGS_START + 0x1C0) // 128  Seeker's Journal "heard" flags (generated, 2.1-05)
#define HOENN_21_JOURNAL_FLAGS_COUNT 0x80
#define HOENN_21_FLAGS_SPARE        (HOENN_21_FLAGS_START + 0x240) //  64  overflow, parts not split yet
#define HOENN_21_FLAGS_BLOCKS_END   HOENN_21_DAILY_FLAGS_START

// ---- Daily (0x2680-0x26BF) and weekly (0x26C0-0x26FF) flags ------------
#define HOENN_21_LANE_A_DAILY       (HOENN_21_DAILY_FLAGS_START + 0x00) // 16
#define HOENN_21_LANE_A_DAILY_COUNT 16
#define HOENN_21_LANE_B_DAILY       (HOENN_21_DAILY_FLAGS_START + 0x10) // 16
#define HOENN_21_LANE_B_DAILY_COUNT 16
#define HOENN_21_DAILY_SPARE        (HOENN_21_DAILY_FLAGS_START + 0x20) // 32
#define HOENN_21_LANE_A_WEEKLY      (HOENN_21_WEEKLY_FLAGS_START + 0x00) // 16
#define HOENN_21_LANE_A_WEEKLY_COUNT 16
#define HOENN_21_LANE_B_WEEKLY      (HOENN_21_WEEKLY_FLAGS_START + 0x10) // 16
#define HOENN_21_LANE_B_WEEKLY_COUNT 16
#define HOENN_21_WEEKLY_SPARE       (HOENN_21_WEEKLY_FLAGS_START + 0x20) // 32

// ---- Vars (0x7100-0x713F) -----------------------------------------------
#define HOENN_21_VARS_SHARED        (HOENN_21_VARS_START + 0x00) //  8
#define HOENN_21_LANE_A_VARS        (HOENN_21_VARS_START + 0x08) // 24
#define HOENN_21_LANE_A_VARS_COUNT  24
#define HOENN_21_LANE_B_VARS        (HOENN_21_VARS_START + 0x20) // 24
#define HOENN_21_LANE_B_VARS_COUNT  24
#define HOENN_21_VARS_SPARE         (HOENN_21_VARS_START + 0x38) //  8
#define HOENN_21_VARS_BLOCKS_END    (HOENN_21_VARS_START + HOENN_21_VARS_COUNT)

// ==== Shared (2.1-02 on) ====================================================

// 2.1-02: scaled-opponent ladders for HoennScaledAce (src/hoenn_scaling.c,
// which explains each). Script constants, not flags: pass one in VAR_0x8004 to
// specialvar VAR_RESULT, HoennScaledAceForScript.
#define HOENN_LADDER_PROGRESS   0 // the progress cap; 85 after Wallace (DAMIAN, part 29's wild events)
#define HOENN_LADDER_TOP_90     1 // PROGRESS, then 85 / 87 / 90 after Wallace / Brendan & May / Red (Silver, rival, friends)
#define HOENN_LADDER_ELITE      2 // next Hoenn leader's ace - 1; 83 with 8 badges, 84 after Wallace (elite trainers)

// ==== Lane A (2.1) ==========================================================

// ---- 2.1-05: the SEEKER'S JOURNAL (src/hoenn_journal.c) -------------------
// Its heard flags are generated (one per event, tools/hoenn/gen_hoenn_journal.py
// from data/hoenn/journal.json): FLAG_HOENN_HEARD_<KEY> in HOENN_21_JOURNAL_FLAGS,
// numbers locked in data/hoenn/journal_flags.json.
#include "constants/hoenn_journal.h"
#define FLAG_HOENN_GOT_SEEKERS_JOURNAL (HOENN_21_LANE_A_FLAGS + 0x00) // Elm's aide gave it

// ---- 2.1-06: the journal's rewards (a region's page all found) -------------
#define FLAG_HOENN_JOURNAL_DOLL_JOHTO  (HOENN_21_LANE_A_FLAGS + 0x01) // Elm gave the CELEBI DOLL
#define FLAG_HOENN_JOURNAL_DOLL_KANTO  (HOENN_21_LANE_A_FLAGS + 0x02) // Oak gave the MEW DOLL
#define FLAG_HOENN_JOURNAL_DOLL_HOENN  (HOENN_21_LANE_A_FLAGS + 0x03) // Birch gave the JIRACHI DOLL
#define FLAG_HOENN_CALL_JOURNAL_JOHTO  (HOENN_21_LANE_A_FLAGS + 0x04) // Elm's call rang
#define FLAG_HOENN_CALL_JOURNAL_KANTO  (HOENN_21_LANE_A_FLAGS + 0x05) // Oak's
#define FLAG_HOENN_CALL_JOURNAL_HOENN  (HOENN_21_LANE_A_FLAGS + 0x06) // Birch's

// ---- 2.1-07: the daily swarm (src/hoenn_swarm.c) ----------------------------
#define FLAG_HOENN_SWARM_MET           (HOENN_21_LANE_A_FLAGS + 0x07) // met a swarm Pokémon (the journal's found)
#define VAR_HOENN_SWARM_TODAY          (HOENN_21_LANE_A_VARS + 0x00)  // today's row in sHoennSwarms + 1 (0 = not rolled yet)

// ==== Lane B (2.1) ============================================
#endif // GUARD_CONSTANTS_HOENN_EVENTS_2_1_H
