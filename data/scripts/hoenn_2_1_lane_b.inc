@ Hoenn & Seek 2.1, lane B (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-b.md).
@ Lane B's 2.1 scenes, phone calls and texts go in this file (or a new
@ data/scripts/hoenn_*.inc of its own). C code: src/hoenn_2_1_lane_b.c.

@ ---------------------------------------------------------------- event calls

@ The PokéGear event-call switch falls through 2.0's lanes, then 2.1's lane A,
@ then 2.1's lane B. Add one case per call:
@ 	case HOENN_CALL_X, HoennSeek_EventScript_CallX
HoennSeek_EventScript_PhoneEvent21LaneB::
	switch VAR_0x8004
	releaseall @ no call matched: the end of the chain
	end

@ ---------------------------------------------------------------- 2.1-34: Teleport to your secret base
@ Design: src/hoenn_2_1_lane_b.c. Teleport from the party menu runs this once
@ your own base's entrance is recorded (you've walked into it); VAR_0x8004 is
@ the Pokémon using it, STR_VAR_2 the last healing spot's town.
HoennSeek_EventScript_TeleportChoice::
	lockall
	bufferpartymonnick STR_VAR_1, VAR_0x8004
	message HoennSeek_Text_TeleportWhere
	waitmessage
	dynmultichoice 17, 6, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_TeleportCenter, HoennSeek_Text_TeleportBase, HoennSeek_Text_TeleportCancel
	goto_if_eq VAR_RESULT, HOENN_TELEPORT_TO_CENTER, HoennSeek_EventScript_TeleportGo
	goto_if_eq VAR_RESULT, HOENN_TELEPORT_TO_BASE, HoennSeek_EventScript_TeleportGo
	closemessage
	releaseall
	end

HoennSeek_EventScript_TeleportGo:
	closemessage
	releaseall
	special HoennTeleportGo
	end

HoennSeek_Text_TeleportWhere:
	.string "Where should {STR_VAR_1} TELEPORT to?$"

HoennSeek_Text_TeleportCenter:
	.string "{STR_VAR_2}$"

HoennSeek_Text_TeleportBase:
	.string "SECRET BASE$"

HoennSeek_Text_TeleportCancel:
	.string "CANCEL$"
