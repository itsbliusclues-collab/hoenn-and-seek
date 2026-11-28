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
	case HOENN_CALL_MISTY_CHAIN, HoennSeek_EventScript_CallMistyChain @ 2.1-22
	case HOENN_CALL_BLUE_CHAIN, HoennSeek_EventScript_CallBlueChain @ 2.1-23
	case HOENN_CALL_RYAN, HoennSeek_EventScript_CallRyan @ 2.1-30
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
	.string "ONIX, move that rock!$"

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

@ ---------------------------------------------------------------- 2.1-22: MISTY's weekend chain
@ Same engine as BROCK's (src/hoenn_fan_chains.c). Four weekends: her ROUTE 20
@ beach (a new finale for the CERULEAN GYM's water show), SEAFOAM 1F's entrance
@ hall (the current that loops under the islands; MYSTIC WATER), a rehearsal in
@ her gym's pool (you pick the finale; a WATER STONE), the beach again (the show
@ went well: the STARMIE DOLL her sisters sewed). Then her pool lines.

HoennSeek_EventScript_ChainSpot_MISTY::
	goto_if_ge VAR_HOENN_CHAIN_MISTY, HOENN_CHAIN_DONE, HoennSeek_EventScript_ChainMistyPool
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainMistySpotWait
	goto_if_eq VAR_HOENN_CHAIN_MISTY, 0, HoennSeek_EventScript_ChainMistyStep1
	goto_if_eq VAR_HOENN_CHAIN_MISTY, 3, HoennSeek_EventScript_ChainMistyStep4
	setvar VAR_RESULT, FALSE
	return

HoennSeek_EventScript_ChainMistyPool:
	callnative HoennSeek_ChainPoolLine
	.2byte HOENN_CHAIN_MISTY
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainMistyReturn
	callstd MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
HoennSeek_EventScript_ChainMistyReturn:
	return

HoennSeek_EventScript_ChainMistySpotWait:
	goto_if_ne VAR_HOENN_CHAIN_MISTY, 1, HoennSeek_EventScript_ChainBrockFalse
	msgbox HoennSeek_Text_ChainMistyWaitSeafoam, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainMistyStep1:
	msgbox HoennSeek_Text_ChainMistyStep1, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	special HoennChainStepDone
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainMistyStep4:
	msgbox HoennSeek_Text_ChainMistyStep4, MSGBOX_DEFAULT
	givedecoration DECOR_STARMIE_DOLL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainMistyNoRoom
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	special HoennChainStepDone
	msgbox HoennSeek_Text_ChainMistyStep4After, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainMistyNoRoom:
	msgbox HoennSeek_Text_ChainMistyNoRoom, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

@ SEAFOAM 1F, the hall inside ROUTE 20's west entrance: step 2 (stage 1), then
@ the rest of that weekend (stage 2).
HoennSeek_EventScript_ChainMistySeafoam::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainMistySeafoamAfter
	msgbox HoennSeek_Text_ChainMistySeafoam1, MSGBOX_DEFAULT
	closemessage
	playse SE_M_WATERFALL
	waitse
	msgbox HoennSeek_Text_ChainMistySeafoam2, MSGBOX_DEFAULT
	giveitem ITEM_MYSTIC_WATER
	msgbox HoennSeek_Text_ChainMistySeafoam3, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	special HoennChainStepDone
	release
	end

HoennSeek_EventScript_ChainMistySeafoamAfter:
	msgbox HoennSeek_Text_ChainMistyWaitGym, MSGBOX_DEFAULT
	release
	end

@ CERULEAN GYM's pool, closed for the weekend: step 3 (stage 2), then the rest
@ of that weekend (stage 3).
HoennSeek_EventScript_ChainMistyGym::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainMistyGymAfter
	msgbox HoennSeek_Text_ChainMistyGym1, MSGBOX_DEFAULT
	message HoennSeek_Text_ChainMistyFinale
	waitmessage
	dynmultichoice 17, 6, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_ChainMistySpin, HoennSeek_Text_ChainMistyRain, HoennSeek_Text_ChainMistyPsyduck
	goto_if_eq VAR_RESULT, 1, HoennSeek_EventScript_ChainMistyRain
	goto_if_eq VAR_RESULT, 2, HoennSeek_EventScript_ChainMistyPsyduck
	msgbox HoennSeek_Text_ChainMistySpinReply, MSGBOX_DEFAULT
	goto HoennSeek_EventScript_ChainMistyGymGift
