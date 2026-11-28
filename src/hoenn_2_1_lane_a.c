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

#endif // IS_HNS
