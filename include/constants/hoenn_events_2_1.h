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
// (no definitions yet)

// ==== Lane B (2.1) ==========================================================

// 2.1-34: Teleport to your secret base. Your own base's entrance, recorded each
// time you walk into it (HoennRecordBaseEntrance, src/hoenn_2_1_lane_b.c).
#define VAR_HOENN_BASE_TELEPORT_ID   (HOENN_21_LANE_B_VARS + 0) // the base's secretBaseId; 0 = none recorded
#define VAR_HOENN_BASE_TELEPORT_MAP  (HOENN_21_LANE_B_VARS + 1) // mapGroup << 8 | mapNum of the map outside
#define VAR_HOENN_BASE_TELEPORT_XY   (HOENN_21_LANE_B_VARS + 2) // x | y << 8: the tile you walked in from
// HoennTeleportGo's choice (VAR_RESULT): the menu's order.
#define HOENN_TELEPORT_TO_CENTER 0
#define HOENN_TELEPORT_TO_BASE   1

#endif // GUARD_CONSTANTS_HOENN_EVENTS_2_1_H