HoennSeek_EventScript_ChainMistyRain:
	msgbox HoennSeek_Text_ChainMistyRainReply, MSGBOX_DEFAULT
	goto HoennSeek_EventScript_ChainMistyGymGift
HoennSeek_EventScript_ChainMistyPsyduck:
	msgbox HoennSeek_Text_ChainMistyPsyduckReply, MSGBOX_DEFAULT
HoennSeek_EventScript_ChainMistyGymGift:
	msgbox HoennSeek_Text_ChainMistyGym2, MSGBOX_DEFAULT
	giveitem ITEM_WATER_STONE
	msgbox HoennSeek_Text_ChainMistyGym3, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_MISTY
	special HoennChainStepDone
	release
	end

HoennSeek_EventScript_ChainMistyGymAfter:
	msgbox HoennSeek_Text_ChainMistyWaitShow, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_CallMistyChain::
	setflag FLAG_HOENN_CALL_MISTY_CHAIN
	setspeaker HoennSeek_Text_NameMay
	pokenavcall HoennSeek_Text_CallMistyChain
	goto HoennSeek_EventScript_CallEnd

HoennSeek_Text_ChainMistyStep1:
	.string "Oh, it's you! Good, I need a fresh\n"
	.string "pair of eyes.\p"
	.string "Every summer my GYM puts on a\n"
	.string "water show, and I want a new\l"
	.string "finale this year.\p"
	.string "There's a current under these\n"
	.string "islands that goes round and round\l"
	.string "and never stops. That's my idea.\p"
	.string "Come and see it with me next\n"
	.string "weekend. It's just inside the\l"
	.string "SEAFOAM ISLANDS, west of here.$"

HoennSeek_Text_ChainMistyWaitSeafoam:
	.string "Next weekend, inside the SEAFOAM\n"
	.string "ISLANDS. Don't be late!$"

HoennSeek_Text_ChainMistySeafoam1:
	.string "You made it! Shh, listen.$"

HoennSeek_Text_ChainMistySeafoam2:
	.string "Hear that? The water under these\n"
	.string "islands loops all the way around\l"
	.string "and comes back. Every single time.\p"
	.string "STARMIE spinning like that current.\n"
	.string "That's the finale!\p"
	.string "Here, I found this on the sea floor\n"
	.string "while I was swimming. You keep it.$"

HoennSeek_Text_ChainMistySeafoam3:
	.string "Next weekend, I'm rehearsing at my\n"
	.string "GYM. It's closed on weekends, so\l"
	.string "the pool's all mine.\p"
	.string "You can be my first audience!$"

HoennSeek_Text_ChainMistyWaitGym:
	.string "Next weekend, at the CERULEAN GYM.\n"
	.string "I'll save you the best seat!$"

HoennSeek_Text_ChainMistyGym1:
	.string "Welcome to the rehearsal! The\n"
	.string "current spin works. STARMIE nailed\l"
	.string "it on the first try.\p"
	.string "But I can't decide how the show\n"
	.string "should end. You pick!$"

HoennSeek_Text_ChainMistyFinale:
	.string "What should the finale be?$"

HoennSeek_Text_ChainMistySpin:
	.string "STARMIE's spin$"

HoennSeek_Text_ChainMistyRain:
	.string "A RAIN DANCE$"

HoennSeek_Text_ChainMistyPsyduck:
	.string "PSYDUCK$"

HoennSeek_Text_ChainMistySpinReply:
	.string "End on the spin itself? Simple and\n"
	.string "strong. I like the way you think.$"

HoennSeek_Text_ChainMistyRainReply:
	.string "Rain over the whole crowd! Everyone\n"
	.string "goes home soaked. Perfect!$"

HoennSeek_Text_ChainMistyPsyduckReply:
	.string "PSYDUCK? It can barely swim!\n"
	.string "…Then again, people love it.\l"
	.string "Fine. PSYDUCK closes the show.$"

HoennSeek_Text_ChainMistyGym2:
	.string "Thanks for watching. Here, a little\n"
	.string "something for my first audience.$"

