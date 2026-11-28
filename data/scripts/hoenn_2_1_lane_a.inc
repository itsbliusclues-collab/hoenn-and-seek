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

@ ==== 2.1-08: SILVER keeps up ====
@ src/hoenn_silver.c explains the ladder and the milestone teams. His Indigo
@ Plateau object is a hint NPC (npc_hints.json): its wrapper calls
@ SilverCelebiLine first, and its own script is IndigoSilver, which hands
@ HnS's team (tier 0) to HnS's script and plays the milestone teams itself.
HoennSeek_EventScript_IndigoSilver::
	specialvar VAR_RESULT, HoennSilverTierForScript
	goto_if_eq VAR_RESULT, 0, IndigoPlateau_EventScript_Silver
	lock
	faceplayer
	goto_if_set FLAG_DAILY_BEAT_SILVER, HoennSeek_EventScript_SilverBeatToday
	call HoennSeek_EventScript_SilverTierIntro
	call HoennSeek_EventScript_SilverBattle
	msgbox HoennSeek_Text_SilverAfter, MSGBOX_DEFAULT
	setflag FLAG_DAILY_BEAT_SILVER
	release
	end

HoennSeek_EventScript_SilverBeatToday::
	msgbox HoennSeek_Text_SilverBeatToday, MSGBOX_DEFAULT
	release
	end

@ The milestone's line (VAR_RESULT = his tier, 1-4).
HoennSeek_EventScript_SilverTierIntro::
	switch VAR_RESULT
	case 1, HoennSeek_EventScript_SilverIntro1
	case 2, HoennSeek_EventScript_SilverIntro2
	case 3, HoennSeek_EventScript_SilverIntro3
	msgbox HoennSeek_Text_SilverIntro4, MSGBOX_DEFAULT
	return
HoennSeek_EventScript_SilverIntro1:
	msgbox HoennSeek_Text_SilverIntro1, MSGBOX_DEFAULT
	return
HoennSeek_EventScript_SilverIntro2:
	msgbox HoennSeek_Text_SilverIntro2, MSGBOX_DEFAULT
	return
HoennSeek_EventScript_SilverIntro3:
	msgbox HoennSeek_Text_SilverIntro3, MSGBOX_DEFAULT
	return

@ HnS's Indigo battle IDs by your starter (his team is the counter-pick);
@ HoennTrainerOverride swaps in the milestone team.
HoennSeek_EventScript_SilverBattle::
	call_if_eq VAR_STARTER_MON, 0, IndigoPlateau_Silver_Battle_Cyndaquil
	call_if_eq VAR_STARTER_MON, 1, IndigoPlateau_Silver_Battle_Totodile
	call_if_eq VAR_STARTER_MON, 2, IndigoPlateau_Silver_Battle_Chikorita
	return

@ Once the Route 22 scene has played (Celebi showed you GIOVANNI and young
@ SILVER), he says what he remembered, once. Called first by his hint wrapper:
@ ends the talk when it speaks, returns otherwise.
HoennSeek_EventScript_SilverCelebiLine::
	goto_if_set FLAG_HOENN_SILVER_CELEBI_LINE, HoennSeek_EventScript_SilverCelebiReturn
	goto_if_lt VAR_ROUTE22_GIOVANNI_STATE, 2, HoennSeek_EventScript_SilverCelebiReturn
	lock
	faceplayer
	msgbox HoennSeek_Text_SilverCelebi, MSGBOX_DEFAULT
	setflag FLAG_HOENN_SILVER_CELEBI_LINE
	release
	end
HoennSeek_EventScript_SilverCelebiReturn::
	return

@ The Ever Grande POKéMON LEAGUE, after you beat WALLACE: he followed you to
@ HOENN. Same battle and daily flag as at INDIGO. The lobby's OnTransition
@ calls LeagueSilverArm.
HoennSeek_EventScript_LeagueSilverArm::
	setflag FLAG_HOENN_HIDE_SILVER_LEAGUE
	goto_if_unset FLAG_HOENN_STAMP_WALLACE, HoennSeek_EventScript_LeagueSilverArmDone
	clearflag FLAG_HOENN_HIDE_SILVER_LEAGUE
HoennSeek_EventScript_LeagueSilverArmDone:
	return

