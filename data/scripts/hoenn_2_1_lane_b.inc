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

	.include "data/scripts/hoenn_fan_chains.inc"