HoennSeek_Text_ChainMistyGym3:
	.string "The show is next weekend. Come and\n"
	.string "find me at my beach afterwards!$"

HoennSeek_Text_ChainMistyWaitShow:
	.string "The show's next weekend! Come and\n"
	.string "find me at my beach afterwards.$"

HoennSeek_Text_ChainMistyStep4:
	.string "There you are! The show was a big\n"
	.string "hit. They want it again next year!\p"
	.string "My sisters sewed this for the show.\n"
	.string "They said you should have it.$"

HoennSeek_Text_ChainMistyStep4After:
	.string "It's in your PC. Don't let it\n"
	.string "get wet!\p"
	.string "Thanks, {PLAYER}. Maybe you're not\n"
	.string "so bad after all.$"

HoennSeek_Text_ChainMistyNoRoom:
	.string "Your PC's too full for a doll?\n"
	.string "Make some room. I'll keep it dry\l"
	.string "for you until then.$"

HoennSeek_Text_CallMistyChain:
	.string "{PLAYER}! It's MAY!\p"
	.string "I just watched the CERULEAN GYM's\n"
	.string "water show on TV! STARMIE spun\l"
	.string "like a whirlpool!\p"
	.string "And MISTY thanked you on air!\n"
	.string "She said you helped her plan it.\p"
	.string "You have to introduce me. I want\n"
	.string "to put on a show like that in a\l"
	.string "CONTEST someday!$"

