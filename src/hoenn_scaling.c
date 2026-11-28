// Hoenn & Seek (part 9): trainer card stamps and gym level scaling.
//
// Hoenn gyms can be done in any order, so their teams scale with how many
// stamps the player already has. The leader's strongest Pokémon is set to
// HOENN_GYM_ACE_MIN at 0 stamps and HOENN_GYM_ACE_MAX at 7 (the eighth gym);
// the rest of the team keeps Emerald's level gaps below it. Gym trainers
// top out HOENN_GYM_TRAINER_GAP levels below the leader.
//
// 2.1 part 02 adds THE SCALED-OPPONENT LADDER (HoennScaledAce, further down):
// the one place that says how strong an opponent who follows your progress
// is (DAMIAN, Silver, the second rival, the three friends, the elite Hoenn
// trainers), so those parts only name a ladder instead of each working it out.
#include "global.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_caps.h"
#include "string_util.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/opponents.h"

#if IS_HNS

#include "data/hoenn_gym_trainers.h"
#include "data/hoenn_scaled_trainers.h"

#define HOENN_GYM_ACE_MIN       68
#define HOENN_GYM_ACE_MAX       78
#define HOENN_GYM_TRAINER_GAP   4
// Part 14: route trainers who know who you are (data/hoenn/trainer_curation.json).
#define HOENN_STRONG_GAP        6   // want the challenge: 62-72
#define HOENN_HOPEFUL_GAP       14  // battle you anyway: 54-64

enum { GYM_ROLE_NONE, GYM_ROLE_TRAINER, GYM_ROLE_LEADER, GYM_ROLE_STRONG, GYM_ROLE_HOPEFUL, GYM_ROLE_REMATCH, GYM_ROLE_LADDER };

// 2.0 part 17r: DAMIAN, Charmander's old trainer outside the Cerulean Gym
// (Emerald's unused TRAINER_LEROY slot). Kanto has no gate, so his ace follows
// your progress: the level cap for your badges (src/hoenn_caps.c, Normal),
// whether or not the cap setting is on; 85 after Wallace, like the rematches.
// Since 2.1 part 02 that's LADDER_PROGRESS (sLadderTrainers below).
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

// ---- 2.1 part 02: the scaled-opponent ladder ----------------------------
//
// Opponents who follow your progress field an ace (their strongest Pokémon)
// at the level one of these ladders gives; the rest of the team keeps its
// gaps below it, like the gyms. One place, so every part that adds such an
// opponent agrees with the others (2.1-plan.md sections 9, 10, 16).
//
//   LADDER_PROGRESS  your progress cap: the Normal LEVEL CAP for where you are
//                    (src/caps.c, src/hoenn_caps.c), whatever the setting,
//                    never below 56 (Lance: HnS's last Johto cap); 85 after
//                    Wallace, like the gym rematches. DAMIAN (2.0 part 17r).
//   LADDER_RIVAL     the same progress cap from the very start (the second
//                    rival meets you before your first badge), then the
//                    post-game steps (Bryan, round 5: top 90):
//                      beat Wallace                      85
//                      beat Brendan & May                87  (their ace)
//                      beat Red on Mt. Silver            90  (the top)
//                    Silver (2.1-08), Ethan/Lyra (2.1-15-17), the three
//                    friends (2.1-12, 14; their HARD battles are all Lv 100
//                    and skip the ladder).
//   LADDER_ELITE     the next Hoenn leader's ace - 1 (section 9): the stamp
//                    curve - 1 (67-77); with all eight stamps the next fight is
//                    Wallace, so 83; after Wallace the leaders rematch at 85, so
//                    84. The ~15 elite Hoenn trainers (2.1-10, 11).
#define LADDER_TOP_WALLACE    85  // = HOENN_GYM_REMATCH_ACE
#define LADDER_TOP_BM         87  // Brendan & May's ace
#define LADDER_TOP            90  // Bryan, round 5
#define HOENN_WALLACE_ACE     84

STATIC_ASSERT(LADDER_PROGRESS == HOENN_LADDER_PROGRESS && LADDER_RIVAL == HOENN_LADDER_RIVAL
           && LADDER_ELITE == HOENN_LADDER_ELITE && LADDER_COUNT == HOENN_LADDER_COUNT, HoennLadderScriptValues);

