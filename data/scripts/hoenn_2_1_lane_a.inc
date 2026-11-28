@ Hoenn & Seek 2.1, lane A (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-a.md).
@ Lane A's 2.1 scenes, phone calls and texts go in this file (or a new
@ data/scripts/hoenn_*.inc of its own). C code: src/hoenn_2_1_lane_a.c.

@ ---------------------------------------------------------------- event calls

@ The PokéGear event-call switch falls through 2.0's lanes, then 2.1's lane A,
@ then 2.1's lane B. Add one case per call:
@ 	case HOENN_CALL_X, HoennSeek_EventScript_CallX
HoennSeek_EventScript_PhoneEvent21LaneA::
	switch VAR_0x8004
	case HOENN_CALL_JOURNAL_JOHTO, HoennSeek_EventScript_CallJournalJohto @ 2.1-06
	case HOENN_CALL_JOURNAL_KANTO, HoennSeek_EventScript_CallJournalKanto
	case HOENN_CALL_JOURNAL_HOENN, HoennSeek_EventScript_CallJournalHoenn
	goto HoennSeek_EventScript_PhoneEvent21LaneB @ lane B's 2.1 calls (hoenn_2_1_lane_b.inc)

@ ==== 2.1-05: the SEEKER'S JOURNAL ====
@ The key item's screen (src/hoenn_journal.c explains the states): pick a page,
@ then an entry, read it, and the list opens again on the same row. Run from
@ the bag or SELECT (ItemUseOutOfBattle_HoennSeekersJournal, src/item_use.c).
@ VAR_0x8004 = page, VAR_0x8005 = entry (its place on the page),
@ VAR_0x8006 = the page to start on (a menu row's id; TODAY first).
HoennSeek_EventScript_SeekersJournal::
	lockall
	setvar VAR_0x8006, HOENN_JOURNAL_PAGE_TODAY @ the menu starts on the row with this id
HoennSeek_EventScript_JournalPages:
	message HoennSeek_Text_JournalOpen
	waitmessage
	special HoennJournalPushPages
	dynmultistack 0, 0, FALSE, 6, FALSE, VAR_0x8006, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, HoennSeek_EventScript_JournalClose
	goto_if_gt VAR_RESULT, HOENN_JOURNAL_PAGE_TODAY, HoennSeek_EventScript_JournalClose @ CLOSE
	copyvar VAR_0x8004, VAR_RESULT
	copyvar VAR_0x8006, VAR_RESULT
	setvar VAR_0x8005, 0
HoennSeek_EventScript_JournalEntries:
	special HoennJournalPushEntries @ STR_VAR_1 page, STR_VAR_2 found, STR_VAR_3 total
	goto_if_eq VAR_0x8004, HOENN_JOURNAL_PAGE_TODAY, HoennSeek_EventScript_JournalTodayHeader
	message HoennSeek_Text_JournalPage
HoennSeek_EventScript_JournalEntryList:
	waitmessage
	dynmultistack 0, 0, FALSE, 6, FALSE, VAR_0x8005, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, HoennSeek_EventScript_JournalPages
	goto_if_eq VAR_RESULT, HOENN_JOURNAL_MENU_BACK, HoennSeek_EventScript_JournalPages
	copyvar VAR_0x8005, VAR_RESULT
	call_if_eq VAR_0x8004, HOENN_JOURNAL_PAGE_TODAY, HoennSeek_EventScript_JournalWhen
	callnative HoennSeek_JournalEntryText
	callstd MSGBOX_DEFAULT
	goto HoennSeek_EventScript_JournalEntries

@ 2.1-06: the TODAY page (STR_VAR_1 = the weekday, STR_VAR_2 = how many are
@ on), and each of its entries' "when" line before the entry itself.
HoennSeek_EventScript_JournalTodayHeader:
	message HoennSeek_Text_JournalToday
	goto HoennSeek_EventScript_JournalEntryList

HoennSeek_EventScript_JournalWhen:
	callnative HoennSeek_JournalWhenText
	callstd MSGBOX_DEFAULT
	return

HoennSeek_EventScript_JournalClose:
	closemessage
	releaseall
	end

@ Elm's aide (NewBarkTown_Lab_hns): the first thing she does once you have a
@ Johto badge is hand over the journal. Called first by her hint wrapper
@ (npc_hints.json "first"), so it ends the talk when it gives the journal and
@ returns otherwise. Elm's escapee call (hoenn_phone.inc) sends you to her.
HoennSeek_EventScript_ElmAideJournal::
	goto_if_unset FLAG_BADGE01_GET, HoennSeek_EventScript_ElmAideJournalReturn
	goto_if_set FLAG_HOENN_GOT_SEEKERS_JOURNAL, HoennSeek_EventScript_ElmAideJournalReturn
	lock
	faceplayer
	msgbox HoennSeek_Text_ElmAideJournal, MSGBOX_DEFAULT
	giveitem ITEM_SEEKERS_JOURNAL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_ElmAideJournalFull
	setflag FLAG_HOENN_GOT_SEEKERS_JOURNAL
	msgbox HoennSeek_Text_ElmAideJournalHowTo, MSGBOX_DEFAULT
	release
	end
HoennSeek_EventScript_ElmAideJournalFull:
	msgbox HoennSeek_Text_ElmAideJournalFull, MSGBOX_DEFAULT
	release
	end
HoennSeek_EventScript_ElmAideJournalReturn::
	return

HoennSeek_Text_JournalOpen:
	.string "SEEKER'S JOURNAL.\n"
	.string "Which page?$"

HoennSeek_Text_JournalPage:
	.string "{STR_VAR_1}:\n"
	.string "found {STR_VAR_2} of {STR_VAR_3}.$"

HoennSeek_Text_JournalToday:
	.string "It's {STR_VAR_1}. On today and\n"
	.string "tonight: {STR_VAR_2}.$"

HoennSeek_Text_ElmAideJournal:
	.string "Oh, {PLAYER}! Congratulations on\n"
	.string "your first BADGE!\p"
	.string "PROF. BIRCH's lab in HOENN made\n"
	.string "their field notes into a notebook\l"
	.string "for TRAINERS, and sent us one.\p"
	.string "The PROF. wants you to have it.\n"
	.string "You'll hear about all sorts of\l"
	.string "rare POKéMON on your travels!$"

HoennSeek_Text_ElmAideJournalHowTo:
	.string "Whenever someone tells you about\n"
	.string "something strange, it goes in\l"
	.string "the JOURNAL.\p"
	.string "Once you've seen it for yourself,\n"
	.string "it's written up as found.\p"
	.string "Read it from your BAG, or register\n"
	.string "it to SELECT!$"

HoennSeek_Text_ElmAideJournalFull:
	.string "Oh, your BAG is full… Make some\n"
	.string "room and come back for it!$"

@ ==== 2.1-06: the SEEKER'S JOURNAL's rewards ====
@ When a region's page is all found, its professor calls (Hoenn21LaneADueCall,
@ src/hoenn_2_1_lane_a.c) and gives a doll at the lab: Elm a CELEBI DOLL, Oak a
@ MEW DOLL, Birch a JIRACHI DOLL (2.1 plan, round 5). The gift doesn't wait
@ for the call: talk to the professor any time once the page is done. Elm's
@ and Oak's objects run the wrappers below (map.json swaps), which fall back
@ to HnS's own scripts; Birch's script calls HoennSeek_EventScript_BirchJournal
@ first. Each professor knows because Oak reads your POKéDEX data and phones
@ the other two (as his weekly calls with Birch already say).

