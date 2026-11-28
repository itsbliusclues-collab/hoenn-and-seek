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

@ ==== 2.1-05: the SEEKER'S JOURNAL ====
@ The key item's screen (src/hoenn_journal.c explains the states): pick a page,
@ then an entry, read it, and the list opens again on the same row. Run from
@ the bag or SELECT (ItemUseOutOfBattle_HoennSeekersJournal, src/item_use.c).
@ VAR_0x8004 = page, VAR_0x8005 = entry (its place on the page),
@ VAR_0x8006 = the page row to start on.
HoennSeek_EventScript_SeekersJournal::
	lockall
	setvar VAR_0x8006, 0
HoennSeek_EventScript_JournalPages:
	message HoennSeek_Text_JournalOpen
	waitmessage
	special HoennJournalPushPages
	dynmultistack 0, 0, FALSE, 6, FALSE, VAR_0x8006, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, HoennSeek_EventScript_JournalClose
	goto_if_ge VAR_RESULT, HOENN_JOURNAL_PAGE_COUNT, HoennSeek_EventScript_JournalClose
	copyvar VAR_0x8004, VAR_RESULT
	copyvar VAR_0x8006, VAR_RESULT
	setvar VAR_0x8005, 0
HoennSeek_EventScript_JournalEntries:
	special HoennJournalPushEntries @ STR_VAR_1 page, STR_VAR_2 found, STR_VAR_3 total
	message HoennSeek_Text_JournalPage
	waitmessage
	dynmultistack 0, 0, FALSE, 6, FALSE, VAR_0x8005, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, HoennSeek_EventScript_JournalPages
	goto_if_eq VAR_RESULT, HOENN_JOURNAL_MENU_BACK, HoennSeek_EventScript_JournalPages
	copyvar VAR_0x8005, VAR_RESULT
	callnative HoennSeek_JournalEntryText
	callstd MSGBOX_DEFAULT
	goto HoennSeek_EventScript_JournalEntries

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

	.include "data/scripts/hoenn_journal_texts.inc"
