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

#if IS_HNS

u16 Hoenn21LaneADueCall(void)
{
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

#endif // IS_HNS