HoennSeek_EventScript_LeagueSilver::
	lock
	faceplayer
	goto_if_set FLAG_HOENN_MET_SILVER_HOENN, HoennSeek_EventScript_LeagueSilverAgain
	msgbox HoennSeek_Text_LeagueSilverMeet, MSGBOX_DEFAULT
	setflag FLAG_HOENN_MET_SILVER_HOENN
HoennSeek_EventScript_LeagueSilverAgain:
	goto_if_set FLAG_DAILY_BEAT_SILVER, HoennSeek_EventScript_SilverBeatToday
	msgbox HoennSeek_Text_LeagueSilverChallenge, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennSeek_EventScript_LeagueSilverLater
	call HoennSeek_EventScript_SilverBattle
	msgbox HoennSeek_Text_SilverAfter, MSGBOX_DEFAULT
	setflag FLAG_DAILY_BEAT_SILVER
	release
	end
HoennSeek_EventScript_LeagueSilverLater:
	msgbox HoennSeek_Text_LeagueSilverLater, MSGBOX_DEFAULT
	release
	end

@ Brendan and May at home (hoenn_teams.inc calls this): they met him at the
@ LEAGUE. Once.
HoennSeek_EventScript_BMSilverLine::
	goto_if_set FLAG_HOENN_BM_SILVER_LINE, HoennSeek_EventScript_SilverCelebiReturn
	msgbox HoennSeek_Text_BMSilverLine, MSGBOX_DEFAULT
	setflag FLAG_HOENN_BM_SILVER_LINE
	return

HoennSeek_Text_SilverIntro1:
	.string "Hmph. So you beat the KANTO\n"
	.string "LEAGUE too.\p"
	.string "I've been training where you\n"
	.string "weren't looking.\p"
	.string "My team's new. Let's see if you\n"
	.string "can keep up!$"

HoennSeek_Text_SilverIntro2:
	.string "{PLAYER}. They say you're winning\n"
	.string "BADGES in HOENN now.\p"
	.string "Don't think I've just been\n"
	.string "standing around.\p"
	.string "My POKéMON listen to me now.\n"
	.string "Every last one. Come on!$"

HoennSeek_Text_SilverIntro3:
	.string "You beat HOENN's champion…\p"
	.string "I went there too. I found a BELDUM\n"
	.string "in a cave and raised it myself.\p"
	.string "Now it's a METAGROSS. Let's go!$"

HoennSeek_Text_SilverIntro4:
	.string "I heard you beat BRENDAN and MAY.\n"
	.string "Both at once.\p"
	.string "Everyone keeps saying your name.\n"
	.string "I'm sick of it.\p"
	.string "Today I'm the one who wins!$"

HoennSeek_Text_SilverAfter:
	.string "…I lost again.\p"
	.string "But it was closer than last time.\n"
	.string "I could feel it.\p"
	.string "Come back tomorrow. Don't you dare\n"
	.string "skip it.$"

HoennSeek_Text_SilverBeatToday:
	.string "We already battled today.\p"
	.string "My POKéMON need rest. Even I know\n"
	.string "that much now.$"

HoennSeek_Text_SilverCelebi:
	.string "…Hey. Something weird happened.\p"
	.string "I remembered a day I'd forgotten.\n"
	.string "ROUTE 22, three years ago.\p"
	.string "My father asked me what made a\n"
	.string "trainer number one.\p"
	.string "I didn't have an answer then.\n"
	.string "I think I do now.\p"
	.string "…It's my POKéMON.\n"
	.string "Don't tell anyone I said that.$"

HoennSeek_Text_LeagueSilverMeet:
	.string "…What are you looking at?\p"
	.string "Yeah, I came to HOENN. You weren't\n"
	.string "getting the strong trainers here\l"
	.string "all to yourself.\p"
	.string "You beat their champion, WALLACE?\n"
	.string "Then you're still the one I have\l"
	.string "to beat. Same as always.$"

HoennSeek_Text_LeagueSilverChallenge:
	.string "Battle me, {PLAYER}.\n"
	.string "Right here.$"

HoennSeek_Text_LeagueSilverLater:
	.string "Hmph. I'll be here.$"

HoennSeek_Text_BMSilverLine:
	.string "Oh! A red-haired trainer from\n"
	.string "JOHTO was at the POKéMON LEAGUE.\p"
	.string "He battled us both without saying\n"
	.string "a word, then asked if we knew you.\p"
	.string "Friend of yours?$"

