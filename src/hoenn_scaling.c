// Hoenn & Seek (part 9): trainer card stamps and gym level scaling.
//
// Hoenn gyms can be done in any order, so their teams scale with how many
// stamps the player already has. The leader's strongest Pokémon is set to
// HOENN_GYM_ACE_MIN at 0 stamps and HOENN_GYM_ACE_MAX at 7 (the eighth gym);
// the rest of the team keeps Emerald's level gaps below it. Gym trainers
// top out HOENN_GYM_TRAINER_GAP levels below the leader.
#include "global.h"
#include "data.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_caps.h"
#include "battle_setup.h"
#include "constants/opponents.h"
#include "constants/hoenn_events_2_1.h"

#if IS_HNS

#include "data/hoenn_gym_trainers.h"
#include "data/hoenn_scaled_trainers.h"

#define HOENN_GYM_ACE_MIN       68
#define HOENN_GYM_ACE_MAX       78
#define HOENN_GYM_TRAINER_GAP   4
// Part 14: route trainers who know who you are (data/hoenn/trainer_curation.json).
#define HOENN_STRONG_GAP        6   // want the challenge: 62-72
#define HOENN_HOPEFUL_GAP       14  // battle you anyway: 54-64

enum { GYM_ROLE_NONE, GYM_ROLE_TRAINER, GYM_ROLE_LEADER, GYM_ROLE_STRONG, GYM_ROLE_HOPEFUL, GYM_ROLE_REMATCH, GYM_ROLE_PROGRESS, GYM_ROLE_SILVER, GYM_ROLE_ELITE, GYM_ROLE_FRIEND };

// 2.0 part 17r: DAMIAN, Charmander's old trainer outside the Cerulean Gym
// (Emerald's unused TRAINER_LEROY slot). Kanto has no gate, so his ace follows
// your progress: the level cap for your badges (src/hoenn_caps.c, Normal),
// whether or not the cap setting is on; 85 after Wallace, like the rematches.
#define HOENN_DAMIAN TRAINER_LEROY
#define HOENN_JOHTO_CHAMPION_CAP 56  // HnS's last Johto cap: the floor before Kanto badges

u32 HoennCountStamps(void)
{
    u32 i, count = 0;

    for (i = 0; i < HOENN_GYM_COUNT; i++)
        if (FlagGet(FLAG_HOENN_STAMP_GYM_1 + i))
            count++;
    return count;
}

#define HOENN_GYM_REMATCH_ACE   85  // after Wallace: leader rematches (part 13b)

u32 HoennGymAceLevel(void)
{
    u32 stamps = HoennCountStamps();
    if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
        return HOENN_GYM_REMATCH_ACE;

    if (stamps > HOENN_GYM_COUNT - 1)
        stamps = HOENN_GYM_COUNT - 1;
    return HOENN_GYM_ACE_MIN + (stamps * (HOENN_GYM_ACE_MAX - HOENN_GYM_ACE_MIN) + (HOENN_GYM_COUNT - 1) / 2) / (HOENN_GYM_COUNT - 1);
}

static u32 GetGymRole(const struct Trainer *trainer)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sHoennGymLeaders); i++)
        if (GetTrainerStructFromId(sHoennGymLeaders[i]) == trainer)
            return GYM_ROLE_LEADER;
    for (i = 0; i < ARRAY_COUNT(sHoennGymTrainers); i++)
        if (GetTrainerStructFromId(sHoennGymTrainers[i]) == trainer)
            return GYM_ROLE_TRAINER;
    for (i = 0; i < ARRAY_COUNT(sHoennStrongTrainers); i++)
        if (GetTrainerStructFromId(sHoennStrongTrainers[i]) == trainer)
            return GYM_ROLE_STRONG;
    for (i = 0; i < ARRAY_COUNT(sHoennHopefulTrainers); i++)
        if (GetTrainerStructFromId(sHoennHopefulTrainers[i]) == trainer)
            return GYM_ROLE_HOPEFUL;
    for (i = 0; i < ARRAY_COUNT(sHoennEliteTrainers); i++) // 2.1-10: the elite (plan section 9)
        if (GetTrainerStructFromId(sHoennEliteTrainers[i]) == trainer)
            return GYM_ROLE_ELITE;
    if (GetTrainerStructFromId(HOENN_DAMIAN) == trainer)
        return GYM_ROLE_PROGRESS;
    if (HoennIsSilverScaled(trainer)) // 2.1-08: SILVER's Indigo and Mt. Moon battles (src/hoenn_silver.c)
        return GYM_ROLE_SILVER;
    if (HoennIsFriendTrainer(trainer)) // 2.1-12: the friend trainers (src/hoenn_friends.c)
        return GYM_ROLE_FRIEND;
    // Part 16c: phone rematch teams top out at the gym-ace level (68-78, 85 after Wallace).
    for (i = 0; i < ARRAY_COUNT(sHoennRematchTrainers); i++)
        if (GetTrainerStructFromId(sHoennRematchTrainers[i]) == trainer)
            return GYM_ROLE_REMATCH;
    return GYM_ROLE_NONE;
}

