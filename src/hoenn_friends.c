// Hoenn & Seek 2.1-12/14: the friend trainers (2.1 plan section 10; lane A).
//
// Bryan's three friends are Easter eggs: out-of-the-way trainers you can
// battle once a day, for prize money only. EXPLORER BRIDGETTE (Route 111, by
// the Desert Ruins), SKYDIVER AIDAN (Route 119's ledge above the Weather
// Institute) and ATHLETE BROCK (the north end of Route 115's beach) share one
// script, HoennSeek_EventScript_FriendTalk (data/scripts/hoenn_2_1_lane_a.inc);
// everything that differs between them is in sFriends.
//
// - IDs: Emerald's unused trainer slots, renamed in data/hoenn/trainer_parties.party
//   (like DAMIAN): BRIDGETTE is TRAINER_KAYLEE, AIDAN TRAINER_DUDLEY, BROCK
//   TRAINER_TERRY. gen_hoenn_ids.py's LATE_TRAINERS gives them the last Hoenn
//   IDs, so no 2.0 trainer moved. Classes EXPLORER / SKYDIVER / ATHLETE are new
//   (include/constants/trainers.h, src/battle_main.c).
// - Levels: the top-90 ladder (HoennScaledAce(HOENN_LADDER_TOP_90): your
//   progress cap, 87 after Brendan & May, 90 after Red), gaps kept;
//   GetGymRole (src/hoenn_scaling.c) asks HoennIsFriendTrainer and
//   HoennFriendHardLevel.
// - HARD (2.1-14): after the final Red (TRAINER_RED_POSTOBC_HNS) the question
//   becomes NORMAL / HARD / NO. HARD puts every Pokémon at Lv 100 (same sets);
//   the first HARD win sets FLAG_HOENN_HARD_BEAT_* (Pangea reads them).
// - RUN works: the battle menu asks "Would you like to forfeit the match?" (the
//   Battle Frontier's question), and a forfeit returns to the field without a
//   whiteout or a money loss, so the script can say a line. Three hook lines in
//   src/battle_main.c and src/battle_setup.c ask HoennIsFriendBattle. A
//   forfeit doesn't use up the day. Losing for real whites out as usual (Bryan).
// - "I've gotten stronger": each friend's var keeps the ace's level from your
//   last normal battle; if the ladder is higher now, they open with that line.
// - Name lines: once, at the first meeting, if your name is one they know
//   (AIDAN, BRIDGET or BROCK, exact spelling, any case).
// - BROCK asks what type of Pokémon you're into at the first meeting
//   (HoennFriendTypeReaction) and calls you his best friend once you've beaten
//   him (FLAG_HOENN_BROCK_BEST_FRIEND).
#include "global.h"
#include "battle.h"
#include "battle_main.h"
#include "battle_setup.h"
#include "data.h"
#include "event_data.h"
#include "hoenn.h"
#include "script.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/opponents.h"

#if IS_HNS

extern const u8 HoennSeek_Text_BridgetteMeet[], HoennSeek_Text_BridgetteNameAidan[], HoennSeek_Text_BridgetteNameBridget[],
    HoennSeek_Text_BridgetteAgain[], HoennSeek_Text_BridgetteStronger[], HoennSeek_Text_BridgetteAsk[],
    HoennSeek_Text_BridgetteGo[], HoennSeek_Text_BridgetteWon[], HoennSeek_Text_BridgetteRan[],
    HoennSeek_Text_BridgetteDecline[], HoennSeek_Text_BridgetteToday[], HoennSeek_Text_BridgetteHardWon[];
extern const u8 HoennSeek_Text_AidanMeet[], HoennSeek_Text_AidanNameAidan[], HoennSeek_Text_AidanNameBridget[],
    HoennSeek_Text_AidanAgain[], HoennSeek_Text_AidanStronger[], HoennSeek_Text_AidanAsk[],
    HoennSeek_Text_AidanGo[], HoennSeek_Text_AidanWon[], HoennSeek_Text_AidanRan[],
    HoennSeek_Text_AidanDecline[], HoennSeek_Text_AidanToday[], HoennSeek_Text_AidanHardWon[];
extern const u8 HoennSeek_Text_BrockMeet[], HoennSeek_Text_BrockNameBrock[],
    HoennSeek_Text_BrockAgain[], HoennSeek_Text_BrockAgainBest[], HoennSeek_Text_BrockStronger[], HoennSeek_Text_BrockAsk[],
    HoennSeek_Text_BrockGo[], HoennSeek_Text_BrockWon[], HoennSeek_Text_BrockRan[],
    HoennSeek_Text_BrockDecline[], HoennSeek_Text_BrockToday[], HoennSeek_Text_BrockHardWon[];

