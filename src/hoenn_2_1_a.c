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
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/hoenn_phone.h"

#if IS_HNS

u16 Hoenn21ADueCall(void)
{
    // (track A's calls go here, one `if` each)
    return Hoenn21BDueCall();
}

#endif // IS_HNS