u8 HoennScaleTrainerMonLevel(const struct Trainer *trainer, u8 level)
{
    u32 role, i, count, maxLevel = 0;
    s32 ace, scaled;

    role = GetGymRole(trainer);
    if (role == GYM_ROLE_NONE)
        return level;
    if (role == GYM_ROLE_FRIEND && HoennFriendHardLevel() != 0) // 2.1-14: HARD, every Pokémon Lv 100
        return HoennFriendHardLevel();

    count = trainer->poolSize ? trainer->poolSize : trainer->partySize;
    for (i = 0; i < count; i++)
        if (trainer->party[i].lvl > maxLevel)
            maxLevel = trainer->party[i].lvl;

    if (role == GYM_ROLE_PROGRESS)
        ace = HoennScaledAce(HOENN_LADDER_PROGRESS);
    else if (role == GYM_ROLE_SILVER) // the top-90 ladder, never below HnS's own levels
        ace = max((s32)HoennScaledAce(HOENN_LADDER_TOP_90), (s32)maxLevel);
    else if (role == GYM_ROLE_FRIEND) // the top-90 ladder
        ace = HoennScaledAce(HOENN_LADDER_TOP_90);
    else if (role == GYM_ROLE_ELITE) // the next leader's ace - 1 (83 with 8 badges, 84 after Wallace)
        ace = HoennScaledAce(HOENN_LADDER_ELITE);
    else
        ace = HoennGymAceLevel();
    if (role == GYM_ROLE_TRAINER)
        ace -= HOENN_GYM_TRAINER_GAP;
    else if (role == GYM_ROLE_STRONG)
        ace -= HOENN_STRONG_GAP;
    else if (role == GYM_ROLE_HOPEFUL)
        ace -= HOENN_HOPEFUL_GAP;
    scaled = ace - (s32)(maxLevel - level);
    if (scaled < 1)
        scaled = 1;
    if (scaled > MAX_LEVEL)
        scaled = MAX_LEVEL;
    return scaled;
}

// ---- 2.1-02: scaled-opponent ladders -------------------------------------
//
// One place for every "scales with your progress" opponent outside the Hoenn
// gyms. Each ladder gives the ace (the team's strongest Pokémon); callers keep
// the team's own gaps below it, as HoennScaleTrainerMonLevel does.
//
//   HOENN_LADDER_PROGRESS  the player's progress cap, as if LEVEL CAP were on
//                          (Normal), whether or not it is: HnS's Johto caps by
//                          badge count, then the post-Johto cap (Kanto badges,
//                          Kanto champion, Hoenn badges, Wallace's 84), and 85
//                          after Wallace like the leader rematches. DAMIAN (2.0
//                          part 16) and lane B's progress-level wild events
//                          (part 29) use it.
//   HOENN_LADDER_TOP_90    PROGRESS up to Wallace, then two post-game steps up
//                          to Bryan's top of 90 (2.1 plan, round 5): 85 after
//                          Wallace, 87 after Brendan & May, 90 after Red.
//                          Silver, the second rival and the friend trainers.
//   HOENN_LADDER_ELITE     Hoenn's elite trainers (2.1 plan section 9): the next
//                          Hoenn leader's ace - 1; Wallace's ace - 1 (83) with
//                          eight badges, 84 after Wallace.

// HnS's Normal level caps by Johto badge count (src/caps.c's table); 0 badges: 10.
static const u8 sJohtoBadgeAces[] = { 10, 15, 19, 24, 29, 31, 33, 42 };

#define LADDER_AFTER_WALLACE    HOENN_GYM_REMATCH_ACE // 85
#define LADDER_AFTER_BM         87
#define LADDER_TOP              90
#define LADDER_WALLACE_ACE      84
#define LADDER_ELITE_GAP        1

static u32 CountJohtoBadges(void)
{
    u32 i, badges = 0;

    for (i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            badges++;
    return badges;
}

static u32 ProgressAce(void)
{
    u32 badges;

    if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
        return LADDER_AFTER_WALLACE;
    badges = CountJohtoBadges();
    if (badges < ARRAY_COUNT(sJohtoBadgeAces))
        return sJohtoBadgeAces[badges];
    return HoennGetPostJohtoLevelCap(HOENN_JOHTO_CHAMPION_CAP, FALSE);
}

u32 HoennScaledAce(u32 ladder)
{
    switch (ladder)
    {
    case HOENN_LADDER_TOP_90:
        if (HasTrainerBeenFought(TRAINER_RED_HNS))
            return LADDER_TOP;
        if (FlagGet(FLAG_HOENN_BEAT_BM))
            return LADDER_AFTER_BM;
        return ProgressAce();
    case HOENN_LADDER_ELITE:
        if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
            return LADDER_WALLACE_ACE;
        if (HoennCountStamps() >= HOENN_GYM_COUNT)
            return LADDER_WALLACE_ACE - LADDER_ELITE_GAP;
        return HoennGymAceLevel() - LADDER_ELITE_GAP;
    case HOENN_LADDER_PROGRESS:
    default:
        return ProgressAce();
    }
}

// specialvar VAR_RESULT, HoennScaledAceForScript: VAR_0x8004 = a HOENN_LADDER_*.
u16 HoennScaledAceForScript(void)
{
    return HoennScaledAce(VarGet(VAR_0x8004));
}

#endif // IS_HNS

#if IS_HNS
#include "script_pokemon_util.h"

// HOENN_SCALED_WILD_GAP (include/hoenn.h): scaled wild Pokémon are this far below the gym ace.

// Special: set up a scripted wild battle against VAR_0x8004's species at the
// stamp curve minus HOENN_SCALED_WILD_GAP (Lv 60 -> 70). Used for the Route 120
// bridge Kecleon (part 11).
void HoennSetupScaledWildBattle(void)
{
    CreateScriptedWildMon(VarGet(VAR_0x8004), HoennGymAceLevel() - HOENN_SCALED_WILD_GAP, ITEM_NONE);
}
#endif
