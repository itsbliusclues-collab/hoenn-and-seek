@ Hoenn & Seek 2.1, lane B (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-b.md).
@ Lane B's 2.1 scenes, phone calls and texts go in this file (or a new
@ data/scripts/hoenn_*.inc of its own). C code: src/hoenn_2_1_lane_b.c.

@ ---------------------------------------------------------------- event calls

@ The PokéGear event-call switch falls through 2.0's lanes, then 2.1's lane A,
@ then 2.1's lane B. Add one case per call:
@ 	case HOENN_CALL_X, HoennSeek_EventScript_CallX
HoennSeek_EventScript_PhoneEvent21LaneB::
	switch VAR_0x8004
	case HOENN_CALL_WEEKENDS, HoennSeek_EventScript_CallWeekends @ 2.1-18
	case HOENN_CALL_BROCK_CHAIN, HoennSeek_EventScript_CallBrockChain @ 2.1-21
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

@ ---------------------------------------------------------------- 2.1-18 to 20: leaders' weekends
@ Design: src/hoenn_2_1_lane_b.c and tools/hoenn/gen_hoenn_weekends.py. The spot
@ and gym guide scripts are generated from data/hoenn/leader_weekends.json.
HoennSeek_EventScript_CallWeekends::
	setflag FLAG_HOENN_CALL_WEEKENDS
	setspeaker HoennSeek_Text_NameMay
	pokenavcall HoennSeek_Text_CallWeekends
	goto HoennSeek_EventScript_CallEnd

HoennSeek_Text_CallWeekends:
	.string "{PLAYER}! It's MAY! Guess what?\p"
	.string "DAD got a memo from the LEAGUE.\n"
	.string "LEADERS can take weekends off\l"
	.string "once you've earned their BADGE!\p"
	.string "So the LEADERS you've beaten are\n"
	.string "out and about on weekends! DAD\l"
	.string "says to ask their GYM GUIDES.\p"
	.string "I wonder where DAD would go on\n"
	.string "a weekend off. Fishing, I bet!$"

	.include "data/scripts/hoenn_weekends.inc"


@ ---------------------------------------------------------------- 2.1-21: BROCK's weekend chain
@ Design: src/hoenn_fan_chains.c (the weekend rule, where BROCK is, the pool).
@ Four steps, one per weekend: the MUSEUM (his breeder dream), MT. MOON (CLEFAIRY
@ egg shells, a MOON STONE), the ROUTE 34 DAY CARE (the lady's story, a breeding
@ question, an EVERSTONE), the MUSEUM again (the ONIX DOLL). Then his pool lines.
@ BROCK's weekend spot calls ChainSpot_BROCK first (gen_hoenn_weekends.py):
@ VAR_RESULT TRUE = it spoke, so the spot skips its fixed line.

HoennSeek_EventScript_ChainSpot_BROCK::
	goto_if_ge VAR_HOENN_CHAIN_BROCK, HOENN_CHAIN_DONE, HoennSeek_EventScript_ChainBrockPool
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBrockSpotWait
	goto_if_eq VAR_HOENN_CHAIN_BROCK, 0, HoennSeek_EventScript_ChainBrockStep1
	goto_if_eq VAR_HOENN_CHAIN_BROCK, 3, HoennSeek_EventScript_ChainBrockStep4
	setvar VAR_RESULT, FALSE
	return

HoennSeek_EventScript_ChainBrockPool:
	callnative HoennSeek_ChainPoolLine
	.2byte HOENN_CHAIN_BROCK
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBrockReturn
	callstd MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
HoennSeek_EventScript_ChainBrockReturn:
	return