struct HoennFriend
{
    u16 trainerId;
    u16 levelVar;
    u16 metFlag;
    u16 dailyFlag;
    u16 hardFlag;
    u16 bestFriendFlag;          // 0: none; else set on the first win, and AGAIN reads againBest
    const u8 *nameLines[4];      // by HOENN_NAME_* (NULL: no line)
    const u8 *texts[HOENN_FRIEND_TEXT_COUNT];
    const u8 *againBest;
};

static const struct HoennFriend sFriends[HOENN_FRIEND_COUNT] =
{
    [HOENN_FRIEND_BRIDGETTE] =
    {
        .trainerId = TRAINER_KAYLEE,
        .levelVar = VAR_HOENN_FRIEND_LV_BRIDGETTE,
        .metFlag = FLAG_HOENN_MET_BRIDGETTE,
        .dailyFlag = FLAG_HOENN_DAILY_BRIDGETTE,
        .hardFlag = FLAG_HOENN_HARD_BEAT_BRIDGETTE,
        .nameLines = {[HOENN_NAME_AIDAN] = HoennSeek_Text_BridgetteNameAidan, [HOENN_NAME_BRIDGET] = HoennSeek_Text_BridgetteNameBridget},
        .texts =
        {
            [HOENN_FRIEND_TEXT_MEET] = HoennSeek_Text_BridgetteMeet,
            [HOENN_FRIEND_TEXT_AGAIN] = HoennSeek_Text_BridgetteAgain,
            [HOENN_FRIEND_TEXT_STRONGER] = HoennSeek_Text_BridgetteStronger,
            [HOENN_FRIEND_TEXT_ASK] = HoennSeek_Text_BridgetteAsk,
            [HOENN_FRIEND_TEXT_GO] = HoennSeek_Text_BridgetteGo,
            [HOENN_FRIEND_TEXT_WON] = HoennSeek_Text_BridgetteWon,
            [HOENN_FRIEND_TEXT_RAN] = HoennSeek_Text_BridgetteRan,
            [HOENN_FRIEND_TEXT_DECLINE] = HoennSeek_Text_BridgetteDecline,
            [HOENN_FRIEND_TEXT_TODAY] = HoennSeek_Text_BridgetteToday,
            [HOENN_FRIEND_TEXT_HARD_WON] = HoennSeek_Text_BridgetteHardWon,
        },
    },
    [HOENN_FRIEND_AIDAN] =
    {
        .trainerId = TRAINER_DUDLEY,
        .levelVar = VAR_HOENN_FRIEND_LV_AIDAN,
        .metFlag = FLAG_HOENN_MET_AIDAN,
        .dailyFlag = FLAG_HOENN_DAILY_AIDAN,
        .hardFlag = FLAG_HOENN_HARD_BEAT_AIDAN,
        .nameLines = {[HOENN_NAME_AIDAN] = HoennSeek_Text_AidanNameAidan, [HOENN_NAME_BRIDGET] = HoennSeek_Text_AidanNameBridget},
        .texts =
        {
            [HOENN_FRIEND_TEXT_MEET] = HoennSeek_Text_AidanMeet,
            [HOENN_FRIEND_TEXT_AGAIN] = HoennSeek_Text_AidanAgain,
            [HOENN_FRIEND_TEXT_STRONGER] = HoennSeek_Text_AidanStronger,
            [HOENN_FRIEND_TEXT_ASK] = HoennSeek_Text_AidanAsk,
            [HOENN_FRIEND_TEXT_GO] = HoennSeek_Text_AidanGo,
            [HOENN_FRIEND_TEXT_WON] = HoennSeek_Text_AidanWon,
            [HOENN_FRIEND_TEXT_RAN] = HoennSeek_Text_AidanRan,
            [HOENN_FRIEND_TEXT_DECLINE] = HoennSeek_Text_AidanDecline,
            [HOENN_FRIEND_TEXT_TODAY] = HoennSeek_Text_AidanToday,
            [HOENN_FRIEND_TEXT_HARD_WON] = HoennSeek_Text_AidanHardWon,
        },
    },
    [HOENN_FRIEND_BROCK] =
    {
        .trainerId = TRAINER_TERRY,
        .levelVar = VAR_HOENN_FRIEND_LV_BROCK,
        .metFlag = FLAG_HOENN_MET_BROCK,
        .dailyFlag = FLAG_HOENN_DAILY_BROCK,
        .hardFlag = FLAG_HOENN_HARD_BEAT_BROCK,
        .bestFriendFlag = FLAG_HOENN_BROCK_BEST_FRIEND,
        .nameLines = {[HOENN_NAME_BROCK] = HoennSeek_Text_BrockNameBrock},
        .texts =
        {
            [HOENN_FRIEND_TEXT_MEET] = HoennSeek_Text_BrockMeet,
            [HOENN_FRIEND_TEXT_AGAIN] = HoennSeek_Text_BrockAgain,
            [HOENN_FRIEND_TEXT_STRONGER] = HoennSeek_Text_BrockStronger,
            [HOENN_FRIEND_TEXT_ASK] = HoennSeek_Text_BrockAsk,
            [HOENN_FRIEND_TEXT_GO] = HoennSeek_Text_BrockGo,
            [HOENN_FRIEND_TEXT_WON] = HoennSeek_Text_BrockWon,
            [HOENN_FRIEND_TEXT_RAN] = HoennSeek_Text_BrockRan,
            [HOENN_FRIEND_TEXT_DECLINE] = HoennSeek_Text_BrockDecline,
            [HOENN_FRIEND_TEXT_TODAY] = HoennSeek_Text_BrockToday,
            [HOENN_FRIEND_TEXT_HARD_WON] = HoennSeek_Text_BrockHardWon,
        },
        .againBest = HoennSeek_Text_BrockAgainBest,
    },
};

