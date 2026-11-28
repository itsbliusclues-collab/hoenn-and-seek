@ Hoenn & Seek 2.1, lane A (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-a.md).
@ Lane A's 2.1 scenes, phone calls and texts go in this file (or a new
@ data/scripts/hoenn_*.inc of its own). C code: src/hoenn_2_1_lane_a.c.

@ ---------------------------------------------------------------- event calls

@ The PokéGear event-call switch falls through 2.0's lanes, then 2.1's lane A,
@ then 2.1's lane B. Add one case per call:
@ 	case HOENN_CALL_X, HoennSeek_EventScript_CallX
HoennSeek_EventScript_PhoneEvent21LaneA::
	switch VAR_0x8004
	goto HoennSeek_EventScript_PhoneEvent21LaneB @ lane B's 2.1 calls (hoenn_2_1_lane_b.inc)