@ At the spot but not ready: only right after step 1 (he's elsewhere for 2 and 3).
HoennSeek_EventScript_ChainBrockSpotWait:
	goto_if_ne VAR_HOENN_CHAIN_BROCK, 1, HoennSeek_EventScript_ChainBrockFalse
	msgbox HoennSeek_Text_ChainBrockWaitMtMoon, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainBrockFalse:
	setvar VAR_RESULT, FALSE
	return

HoennSeek_EventScript_ChainBrockStep1:
	msgbox HoennSeek_Text_ChainBrockStep1, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	special HoennChainStepDone
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainBrockStep4:
	msgbox HoennSeek_Text_ChainBrockStep4, MSGBOX_DEFAULT
	givedecoration DECOR_ONIX_DOLL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBrockNoRoom
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	special HoennChainStepDone
	msgbox HoennSeek_Text_ChainBrockStep4After, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

@ No room in the PC for the doll: the step waits (he's still ready, at the spot).
HoennSeek_EventScript_ChainBrockNoRoom:
	msgbox HoennSeek_Text_ChainBrockNoRoom, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

@ MT. MOON, by the fossil dig: there only while step 2 is ready (stage 1) or
@ for the rest of the weekend he did it (stage 2).
HoennSeek_EventScript_ChainBrockMtMoon::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBrockMtMoonAfter
	msgbox HoennSeek_Text_ChainBrockMtMoon1, MSGBOX_DEFAULT
	closemessage
	playse SE_M_STRENGTH
	setvar VAR_0x8004, 1  @ vertical pan
	setvar VAR_0x8005, 0  @ horizontal pan
	setvar VAR_0x8006, 4  @ number of shakes
	setvar VAR_0x8007, 4  @ shake delay
	special ShakeCamera
	waitstate
	msgbox HoennSeek_Text_ChainBrockMtMoon2, MSGBOX_DEFAULT
	giveitem ITEM_MOON_STONE
	msgbox HoennSeek_Text_ChainBrockMtMoon3, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	special HoennChainStepDone
	release
	end

HoennSeek_EventScript_ChainBrockMtMoonAfter:
	msgbox HoennSeek_Text_ChainBrockWaitDayCare, MSGBOX_DEFAULT
	release
	end

@ The ROUTE 34 DAY CARE: step 3 (stage 2), then the rest of that weekend (stage 3).
HoennSeek_EventScript_ChainBrockDayCare::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBrockDayCareAfter
	msgbox HoennSeek_Text_ChainBrockDayCare1, MSGBOX_DEFAULT
	message HoennSeek_Text_ChainBrockQuiz
	waitmessage
	dynmultichoice 19, 6, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_ChainBrockEverstone, HoennSeek_Text_ChainBrockOranBerry, HoennSeek_Text_ChainBrockMoonStone
	goto_if_eq VAR_RESULT, 0, HoennSeek_EventScript_ChainBrockQuizRight
	msgbox HoennSeek_Text_ChainBrockQuizWrong, MSGBOX_DEFAULT
	goto HoennSeek_EventScript_ChainBrockDayCareGift
HoennSeek_EventScript_ChainBrockQuizRight:
	msgbox HoennSeek_Text_ChainBrockQuizRight, MSGBOX_DEFAULT
HoennSeek_EventScript_ChainBrockDayCareGift:
	giveitem ITEM_EVERSTONE
	msgbox HoennSeek_Text_ChainBrockDayCare2, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_BROCK
	special HoennChainStepDone
	release
	end

HoennSeek_EventScript_ChainBrockDayCareAfter:
	msgbox HoennSeek_Text_ChainBrockWaitMuseum, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_CallBrockChain::
	setflag FLAG_HOENN_CALL_BROCK_CHAIN
	setspeaker HoennSeek_Text_NameBrendan
	pokenavcall HoennSeek_Text_CallBrockChain
	goto HoennSeek_EventScript_CallEnd

HoennSeek_Text_ChainBrockStep1:
	.string "Oh, {PLAYER}. Can I tell\n"
	.string "you something?\p"
	.string "I love being a GYM LEADER. But what\n"
	.string "I really want is to be a POKéMON\l"
	.string "BREEDER. The best one there is.\p"
	.string "Raising ten brothers and sisters\n"
	.string "is good practice, you know.\p"
	.string "Something odd turned up in MT. MOON\n"
	.string "last week. I'm going to look into\l"
	.string "it next weekend.\p"
	.string "Meet me there, by the fossil dig?\n"
	.string "Bring a good eye.$"

HoennSeek_Text_ChainBrockWaitMtMoon:
	.string "Next weekend, MT. MOON, by the\n"
	.string "fossil dig. I'll be waiting!$"

HoennSeek_Text_ChainBrockMtMoon1:
	.string "{PLAYER}, you came! Look at this.\p"
	.string "These are pieces of eggshell.\n"
	.string "CLEFAIRY eggs, and fresh ones.\p"
	.string "Something's under them. Stand back.\n"
	.string "GOLEM, lift that rock!$"

HoennSeek_Text_ChainBrockMtMoon2:
	.string "A MOON STONE! The eggs were laid\n"
	.string "right on top of it.\p"
	.string "I've read that CLEFAIRY eggs only\n"
	.string "hatch near MOON STONES. I never\l"
	.string "believed it. Here, you keep it.$"

HoennSeek_Text_ChainBrockMtMoon3:
	.string "The DAY CARE lady on ROUTE 34 has\n"
	.string "raised every kind of POKéMON there\l"
	.string "is. She'll know about this.\p"
	.string "Meet me at her DAY CARE next\n"
	.string "weekend, all right?$"

HoennSeek_Text_ChainBrockWaitDayCare:
	.string "Next weekend, the DAY CARE on\n"
	.string "ROUTE 34. I'll bring the shells!$"

HoennSeek_Text_ChainBrockDayCare1:
	.string "{PLAYER}! I showed her the shells.\n"
	.string "She knew right away.\p"
	.string "Years ago a HIKER left a CLEFAIRY\n"
	.string "egg here. It wouldn't hatch, not\l"
	.string "for months.\p"
	.string "Then the HIKER came back with a\n"
	.string "MOON STONE in his pack. It hatched\l"
	.string "that very night!\p"
	.string "She asked me a breeding question\n"
	.string "to test me. Let's see how you do.$"

HoennSeek_Text_ChainBrockQuiz:
	.string "Which item, held by a parent, passes\n"
	.string "its NATURE on to the egg?$"

HoennSeek_Text_ChainBrockEverstone:
	.string "EVERSTONE$"

HoennSeek_Text_ChainBrockOranBerry:
	.string "ORAN BERRY$"

HoennSeek_Text_ChainBrockMoonStone:
	.string "MOON STONE$"

HoennSeek_Text_ChainBrockQuizRight:
	.string "The EVERSTONE! That's what I said,\n"
	.string "too. She says we'll both make\l"
	.string "good BREEDERS.$"

HoennSeek_Text_ChainBrockQuizWrong:
	.string "Close! It's the EVERSTONE. Don't\n"
	.string "worry, I had to look it up once.$"

HoennSeek_Text_ChainBrockDayCare2:
	.string "Here, a spare one from my pack.\p"
	.string "Come by the MUSEUM next weekend.\n"
	.string "My family wants to thank you.$"

HoennSeek_Text_ChainBrockWaitMuseum:
	.string "Next weekend, at the MUSEUM. My\n"
	.string "family's working on a surprise!$"

HoennSeek_Text_ChainBrockStep4:
	.string "{PLAYER}! My brothers and sisters\n"
	.string "made this for you. All ten of them.\p"
	.string "It's an ONIX, like my first partner.\n"
	.string "They stitched it themselves.$"

HoennSeek_Text_ChainBrockStep4After:
	.string "It's in your PC. Put it somewhere\n"
	.string "it can see the room.\p"
	.string "Thanks for coming along, {PLAYER}.\n"
	.string "A breeder needs a good friend.$"

HoennSeek_Text_ChainBrockNoRoom:
	.string "Your PC's full of decorations?\n"
	.string "Make some room, and I'll hold on\l"
	.string "to it until then.$"

HoennSeek_Text_CallBrockChain:
	.string "{PLAYER}! It's BRENDAN.\p"
	.string "I went to the PEWTER MUSEUM to\n"
	.string "see the fossils, and BROCK's little\l"
	.string "brother was there.\p"
	.string "He told me you helped BROCK find\n"
	.string "a MOON STONE in MT. MOON!\p"
	.string "He also said the ONIX DOLL took\n"
	.string "them three weeks to stitch.\p"
	.string "I want to meet BROCK. A LEADER\n"
	.string "who wants to be a BREEDER! Cool!$"

	.include "data/scripts/hoenn_fan_chains.inc"