// Set by the script just before a HARD battle, cleared when it ends. Not
// saved: the game can't be saved mid-battle.
static EWRAM_DATA bool8 sHardBattle = FALSE;

bool32 HoennIsFriendTrainer(const struct Trainer *trainer)
{
    u32 i;
    for (i = 0; i < HOENN_FRIEND_COUNT; i++)
        if (GetTrainerStructFromId(sFriends[i].trainerId) == trainer)
            return TRUE;
    return FALSE;
}

// The friend whose trainer is the current (or just-ended) battle's opponent.
static s32 BattleFriend(void)
{
    u32 i;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return -1;
    for (i = 0; i < HOENN_FRIEND_COUNT; i++)
        if (TRAINER_BATTLE_PARAM.opponentA == sFriends[i].trainerId)
            return i;
    return -1;
}

// During a battle (and right after it): is it against a friend?
bool32 HoennIsFriendBattle(void)
{
    return BattleFriend() >= 0;
}

// GetGymRole's friend level: 0 for the ladder, MAX_LEVEL in a HARD battle.
u32 HoennFriendHardLevel(void)
{
    return sHardBattle ? MAX_LEVEL : 0;
}

static const struct HoennFriend *ScriptFriend(void)
{
    u32 friend = VarGet(VAR_0x8004);
    return friend < HOENN_FRIEND_COUNT ? &sFriends[friend] : &sFriends[0];
}

// specialvar VAR_RESULT, HoennFriendTalkState (VAR_0x8004 = HOENN_FRIEND_*):
// a HOENN_FRIEND_STATE_*; the first meeting sets the met flag.
u16 HoennFriendTalkState(void)
{
    const struct HoennFriend *f = ScriptFriend();
    u16 last;

    if (FlagGet(f->dailyFlag))
        return HOENN_FRIEND_STATE_TODAY;
    if (!FlagGet(f->metFlag))
    {
        FlagSet(f->metFlag);
        return HOENN_FRIEND_STATE_FIRST;
    }
    last = VarGet(f->levelVar);
    if (last != 0 && HoennScaledAce(HOENN_LADDER_TOP_90) > last)
        return HOENN_FRIEND_STATE_STRONGER;
    return HOENN_FRIEND_STATE_AGAIN;
}

// specialvar VAR_RESULT, HoennFriendHardUnlocked: the final Red is beaten.
u16 HoennFriendHardUnlocked(void)
{
    return HasTrainerBeenFought(TRAINER_RED_POSTOBC_HNS);
}

// special HoennFriendSetHard: VAR_0x8005 TRUE for a HARD battle.
void HoennFriendSetHard(void)
{
    sHardBattle = (VarGet(VAR_0x8005) != 0);
}