@ ==== 2.1-09: SILVER's starter follows him ====
@ The object is spawned by src/hoenn_silver_starter.c (no template); talking
@ to it runs this through GetObjectEventScriptPointerByLocalIdAndMap.
HoennSeek_EventScript_SilverStarter::
	lock
	faceplayer
	specialvar VAR_RESULT, HoennSilverStarterForScript
	playmoncry VAR_RESULT, CRY_MODE_NORMAL
	msgbox HoennSeek_Text_SilverStarter, MSGBOX_DEFAULT
	waitmoncry
	release
	end

HoennSeek_Text_SilverStarter:
	.string "SILVER's {STR_VAR_1} gives you\n"
	.string "a long look.\p"
	.string "It doesn't stray far\n"
	.string "from its trainer.$"

@ ==== 2.1-12/14: the friend trainers ====
@ src/hoenn_friends.c explains the system. One script for all three; each
@ friend's lines are picked in C by VAR_0x8004 (friendmsg). Only the battle
@ itself needs one stub per friend (trainerbattle takes a constant).
	.macro friendmsg which:req, type=MSGBOX_DEFAULT
	callnative HoennSeek_FriendText
	.byte \which
	callstd \type
	.endm

HoennSeek_EventScript_FriendBridgette::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_FRIEND_BRIDGETTE
	goto HoennSeek_EventScript_FriendTalk

HoennSeek_EventScript_FriendAidan::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_FRIEND_AIDAN
	goto HoennSeek_EventScript_FriendTalk

HoennSeek_EventScript_FriendBrock::
	lock
	faceplayer
	setvar VAR_0x8004, HOENN_FRIEND_BROCK
	goto HoennSeek_EventScript_FriendTalk

HoennSeek_EventScript_FriendTalk::
	specialvar VAR_RESULT, HoennFriendTalkState
	goto_if_eq VAR_RESULT, HOENN_FRIEND_STATE_TODAY, HoennSeek_EventScript_FriendToday
	goto_if_eq VAR_RESULT, HOENN_FRIEND_STATE_FIRST, HoennSeek_EventScript_FriendFirst
	goto_if_eq VAR_RESULT, HOENN_FRIEND_STATE_STRONGER, HoennSeek_EventScript_FriendStronger
	callnative HoennSeek_FriendText
	.byte HOENN_FRIEND_TEXT_AGAIN
	goto HoennSeek_EventScript_FriendAsk

HoennSeek_EventScript_FriendStronger::
	callnative HoennSeek_FriendText
	.byte HOENN_FRIEND_TEXT_STRONGER
	goto HoennSeek_EventScript_FriendAsk

HoennSeek_EventScript_FriendFirst::
	friendmsg HOENN_FRIEND_TEXT_MEET
	specialvar VAR_RESULT, HoennFriendHasNameLine
	call_if_eq VAR_RESULT, TRUE, HoennSeek_EventScript_FriendNameLine
	call_if_eq VAR_0x8004, HOENN_FRIEND_BROCK, HoennSeek_EventScript_BrockTypeQuestion
	callnative HoennSeek_FriendText
	.byte HOENN_FRIEND_TEXT_ASK
@ data[0] holds the question: YES/NO, or NORMAL/HARD/NO after the final Red.
HoennSeek_EventScript_FriendAsk::
	specialvar VAR_RESULT, HoennFriendHardUnlocked
	goto_if_eq VAR_RESULT, TRUE, HoennSeek_EventScript_FriendAskHard
	callstd MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennSeek_EventScript_FriendDecline
	setvar VAR_0x8005, FALSE
	goto HoennSeek_EventScript_FriendBattle

HoennSeek_EventScript_FriendAskHard::
	message NULL
	waitmessage
	dynmultichoice 21, 6, FALSE, 3, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_FriendNormal, HoennSeek_Text_FriendHard, gText_No
	goto_if_eq VAR_RESULT, HOENN_FRIEND_CHOICE_NORMAL, HoennSeek_EventScript_FriendNormal
	goto_if_eq VAR_RESULT, HOENN_FRIEND_CHOICE_HARD, HoennSeek_EventScript_FriendHard
	goto HoennSeek_EventScript_FriendDecline

HoennSeek_EventScript_FriendNormal::
	setvar VAR_0x8005, FALSE
	goto HoennSeek_EventScript_FriendBattle

HoennSeek_EventScript_FriendHard::
	setvar VAR_0x8005, TRUE
