@ Hoenn & Seek 2.1-32: ANAMARIA, the professional organizer (Cherrygrove House 3).
@ She sorts your PC boxes BY NUMBER, BY TYPE or BY LEVEL (src/hoenn_organizer.c,
@ HoennOrganizeBoxes), leaving your first few boxes alone if you ask; she remembers
@ how many (VAR_HOENN_ORGANIZER_SKIP = boxes + 1, 0 = never asked). Her SKITTY plays
@ its cry (HoennSeek_EventScript_HousePet). The youngster who used to live here drops
@ by once to complain, then goes (FLAG_HOENN_HIDE_ORGANIZER_YOUNGSTER).
@ Her lines are drafts until the 2.1-36c dialogue pass.

HoennSeek_EventScript_Anamaria::
	lock
	faceplayer
	goto_if_set FLAG_HOENN_MET_ANAMARIA, HoennSeek_EventScript_AnamariaAgain
	msgbox HoennSeek_Text_AnamariaIntro, MSGBOX_DEFAULT
	setflag FLAG_HOENN_MET_ANAMARIA
	goto HoennSeek_EventScript_AnamariaMenu

HoennSeek_EventScript_AnamariaAgain::
	msgbox HoennSeek_Text_AnamariaAgain, MSGBOX_DEFAULT
HoennSeek_EventScript_AnamariaMenu::
	message HoennSeek_Text_AnamariaHow
	waitmessage
	dynmultichoice 17, 0, FALSE, 4, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_OrganizeByNumber, HoennSeek_Text_OrganizeByType, HoennSeek_Text_OrganizeByLevel, HoennSeek_Text_OrganizeNotNow
	goto_if_gt VAR_RESULT, HOENN_ORGANIZE_BY_LEVEL, HoennSeek_EventScript_AnamariaBye
	copyvar VAR_0x8006, VAR_RESULT
	goto_if_eq VAR_HOENN_ORGANIZER_SKIP, 0, HoennSeek_EventScript_AnamariaAskSkip
	@ "Like last time?"
	copyvar VAR_0x8005, VAR_HOENN_ORGANIZER_SKIP
	subvar VAR_0x8005, 1
	goto_if_eq VAR_0x8005, 0, HoennSeek_EventScript_AnamariaLastAll
	goto_if_eq VAR_0x8005, 1, HoennSeek_EventScript_AnamariaLastOne
	buffernumberstring STR_VAR_1, VAR_0x8005
	msgbox HoennSeek_Text_AnamariaLastSome, MSGBOX_YESNO
	goto HoennSeek_EventScript_AnamariaLastAnswer

HoennSeek_EventScript_AnamariaLastAll::
	msgbox HoennSeek_Text_AnamariaLastAll, MSGBOX_YESNO
	goto HoennSeek_EventScript_AnamariaLastAnswer

HoennSeek_EventScript_AnamariaLastOne::
	msgbox HoennSeek_Text_AnamariaLastOne, MSGBOX_YESNO
HoennSeek_EventScript_AnamariaLastAnswer::
	goto_if_eq VAR_RESULT, YES, HoennSeek_EventScript_AnamariaConfirm
HoennSeek_EventScript_AnamariaAskSkip::
	message HoennSeek_Text_AnamariaSkipHowMany
	waitmessage
	dynmultichoice 17, 0, FALSE, 6, 0, DYN_MULTICHOICE_CB_NONE, HoennSeek_Text_OrganizeSkip0, HoennSeek_Text_OrganizeSkip1, HoennSeek_Text_OrganizeSkip2, HoennSeek_Text_OrganizeSkip3, HoennSeek_Text_OrganizeSkip4, HoennSeek_Text_OrganizeSkip5, HoennSeek_Text_OrganizeSkip6, HoennSeek_Text_OrganizeSkip7, HoennSeek_Text_OrganizeSkip8, HoennSeek_Text_OrganizeSkip9, HoennSeek_Text_OrganizeSkip10, HoennSeek_Text_OrganizeSkip11, HoennSeek_Text_OrganizeSkip12, HoennSeek_Text_OrganizeSkip13
	goto_if_gt VAR_RESULT, 13, HoennSeek_EventScript_AnamariaBye
	copyvar VAR_0x8005, VAR_RESULT
	copyvar VAR_HOENN_ORGANIZER_SKIP, VAR_RESULT
	addvar VAR_HOENN_ORGANIZER_SKIP, 1
