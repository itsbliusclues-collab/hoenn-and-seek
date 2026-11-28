// Hoenn & Seek 2.1, lane B (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-b.md).
//
// Each 2.1 lane keeps its C code in its own file (or a new src/hoenn_*.c of its
// own) so the two lanes never edit the same lines. Scripts and texts:
// data/scripts/hoenn_2_1_lane_b.inc. Flags and vars: lane B's section of
// constants/hoenn_events_2_1.h.
//
// - Hoenn21LaneBDueCall(): lane B's PokéGear event calls. The chain is
//   GetDueEventCall (src/hoenn_phone.c) → ... → HoennLaneBDueCall (2.0's last)
//   → Hoenn21LaneADueCall → Hoenn21LaneBDueCall. Add one `if` per call, in the
//   order they should ring; IDs come from lane B's 2.1 range in constants/hoenn_phone.h.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/flags.h"
#include "constants/hoenn_phone.h"

#if IS_HNS

u16 Hoenn21LaneBDueCall(void)
{
    return HOENN_CALL_NONE; // the end of the chain
}

#endif // IS_HNS
