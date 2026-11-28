@ ==== 2.1-31: DAN, the history broadcaster ====
@ PEWTER MUSEUM 2F (LOCALID_HOENN_DAN). One installment per talk; finishing an
@ episode sets FLAG_HOENN_WEEKLY_DAN, so the next starts with the new week.
@ Every text is in data/hoenn/dan_episodes.json (generated into
@ hoenn_dan_texts.inc); the state is in src/hoenn_dan.c, whose header explains it.

HoennSeek_EventScript_Dan::
	lock
	faceplayer
	goto_if_set FLAG_HOENN_MET_DAN, HoennSeek_EventScript_DanMet
	msgbox HoennSeek_Text_Dan_Intro, MSGBOX_DEFAULT
	setflag FLAG_HOENN_MET_DAN
HoennSeek_EventScript_DanMet:
	specialvar VAR_RESULT, HoennDanState
	switch VAR_RESULT
	case HOENN_DAN_NEW, HoennSeek_EventScript_DanNew
	case HOENN_DAN_CONTINUE, HoennSeek_EventScript_DanContinue
	case HOENN_DAN_WAIT, HoennSeek_EventScript_DanWait
	goto HoennSeek_EventScript_DanDone

HoennSeek_EventScript_DanNew:
	msgbox HoennSeek_Text_Dan_NewEpisode, MSGBOX_DEFAULT
	goto HoennSeek_EventScript_DanTell

HoennSeek_EventScript_DanContinue:
	msgbox HoennSeek_Text_Dan_Continue, MSGBOX_DEFAULT
HoennSeek_EventScript_DanTell:
	callnative HoennSeek_DanText
	.byte HOENN_DAN_TEXT_PART
	callstd MSGBOX_DEFAULT
	special HoennDanPartDone
	goto_if_eq VAR_RESULT, TRUE, HoennSeek_EventScript_DanEpisodeEnd
	msgbox HoennSeek_Text_Dan_Pause, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_DanEpisodeEnd:
	msgbox HoennSeek_Text_Dan_EpisodeEnd, MSGBOX_DEFAULT
	callnative HoennSeek_DanText
	.byte HOENN_DAN_TEXT_TEASER
	callstd MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_DanWait:
	msgbox HoennSeek_Text_Dan_Wait, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennSeek_EventScript_DanBye
	goto HoennSeek_EventScript_DanReplay

HoennSeek_EventScript_DanDone:
	msgbox HoennSeek_Text_Dan_Done, MSGBOX_YESNO
	goto_if_eq VAR_RESULT, NO, HoennSeek_EventScript_DanBye
HoennSeek_EventScript_DanReplay:
	message HoennSeek_Text_Dan_Which
	waitmessage
	special HoennDanPushEpisodes
	dynmultistack 12, 0, FALSE, 6, FALSE, 0, DYN_MULTICHOICE_CB_NONE
	goto_if_eq VAR_RESULT, MULTI_B_PRESSED, HoennSeek_EventScript_DanBye
	copyvar VAR_0x8005, VAR_RESULT
	setvar VAR_0x8006, 0
HoennSeek_EventScript_DanReplayPart:
	callnative HoennSeek_DanText
	.byte HOENN_DAN_TEXT_REPLAY
	callstd MSGBOX_DEFAULT
	specialvar VAR_RESULT, HoennDanReplayNext
	goto_if_eq VAR_RESULT, TRUE, HoennSeek_EventScript_DanReplayPart
	msgbox HoennSeek_Text_Dan_ReplayEnd, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_DanBye:
	msgbox HoennSeek_Text_Dan_Bye, MSGBOX_DEFAULT
	release
	end

@ Brendan's event call (HOENN_CALL_DAN, src/hoenn_2_1_lane_a.c): you've heard
@ three episodes to the end. Brendan & May train in Kanto until Wallace.
HoennSeek_EventScript_CallDan::
	setflag FLAG_HOENN_CALL_DAN
	setspeaker HoennSeek_Text_NameBrendan
	pokenavcall HoennSeek_Text_Dan_CallBrendan
	goto HoennSeek_EventScript_CallEnd
