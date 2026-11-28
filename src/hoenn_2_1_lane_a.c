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

#if IS_HNS

// 2.1-15: ETHAN or LYRA calls at Johto milestones once you have their number.
// Only the highest milestone reached rings; the ones below it are marked done,
// as Brendan & May's badge calls are (src/hoenn_phone.c).
static u16 NeighborDueCall(void)
{
    static const u16 sCalls[][2] = { // badges needed (9 = Johto champion), call flag; highest first
        {9, FLAG_HOENN_CALL_NEIGHBOR_CHAMPION},
        {8, FLAG_HOENN_CALL_NEIGHBOR_BADGE8},
        {7, FLAG_HOENN_CALL_NEIGHBOR_BADGE7},
        {4, FLAG_HOENN_CALL_NEIGHBOR_BADGE4},
        {3, FLAG_HOENN_CALL_NEIGHBOR_BADGE3},
        {1, FLAG_HOENN_CALL_NEIGHBOR_BADGE1},
    };
    u32 i, j, reached = 0;

    if (!FlagGet(FLAG_HOENN_NEIGHBOR_PHONE_ETHAN) && !FlagGet(FLAG_HOENN_NEIGHBOR_PHONE_LYRA))
        return HOENN_CALL_NONE;
    for (i = 0; i < 8; i++)
        if (FlagGet(FLAG_BADGE01_GET + i))
            reached++;
    if (FlagGet(FLAG_SYS_GAME_CLEAR))
        reached = 9;
    for (i = 0; i < ARRAY_COUNT(sCalls); i++)
    {
        if (reached < sCalls[i][0])
            continue;
        if (FlagGet(sCalls[i][1]))
            return HOENN_CALL_NONE; // the highest reached already rang
        for (j = i + 1; j < ARRAY_COUNT(sCalls); j++)
            FlagSet(sCalls[j][1]);
        return HOENN_CALL_NEIGHBOR_CHAMPION - i;
    }
    return HOENN_CALL_NONE;
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