// specialvar VAR_RESULT, HoennFriendBattleEnded: after the battle. Puts the
// friend back in VAR_0x8004 (the battle may have used it), records the
// ladder level of a normal battle, sets the daily flag on a win (and the HARD
// and best-friend flags), and returns the outcome (B_OUTCOME_WON, or
// B_OUTCOME_FORFEITED for RUN; a loss never gets here, it whites out).
u16 HoennFriendBattleEnded(void)
{
    s32 friend = BattleFriend();
    const struct HoennFriend *f;
    bool32 hard = sHardBattle;

    sHardBattle = FALSE;
    if (friend < 0)
        return gBattleOutcome;
    gSpecialVar_0x8004 = friend;
    gSpecialVar_0x8005 = hard;
    f = &sFriends[friend];
    if (!hard)
        VarSet(f->levelVar, HoennScaledAce(HOENN_LADDER_TOP_90));
    if (gBattleOutcome == B_OUTCOME_WON)
    {
        FlagSet(f->dailyFlag);
        if (hard)
            FlagSet(f->hardFlag);
        if (f->bestFriendFlag != 0)
            FlagSet(f->bestFriendFlag);
    }
    return gBattleOutcome;
}

// The player's name, upper-cased, equals text (an upper-case game string)?
static bool32 PlayerNameIs(const u8 *text)
{
    u32 i;
    for (i = 0; i < PLAYER_NAME_LENGTH; i++)
    {
        u8 c = gSaveBlock2Ptr->playerName[i];
        if (c >= CHAR_a && c <= CHAR_z)
            c = c - CHAR_a + CHAR_A;
        if (c != text[i])
            return FALSE;
        if (c == EOS)
            return TRUE;
    }
    return text[i] == EOS;
}

// Which name you have (HOENN_NAME_OTHER, HOENN_NAME_AIDAN, ...).
static u32 PlayerNameKind(void)
{
    static const u8 sAidan[] = _("AIDAN");
    static const u8 sBridget[] = _("BRIDGET");
    static const u8 sBrock[] = _("BROCK");

    if (PlayerNameIs(sAidan))
        return HOENN_NAME_AIDAN;
    if (PlayerNameIs(sBridget))
        return HOENN_NAME_BRIDGET;
    if (PlayerNameIs(sBrock))
        return HOENN_NAME_BROCK;
    return HOENN_NAME_OTHER;
}

// specialvar VAR_RESULT, HoennFriendHasNameLine (VAR_0x8004 = friend).
u16 HoennFriendHasNameLine(void)
{
    return ScriptFriend()->nameLines[PlayerNameKind()] != NULL;
}

// callnative HoennSeek_FriendText, .byte HOENN_FRIEND_TEXT_*: data[0] = that
// line of friend VAR_0x8004 (for callstd MSGBOX_*, like the hint NPCs).
void HoennSeek_FriendText(struct ScriptContext *ctx)
{
    u32 which = ScriptReadByte(ctx);
    const struct HoennFriend *f = ScriptFriend();
    const u8 *text;

    if (which == HOENN_FRIEND_TEXT_NAME)
        text = f->nameLines[PlayerNameKind()];
    else if (which == HOENN_FRIEND_TEXT_AGAIN && f->bestFriendFlag != 0 && FlagGet(f->bestFriendFlag))
        text = f->againBest;
    else
        text = which < HOENN_FRIEND_TEXT_COUNT ? f->texts[which] : NULL;
    if (text == NULL)
        text = f->texts[HOENN_FRIEND_TEXT_AGAIN];
    ctx->data[0] = (uintptr_t)text;
}

// BROCK's question at the first meeting: the menu's types, in order.
static const u8 sBrockTypes[] =
{
    TYPE_NORMAL, TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_ICE,
    TYPE_FIGHTING, TYPE_POISON, TYPE_GROUND, TYPE_FLYING, TYPE_PSYCHIC, TYPE_BUG,
    TYPE_ROCK, TYPE_GHOST, TYPE_DRAGON, TYPE_DARK, TYPE_STEEL, TYPE_FAIRY,
};

// specialvar VAR_RESULT, HoennFriendTypeReaction: VAR_RESULT before the call
// is the menu index (VAR_0x8005). STR_VAR_1 = the type; if one of BROCK's
// Pokémon has it, STR_VAR_2 = its species and the answer is TRUE.
u16 HoennFriendTypeReaction(void)
{
    const struct Trainer *trainer = GetTrainerStructFromId(sFriends[HOENN_FRIEND_BROCK].trainerId);
    u32 index = VarGet(VAR_0x8005), type, i;

    if (index >= ARRAY_COUNT(sBrockTypes))
        index = 0;
    type = sBrockTypes[index];
    StringCopy(gStringVar1, gTypesInfo[type].name);
    for (i = trainer->partySize; i-- > 0;) // from the ace down: FIGHTING names MACHAMP
    {
        u32 species = trainer->party[i].species;
        if (GetSpeciesType(species, 0) == type || GetSpeciesType(species, 1) == type)
        {
            StringCopy(gStringVar2, GetSpeciesName(species));
            return TRUE;
        }
    }
    return FALSE;
}

#endif // IS_HNS