HoennSeek_EventScript_CallJournalJohto::
	setflag FLAG_HOENN_CALL_JOURNAL_JOHTO
	setspeaker HoennSeek_Text_NameElm
	pokenavcall HoennSeek_Text_CallJournalJohto
	goto HoennSeek_EventScript_CallEnd
HoennSeek_EventScript_CallJournalKanto::
	setflag FLAG_HOENN_CALL_JOURNAL_KANTO
	setspeaker HoennSeek_Text_NameOak
	pokenavcall HoennSeek_Text_CallJournalKanto
	goto HoennSeek_EventScript_CallEnd
HoennSeek_EventScript_CallJournalHoenn::
	setflag FLAG_HOENN_CALL_JOURNAL_HOENN
	setspeaker HoennSeek_Text_NameBirch
	pokenavcall HoennSeek_Text_CallJournalHoenn
	goto HoennSeek_EventScript_CallEnd

HoennSeek_EventScript_ElmJournal::
	call HoennSeek_EventScript_ElmJournalDoll
	goto NewBarkTown_Lab_EventScript_Elm
HoennSeek_EventScript_ElmJournalDoll:
	goto_if_set FLAG_HOENN_JOURNAL_DOLL_JOHTO, HoennSeek_EventScript_JournalDollReturn
	setvar VAR_0x8004, HOENN_JOURNAL_PAGE_JOHTO
	specialvar VAR_RESULT, HoennJournalPageDone
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_JournalDollReturn
	lock
	faceplayer
	msgbox HoennSeek_Text_ElmJournalDoll, MSGBOX_DEFAULT
	givedecoration DECOR_CELEBI_DOLL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_JournalDollNoRoom
	setflag FLAG_HOENN_JOURNAL_DOLL_JOHTO
	setflag FLAG_HOENN_CALL_JOURNAL_JOHTO
	msgbox HoennSeek_Text_ElmJournalDollAfter, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_OakJournal::
	call HoennSeek_EventScript_OakJournalDoll
	goto PalletTown_Lab_EventScript_Oak
