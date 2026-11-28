// Hoenn & Seek 2.1, lane A (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-a.md).
//
// Each 2.1 lane keeps its C code in its own file (or a new src/hoenn_*.c of its
// own) so the two lanes never edit the same lines. Scripts and texts:
// data/scripts/hoenn_2_1_lane_a.inc. Flags and vars: lane A's section of
// constants/hoenn_events_2_1.h.
//
// - Hoenn21LaneADueCall(): lane A's PokéGear event calls. The chain is
//   GetDueEventCall (src/hoenn_phone.c) → ... → HoennLaneBDueCall (2.0's last)
//   → Hoenn21LaneADueCall → Hoenn21LaneBDueCall. Add one `if` per call, in the
//   order they should ring; IDs come from lane A's 2.1 range in constants/hoenn_phone.h.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/flags.h"
#include "constants/hoenn_phone.h"
#include "berry.h"
#include "event_object_movement.h"
#include "item.h"
#include "item_menu.h"
#include "constants/berry.h"
#include "battle.h"
#include "battle_setup.h"
#include "battle_transition.h"
#include "pokemon.h"
#include "field_player_avatar.h"
#include "script.h"
#include "metatile_behavior.h"
#include "fieldmap.h"
#include "string_util.h"
#include "constants/battle.h"
#include "constants/battle_string_ids.h"
#include "constants/trainers.h"

#if IS_HNS

// 2.1-15: ETHAN or LYRA, your childhood friend next door, calls about every
// other BADGE in Johto, Kanto badges 2/4/6, the Johto title and WALLACE (Aidan:
// a call per badge was too many). Only the highest stage reached rings and the
// ones below it are marked done, as Brendan & May's badge calls are
// (src/hoenn_phone.c). At eight Kanto badges there's no call: they decide to
// stay on as PROF. OAK's field aide in a scene in his lab (2.1-16), so Hoenn
// only gets the WALLACE call; their in-person scenes carry the tips.
static bool32 NeighborHasPhone(void)
{
    return FlagGet(FLAG_HOENN_NEIGHBOR_PHONE_ETHAN) || FlagGet(FLAG_HOENN_NEIGHBOR_PHONE_LYRA);
}

static u32 CountFlags(u16 first, u32 count)
{
    u32 i, n = 0;

    for (i = 0; i < count; i++)
        if (FlagGet(first + i))
            n++;
    return n;
}

// Your progress as a stage: 0 = under two Johto badges, 1-4 = 2/4/6/8 Johto
// badges, 5 = Johto champion, 6-8 = 2/4/6 Kanto badges (8 counts as 6), 9 =
// WALLACE. Stage n
// rings HOENN_CALL_NEIGHBOR_BADGE2 + n - 1 and owns FLAG_HOENN_CALL_NEIGHBOR_FIRST + n - 1.
#define NEIGHBOR_STAGE_COUNT (HOENN_CALL_NEIGHBOR_WALLACE - HOENN_CALL_NEIGHBOR_BADGE2 + 1)
STATIC_ASSERT(NEIGHBOR_STAGE_COUNT == 9, NeighborStageCount);
STATIC_ASSERT(NEIGHBOR_STAGE_COUNT == FLAG_HOENN_CALL_NEIGHBOR_LAST - FLAG_HOENN_CALL_NEIGHBOR_FIRST + 1, NeighborStageFlags);

static u32 NeighborStage(void)
{
    u32 kanto;

    if (FlagGet(FLAG_HOENN_STAMP_WALLACE))
        return 9;
    kanto = CountFlags(FLAG_BADGE09_GET, 8);
    if (kanto >= 2)
        return 5 + min(kanto, 7) / 2;
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        return 5;
    return CountFlags(FLAG_BADGE01_GET, 8) / 2;
}

// For scripts (in-person lines): specialvar VAR_RESULT, HoennNeighborStage
u16 HoennNeighborStage(void)
{
    return NeighborStage();
}

static u16 NeighborDueCall(void)
{
    u32 i, stage;

    if (!NeighborHasPhone())
        return HOENN_CALL_NONE;
    stage = NeighborStage();
    if (stage != 0 && !FlagGet(FLAG_HOENN_CALL_NEIGHBOR_FIRST + stage - 1))
    {
        for (i = 0; i + 1 < stage; i++)
            FlagSet(FLAG_HOENN_CALL_NEIGHBOR_FIRST + i);
        return HOENN_CALL_NEIGHBOR_BADGE2 + stage - 1;
    }
    return HOENN_CALL_NONE;
}

