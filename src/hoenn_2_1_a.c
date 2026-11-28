// Hoenn & Seek 2.1, track A: systems and trainers (parts 2.1-02, 04-17, 34).
//
// 2.1 is built by two Claude builders in parallel (hoenn-docs/2.1-tracks.md
// has the plan, hoenn-docs/tracks/ the rules and each track's status). Each
// track keeps its C code in its own file so the two never edit the same lines:
// track A's goes here (or in a file of its own for a big system: the Seeker's
// Journal, the swarms), track B's in src/hoenn_2_1_b.c. Scripts and texts:
// data/scripts/hoenn_2_1_a.inc. Flags and vars: track A's section of
// constants/hoenn_events_2_1.h. Declarations: track A's section of
// include/hoenn_2_1.h.
//
// - Hoenn21ADueCall(): track A's PokéGear event calls. The chain is
//   GetDueEventCall (src/hoenn_phone.c) → HoennSpeciesDueCall →
//   HoennHiddenDueCall → HoennLaneADueCall → HoennLaneBDueCall (2.0) →
//   Hoenn21ADueCall → Hoenn21BDueCall. Add one `if` per call, in the order
//   they should ring; IDs come from track A's range (100-129) in
//   constants/hoenn_phone.h, and each call's script case goes in
//   HoennSeek_EventScript_PhoneEvent21A (data/scripts/hoenn_2_1_a.inc).
// - HoennMoveKeyItemsToSaveBlock3() (2.1-04): the key-item pocket. 2.0 had 60
//   key-item slots and could hand out about 60 different key items, so 2.1's
//   new ones wouldn't fit (the audit is hoenn-docs/key-items.md). SaveBlock1
//   is nearly full and moving struct Bag would shift everything after it, so
//   the pocket is now 80 slots at the end of SaveBlock3 (hoennKeyItems;
//   SetBagItemsPointers in src/item.c points the pocket there). Save version 8
//   runs this once on an older save: the slots are copied as they are
//   (quantities stay XORed with the same encryption key) and SaveBlock1's old
//   slots are zeroed; they're unused from then on.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/hoenn_phone.h"
#include "berry.h"
#include "event_object_movement.h"
#include "item.h"
#include "item_menu.h"
#include "constants/berry.h"

#if IS_HNS

u16 Hoenn21ADueCall(void)
{
    // (track A's calls go here, one `if` each)
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
    return Hoenn21BDueCall();
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

STATIC_ASSERT(BAG_KEYITEMS_COUNT >= BAG_KEYITEMS_SB1_COUNT, KeyPocketGrows);

void HoennMoveKeyItemsToSaveBlock3(void)
{
    u32 i;

    for (i = 0; i < BAG_KEYITEMS_COUNT; i++)
    {
        if (i < BAG_KEYITEMS_SB1_COUNT)
            gSaveBlock3Ptr->hoennKeyItems[i] = gSaveBlock1Ptr->bag.keyItems[i];
        else
            gSaveBlock3Ptr->hoennKeyItems[i] = (struct ItemSlot){0};
    }
    for (i = 0; i < BAG_KEYITEMS_SB1_COUNT; i++)
        gSaveBlock1Ptr->bag.keyItems[i] = (struct ItemSlot){0};
}

#endif // IS_HNS
