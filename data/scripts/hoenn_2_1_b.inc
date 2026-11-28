@ Hoenn & Seek 2.1, track B: the world and its people (parts 2.1-01, 03, 18-33).
@ Track B's scenes, phone calls and texts go in this file (or a new
@ data/scripts/hoenn_*.inc of its own); track A has its own file, so the two
@ never edit the same lines. Rules: hoenn-docs/tracks/README.md. C code:
@ src/hoenn_2_1_b.c. Flags and vars: constants/hoenn_events_2_1.h.

@ ---------------------------------------------------------------- event calls

@ The end of the PokéGear event-call switch (see hoenn_2_1_a.inc). Add one
@ case per call, above the releaseall:
@ 	case HOENN_CALL_X, HoennSeek_EventScript_CallX
HoennSeek_EventScript_PhoneEvent21B::
	@ (track B's cases go here)
	releaseall @ no call matched: the end of the chain
	end

@ (track B's parts add their sections below, "@ ==== 2.1-NN: ... ====")