// 2.1-15: the Route 29 catching demo. It's Emerald's Wally tutorial battle
// (BATTLE_TYPE_CATCH_TUTORIAL, the Wally controller) with your friend's MARILL
// in the party slot and their name and back pic in place of WALLY's: the
// hooks in battle_controller_wally.c, reshow_battle_screen.c and
// battle_message.c ask HoennCatchTutorial*() below while sNeighborDemo is set.
static EWRAM_DATA bool8 sNeighborDemo = FALSE;
static EWRAM_DATA u8 sNeighborUsedItem[24] = {0};
static EWRAM_DATA u8 sNeighborWhatWill[28] = {0};

extern const u8 HoennSeek_Text_NameEthan[];
extern const u8 HoennSeek_Text_NameLyra[];

static const u8 *NeighborName(void)
{
    return gSaveBlock2Ptr->playerGender == FEMALE ? HoennSeek_Text_NameEthan : HoennSeek_Text_NameLyra;
}

// special HoennLoadNeighborMarill (after SavePlayerParty): MARILL Lv 5 with
// only TACKLE and THICK FAT (Huge Power could knock the wild Pokémon out).
void HoennLoadNeighborMarill(void)
{
    u16 data;

    CreateRandomMon(&gPlayerParty[0], SPECIES_MARILL, 5);
    data = 0;
    SetMonData(&gPlayerParty[0], MON_DATA_ABILITY_NUM, &data);
    data = MOVE_TACKLE;
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE1, &data);
    data = MOVE_NONE;
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE2, &data);
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE3, &data);
    SetMonData(&gPlayerParty[0], MON_DATA_MOVE4, &data);
}

// special HoennStartNeighborDemo: VAR_0x8004 = the wild species (Lv 3).
void HoennStartNeighborDemo(void)
{
    sNeighborDemo = TRUE;
    StringCopy(sNeighborUsedItem, NeighborName());
    StringAppend(sNeighborUsedItem, COMPOUND_STRING(" used {B_LAST_ITEM}!"));
    StringCopy(sNeighborWhatWill, COMPOUND_STRING("What will\n"));
    StringAppend(sNeighborWhatWill, NeighborName());
    StringAppend(sNeighborWhatWill, COMPOUND_STRING(" do?"));
    StartWallyTutorialBattle(); // the battle starts a few frames later, so the foe can be swapped
    CreateMaleMon(&gEnemyParty[0], gSpecialVar_0x8004, 3);
}

// special HoennEndNeighborDemo (before LoadPlayerParty).
void HoennEndNeighborDemo(void)
{
    sNeighborDemo = FALSE;
}

u16 HoennCatchTutorialBackPic(void)
{
    if (!sNeighborDemo)
        return TRAINER_PIC_BACK_WALLY;
    return gSaveBlock2Ptr->playerGender == FEMALE ? TRAINER_PIC_BACK_GOLD_HNS : TRAINER_PIC_BACK_KRIS_HNS;
}

const u8 *HoennCatchTutorialString(u32 stringId, const u8 *str)
{
    if (sNeighborDemo && stringId == STRINGID_WALLYUSEDITEM)
        return sNeighborUsedItem;
    return str;
}

const u8 *HoennCatchTutorialWhatWill(const u8 *str)
{
    return sNeighborDemo ? sNeighborWhatWill : str;
}

u16 Hoenn21LaneADueCall(void)
{
    u16 call = NeighborDueCall();

    if (call != HOENN_CALL_NONE)
        return call;

    // 2.1-06: a region's journal page is all found, and its doll not given yet.
    if (FlagGet(FLAG_HOENN_GOT_SEEKERS_JOURNAL))
    {
        static const u16 sJournalCalls[][3] = {
            {HOENN_JOURNAL_PAGE_JOHTO, FLAG_HOENN_CALL_JOURNAL_JOHTO, FLAG_HOENN_JOURNAL_DOLL_JOHTO},
            {HOENN_JOURNAL_PAGE_KANTO, FLAG_HOENN_CALL_JOURNAL_KANTO, FLAG_HOENN_JOURNAL_DOLL_KANTO},
            {HOENN_JOURNAL_PAGE_HOENN, FLAG_HOENN_CALL_JOURNAL_HOENN, FLAG_HOENN_JOURNAL_DOLL_HOENN},
        };
        u32 i;

        for (i = 0; i < ARRAY_COUNT(sJournalCalls); i++)
            if (!FlagGet(sJournalCalls[i][1]) && !FlagGet(sJournalCalls[i][2])
             && HoennJournalPageComplete(sJournalCalls[i][0]))
                return HOENN_CALL_JOURNAL_JOHTO + i;
    }
    // 2.1-31: Brendan caught DAN's show in PEWTER (Brendan & May train in Kanto).
    if (FlagGet(FLAG_HOENN_BM_PHONE) && !FlagGet(FLAG_HOENN_CALL_DAN) && HoennDanCallDue())
        return HOENN_CALL_DAN;
    return Hoenn21LaneBDueCall(); // lane B's 2.1 calls come next
}

