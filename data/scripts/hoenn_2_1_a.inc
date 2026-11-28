@ Hoenn & Seek 2.1, track A: systems and trainers (parts 2.1-02, 04-17, 34).
@ Track A's scenes, phone calls and texts go in this file (or a new
@ data/scripts/hoenn_*.inc of its own); track B has its own file, so the two
@ never edit the same lines. Rules: hoenn-docs/tracks/README.md. C code:
@ src/hoenn_2_1_a.c. Flags and vars: constants/hoenn_events_2_1.h.

@ ---------------------------------------------------------------- event calls

@ The PokéGear event-call switch (HoennSeek_EventScript_PhoneEvent,
@ hoenn_phone.inc) falls through 2.0's lane A and lane B cases to track A's,
@ then to track B's. Add one case per call, above the goto:
@ 	case HOENN_CALL_X, HoennSeek_EventScript_CallX
HoennSeek_EventScript_PhoneEvent21A::
	@ (track A's cases go here)
	goto HoennSeek_EventScript_PhoneEvent21B @ track B's calls (hoenn_2_1_b.inc)

@ (track A's parts add their sections below, "@ ==== 2.1-NN: ... ====")