// Trainers whose teams the ladder scales. Each 2.1 part that adds one appends
// a row here (track A owns this table).
static const struct { u16 trainerId; u8 ladder; } sLadderTrainers[] =
{
    { HOENN_DAMIAN, LADDER_PROGRESS }, // 2.0 part 17r
};

// The progress cap: where you are in the game, as if LEVEL CAP were on
// (Normal). Uncapped (MAX_LEVEL) after Wallace; the ladders have their own
// post-game steps.
u32 HoennProgressCap(void)
{
    return HoennNormalLevelCap();
}

u32 HoennScaledAce(enum HoennLadder ladder)
{
    u32 stamps;

    switch (ladder)
    {
    case LADDER_PROGRESS:
        if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
            return LADDER_TOP_WALLACE;
        // DAMIAN only meets you in Kanto, after eight Johto badges.
        return HoennGetPostJohtoLevelCap(HOENN_JOHTO_CHAMPION_CAP, FALSE);
    case LADDER_RIVAL:
        if (HasTrainerBeenFought(TRAINER_RED_HNS) && FlagGet(FLAG_HOENN_BEAT_BM))
            return LADDER_TOP;
        if (FlagGet(FLAG_HOENN_BEAT_BM))
            return LADDER_TOP_BM;
        if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
            return LADDER_TOP_WALLACE;
        return HoennProgressCap();
    case LADDER_ELITE:
        if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
            return HOENN_GYM_REMATCH_ACE - 1;
        stamps = HoennCountStamps();
        if (stamps >= HOENN_GYM_COUNT)
            return HOENN_WALLACE_ACE - 1;
        return HoennGymAceLevel() - 1;
    default:
        return MAX_LEVEL;
    }
}

// specialvar VAR_RESULT, HoennScaledAceFor with HOENN_LADDER_* in VAR_0x8004:
// for scripts that say a level ("I've gotten stronger: Lv 72 now!").
u16 HoennScaledAceFor(void)
{
    u32 ladder = VarGet(VAR_0x8004);
    return ladder < LADDER_COUNT ? HoennScaledAce(ladder) : 0;
}

// The ladder a trainer is on, LADDER_COUNT for none.
u32 HoennTrainerLadder(u32 trainerId)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLadderTrainers); i++)
        if (sLadderTrainers[i].trainerId == trainerId)
            return sLadderTrainers[i].ladder;
    return LADDER_COUNT;
}

// Debug menu (H&S time & weather… → LADDERS): every ladder's ace right now.
void HoennDebugBufferLadders(void)
{
    u8 *s = gStringVar4;

    s = StringCopy(s, COMPOUND_STRING("Ladder aces: progress "));
    s = ConvertIntToDecimalStringN(s, HoennScaledAce(LADDER_PROGRESS), STR_CONV_MODE_LEFT_ALIGN, 3);
    s = StringCopy(s, COMPOUND_STRING(",\nrival "));
    s = ConvertIntToDecimalStringN(s, HoennScaledAce(LADDER_RIVAL), STR_CONV_MODE_LEFT_ALIGN, 3);
    s = StringCopy(s, COMPOUND_STRING(", elite "));
    s = ConvertIntToDecimalStringN(s, HoennScaledAce(LADDER_ELITE), STR_CONV_MODE_LEFT_ALIGN, 3);
    s = StringCopy(s, COMPOUND_STRING(" (cap "));
    s = ConvertIntToDecimalStringN(s, HoennProgressCap(), STR_CONV_MODE_LEFT_ALIGN, 3);
    StringCopy(s, COMPOUND_STRING(")"));
}

static u32 LadderOfTrainer(const struct Trainer *trainer)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLadderTrainers); i++)
        if (GetTrainerStructFromId(sLadderTrainers[i].trainerId) == trainer)
            return sLadderTrainers[i].ladder;
    return LADDER_COUNT;
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
    for (i = 0; i < ARRAY_COUNT(sLadderTrainers); i++)
        if (GetTrainerStructFromId(sLadderTrainers[i].trainerId) == trainer)
            return GYM_ROLE_LADDER;
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

    count = trainer->poolSize ? trainer->poolSize : trainer->partySize;
    for (i = 0; i < count; i++)
        if (trainer->party[i].lvl > maxLevel)
            maxLevel = trainer->party[i].lvl;

    if (role == GYM_ROLE_LADDER)
        ace = HoennScaledAce(LadderOfTrainer(trainer));
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