// 2.1-07: plant VAR_ITEM_ID (a BERRY) in the empty soil patch you're talking
// to (HoennSeek_EventScript_BerrySoil). HnS's own planting special replants
// the last picked BERRY instead, so it can't be used for a first planting.
void HoennPlantBerryInSoil(void)
{
    PlantBerryTree(GetObjectEventBerryTreeId(gSelectedObjectEvent), ItemIdToBerryType(gSpecialVar_ItemId), BERRY_STAGE_PLANTED, TRUE);
    RemoveBagItem(gSpecialVar_ItemId, 1);
    ObjectEventInteractionGetBerryTreeData();
}


// 2.1-16: ETHAN or LYRA in person. They're already in a town when you first
// walk in (5 Johto towns, 3 Kanto ones), spot you, give a tip about the place
// and move along; after all 8 Kanto badges they tell PROF. OAK, in his lab,
// that they'll stay on as his field aide (Aidan: a scene, not a call). One
// object per map (graphics VAR_OBJ_GFX_ID_E, flag FLAG_HOENN_HIDE_NEIGHBOR_TOWN),
// decided by HoennNeighborTownArm in the map's OnTransition. The town objects
// are "trainers" whose script isn't a battle, so they walk up to you when you
// cross their sight (the fake-fight path, part 14). Scripts and texts:
// data/scripts/hoenn_neighbor_towns.inc.
extern const u8 HoennSeek_Text_NeighborTownViolet[];
extern const u8 HoennSeek_Text_NeighborTownAzalea[];
extern const u8 HoennSeek_Text_NeighborTownGoldenrod[];
extern const u8 HoennSeek_Text_NeighborTownEcruteak[];
extern const u8 HoennSeek_Text_NeighborTownOlivine[];
extern const u8 HoennSeek_Text_NeighborTownVermilion[];
extern const u8 HoennSeek_Text_NeighborTownCerulean[];
extern const u8 HoennSeek_Text_NeighborTownPewter[];

static const u8 *const sNeighborTownTexts[] = {
    [HOENN_NEIGHBOR_TOWN_VIOLET]    = HoennSeek_Text_NeighborTownViolet,
    [HOENN_NEIGHBOR_TOWN_AZALEA]    = HoennSeek_Text_NeighborTownAzalea,
    [HOENN_NEIGHBOR_TOWN_GOLDENROD] = HoennSeek_Text_NeighborTownGoldenrod,
    [HOENN_NEIGHBOR_TOWN_ECRUTEAK]  = HoennSeek_Text_NeighborTownEcruteak,
    [HOENN_NEIGHBOR_TOWN_OLIVINE]   = HoennSeek_Text_NeighborTownOlivine,
    [HOENN_NEIGHBOR_TOWN_VERMILION] = HoennSeek_Text_NeighborTownVermilion,
    [HOENN_NEIGHBOR_TOWN_CERULEAN]  = HoennSeek_Text_NeighborTownCerulean,
    [HOENN_NEIGHBOR_TOWN_PEWTER]    = HoennSeek_Text_NeighborTownPewter,
};
STATIC_ASSERT(HOENN_NEIGHBOR_TOWN_COUNT == FLAG_HOENN_NEIGHBOR_TOWN_LAST - FLAG_HOENN_NEIGHBOR_TOWN_FIRST + 1, NeighborTownFlags);
STATIC_ASSERT(ARRAY_COUNT(sNeighborTownTexts) == HOENN_NEIGHBOR_TOWN_OAKS_LAB, NeighborTownTexts);