@ ---------------------------------------------------------------- 2.1-23: BLUE's weekend chain
@ Same engine as BROCK's (src/hoenn_fan_chains.c). Four weekends: ROUTE 22 (his
@ grandpa and sister say he never visits; he'll go if you come), DAISY's house
@ in PALLET (her stories about him; a SOOTHE BELL), OAK's lab (OAK's question
@ about great trainers; a RARE CANDY), ROUTE 22 again (DAISY's PIDGEOT DOLL).

HoennSeek_EventScript_ChainSpot_BLUE::
	goto_if_ge VAR_HOENN_CHAIN_BLUE, HOENN_CHAIN_DONE, HoennSeek_EventScript_ChainBluePool
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBlueSpotWait
	goto_if_eq VAR_HOENN_CHAIN_BLUE, 0, HoennSeek_EventScript_ChainBlueStep1
	goto_if_eq VAR_HOENN_CHAIN_BLUE, 3, HoennSeek_EventScript_ChainBlueStep4
	setvar VAR_RESULT, FALSE
	return

HoennSeek_EventScript_ChainBluePool:
	callnative HoennSeek_ChainPoolLine
	.2byte HOENN_CHAIN_BLUE
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBlueReturn
	callstd MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
HoennSeek_EventScript_ChainBlueReturn:
	return

HoennSeek_EventScript_ChainBlueSpotWait:
	goto_if_ne VAR_HOENN_CHAIN_BLUE, 1, HoennSeek_EventScript_ChainBrockFalse
	msgbox HoennSeek_Text_ChainBlueWaitDaisy, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainBlueStep1:
	msgbox HoennSeek_Text_ChainBlueStep1, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	special HoennChainStepDone
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainBlueStep4:
	msgbox HoennSeek_Text_ChainBlueStep4, MSGBOX_DEFAULT
	givedecoration DECOR_PIDGEOT_DOLL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBlueNoRoom
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	special HoennChainStepDone
	msgbox HoennSeek_Text_ChainBlueStep4After, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

HoennSeek_EventScript_ChainBlueNoRoom:
	msgbox HoennSeek_Text_ChainBlueNoRoom, MSGBOX_DEFAULT
	setvar VAR_RESULT, TRUE
	return

@ DAISY's house in PALLET TOWN: step 2 (stage 1), then the rest of that weekend.
HoennSeek_EventScript_ChainBlueDaisy::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBlueDaisyAfter
	msgbox HoennSeek_Text_ChainBlueDaisy1, MSGBOX_DEFAULT
	giveitem ITEM_SOOTHE_BELL
	msgbox HoennSeek_Text_ChainBlueDaisy2, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	special HoennChainStepDone
	release
	end

HoennSeek_EventScript_ChainBlueDaisyAfter:
	msgbox HoennSeek_Text_ChainBlueWaitLab, MSGBOX_DEFAULT
	release
	end

@ OAK's lab: step 3 (stage 2), then the rest of that weekend.
HoennSeek_EventScript_ChainBlueLab::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	specialvar VAR_RESULT, HoennChainStepReady
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ChainBlueLabAfter
	msgbox HoennSeek_Text_ChainBlueLab1, MSGBOX_DEFAULT
	message HoennSeek_Text_ChainBlueQuestion
	waitmessage
	dynmultichoice 18, 6, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_ChainBluePower, HoennSeek_Text_ChainBlueFriends, HoennSeek_Text_ChainBlueRivals
	goto_if_eq VAR_RESULT, 1, HoennSeek_EventScript_ChainBlueFriends
	goto_if_eq VAR_RESULT, 2, HoennSeek_EventScript_ChainBlueRivals
	msgbox HoennSeek_Text_ChainBluePowerReply, MSGBOX_DEFAULT
	goto HoennSeek_EventScript_ChainBlueLabGift
HoennSeek_EventScript_ChainBlueFriends:
	msgbox HoennSeek_Text_ChainBlueFriendsReply, MSGBOX_DEFAULT
	goto HoennSeek_EventScript_ChainBlueLabGift
HoennSeek_EventScript_ChainBlueRivals:
	msgbox HoennSeek_Text_ChainBlueRivalsReply, MSGBOX_DEFAULT
HoennSeek_EventScript_ChainBlueLabGift:
	msgbox HoennSeek_Text_ChainBlueLab2, MSGBOX_DEFAULT
	giveitem ITEM_RARE_CANDY
	msgbox HoennSeek_Text_ChainBlueLab3, MSGBOX_DEFAULT
	setvar VAR_0x8004, HOENN_CHAIN_BLUE
	special HoennChainStepDone
	release
	end

HoennSeek_EventScript_ChainBlueLabAfter:
	msgbox HoennSeek_Text_ChainBlueWaitRoute22, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_CallBlueChain::
	setflag FLAG_HOENN_CALL_BLUE_CHAIN
	setspeaker HoennSeek_Text_NameBrendan
	pokenavcall HoennSeek_Text_CallBlueChain
	goto HoennSeek_EventScript_CallEnd

HoennSeek_Text_ChainBlueStep1:
	.string "Hey. You got a minute?\p"
	.string "Gramps says I never visit. DAISY\n"
	.string "says it too. Every time she calls.\p"
	.string "Fine. I'll go home next weekend.\p"
	.string "…Come with me? If you're there,\n"
	.string "they'll ask you questions instead\l"
	.string "of me. It's a good plan.\p"
	.string "DAISY's house, in PALLET TOWN.\n"
	.string "Don't make me wait!$"

HoennSeek_Text_ChainBlueWaitDaisy:
	.string "Next weekend, DAISY's house in\n"
	.string "PALLET TOWN. Don't bail on me.$"

HoennSeek_Text_ChainBlueDaisy1:
	.string "You came. Good. DAISY's been telling\n"
	.string "stories since I walked in.\p"
	.string "The one where my PIDGEY flew off\n"
	.string "and I cried for a whole day? Not\l"
	.string "true. It was half a day.\p"
	.string "She wants you to have this. She\n"
	.string "says it's for putting up with me.$"

HoennSeek_Text_ChainBlueDaisy2:
	.string "Gramps wants to see my team at\n"
	.string "the lab next weekend.\p"
	.string "You're coming to that too. That's\n"
	.string "not a question.$"

HoennSeek_Text_ChainBlueWaitLab:
	.string "Next weekend, Gramps's lab. Don't\n"
	.string "be late, he notices.$"

HoennSeek_Text_ChainBlueLab1:
	.string "Gramps looked over my whole team.\n"
	.string "He said they're in great shape.\p"
	.string "Then he asked me something I\n"
	.string "couldn't answer. So I'll ask you.$"

HoennSeek_Text_ChainBlueQuestion:
	.string "What makes a great trainer?$"

HoennSeek_Text_ChainBluePower:
	.string "Strong POKéMON$"

HoennSeek_Text_ChainBlueFriends:
	.string "Caring for them$"

HoennSeek_Text_ChainBlueRivals:
	.string "A good rival$"

HoennSeek_Text_ChainBluePowerReply:
	.string "Strong POKéMON. That's what I\n"
	.string "said when I was ten. Gramps just\l"
	.string "laughed. Now I get why.$"

HoennSeek_Text_ChainBlueFriendsReply:
	.string "Caring for them… That's what\n"
	.string "Gramps told me, way back when I\l"
	.string "lost to RED. I didn't listen then.$"

HoennSeek_Text_ChainBlueRivalsReply:
	.string "A good rival, huh? Ha! Then I'm\n"
	.string "a great trainer, because I've\l"
	.string "got the best one.$"

HoennSeek_Text_ChainBlueLab2:
	.string "Gramps said to give you this. He\n"
	.string "says you're a good influence.$"

HoennSeek_Text_ChainBlueLab3:
	.string "I'm back on ROUTE 22 next weekend.\n"
	.string "DAISY said she's got something\l"
	.string "for you. Come and get it.$"

HoennSeek_Text_ChainBlueWaitRoute22:
	.string "Next weekend, ROUTE 22. DAISY's\n"
	.string "sending something for you.$"

HoennSeek_Text_ChainBlueStep4:
	.string "There you are. DAISY made this\n"
	.string "for you. She made me carry it all\l"
	.string "the way here.\p"
	.string "It's a PIDGEOT. Like mine. Not as\n"
	.string "cool as mine, obviously.$"

HoennSeek_Text_ChainBlueStep4After:
	.string "It's in your PC.\p"
	.string "…Hey. Thanks for coming home with\n"
	.string "me. Don't make it weird. Smell ya\l"
	.string "later, {PLAYER}!$"

HoennSeek_Text_ChainBlueNoRoom:
	.string "Your PC's full? Of course it is.\n"
	.string "Clear some space. I'll hold on to\l"
	.string "it, but only because DAISY said so.$"

HoennSeek_Text_CallBlueChain:
	.string "{PLAYER}! It's BRENDAN.\p"
	.string "I went to challenge BLUE at the\n"
	.string "VIRIDIAN GYM, and he spent the\l"
	.string "whole time bragging about you!\p"
	.string "He said you went home to PALLET\n"
	.string "TOWN with him to see PROF. OAK.\p"
	.string "Then he said, “Don't tell anyone\n"
	.string "I said that.” So, uh, I didn't!\l"
	.string "Except you. Bye!$"

	.include "data/scripts/hoenn_fan_chains.inc"


@ ---------------------------------------------------------------- 2.1-30: RYAN and THE LIBRARIAN
@ Design: src/hoenn_ryan.c. RYAN runs a different BATTLE FRONTIER facility each
@ weekday and stands in its lobby, talking to everyone watching at home; THE
@ LIBRARIAN stands near him, catalogues every word and translates his Mandarin.
@ Every line of both characters is here, in one place (drafts until the
@ dialogue pass with Bryan, 2.1-36c). Pinyin uses the toned vowels of
@ tools/hoenn/gen_hoenn_pinyin_glyphs.py.

HoennSeek_EventScript_Ryan::
	lock
	faceplayer
	goto_if_unset FLAG_HOENN_RYAN_MET, HoennSeek_EventScript_RyanFirst
	callnative HoennSeek_RyanLine
	callstd MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_RyanFirst:
	msgbox HoennSeek_Text_RyanFirst, MSGBOX_DEFAULT
	special HoennRyanMet
	release
	end

HoennSeek_EventScript_Librarian::
	lock
	faceplayer
	callnative HoennSeek_LibrarianLine
	callstd MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_CallRyan::
	setflag FLAG_HOENN_CALL_RYAN
	setspeaker HoennSeek_Text_NameMay
	pokenavcall HoennSeek_Text_CallRyan
	goto HoennSeek_EventScript_CallEnd

@ ---- RYAN: the first meeting
HoennSeek_Text_RyanFirst:
	.string "…and that's why you never trust\n"
	.string "a door. Hang on, everyone, we've\l"
	.string "got a visitor!\p"
	.string "Hi! I'm RYAN. I do runs. FACTORY,\n"
	.string "PYRAMID, PIKE, whatever the day\l"
	.string "says. And I talk the whole way\l"
	.string "through, for everyone at home.\p"
	.string "Nǚshìmen, xiānshengmen, huānyíng!\p"
	.string "That was Mandarin! I'm learning.\n"
	.string "Badly! Out loud! In public!\p"
	.string "Oh, and that's THE LIBRARIAN.\n"
	.string "THE LIBRARIAN writes down every\l"
	.string "word I say. I didn't hire anyone.\l"
	.string "Nobody knows who did.\p"
	.string "If you want to know what I just\n"
	.string "said, ask THE LIBRARIAN. I'm a very\l"
	.string "well-documented man.$"

@ ---- RYAN: the run of the day (the first talk each day; src/hoenn_ryan.c sRunTexts)
HoennSeek_Text_RyanRunSun::
	.string "Sunday is TOWER day. No rentals,\n"
	.string "no doors, no dark. Just my team\l"
	.string "and a ladder that never ends.\p"
	.string "Very relaxing. I'm going to lose\n"
	.string "to a SHEDINJA. Zhè shì mìngyùn.\p"
	.string "Tomorrow's run: the FACTORY!\n"
	.string "Come say hi.$"

HoennSeek_Text_RyanRunMon::
	.string "Monday: the BATTLE FACTORY! You\n"
	.string "don't bring a team. They hand you\l"
	.string "six strangers and say good luck.\p"
	.string "That's a roguelike! That's the\n"
	.string "purest thing in this building!\p"
	.string "First rental: a SUDOWOODO.\n"
	.string "…Zhè ge hěn qiáng. I'm keeping it.\p"
	.string "Tomorrow's run: the PIKE.$"

HoennSeek_Text_RyanRunTue::
	.string "Tuesday is PIKE day. Three doors.\n"
	.string "One heals you, one has a nurse,\l"
	.string "one has a BRELOOM with SPORE.\p"
	.string "Hóng mén háishi lǜ mén?\n"
	.string "They're all the same color.\l"
	.string "I ask every week anyway.\p"
	.string "Tomorrow's run: the ARENA.$"

HoennSeek_Text_RyanRunWed::
	.string "Wednesday: the ARENA. Three turns,\n"
	.string "then a judge scores MIND, SKILL\l"
	.string "and BODY.\p"
	.string "I lose on MIND every single time.\n"
	.string "The judge can tell. Wǒ shì lǘ.\p"
	.string "Tomorrow's run: the PALACE.$"

HoennSeek_Text_RyanRunThu::
	.string "Thursday is PALACE day, where your\n"
	.string "POKéMON pick their own moves. It's\l"
	.string "like watching someone else play.\p"
	.string "My GENGAR used PROTECT four times\n"
	.string "in a row. Bù kěnéng!\p"
	.string "Tomorrow's run: the DOME.$"

HoennSeek_Text_RyanRunFri::
	.string "Friday: the DOME! A tournament.\n"
	.string "They show you the whole bracket\l"
	.string "first, so you can see exactly who's\l"
	.string "going to beat you.\p"
	.string "Round two. It's always round two.\n"
	.string "Wǒ wán le.\p"
	.string "Tomorrow's run: the PYRAMID!$"

HoennSeek_Text_RyanRunSat::
	.string "Saturday. The PYRAMID. The real\n"
	.string "one. It's dark, you can't bring\l"
	.string "items, you find it all on the floor.\p"
	.string "Floor by floor, everyone at home.\n"
	.string "This is the dream. Jiāyóu!\p"
	.string "Tomorrow's run: the TOWER, to\n"
	.string "rest my nerves.$"

@ ---- RYAN: the pool (src/hoenn_ryan.c sPool; holding lines take turns)
HoennSeek_Text_RyanNoRuns::
	.string "Wait. You've never done a run here?\n"
	.string "Not one?\p"
	.string "You get to try all seven for the\n"
	.string "first time. I'm so jealous. Jiāyóu!$"

HoennSeek_Text_RyanItemLuck::
	.string "I opened eleven item balls in the\n"
	.string "PYRAMID last week. Ten were… I want\l"
	.string "to say tragic.\p"
	.string "One was a SITRUS BERRY. I ate it in\n"
	.string "the hallway. Out of spite.\l"
	.string "Wǒ de yùnqì tài chà le.$"

HoennSeek_Text_RyanMiddle::
	.string "People ask if I'm good at these.\n"
	.string "I'm not! I'm consistent. I am\l"
	.string "consistently in the middle.\p"
	.string "There's a whole community of us\n"
	.string "down here. Méi wèntí.$"

HoennSeek_Text_RyanTheOne::
	.string "Every run, I say: this is the one.\p"
	.string "It is never the one. But one day\n"
	.string "it will be, and I'll have been right\l"
	.string "the whole time. Zài lái yí cì!$"

HoennSeek_Text_RyanRandom::
	.string "The FACTORY's scientist says the\n"
	.string "rentals are random. I said, random\l"
	.string "for who? He didn't answer.\p"
	.string "That's how I know.\n"
	.string "Suíjīshù bù xǐhuan wǒ.$"

HoennSeek_Text_RyanNotebook::
	.string "Don't look now, but THE LIBRARIAN's\n"
	.string "here again. I've never seen them\l"
	.string "arrive. I've never seen them leave.\p"
	.string "I just turn around and there's a\n"
	.string "new notebook. Xiǎoxīn!$"

HoennSeek_Text_RyanStop::
	.string "You know the best held item? Not\n"
	.string "LEFTOVERS. Not a CHOICE SCARF.\l"
	.string "It's knowing when to stop.\p"
	.string "I've never found one. Tài nán le.$"

HoennSeek_Text_RyanTones::
	.string "Why Mandarin? Practice! Four tones,\n"
	.string "and every sentence I get one wrong.\p"
	.string "It's a roguelike too, if you think\n"
	.string "about it. Wǒ zhēn shì tiāncái.$"

HoennSeek_Text_RyanFactoryRecord::
	.string "Your FACTORY record is better than\n"
	.string "mine. I looked it up. Bù kěnéng!\p"
	.string "I'm not upset. I'm taking notes.\n"
	.string "THE LIBRARIAN is taking notes.$"

HoennSeek_Text_RyanPyramidRecord::
	.string "You got past floor 21 of the\n"
	.string "PYRAMID? In the dark? With what\l"
	.string "you found on the floor?\p"
	.string "Teach me. No, don't. I'd just do it\n"
	.string "worse. Wǒ zhēn shì tiāncái.$"

HoennSeek_Text_RyanPikeRecord::
	.string "You've picked the right door\n"
	.string "in the PIKE more than I have\l"
	.string "picked any door at all.\p"
	.string "Hóng mén háishi lǜ mén?\n"
	.string "You'd know. I wouldn't.$"

HoennSeek_Text_RyanTowerRecord::
	.string "Thirty-five wins in a row in the\n"
	.string "TOWER. You did the thing where you\l"
	.string "just keep winning.\p"
	.string "I've heard about it. Zhè ge hěn\n"
	.string "qiáng. That's you. You're “zhè ge.”$"

HoennSeek_Text_RyanSilverSymbols::
	.string "All seven SILVER SYMBOLS!\n"
	.string "The FRONTIER BRAINS must be\l"
	.string "nervous about you.\p"
	.string "They've never been nervous about\n"
	.string "me. Wǒ wán le.$"

HoennSeek_Text_RyanGoldSymbol::
	.string "Is that a GOLD SYMBOL?\n"
	.string "Can I hold it? I won't put it on\l"
	.string "the broadcast. Bù kěnéng…\p"
	.string "Okay, I'll put it on the broadcast\n"
	.string "a little.$"

HoennSeek_Text_RyanPoints::
	.string "You're sitting on a lot of BATTLE\n"
	.string "POINTS. Spend them!\p"
	.string "Points in the bank are a run you\n"
	.string "didn't take. Zài lái yí cì!$"

@ ---- THE LIBRARIAN (src/hoenn_ryan.c; STR_VAR_1 = the entry number)
HoennSeek_Text_LibrarianNotMet::
	.string "… … …\p"
	.string "(The person is writing in a thick\n"
	.string "notebook and doesn't look up.)$"

HoennSeek_Text_LibrarianVolume::
	.string "…Volume 31. Everything the subject\n"
	.string "has said this season, in order.\p"
	.string "The subject does not know about\n"
	.string "volumes 1 to 30.$"

HoennSeek_Text_LibrarianTheOne::
	.string "…The subject says “this is the one”\n"
	.string "about four times a day.\p"
	.string "It has been the one twice.$"

HoennSeek_Text_LibrarianTones::
	.string "…I translate. I don't comment.\p"
	.string "The subject's tones are improving.\n"
	.string "That is not a comment.$"

HoennSeek_Text_LibrarianBorrow::
	.string "…You may borrow a volume. Return it\n"
	.string "to any FRONTIER lobby.\p"
	.string "I'll find it.$"

HoennSeek_Text_LibrarianPhraseWelcome::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Nǚshìmen, xiānshengmen, huānyíng!”\n"
	.string "“Ladies and gentlemen, welcome!”\p"
	.string "The subject greets an audience that\n"
	.string "isn't here. Every time.$"

HoennSeek_Text_LibrarianPhraseBadLuck::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Wǒ de yùnqì tài chà le.”\n"
	.string "“My luck is far too bad.”\p"
	.string "The most common entry.\n"
	.string "Volume 12 is only this.$"

HoennSeek_Text_LibrarianPhraseStrong::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Zhè ge hěn qiáng.”\n"
	.string "“This one is strong.”\p"
	.string "Usually said of the opponent.\n"
	.string "Today, of a SUDOWOODO.$"

HoennSeek_Text_LibrarianPhraseAddOil::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Jiāyóu!”\n"
	.string "Literally, “add oil.” It means\l"
	.string "“go for it!”\p"
	.string "The subject finds “add oil” funny\n"
	.string "every single time.$"

HoennSeek_Text_LibrarianPhraseFinished::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Wǒ wán le.”\n"
	.string "“I'm finished.”\p"
	.string "Said before the battle. Accurate.$"

HoennSeek_Text_LibrarianPhraseNoProblem::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Méi wèntí.”\n"
	.string "“No problem.”\p"
	.string "There was a problem.$"

HoennSeek_Text_LibrarianPhraseImpossible::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Bù kěnéng!” “Impossible!”\p"
	.string "It was possible.$"

HoennSeek_Text_LibrarianPhraseAgain::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Zài lái yí cì!”\n"
	.string "“One more time!”\p"
	.string "It is never one more time.$"

HoennSeek_Text_LibrarianPhraseRandom::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Suíjīshù bù xǐhuan wǒ.”\n"
	.string "“The random numbers don't like me.”\p"
	.string "I checked. They have no opinion\n"
	.string "of the subject.$"

HoennSeek_Text_LibrarianPhraseFate::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Zhè shì mìngyùn.”\n"
	.string "“This is fate.”\p"
	.string "Fate, in this case, is a SHEDINJA\n"
	.string "with a FOCUS SASH.$"

HoennSeek_Text_LibrarianPhraseCareful::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Xiǎoxīn!” “Careful!”\p"
	.string "Addressed to me.\n"
	.string "I was not in any danger.$"

HoennSeek_Text_LibrarianPhraseGenius::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Wǒ zhēn shì tiāncái.”\n"
	.string "“I really am a genius.”\p"
	.string "Sarcasm. He got all four tones\n"
	.string "right, though.$"

HoennSeek_Text_LibrarianPhraseTooHard::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Tài nán le.”\n"
	.string "“It's too hard.”\p"
	.string "It is.$"

HoennSeek_Text_LibrarianPhraseDoors::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Hóng mén háishi lǜ mén?”\n"
	.string "“Red door or green door?”\p"
	.string "The doors in the PIKE are all the\n"
	.string "same color. He knows.$"

HoennSeek_Text_LibrarianPhraseDonkey::
	.string "…Entry {STR_VAR_1}.\p"
	.string "“Wǒ shì lǘ.”\n"
	.string "“I am a donkey.”\p"
	.string "Self-assessment.\n"
	.string "Entered without comment.$"

@ ---- May's call (2.1-30): BRENDAN found RYAN's broadcast first
HoennSeek_Text_CallRyan:
	.string "{PLAYER}! It's MAY! You were on\n"
	.string "a broadcast! Well, sort of.\p"
	.string "BRENDAN found this trainer at the\n"
	.string "BATTLE FRONTIER who talks through\l"
	.string "every battle for people at home.\p"
	.string "And there you were! He said you\n"
	.string "looked like a natural. Then he lost\l"
	.string "to a SHEDINJA.\p"
	.string "BRENDAN's been saying “jiāyóu”\n"
	.string "all day. He won't say what it means.$"