HoennSeek_EventScript_FriendBattle::
	friendmsg HOENN_FRIEND_TEXT_GO
	closemessage
	special HoennFriendSetHard
	setdynamicswitchaifunc HoennFriendSwitchFunc
	goto_if_eq VAR_0x8004, HOENN_FRIEND_AIDAN, HoennSeek_EventScript_FriendBattleAidan
	goto_if_eq VAR_0x8004, HOENN_FRIEND_BROCK, HoennSeek_EventScript_FriendBattleBrock
	cleartrainerflag TRAINER_KAYLEE
	trainerbattle_no_intro TRAINER_KAYLEE, HoennSeek_Text_BridgetteDefeat
	goto HoennSeek_EventScript_FriendBattleEnded

HoennSeek_EventScript_FriendBattleAidan::
	cleartrainerflag TRAINER_DUDLEY
	trainerbattle_no_intro TRAINER_DUDLEY, HoennSeek_Text_AidanDefeat
	goto HoennSeek_EventScript_FriendBattleEnded

HoennSeek_EventScript_FriendBattleBrock::
	cleartrainerflag TRAINER_TERRY
	trainerbattle_no_intro TRAINER_TERRY, HoennSeek_Text_BrockDefeat
@ HoennFriendBattleEnded puts the friend back in VAR_0x8004 and HARD in 0x8005.
HoennSeek_EventScript_FriendBattleEnded::
	specialvar VAR_RESULT, HoennFriendBattleEnded
	goto_if_eq VAR_RESULT, B_OUTCOME_FORFEITED, HoennSeek_EventScript_FriendRan
	goto_if_eq VAR_0x8005, TRUE, HoennSeek_EventScript_FriendHardWon
	friendmsg HOENN_FRIEND_TEXT_WON
	release
	end

HoennSeek_EventScript_FriendHardWon::
	friendmsg HOENN_FRIEND_TEXT_HARD_WON
	release
	end

HoennSeek_EventScript_FriendRan::
	friendmsg HOENN_FRIEND_TEXT_RAN
	release
	end

HoennSeek_EventScript_FriendDecline::
	friendmsg HOENN_FRIEND_TEXT_DECLINE
	release
	end

HoennSeek_EventScript_FriendToday::
	friendmsg HOENN_FRIEND_TEXT_TODAY
	release
	end

HoennSeek_EventScript_FriendNameLine::
	friendmsg HOENN_FRIEND_TEXT_NAME
	return

