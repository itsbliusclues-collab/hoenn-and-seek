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
    return Hoenn21LaneBDueCall(); // lane B's 2.1 calls come next
}

#endif // IS_HNS
