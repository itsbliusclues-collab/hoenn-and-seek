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

// ---- 2.1-08: SILVER keeps up (src/hoenn_silver.c) ---------------------------
#define FLAG_HOENN_HIDE_SILVER_LEAGUE  (HOENN_21_LANE_A_FLAGS + 0x08) // his object at the Ever Grande Pokémon League (set until Wallace is beaten)
#define FLAG_HOENN_MET_SILVER_HOENN    (HOENN_21_LANE_A_FLAGS + 0x09) // talked to him there once
#define FLAG_HOENN_SILVER_CELEBI_LINE  (HOENN_21_LANE_A_FLAGS + 0x0A) // he told you what he remembered after the Route 22 scene
#define FLAG_HOENN_BM_SILVER_LINE      (HOENN_21_LANE_A_FLAGS + 0x0B) // Brendan or May told you they met him

// 2.1-12/14: the three friend trainers (src/hoenn_friends.c; one per friend, in HOENN_FRIEND_* order)
#define FLAG_HOENN_MET_BRIDGETTE       (HOENN_21_LANE_A_FLAGS + 0x0C) // talked once (her name line is first-meeting only)
#define FLAG_HOENN_MET_AIDAN           (HOENN_21_LANE_A_FLAGS + 0x0D)
#define FLAG_HOENN_MET_BROCK           (HOENN_21_LANE_A_FLAGS + 0x0E)
#define FLAG_HOENN_HARD_BEAT_BRIDGETTE (HOENN_21_LANE_A_FLAGS + 0x0F) // first HARD win (2.1-14; Pangea reads them)
#define FLAG_HOENN_HARD_BEAT_AIDAN     (HOENN_21_LANE_A_FLAGS + 0x10)
#define FLAG_HOENN_HARD_BEAT_BROCK     (HOENN_21_LANE_A_FLAGS + 0x11)
#define FLAG_HOENN_BROCK_BEST_FRIEND   (HOENN_21_LANE_A_FLAGS + 0x12) // 2.1-14: you've beaten ATHLETE BROCK once (he calls you his best friend)
// 2.1-15: the second rival, ETHAN or LYRA (Elm's neighbor; the other gender from the player)
#define FLAG_HOENN_HIDE_LAB_NEIGHBOR       (HOENN_21_LANE_A_FLAGS + 0x13) // 2.1-15: ETHAN/LYRA in Elm's lab (decided on every arrival)
#define FLAG_HOENN_NEIGHBOR_PHONE_ETHAN    (HOENN_21_LANE_A_FLAGS + 0x14) // number registered (a female player's neighbor)
#define FLAG_HOENN_NEIGHBOR_PHONE_LYRA     (HOENN_21_LANE_A_FLAGS + 0x15) // (a male player's neighbor)
#define FLAG_HOENN_HIDE_ROUTE29_NEIGHBOR   (HOENN_21_LANE_A_FLAGS + 0x16) // their object on Route 29 (only out during the catching demo)
#define FLAG_HOENN_CALL_NEIGHBOR_FIRST     (HOENN_21_LANE_A_FLAGS + 0x17) // their milestone calls, one flag each, in
#define FLAG_HOENN_CALL_NEIGHBOR_LAST      (HOENN_21_LANE_A_FLAGS + 0x1F) //   HOENN_CALL_NEIGHBOR_* order (9 flags)
#define FLAG_HOENN_NEIGHBOR_TOWN_FIRST     (HOENN_21_LANE_A_FLAGS + 0x20) // 2.1-16: their in-person scenes done, one flag
#define FLAG_HOENN_NEIGHBOR_TOWN_LAST      (HOENN_21_LANE_A_FLAGS + 0x28) //   per HOENN_NEIGHBOR_TOWN_* below (9 flags)
#define FLAG_HOENN_NEIGHBOR_STAYED         (FLAG_HOENN_NEIGHBOR_TOWN_FIRST + 8) // the Oak's-lab scene: they stay in Kanto as Oak's field aide
#define FLAG_HOENN_HIDE_NEIGHBOR_TOWN      (HOENN_21_LANE_A_FLAGS + 0x29) // their object in each scene's map (decided on every arrival)
#define VAR_HOENN_NEIGHBOR_STATE           (HOENN_21_LANE_A_VARS + 0x04)  // HOENN_NEIGHBOR_* below
// VAR_HOENN_NEIGHBOR_STATE
#define HOENN_NEIGHBOR_NOT_MET   0 // in Elm's lab from your first visit
#define HOENN_NEIGHBOR_WAITING   1 // you have your starter: they want a word before you leave the lab
#define HOENN_NEIGHBOR_PHONE     2 // number given; the catching demo waits on Route 29
#define HOENN_NEIGHBOR_DEMO_DONE 3 // they showed you how to catch and gave you POKé BALLS
// 2.1-16: their in-person scenes (VAR_0x8004 for the specials). Each town's is
// out while you're short of the badge that's next there; the lab's after 16.
#define HOENN_NEIGHBOR_TOWN_VIOLET    0 // before FALKNER
#define HOENN_NEIGHBOR_TOWN_AZALEA    1 // before your 2nd Johto badge
#define HOENN_NEIGHBOR_TOWN_GOLDENROD 2 // before your 3rd
#define HOENN_NEIGHBOR_TOWN_ECRUTEAK  3 // before your 4th
#define HOENN_NEIGHBOR_TOWN_OLIVINE   4 // before your 6th
#define HOENN_NEIGHBOR_TOWN_VERMILION 5 // Johto champion, before all 8 Kanto badges
#define HOENN_NEIGHBOR_TOWN_CERULEAN  6 // same
#define HOENN_NEIGHBOR_TOWN_PEWTER    7 // same
#define HOENN_NEIGHBOR_TOWN_OAKS_LAB  8 // all 8 Kanto badges: they tell OAK they'll stay
#define HOENN_NEIGHBOR_TOWN_COUNT     9
#define FLAG_HOENN_DAILY_BRIDGETTE     (HOENN_21_LANE_A_DAILY + 0)    // battled today
#define FLAG_HOENN_DAILY_AIDAN         (HOENN_21_LANE_A_DAILY + 1)
#define FLAG_HOENN_DAILY_BROCK         (HOENN_21_LANE_A_DAILY + 2)
#define VAR_HOENN_FRIEND_LV_BRIDGETTE  (HOENN_21_LANE_A_VARS + 0x01)  // her ace's level in your last battle (0 = never)
#define VAR_HOENN_FRIEND_LV_AIDAN      (HOENN_21_LANE_A_VARS + 0x02)
#define VAR_HOENN_FRIEND_LV_BROCK      (HOENN_21_LANE_A_VARS + 0x03)
#define HOENN_FRIEND_BRIDGETTE 0 // VAR_0x8004 for the friends' specials
#define HOENN_FRIEND_AIDAN     1
#define HOENN_FRIEND_BROCK     2
#define HOENN_FRIEND_COUNT     3
#define HOENN_NAME_OTHER       0 // HoennFriendNameLine
#define HOENN_NAME_AIDAN       1
#define HOENN_NAME_BRIDGET     2
#define HOENN_NAME_BROCK       3
#define HOENN_FRIEND_STATE_FIRST    0 // HoennFriendTalkState: first meeting (sets the met flag)
#define HOENN_FRIEND_STATE_AGAIN    1
#define HOENN_FRIEND_STATE_STRONGER 2 // the ladder is higher than at your last battle
#define HOENN_FRIEND_STATE_TODAY    3 // already battled today
#define HOENN_FRIEND_TEXT_MEET      0 // callnative HoennSeek_FriendText, .byte HOENN_FRIEND_TEXT_*
#define HOENN_FRIEND_TEXT_NAME      1 // the line for your name (only when HoennFriendHasNameLine)
#define HOENN_FRIEND_TEXT_AGAIN     2 // ends with the "battle?" question
#define HOENN_FRIEND_TEXT_STRONGER  3 // ends with the "battle?" question
#define HOENN_FRIEND_TEXT_ASK       4 // first meeting's question
#define HOENN_FRIEND_TEXT_GO        5
#define HOENN_FRIEND_TEXT_WON       6
#define HOENN_FRIEND_TEXT_RAN       7
#define HOENN_FRIEND_TEXT_DECLINE   8
#define HOENN_FRIEND_TEXT_TODAY     9
#define HOENN_FRIEND_TEXT_HARD_WON  10
#define HOENN_FRIEND_TEXT_COUNT     11
#define HOENN_FRIEND_CHOICE_NORMAL  0 // the NORMAL / HARD / NO menu after the final Red
#define HOENN_FRIEND_CHOICE_HARD    1