HoennSeek_EventScript_AnamariaConfirm::
	goto_if_ne VAR_0x8006, HOENN_ORGANIZE_BY_TYPE, HoennSeek_EventScript_AnamariaSort
	msgbox HoennSeek_Text_AnamariaTypeLabels, MSGBOX_YESNO
	goto_if_ne VAR_RESULT, YES, HoennSeek_EventScript_AnamariaBye
HoennSeek_EventScript_AnamariaSort::
	msgbox HoennSeek_Text_AnamariaLeaveItToMe, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	playmoncry SPECIES_SKITTY, CRY_MODE_NORMAL
	waitmoncry
	copyvar VAR_0x8004, VAR_0x8006
	specialvar VAR_RESULT, HoennOrganizeBoxes
	playse SE_PC_LOGIN
	waitse
	fadescreen FADE_FROM_BLACK
	goto_if_eq VAR_RESULT, HOENN_ORGANIZE_EMPTY, HoennSeek_EventScript_AnamariaEmpty
	goto_if_eq VAR_RESULT, HOENN_ORGANIZE_FAILED, HoennSeek_EventScript_AnamariaFailed
	goto_if_eq VAR_RESULT, HOENN_ORGANIZE_PACKED, HoennSeek_EventScript_AnamariaPacked
	goto_if_eq VAR_0x8006, HOENN_ORGANIZE_BY_TYPE, HoennSeek_EventScript_AnamariaDoneType
	goto_if_eq VAR_0x8006, HOENN_ORGANIZE_BY_LEVEL, HoennSeek_EventScript_AnamariaDoneLevel
	msgbox HoennSeek_Text_AnamariaDoneNumber, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_AnamariaDoneType::
	msgbox HoennSeek_Text_AnamariaDoneType, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_AnamariaDoneLevel::
	msgbox HoennSeek_Text_AnamariaDoneLevel, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_AnamariaPacked::
	msgbox HoennSeek_Text_AnamariaPacked, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_AnamariaEmpty::
	msgbox HoennSeek_Text_AnamariaEmpty, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_AnamariaFailed::
	msgbox HoennSeek_Text_AnamariaFailed, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_AnamariaBye::
	msgbox HoennSeek_Text_AnamariaBye, MSGBOX_DEFAULT
	release
	end

HoennSeek_EventScript_OrganizerYoungster::
	lock
	faceplayer
	msgbox HoennSeek_Text_OrganizerYoungster, MSGBOX_DEFAULT
	closemessage
	fadescreen FADE_TO_BLACK
	setflag FLAG_HOENN_HIDE_ORGANIZER_YOUNGSTER
	removeobject LOCALID_HOENN_ORGANIZER_YOUNGSTER
	fadescreen FADE_FROM_BLACK
	release
	end

HoennSeek_Text_AnamariaIntro:
	.string "Oh! A visitor! Mind the labels,\n"
	.string "please. I'm ANAMARIA.\p"
	.string "I'm a professional organizer. I've\n"
	.string "only just moved in, and I've already\l"
	.string "labeled everything twice.\p"
	.string "You're a TRAINER, so I bet your PC\n"
	.string "boxes are a jumble.\p"
	.string "Let me sort them for you! The\n"
	.string "POKéMON with you stay with you.$"

HoennSeek_Text_AnamariaAgain:
	.string "Back for a tidy-up? Your boxes\n"
	.string "just can't stay neat, can they?$"

HoennSeek_Text_AnamariaHow:
	.string "How shall I sort your boxes?$"

HoennSeek_Text_OrganizeByNumber:
	.string "BY NUMBER$"

HoennSeek_Text_OrganizeByType:
	.string "BY TYPE$"

HoennSeek_Text_OrganizeByLevel:
	.string "BY LEVEL$"

HoennSeek_Text_OrganizeNotNow:
	.string "NOT NOW$"

HoennSeek_Text_AnamariaLastAll:
	.string "Every box, like last time?$"

HoennSeek_Text_AnamariaLastOne:
	.string "Leave your first box alone,\n"
	.string "like last time?$"

HoennSeek_Text_AnamariaLastSome:
	.string "Leave your first {STR_VAR_1} boxes\n"
	.string "alone, like last time?$"