HoennSeek_EventScript_OakJournalDoll:
	goto_if_set FLAG_HOENN_JOURNAL_DOLL_KANTO, HoennSeek_EventScript_JournalDollReturn
	setvar VAR_0x8004, HOENN_JOURNAL_PAGE_KANTO
	specialvar VAR_RESULT, HoennJournalPageDone
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_JournalDollReturn
	lock
	faceplayer
	msgbox HoennSeek_Text_OakJournalDoll, MSGBOX_DEFAULT
	givedecoration DECOR_MEW_DOLL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_JournalDollNoRoom
	setflag FLAG_HOENN_JOURNAL_DOLL_KANTO
	setflag FLAG_HOENN_CALL_JOURNAL_KANTO
	msgbox HoennSeek_Text_OakJournalDollAfter, MSGBOX_DEFAULT
	release
	end

@ Called by HoennSeek_EventScript_Birch (hoenn_arrival.inc) after faceplayer.
HoennSeek_EventScript_BirchJournal::
	goto_if_set FLAG_HOENN_JOURNAL_DOLL_HOENN, HoennSeek_EventScript_JournalDollReturn
	goto_if_unset FLAG_HOENN_RECEIVED_MUDKIP, HoennSeek_EventScript_JournalDollReturn @ his welcome and MUDKIP come first
	setvar VAR_0x8004, HOENN_JOURNAL_PAGE_HOENN
	specialvar VAR_RESULT, HoennJournalPageDone
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_JournalDollReturn
	msgbox HoennSeek_Text_BirchJournalDoll, MSGBOX_DEFAULT
	givedecoration DECOR_JIRACHI_DOLL
	goto_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_JournalDollNoRoom
	setflag FLAG_HOENN_JOURNAL_DOLL_HOENN
	setflag FLAG_HOENN_CALL_JOURNAL_HOENN
	msgbox HoennSeek_Text_BirchJournalDollAfter, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_JournalDollNoRoom:
	msgbox HoennSeek_Text_JournalDollNoRoom, MSGBOX_DEFAULT
	release
	end
HoennSeek_EventScript_JournalDollReturn:
	return

HoennSeek_Text_NameOak:
	.string "PROF. OAK$"
HoennSeek_Text_NameBirch:
	.string "PROF. BIRCH$"

HoennSeek_Text_CallJournalJohto:
	.string "Hello, {PLAYER}? It's PROF. ELM!\p"
	.string "PROF. OAK was going over your\n"
	.string "POKéDEX data, and he called me.\p"
	.string "You've seen everything on the\n"
	.string "JOHTO page of your JOURNAL!\p"
	.string "Come by my lab in NEW BARK TOWN.\n"
	.string "I have something for you!$"

HoennSeek_Text_CallJournalKanto:
	.string "Hello, {PLAYER}! PROF. OAK here.\p"
	.string "I was reading your POKéDEX data\n"
	.string "just now, and what do I see?\p"
	.string "Every rumor on the KANTO page of\n"
	.string "your JOURNAL, seen for yourself!\p"
	.string "Drop by my lab in PALLET TOWN.\n"
	.string "I've put something aside for you.$"