// ==== Lane B (2.1) ============================================

// 2.1-34: Teleport to your secret base. Your own base's entrance, recorded each
// time you walk into it (HoennRecordBaseEntrance, src/hoenn_2_1_lane_b.c).
#define VAR_HOENN_BASE_TELEPORT_ID   (HOENN_21_LANE_B_VARS + 0) // the base's secretBaseId; 0 = none recorded
#define VAR_HOENN_BASE_TELEPORT_MAP  (HOENN_21_LANE_B_VARS + 1) // mapGroup << 8 | mapNum of the map outside
#define VAR_HOENN_BASE_TELEPORT_XY   (HOENN_21_LANE_B_VARS + 2) // x | y << 8: the tile you walked in from
// HoennTeleportGo's choice (VAR_RESULT): the menu's order.
#define HOENN_TELEPORT_TO_CENTER 0
#define HOENN_TELEPORT_TO_BASE   1

// 2.1-18 to 20: gym leaders' weekends. Three flags per leader (gym, spot, dojo),
// 24 leaders; the names are generated (constants/hoenn_weekends.h, from
// data/hoenn/leader_weekends.json, whose "slot" picks the leader's three).
#define HOENN_21_WEEKEND_FLAGS       (HOENN_21_LANE_B_FLAGS + 0x00) // 72 flags: +0x00-0x47
#define FLAG_HOENN_CALL_WEEKENDS     (HOENN_21_LANE_B_FLAGS + 0x48) // May: the League's weekend memo
#include "constants/hoenn_weekends.h"
#endif // GUARD_CONSTANTS_HOENN_EVENTS_2_1_H
