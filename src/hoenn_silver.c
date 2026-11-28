// Hoenn & Seek 2.1-08: SILVER keeps up (2.1 plan section 4; lane A).
//
// HnS's Silver ends with a daily rematch at the Indigo Plateau Pokémon Center
// (Lv 68, TRAINER_RIVAL_<X>_7_HNS) and a Mt. Moon battle (Lv 64, _6_HNS). The
// player's level cap passes 68 early in Kanto, so he stopped mattering. Two
// things fix that:
//
// - Levels: both battles join the top-90 ladder (HoennScaledAce, src/hoenn_scaling.c):
//   his ace is your progress cap (85 after Wallace, 87 after Brendan & May,
//   90 after Red), never below HnS's own levels, gaps inside the team kept.
//   GetGymRole asks HoennIsSilverScaled.
// - Teams: at four milestones his Indigo team is swapped for a stronger one
//   (src/data/hoenn_silver.party): 1 Kanto champion (Weavile joins; held items,
//   real movesets, EVs), 2 four Hoenn badges (Magnezone; Smart AI),
//   3 Wallace beaten (Metagross, caught in Hoenn after he followed you there),
//   4 Brendan & May beaten (Gengar; AI with prediction). HnS's team names are
//   kept: the CHIKORITA team has Meganium, CYNDAQUIL's Typhlosion, TOTODILE's
//   Feraligatr. The swap is HoennTrainerOverride, one hook line in
//   GetTrainerStructFromId (include/data.h), so the name, class, AI, items and
//   party all come from the tier's struct wherever the battle code looks.
//
// The same trainer IDs battle at the Ever Grande Pokémon League after you beat
// Wallace (data/scripts/hoenn_2_1_lane_a.inc), sharing his daily flag.
#include "global.h"
#include "data.h"
#include "battle.h"
#include "battle_transition.h"
#include "trainer_pools.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/pokeball.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/flags.h"
#include "constants/opponents.h"
#include "constants/trainers.h"
#include "constants/battle_ai.h"

#if IS_HNS

enum
{
    HOENN_SILVER_T1_CHIKORITA, HOENN_SILVER_T1_CYNDAQUIL, HOENN_SILVER_T1_TOTODILE,
    HOENN_SILVER_T2_CHIKORITA, HOENN_SILVER_T2_CYNDAQUIL, HOENN_SILVER_T2_TOTODILE,
    HOENN_SILVER_T3_CHIKORITA, HOENN_SILVER_T3_CYNDAQUIL, HOENN_SILVER_T3_TOTODILE,
    HOENN_SILVER_T4_CHIKORITA, HOENN_SILVER_T4_CYNDAQUIL, HOENN_SILVER_T4_TOTODILE,
    HOENN_SILVER_TEAM_COUNT,
};
#define HOENN_SILVER_TIERS 4
#define HOENN_SILVER_TEAMS 3 // one per HnS team (CHIKORITA, CYNDAQUIL, TOTODILE)

static const struct Trainer sSilverTiers[DIFFICULTY_COUNT][HOENN_SILVER_TEAM_COUNT] =
{
#include "data/hoenn_silver.h"
};

// HnS's Indigo rematch IDs, in the tiers' team order.
static const u16 sSilverIndigo[HOENN_SILVER_TEAMS] =
{
    TRAINER_RIVAL_CHIKORITA_7_HNS, TRAINER_RIVAL_CYNDAQUIL_7_HNS, TRAINER_RIVAL_TOTODILE_7_HNS,
};
// Battles that scale with the ladder (Indigo and Mt. Moon).
static const u16 sSilverScaled[] =
{
    TRAINER_RIVAL_CHIKORITA_7_HNS, TRAINER_RIVAL_CYNDAQUIL_7_HNS, TRAINER_RIVAL_TOTODILE_7_HNS,
    TRAINER_RIVAL_CHIKORITA_6_HNS, TRAINER_RIVAL_CYNDAQUIL_6_HNS, TRAINER_RIVAL_TOTODILE_6_HNS,
};

// 0 = HnS's own team, 1-4 = the milestone teams (highest reached).
u32 HoennSilverTier(void)
{
    if (FlagGet(FLAG_HOENN_BEAT_BM))
        return 4;
    if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
        return 3;
    if (HoennGetStampCount() >= 4)
        return 2;
    if (FlagGet(FLAG_IS_KANTO_CHAMPION))
        return 1;
    return 0;
}

// specialvar VAR_RESULT, HoennSilverTierForScript
u16 HoennSilverTierForScript(void)
{
    return HoennSilverTier();
}

const struct Trainer *HoennTrainerOverride(u16 trainerId, const struct Trainer *trainer)
{
    u32 i, tier;

    for (i = 0; i < HOENN_SILVER_TEAMS; i++)
    {
        if (trainerId != sSilverIndigo[i])
            continue;
        tier = HoennSilverTier();
        if (tier == 0)
            return trainer;
        return &sSilverTiers[DIFFICULTY_NORMAL][(tier - 1) * HOENN_SILVER_TEAMS + i];
    }
    return trainer;
}

// Is this (any difficulty's struct, or a tier's) one of the scaled Silver battles?
bool32 HoennIsSilverScaled(const struct Trainer *trainer)
{
    u32 i, d;

    if (trainer >= &sSilverTiers[0][0] && trainer < &sSilverTiers[0][0] + ARRAY_COUNT(sSilverTiers) * HOENN_SILVER_TEAM_COUNT)
        return TRUE;
    for (i = 0; i < ARRAY_COUNT(sSilverScaled); i++)
        for (d = 0; d < DIFFICULTY_COUNT; d++)
            if (trainer == &gTrainers[d][sSilverScaled[i]])
                return TRUE;
    return FALSE;
}

#endif // IS_HNS
