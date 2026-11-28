// Hoenn & Seek 2.1, track B: the world and its people (parts 2.1-01, 03,
// 18-33).
//
// 2.1 is built by two Claude builders in parallel (hoenn-docs/2.1-tracks.md
// has the plan, hoenn-docs/tracks/ the rules and each track's status). Each
// track keeps its C code in its own file so the two never edit the same lines:
// track B's goes here (or in a file of its own for a big system: Anamaria's
// box sorting, the people's Pokémon), track A's in src/hoenn_2_1_a.c. Scripts
// and texts: data/scripts/hoenn_2_1_b.inc. Flags and vars: track B's section
// of constants/hoenn_events_2_1.h. Declarations: track B's section of
// include/hoenn_2_1.h.
//
// - Hoenn21BDueCall(): track B's PokéGear event calls, the end of the chain
//   (see src/hoenn_2_1_a.c). Add one `if` per call, in the order they should
//   ring; IDs come from track B's range (130-159) in constants/hoenn_phone.h,
//   and each call's script case goes in HoennSeek_EventScript_PhoneEvent21B
//   (data/scripts/hoenn_2_1_b.inc).
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/hoenn_phone.h"

#if IS_HNS

u16 Hoenn21BDueCall(void)
{
    // (track B's calls go here, one `if` each)
    return HOENN_CALL_NONE; // the end of the chain
}

#endif // IS_HNS