HoennSeek_Text_AnamariaSkipHowMany:
	.string "How many of your first boxes\n"
	.string "should I leave alone?$"

HoennSeek_Text_OrganizeSkip0:
	.string "NONE$"

HoennSeek_Text_OrganizeSkip1:
	.string "1 BOX$"

HoennSeek_Text_OrganizeSkip2:
	.string "2 BOXES$"

HoennSeek_Text_OrganizeSkip3:
	.string "3 BOXES$"

HoennSeek_Text_OrganizeSkip4:
	.string "4 BOXES$"

HoennSeek_Text_OrganizeSkip5:
	.string "5 BOXES$"

HoennSeek_Text_OrganizeSkip6:
	.string "6 BOXES$"

HoennSeek_Text_OrganizeSkip7:
	.string "7 BOXES$"

HoennSeek_Text_OrganizeSkip8:
	.string "8 BOXES$"

HoennSeek_Text_OrganizeSkip9:
	.string "9 BOXES$"

HoennSeek_Text_OrganizeSkip10:
	.string "10 BOXES$"

HoennSeek_Text_OrganizeSkip11:
	.string "11 BOXES$"

HoennSeek_Text_OrganizeSkip12:
	.string "12 BOXES$"

HoennSeek_Text_OrganizeSkip13:
	.string "13 BOXES$"

HoennSeek_Text_AnamariaTypeLabels:
	.string "Each type gets a box of its own,\n"
	.string "with a label and a wallpaper.\p"
	.string "That means new names on the boxes\n"
	.string "I fill. Is that all right?$"

HoennSeek_Text_AnamariaLeaveItToMe:
	.string "Leave it to me! SKITTY, fetch\n"
	.string "the label maker!$"

HoennSeek_Text_AnamariaDoneNumber:
	.string "All done! {STR_VAR_1} POKéMON, in POKéDEX\n"
	.string "order, eggs at the very end.$"

HoennSeek_Text_AnamariaDoneType:
	.string "All done! {STR_VAR_1} POKéMON, one type\n"
	.string "to a box, and every box labeled.$"

HoennSeek_Text_AnamariaDoneLevel:
	.string "All done! {STR_VAR_1} POKéMON, strongest\n"
	.string "first, eggs at the very end.$"

HoennSeek_Text_AnamariaPacked:
	.string "All done! {STR_VAR_1} POKéMON by type.\p"
	.string "Your boxes are too full to give each\n"
	.string "type its own box, so I packed them\l"
	.string "in tight. Still labeled, though!$"

HoennSeek_Text_AnamariaEmpty:
	.string "Hm? Those boxes are empty. Nothing\n"
	.string "to sort! Come back when they're not.$"

HoennSeek_Text_AnamariaFailed:
	.string "Hold on. Something doesn't add up,\n"
	.string "so I've stopped.\p"
	.string "Better safe than sorry with\n"
	.string "other people's POKéMON!$"

HoennSeek_Text_AnamariaBye:
	.string "Whenever you like! A tidy box is\n"
	.string "a happy box.$"

HoennSeek_Text_OrganizerYoungster:
	.string "Hey! This is my house! I just came\n"
	.string "home and it's ALL labeled!\p"
	.string "Even my bed has a label. It says\n"
	.string "BED. I KNOW it's a bed!\p"
	.string "And she put my comics in number\n"
	.string "order... Actually, that's handy.\p"
	.string "...I'm going to go stay with my\n"
	.string "friend down the road.$"

@ ETHAN's or LYRA's contact line once you've met her (src/pokenav_match_call_data.c),
@ until their first milestone line takes over. CHERRYGROVE is next door to NEW BARK.
HoennSeek_Text_EthanPhoneAnamaria::
	.string "ETHAN: You met the lady in\n"
	.string "CHERRYGROVE? She came over to NEW\l"
	.string "BARK and labeled our mailbox.\p"
	.string "It says MAIL. My mom loves her.$"

HoennSeek_Text_LyraPhoneAnamaria::
	.string "LYRA: You met the lady in\n"
	.string "CHERRYGROVE? She came over to NEW\l"
	.string "BARK and labeled our mailbox.\p"
	.string "It says MAIL. My mom loves her.$"