// The Johto badge count each town's scene must still be under.
static const u8 sNeighborTownBadgesBelow[] = {
    [HOENN_NEIGHBOR_TOWN_VIOLET] = 1, [HOENN_NEIGHBOR_TOWN_AZALEA] = 2,
    [HOENN_NEIGHBOR_TOWN_GOLDENROD] = 3, [HOENN_NEIGHBOR_TOWN_ECRUTEAK] = 4,
    [HOENN_NEIGHBOR_TOWN_OLIVINE] = 6,
};

static bool32 NeighborTownOut(u32 town)
{
    u32 kanto = CountFlags(FLAG_BADGE09_GET, 8);

    if (VarGet(VAR_HOENN_NEIGHBOR_STATE) < HOENN_NEIGHBOR_PHONE)
        return FALSE;
    if (town == HOENN_NEIGHBOR_TOWN_OAKS_LAB)
        return kanto == 8 || FlagGet(FLAG_HOENN_NEIGHBOR_STAYED); // they work there afterwards
    if (FlagGet(FLAG_HOENN_NEIGHBOR_TOWN_FIRST + town))
        return FALSE;
    if (town < ARRAY_COUNT(sNeighborTownBadgesBelow))
        return CountFlags(FLAG_BADGE01_GET, 8) < sNeighborTownBadgesBelow[town];
    return FlagGet(FLAG_SYS_GAME_CLEAR) && kanto < 8;
}

// specialvar VAR_RESULT, HoennNeighborTownArm (VAR_0x8004 = town): shows or
// hides their object; TRUE if it's out.
u16 HoennNeighborTownArm(void)
{
    if (gSpecialVar_0x8004 < HOENN_NEIGHBOR_TOWN_COUNT && NeighborTownOut(gSpecialVar_0x8004))
    {
        FlagClear(FLAG_HOENN_HIDE_NEIGHBOR_TOWN);
        return TRUE;
    }
    FlagSet(FLAG_HOENN_HIDE_NEIGHBOR_TOWN);
    return FALSE;
}

// callnative HoennSeek_NeighborTownText: data[0] = town VAR_0x8004's line (for
// callstd MSGBOX_*), and marks that scene done.
void HoennSeek_NeighborTownText(struct ScriptContext *ctx)
{
    u32 town = gSpecialVar_0x8004;

    if (town >= ARRAY_COUNT(sNeighborTownTexts))
        town = 0;
    ctx->data[0] = (u32)sNeighborTownTexts[town];
    FlagSet(FLAG_HOENN_NEIGHBOR_TOWN_FIRST + town);
}

static bool32 NeighborOpenLand(s32 x, s32 y)
{
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];

    if (MapGridGetCollisionAt(x, y))
        return FALSE;
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y)))
        return FALSE;
    if (x == player->currentCoords.x && y == player->currentCoords.y)
        return FALSE;
    return GetObjectEventIdByXY(x, y) == OBJECT_EVENTS_COUNT;
}

// specialvar VAR_RESULT, HoennNeighborLeaveDir: the way the object you just
// talked to (or that walked up to you) moves along: 4 tiles of open land, away
// from you first, then to the sides, then toward you. DIR_NONE if there's none.
u16 HoennNeighborLeaveDir(void)
{
    static const s8 sDeltas[][2] = {
        [DIR_SOUTH] = {0, 1}, [DIR_NORTH] = {0, -1}, [DIR_WEST] = {-1, 0}, [DIR_EAST] = {1, 0},
    };
    struct ObjectEvent *obj = &gObjectEvents[gSelectedObjectEvent];
    struct ObjectEvent *player = &gObjectEvents[gPlayerAvatar.objectEventId];
    u8 order[4];
    s32 x = obj->currentCoords.x, y = obj->currentCoords.y;
    u32 i, k, away;

    if (x < player->currentCoords.x)
        away = DIR_WEST;
    else if (x > player->currentCoords.x)
        away = DIR_EAST;
    else if (y < player->currentCoords.y)
        away = DIR_NORTH;
    else
        away = DIR_SOUTH;
    order[0] = away;
    order[1] = (away == DIR_NORTH || away == DIR_SOUTH) ? DIR_WEST : DIR_NORTH;
    order[2] = GetOppositeDirection(order[1]);
    order[3] = GetOppositeDirection(away);
    for (i = 0; i < ARRAY_COUNT(order); i++)
    {
        for (k = 1; k <= 4; k++)
            if (!NeighborOpenLand(x + sDeltas[order[i]][0] * k, y + sDeltas[order[i]][1] * k))
                break;
        if (k > 4)
            return order[i];
    }
    return DIR_NONE;
}

#endif // IS_HNS