@ BROCK sizes you up: what type of Pokémon are you into? (B picks NORMAL's
@ answer too: he doesn't take no for an answer.)
HoennSeek_EventScript_BrockTypeQuestion::
	message HoennSeek_Text_BrockTypeQuestion
	waitmessage
	dynmultichoice 18, 0, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_TypeNormal, HoennSeek_Text_TypeFire, HoennSeek_Text_TypeWater, HoennSeek_Text_TypeGrass, HoennSeek_Text_TypeElectric, HoennSeek_Text_TypeIce, HoennSeek_Text_TypeFighting, HoennSeek_Text_TypePoison, HoennSeek_Text_TypeGround, HoennSeek_Text_TypeFlying, HoennSeek_Text_TypePsychic, HoennSeek_Text_TypeBug, HoennSeek_Text_TypeRock, HoennSeek_Text_TypeGhost, HoennSeek_Text_TypeDragon, HoennSeek_Text_TypeDark, HoennSeek_Text_TypeSteel, HoennSeek_Text_TypeFairy
	copyvar VAR_0x8005, VAR_RESULT
	specialvar VAR_RESULT, HoennFriendTypeReaction
	call_if_eq VAR_RESULT, TRUE, HoennSeek_EventScript_BrockTypeShared
	call_if_eq VAR_RESULT, FALSE, HoennSeek_EventScript_BrockTypeOther
	setvar VAR_0x8004, HOENN_FRIEND_BROCK
	return

HoennSeek_EventScript_BrockTypeShared::
	msgbox HoennSeek_Text_BrockTypeShared, MSGBOX_DEFAULT
	return

HoennSeek_EventScript_BrockTypeOther::
	msgbox HoennSeek_Text_BrockTypeOther, MSGBOX_DEFAULT
	return

HoennSeek_Text_FriendNormal:
	.string "NORMAL$"

HoennSeek_Text_FriendHard:
	.string "HARD$"

HoennSeek_Text_TypeNormal:
	.string "NORMAL$"
HoennSeek_Text_TypeFire:
	.string "FIRE$"
HoennSeek_Text_TypeWater:
	.string "WATER$"
HoennSeek_Text_TypeGrass:
	.string "GRASS$"
HoennSeek_Text_TypeElectric:
	.string "ELECTRIC$"
HoennSeek_Text_TypeIce:
	.string "ICE$"
HoennSeek_Text_TypeFighting:
	.string "FIGHTING$"
HoennSeek_Text_TypePoison:
	.string "POISON$"
HoennSeek_Text_TypeGround:
	.string "GROUND$"
HoennSeek_Text_TypeFlying:
	.string "FLYING$"
HoennSeek_Text_TypePsychic:
	.string "PSYCHIC$"
HoennSeek_Text_TypeBug:
	.string "BUG$"
HoennSeek_Text_TypeRock:
	.string "ROCK$"
HoennSeek_Text_TypeGhost:
	.string "GHOST$"
HoennSeek_Text_TypeDragon:
	.string "DRAGON$"
HoennSeek_Text_TypeDark:
	.string "DARK$"
HoennSeek_Text_TypeSteel:
	.string "STEEL$"
HoennSeek_Text_TypeFairy:
	.string "FAIRY$"

@ ---- EXPLORER BRIDGETTE (Route 111) ----
HoennSeek_Text_BridgetteMeet::
	.string "Oh! Hello! I didn't hear you\n"
	.string "over the wind.\p"
	.string "I'm BRIDGETTE. I came all the way\n"
	.string "from SINNOH to explore ruins.\p"
	.string "This one hums at night, did you\n"
	.string "know? I've been mapping it for\l"
	.string "weeks now.$"

HoennSeek_Text_BridgetteNameAidan::
	.string "Wait, your name's {PLAYER}?\n"
	.string "My boyfriend is named AIDAN too!$"

HoennSeek_Text_BridgetteNameBridget::
	.string "{PLAYER}? Hey, that's my name!\n"
	.string "Well, almost.$"

HoennSeek_Text_BridgetteAgain::
	.string "Back again? The ruins aren't\n"
	.string "going anywhere. Neither am I.\p"
	.string "Up for another battle?$"

HoennSeek_Text_BridgetteStronger::
	.string "Oh, it's you! I've been training\n"
	.string "between digs.\p"
	.string "My team's gotten stronger since\n"
	.string "our last battle. Want to see?$"

HoennSeek_Text_BridgetteAsk::
	.string "You look like a strong TRAINER.\n"
	.string "How about a battle?$"

HoennSeek_Text_BridgetteGo::
	.string "Let's go! I'll try not to get\n"
	.string "sand in your eyes.$"

HoennSeek_Text_BridgetteDefeat::
	.string "You won! I knew this desert was\n"
	.string "hiding something good.$"

HoennSeek_Text_BridgetteWon::
	.string "That was fun! Come back tomorrow.\n"
	.string "I'll have dug up a new strategy.$"

HoennSeek_Text_BridgetteRan::
	.string "Giving up already? The desert\n"
	.string "gets to everyone.\p"
	.string "Come back when you're ready.$"

HoennSeek_Text_BridgetteDecline::
	.string "That's fine. I'll be right\n"
	.string "here, digging away.$"

HoennSeek_Text_BridgetteToday::
	.string "I'm all battled out for today.\n"
	.string "The ruins come first!\p"
	.string "Come back tomorrow.$"

HoennSeek_Text_BridgetteHardWon::
	.string "Every one of them at their best,\n"
	.string "and you still dug your way out.\p"
	.string "I'm writing this one down in my\n"
	.string "field notes. In ink!$"

@ ---- SKYDIVER AIDAN (Route 119, the ledge above the Weather Institute) ----
HoennSeek_Text_AidanMeet::
	.string "Whoa, hey! Careful near the edge.\p"
	.string "I'm AIDAN. I skydive! I come up\n"
	.string "here to read the wind before\l"
	.string "a jump.\p"
	.string "The INSTITUTE folks tell me when\n"
	.string "the updrafts are good. Today's\l"
	.string "looking perfect.$"

HoennSeek_Text_AidanNameAidan::
	.string "Hold on, you're {PLAYER} too?\n"
	.string "That's my name!$"

HoennSeek_Text_AidanNameBridget::
	.string "{PLAYER}? No way. My girlfriend's\n"
	.string "named BRIDGETTE!$"

HoennSeek_Text_AidanAgain::
	.string "Back up here? The view's worth\n"
	.string "the climb, right?\p"
	.string "Want to battle before I jump?$"

HoennSeek_Text_AidanStronger::
	.string "Hey! I've been jumping every day.\n"
	.string "My team's gotten stronger too.\p"
	.string "Want to see how high we fly now?$"

HoennSeek_Text_AidanAsk::
	.string "You look like you've got guts.\n"
	.string "Battle me before my next jump?$"

HoennSeek_Text_AidanGo::
	.string "Three, two, one... Let's go!$"

HoennSeek_Text_AidanDefeat::
	.string "Whoa! That was a free fall!$"

HoennSeek_Text_AidanWon::
	.string "You knocked the wind right out\n"
	.string "of my sails!\p"
	.string "I'm done for today. Come find\n"
	.string "me tomorrow!$"

HoennSeek_Text_AidanHardWon::
	.string "Everyone at full strength and\n"
	.string "you still came out on top?\p"
	.string "That's the biggest rush I've had\n"
	.string "without a parachute!$"

HoennSeek_Text_AidanRan::
	.string "Pulled the cord early, huh?\n"
	.string "No shame in a safe landing.$"

HoennSeek_Text_AidanDecline::
	.string "Suit yourself! I'll be up here\n"
	.string "watching the clouds.$"

HoennSeek_Text_AidanToday::
	.string "I'm done battling for today.\n"
	.string "Saving my energy for the jump!\p"
	.string "Come back tomorrow.$"

@ ---- ATHLETE BROCK (Route 115, the north end of the beach) ----
HoennSeek_Text_BrockMeet::
	.string "HEY! You! Yeah, YOU!\p"
	.string "I'm BROCK! Not the gym leader!\n"
	.string "I train right here on the sand!$"

HoennSeek_Text_BrockNameBrock::
	.string "Wait, your name's {PLAYER}?!\p"
	.string "With a name like that, you must\n"
	.string "be a very good TRAINER!$"

HoennSeek_Text_BrockTypeQuestion::
	.string "Before anything else, tell me!\n"
	.string "What type of POKéMON are you into?$"

HoennSeek_Text_BrockTypeShared::
	.string "{STR_VAR_1}!! Now THAT'S taste!\p"
	.string "My {STR_VAR_2} is going to\n"
	.string "love you!$"

HoennSeek_Text_BrockTypeOther::
	.string "{STR_VAR_1}? Ha! None of mine are,\n"
	.string "but I respect a TRAINER who\l"
	.string "knows what they like!$"

HoennSeek_Text_BrockAgain::
	.string "You came back! I knew it!\p"
	.string "Want to go another round?$"

HoennSeek_Text_BrockAgainBest::
	.string "BEST FRIEND!! You're here!\n"
	.string "I've been training all day!\p"
	.string "Another round? Say yes!$"

HoennSeek_Text_BrockStronger::
	.string "I've been doing sprints in the\n"
	.string "sand since our last battle!\p"
	.string "My team got stronger too. Ready\n"
	.string "to feel it?$"

HoennSeek_Text_BrockAsk::
	.string "Now that we know each other,\n"
	.string "how about a battle?$"

HoennSeek_Text_BrockGo::
	.string "Give me everything you've got!$"

HoennSeek_Text_BrockDefeat::
	.string "WHAT A BATTLE!!\n"
	.string "My heart is pounding!$"

HoennSeek_Text_BrockWon::
	.string "That settles it. From now on,\n"
	.string "you're my BEST FRIEND!\p"
	.string "Come back tomorrow, best friend!$"

HoennSeek_Text_BrockHardWon::
	.string "Every one of my team at full\n"
	.string "power, and you beat them all!\p"
	.string "Best friend for life.\n"
	.string "No take-backs!$"

HoennSeek_Text_BrockRan::
	.string "Taking a breather? Smart! Come\n"
	.string "back when your legs are ready!$"

HoennSeek_Text_BrockDecline::
	.string "No? That's fine! I'll be right\n"
	.string "here doing push-ups!$"

HoennSeek_Text_BrockToday::
	.string "I'm all trained out for today,\n"
	.string "best friend!\p"
	.string "Come back tomorrow!$"
