// Hoenn & Seek 2.1-12: the friend trainers (2.1 plan section 10; lane A).
//
// Bryan's three friends are Easter eggs: out-of-the-way trainers you can
// battle once a day, for prize money only. 2.1-12 builds the shared system and
// EXPLORER BRIDGETTE (Route 111, by the Desert Ruins); 2.1-14 adds SKYDIVER
// AIDAN and ATHLETE BROCK to the same table and the post-game HARD mode.
//
// - IDs: Emerald's unused trainer slots, renamed in data/hoenn/trainer_parties.party
//   (like DAMIAN): BRIDGETTE is TRAINER_KAYLEE. Classes EXPLORER / SKYDIVER /
//   ATHLETE are new (include/constants/trainers.h, src/battle_main.c).
// - Levels: the top-90 ladder (HoennScaledAce(HOENN_LADDER_TOP_90): your
//   progress cap, 87 after Brendan & May, 90 after Red), gaps kept;
//   GetGymRole (src/hoenn_scaling.c) asks HoennIsFriendTrainer.
// - RUN works: the battle menu asks "Would you like to forfeit the match?" (the
//   Battle Frontier's question), and a forfeit returns to the field without a
//   whiteout or a money loss, so the script can say a line. Three hook lines in
//   src/battle_main.c and src/battle_setup.c ask HoennIsFriendBattle. Losing
//   for real whites out as usual (Bryan).
// - "I've gotten stronger": each friend's var keeps the ace's level from your
//   last battle; if the ladder is higher now, they open with that line.
// - Name lines: once, at the first meeting, if your name is one they know
//   (HoennFriendNameLine: AIDAN, BRIDGET or BROCK, exact spelling, any case).
#include "global.h"
#include "battle.h"
#include "battle_setup.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/battle.h"
#include "constants/characters.h"
#include "constants/opponents.h"

#if IS_HNS

struct HoennFriend
{
    u16 trainerId;
    u16 levelVar;
};

static const struct HoennFriend sFriends[HOENN_FRIEND_COUNT] =
{
    [HOENN_FRIEND_BRIDGETTE] = {TRAINER_KAYLEE, VAR_HOENN_FRIEND_LV_BRIDGETTE},
};

bool32 HoennIsFriendTrainer(const struct Trainer *trainer)
{
    u32 i;
    for (i = 0; i < HOENN_FRIEND_COUNT; i++)
        if (sFriends[i].trainerId != 0 && GetTrainerStructFromId(sFriends[i].trainerId) == trainer)
            return TRUE;
    return FALSE;
}

// During a battle (and right after it): is it against a friend?
bool32 HoennIsFriendBattle(void)
{
    u32 i;
    if (!(gBattleTypeFlags & BATTLE_TYPE_TRAINER))
        return FALSE;
    for (i = 0; i < HOENN_FRIEND_COUNT; i++)
        if (sFriends[i].trainerId != 0 && TRAINER_BATTLE_PARAM.opponentA == sFriends[i].trainerId)
            return TRUE;
    return FALSE;
}

// specialvar VAR_RESULT, HoennFriendStronger (VAR_0x8004 = HOENN_FRIEND_*):
// TRUE if you've battled them before and their ace would be higher now.
u16 HoennFriendStronger(void)
{
    u32 friend = VarGet(VAR_0x8004);
    u16 last;

    if (friend >= HOENN_FRIEND_COUNT)
        return FALSE;
    last = VarGet(sFriends[friend].levelVar);
    return last != 0 && HoennScaledAce(HOENN_LADDER_TOP_90) > last;
}

// specialvar VAR_RESULT, HoennFriendBattleEnded (VAR_0x8004 = HOENN_FRIEND_*):
// records the level they battled at and returns the outcome (B_OUTCOME_WON,
// B_OUTCOME_FORFEITED for RUN; a loss never gets here, it whites out).
u16 HoennFriendBattleEnded(void)
{
    u32 friend = VarGet(VAR_0x8004);

    if (friend < HOENN_FRIEND_COUNT)
        VarSet(sFriends[friend].levelVar, HoennScaledAce(HOENN_LADDER_TOP_90));
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

// specialvar VAR_RESULT, HoennFriendNameLine: which name you have
// (HOENN_NAME_OTHER, HOENN_NAME_AIDAN, HOENN_NAME_BRIDGET, HOENN_NAME_BROCK).
u16 HoennFriendNameLine(void)
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

#endif // IS_HNS