HoennSeek_Text_CallJournalHoenn:
	.string "{PLAYER}? PROF. BIRCH here!\p"
	.string "PROF. OAK just phoned me about\n"
	.string "your POKéDEX data. Incredible!\p"
	.string "You've filled in the whole HOENN\n"
	.string "page of the JOURNAL my lab made!\p"
	.string "Come see me in LITTLEROOT TOWN.\n"
	.string "I have a little thank-you gift!$"

HoennSeek_Text_ElmJournalDoll:
	.string "ELM: {PLAYER}! The whole JOHTO\n"
	.string "page of your JOURNAL, found!\p"
	.string "When I was a boy, everyone talked\n"
	.string "about the CELEBI of ILEX FOREST.\p"
	.string "I had a doll made of it. I think\n"
	.string "it belongs in your secret base.$"

HoennSeek_Text_ElmJournalDollAfter:
	.string "ELM: Every rumor in JOHTO, seen\n"
	.string "with your own eyes. Amazing!$"

HoennSeek_Text_OakJournalDoll:
	.string "OAK: Ah, {PLAYER}! Every page of\n"
	.string "KANTO's rumors, seen for yourself!\p"
	.string "Some still say MEW is only a\n"
	.string "legend. You know better.\p"
	.string "Here, this doll is for you. Put it\n"
	.string "somewhere it can keep you company!$"

HoennSeek_Text_OakJournalDollAfter:
	.string "OAK: A TRAINER who hears a rumor\n"
	.string "and goes to see it. Wonderful!$"

HoennSeek_Text_BirchJournalDoll:
	.string "BIRCH: {PLAYER}! The whole HOENN\n"
	.string "page, found! My aides are amazed.\p"
	.string "They say JIRACHI grants wishes.\n"
	.string "Well, you granted one of mine!\p"
	.string "Here's a JIRACHI DOLL for your\n"
	.string "secret base. Thank you!$"

HoennSeek_Text_BirchJournalDollAfter:
	.string "BIRCH: Fieldwork like that is what\n"
	.string "my lab is all about!$"

HoennSeek_Text_JournalDollNoRoom:
	.string "Your PC has no room for it? Make\n"
	.string "some space and come back!$"

@ ==== 2.1-07: the daily swarm ====
@ src/hoenn_swarm.c explains it. Every TV opens with today's swarm (before the
@ forecast and the normal show; hoenn_story.inc's TV news calls this), which
@ puts it in the SEEKER'S JOURNAL as heard.
HoennSeek_EventScript_TVSwarm::
	special HoennSwarmBufferNames
	setflag FLAG_HOENN_HEARD_DAILY_SWARM
	msgbox HoennSeek_Text_TVSwarm, MSGBOX_DEFAULT
	return

@ The berry check (2.1 plan section 2). HnS's berry tree script only picks
@ (and replants the same berry): it has no planting, so Hoenn's 8 empty soil
@ patches (Routes 104, 118, 121, 123) said "not ripe yet" forever. Empty soil
@ now asks for a BERRY from the bag (berry_tree.inc's NO_BERRY case comes
@ here); the tree then grows and replants like every HnS tree. Watering stays
@ off, as in HnS (the WAILMER PAIL still has no use).
HoennSeek_EventScript_BerrySoil::
	lock
	msgbox HoennSeek_Text_BerrySoil, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennSeek_EventScript_BerrySoilEnd
	special Bag_ChooseBerry
	waitstate
	goto_if_eq VAR_ITEM_ID, ITEM_NONE, HoennSeek_EventScript_BerrySoilEnd
	special HoennPlantBerryInSoil @ removes the BERRY
	bufferitemname STR_VAR_1, VAR_ITEM_ID
	msgbox HoennSeek_Text_BerryPlanted, MSGBOX_DEFAULT
HoennSeek_EventScript_BerrySoilEnd:
	release
	end

HoennSeek_Text_BerrySoil:
	.string "It's soft, loamy soil.\n"
	.string "Want to plant a BERRY?$"

HoennSeek_Text_BerryPlanted:
	.string "{PLAYER} planted one {STR_VAR_1}\n"
	.string "in the soil.$"

HoennSeek_Text_TVSwarm:
	.string "Today's POKéMON news: a swarm of\n"
	.string "{STR_VAR_1} has been spotted\l"
	.string "on {STR_VAR_2}!$"

	.include "data/scripts/hoenn_journal_texts.inc"
