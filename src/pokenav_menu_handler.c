#include "global.h"
#include "pokenav.h"
#include "event_data.h"
#include "main.h"
#include "sound.h"
#include "constants/songs.h"

struct Pokenav_Menu
{
    u16 menuType;
    s16 cursorPos;
    u16 currMenuItem;
    u16 helpBarIndex;
    u32 menuId;
    u32 (*callback)(struct Pokenav_Menu *);
#if IS_HNS
    u8 mainItems[MAX_POKENAV_MENUITEMS]; // POKENAV_MENU_TYPE_HOENN_MAIN's buttons (BuildHoennMainMenu)
    u8 mainCount;
#endif
};

static bool32 UpdateMenuCursorPos(struct Pokenav_Menu *);
static void ReturnToConditionMenu(struct Pokenav_Menu *);
static void ReturnToMainMenu(struct Pokenav_Menu *);
static u32 GetMenuId(struct Pokenav_Menu *);
static void SetMenuIdAndCB(struct Pokenav_Menu *, u32);
static u32 CB2_ReturnToConditionMenu(struct Pokenav_Menu *);
static u32 CB2_ReturnToMainMenu(struct Pokenav_Menu *);
static u32 HandleConditionSearchMenuInput(struct Pokenav_Menu *);
static u32 HandleConditionMenuInput(struct Pokenav_Menu *);
static u32 HandleCantOpenRibbonsInput(struct Pokenav_Menu *);
static u32 HandleMainMenuInputEndTutorial(struct Pokenav_Menu *);
static u32 HandleMainMenuInputTutorial(struct Pokenav_Menu *);
static u32 HandleMainMenuInput(struct Pokenav_Menu *);
static u32 (*GetMainMenuInputHandler(void))(struct Pokenav_Menu *);
static void SetMenuInputHandler(struct Pokenav_Menu *);

// Number of entries - 1 for that menu type
static const u8 sLastCursorPositions[] =
{
    [POKENAV_MENU_TYPE_DEFAULT]           = 2,
    [POKENAV_MENU_TYPE_UNLOCK_MC]         = 3,
    [POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS] = 4,
#if IS_HNS
    [POKENAV_MENU_TYPE_UNLOCK_MC_RADIO]                = 4,
    [POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS_RADIO]        = 5,
    [POKENAV_MENU_TYPE_DEFAULT_NO_COND]                = 1,
    [POKENAV_MENU_TYPE_UNLOCK_MC_NO_COND]              = 2,
#endif
    [POKENAV_MENU_TYPE_CONDITION]         = 2,
    [POKENAV_MENU_TYPE_CONDITION_SEARCH]  = 5
};

static const u8 sMenuItems[][MAX_POKENAV_MENUITEMS] =
{
    [POKENAV_MENU_TYPE_DEFAULT] =
    {
        POKENAV_MENUITEM_MAP,
        POKENAV_MENUITEM_CONDITION,
        [2 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
    [POKENAV_MENU_TYPE_UNLOCK_MC] =
    {
        POKENAV_MENUITEM_MAP,
        POKENAV_MENUITEM_CONDITION,
        POKENAV_MENUITEM_MATCH_CALL,
        [3 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
    [POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS] =
    {
        POKENAV_MENUITEM_MAP,
        POKENAV_MENUITEM_CONDITION,
        POKENAV_MENUITEM_MATCH_CALL,
        POKENAV_MENUITEM_RIBBONS,
        [4 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
#if IS_HNS
    [POKENAV_MENU_TYPE_UNLOCK_MC_RADIO] =
    {
        POKENAV_MENUITEM_MAP,
        POKENAV_MENUITEM_CONDITION,
        POKENAV_MENUITEM_MATCH_CALL,
        POKENAV_MENUITEM_RADIO,
        [4 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
    [POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS_RADIO] =
    {
        POKENAV_MENUITEM_MAP,
        POKENAV_MENUITEM_CONDITION,
        POKENAV_MENUITEM_MATCH_CALL,
        POKENAV_MENUITEM_RADIO,
        POKENAV_MENUITEM_RIBBONS,
        POKENAV_MENUITEM_SWITCH_OFF
    },
    [POKENAV_MENU_TYPE_DEFAULT_NO_COND] =
    {
        POKENAV_MENUITEM_MAP,
        [1 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
    [POKENAV_MENU_TYPE_UNLOCK_MC_NO_COND] =
    {
        POKENAV_MENUITEM_MAP,
        POKENAV_MENUITEM_MATCH_CALL,
        [2 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
#endif
    [POKENAV_MENU_TYPE_CONDITION] =
    {
        POKENAV_MENUITEM_CONDITION_PARTY,
        POKENAV_MENUITEM_CONDITION_SEARCH,
        POKENAV_MENUITEM_CONDITION_CANCEL,
        [3 ... MAX_POKENAV_MENUITEMS - 1] = POKENAV_MENUITEM_SWITCH_OFF
    },
    [POKENAV_MENU_TYPE_CONDITION_SEARCH] =
    {
        POKENAV_MENUITEM_CONDITION_SEARCH_COOL,
        POKENAV_MENUITEM_CONDITION_SEARCH_BEAUTY,
        POKENAV_MENUITEM_CONDITION_SEARCH_CUTE,
        POKENAV_MENUITEM_CONDITION_SEARCH_SMART,
        POKENAV_MENUITEM_CONDITION_SEARCH_TOUGH,
        POKENAV_MENUITEM_CONDITION_SEARCH_CANCEL
    },
};

#if IS_HNS
// Hoenn & Seek 2.1-06b: HnS's main menu is built here, from the flags, in one
// place, instead of one enumerated menu type per combination (the JOURNAL
// button would have doubled them). Same order and conditions as HnS's types,
// with the JOURNAL after RADIO once Elm's aide has given it.
static void BuildHoennMainMenu(struct Pokenav_Menu *menu)
{
    u32 n = 0;
    bool32 matchCall = FlagGet(FLAG_ADDED_MATCH_CALL_TO_POKENAV);

    menu->mainItems[n++] = POKENAV_MENUITEM_MAP;
    // HnS only dropped CONDITION from its map-only and map + CALL menus, so
    // it still shows with RADIO or RIBBONS before FLAG_ENABLE_CONDITION.
    if (FlagGet(FLAG_ENABLE_CONDITION)
     || (matchCall && (FlagGet(FLAG_ENABLE_RADIO) || FlagGet(FLAG_SYS_RIBBON_GET))))
        menu->mainItems[n++] = POKENAV_MENUITEM_CONDITION;
    if (matchCall)
        menu->mainItems[n++] = POKENAV_MENUITEM_MATCH_CALL;
    if (matchCall && FlagGet(FLAG_ENABLE_RADIO))
        menu->mainItems[n++] = POKENAV_MENUITEM_RADIO;
    if (FlagGet(FLAG_HOENN_GOT_SEEKERS_JOURNAL))
        menu->mainItems[n++] = POKENAV_MENUITEM_JOURNAL;
    if (matchCall && FlagGet(FLAG_SYS_RIBBON_GET))
        menu->mainItems[n++] = POKENAV_MENUITEM_RIBBONS;
    menu->mainItems[n++] = POKENAV_MENUITEM_SWITCH_OFF;
    menu->mainCount = n;
}
#endif

// The item on row `pos` of the menu and its last row (the menu type's tables,
// or the built list for HnS's main menu).
static u8 MenuItemAt(struct Pokenav_Menu *menu, s32 pos)
{
#if IS_HNS
    if (menu->menuType == POKENAV_MENU_TYPE_HOENN_MAIN)
        return menu->mainItems[pos];
#endif
    return sMenuItems[menu->menuType][pos];
}

static s16 LastCursorPos(struct Pokenav_Menu *menu)
{
#if IS_HNS
    if (menu->menuType == POKENAV_MENU_TYPE_HOENN_MAIN)
        return menu->mainCount - 1;
#endif
    return LastCursorPos(menu);
}

// The cursor position is a row index into sMenuItems, which is not the same as
// the menu item ID once optional entries (Radio, no-Condition) shift the rows.
static s16 GetCursorPosOfMenuItem(struct Pokenav_Menu *menu, u8 menuItem)
{
    s32 i;

    for (i = 0; i <= LastCursorPos(menu); i++)
    {
        if (MenuItemAt(menu, i) == menuItem)
            return i;
    }

    return 0;
}

static u8 GetPokenavMainMenuType(void)
{
    u8 menuType = POKENAV_MENU_TYPE_DEFAULT;

#if IS_HNS
    return POKENAV_MENU_TYPE_HOENN_MAIN; // 2.1-06b: built from the flags (BuildHoennMainMenu)
#endif

    if (FlagGet(FLAG_ADDED_MATCH_CALL_TO_POKENAV))
    {
        menuType = POKENAV_MENU_TYPE_UNLOCK_MC;

        if (FlagGet(FLAG_SYS_RIBBON_GET))
            menuType = POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS;
    }

#if IS_HNS
    if (FlagGet(FLAG_ENABLE_RADIO))
    {
        if (menuType == POKENAV_MENU_TYPE_UNLOCK_MC)
            menuType = POKENAV_MENU_TYPE_UNLOCK_MC_RADIO;
        else if (menuType == POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS)
            menuType = POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS_RADIO;
    }

    if (!FlagGet(FLAG_ENABLE_CONDITION))
    {
        if (menuType == POKENAV_MENU_TYPE_DEFAULT)
            menuType = POKENAV_MENU_TYPE_DEFAULT_NO_COND;
        else if (menuType == POKENAV_MENU_TYPE_UNLOCK_MC)
            menuType = POKENAV_MENU_TYPE_UNLOCK_MC_NO_COND;
    }
#endif

    return menuType;
}

static void SetMainMenuType(struct Pokenav_Menu *menu)
{
    menu->menuType = GetPokenavMainMenuType();
#if IS_HNS
    BuildHoennMainMenu(menu);
#endif
}

bool32 PokenavCallback_Init_MainMenuCursorOnMap(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    SetMainMenuType(menu);
    menu->cursorPos = GetCursorPosOfMenuItem(menu, POKENAV_MENUITEM_MAP);
    menu->currMenuItem = POKENAV_MENUITEM_MAP;
    menu->helpBarIndex = HELPBAR_NONE;
    SetMenuInputHandler(menu);
    return TRUE;
}

bool32 PokenavCallback_Init_MainMenuCursorOnMatchCall(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    SetMainMenuType(menu);
    menu->cursorPos = GetCursorPosOfMenuItem(menu, POKENAV_MENUITEM_MATCH_CALL);
    menu->currMenuItem = POKENAV_MENUITEM_MATCH_CALL;
    menu->helpBarIndex = HELPBAR_NONE;
    SetMenuInputHandler(menu);
    return TRUE;
}

bool32 PokenavCallback_Init_MainMenuCursorOnRibbons(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    SetMainMenuType(menu);
    menu->cursorPos = GetCursorPosOfMenuItem(menu, POKENAV_MENUITEM_RIBBONS);
    menu->currMenuItem = POKENAV_MENUITEM_RIBBONS;
    SetMenuInputHandler(menu);
    return TRUE;
}

#if IS_HNS
bool32 PokenavCallback_Init_MainMenuCursorOnRadio(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    SetMainMenuType(menu);
    menu->cursorPos = GetCursorPosOfMenuItem(menu, POKENAV_MENUITEM_RADIO);
    menu->currMenuItem = POKENAV_MENUITEM_RADIO;
    menu->helpBarIndex = HELPBAR_NONE;
    SetMenuInputHandler(menu);
    return TRUE;
}
#endif

#if IS_HNS
bool32 PokenavCallback_Init_MainMenuCursorOnJournal(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    SetMainMenuType(menu);
    menu->cursorPos = GetCursorPosOfMenuItem(menu, POKENAV_MENUITEM_JOURNAL);
    menu->currMenuItem = POKENAV_MENUITEM_JOURNAL;
    menu->helpBarIndex = HELPBAR_NONE;
    SetMenuInputHandler(menu);
    return TRUE;
}
#endif

bool32 PokenavCallback_Init_ConditionMenu(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    menu->menuType = POKENAV_MENU_TYPE_CONDITION;
    menu->cursorPos = 0;   //party
    menu->currMenuItem = POKENAV_MENUITEM_CONDITION_PARTY;
    menu->helpBarIndex = HELPBAR_NONE;
    SetMenuInputHandler(menu);
    return TRUE;
}

bool32 PokenavCallback_Init_ConditionSearchMenu(void)
{
    struct Pokenav_Menu *menu = AllocSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER, sizeof(struct Pokenav_Menu));
    if (!menu)
        return FALSE;

    menu->menuType = POKENAV_MENU_TYPE_CONDITION_SEARCH;
    menu->cursorPos = GetSelectedConditionSearch();
    menu->currMenuItem = menu->cursorPos + POKENAV_MENUITEM_CONDITION_SEARCH_COOL;
    menu->helpBarIndex = HELPBAR_NONE;
    SetMenuInputHandler(menu);
    return TRUE;
}

static void SetMenuInputHandler(struct Pokenav_Menu *menu)
{
    switch (menu->menuType)
    {
    case POKENAV_MENU_TYPE_DEFAULT:
        SetPokenavMode(POKENAV_MODE_NORMAL);
        // fallthrough
    case POKENAV_MENU_TYPE_UNLOCK_MC:
    case POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS:
#if IS_HNS
    case POKENAV_MENU_TYPE_UNLOCK_MC_RADIO:
    case POKENAV_MENU_TYPE_UNLOCK_MC_RIBBONS_RADIO:
    case POKENAV_MENU_TYPE_DEFAULT_NO_COND:
    case POKENAV_MENU_TYPE_UNLOCK_MC_NO_COND:
    case POKENAV_MENU_TYPE_HOENN_MAIN:
#endif
        menu->callback = GetMainMenuInputHandler();
        break;
    case POKENAV_MENU_TYPE_CONDITION:
        menu->callback = HandleConditionMenuInput;
        break;
    case POKENAV_MENU_TYPE_CONDITION_SEARCH:
        menu->callback = HandleConditionSearchMenuInput;
        break;
    }
}

static u32 (*GetMainMenuInputHandler(void))(struct Pokenav_Menu *)
{
    switch (GetPokenavMode())
    {
    default:
    case POKENAV_MODE_NORMAL:
        return HandleMainMenuInput;
    case POKENAV_MODE_FORCE_CALL_READY:
        return HandleMainMenuInputTutorial;
    case POKENAV_MODE_FORCE_CALL_EXIT:
        return HandleMainMenuInputEndTutorial;
    }
}

u32 GetMenuHandlerCallback(void)
{
    struct Pokenav_Menu *menu = GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
    return menu->callback(menu);
}

void FreeMenuHandlerSubstruct1(void)
{
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
}

static u32 HandleMainMenuInput(struct Pokenav_Menu *menu)
{
    if (UpdateMenuCursorPos(menu))
        return POKENAV_MENU_FUNC_MOVE_CURSOR;

    if (JOY_NEW(A_BUTTON))
    {
        switch (MenuItemAt(menu, menu->cursorPos))
        {
        case POKENAV_MENUITEM_MAP:
            menu->helpBarIndex = gSaveBlock2Ptr->regionMapZoom ? HELPBAR_MAP_ZOOMED_IN : HELPBAR_MAP_ZOOMED_OUT;
            SetMenuIdAndCB(menu, POKENAV_REGION_MAP);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        case POKENAV_MENUITEM_CONDITION:
            menu->menuType = POKENAV_MENU_TYPE_CONDITION;
            menu->cursorPos = 0;
            menu->currMenuItem = sMenuItems[POKENAV_MENU_TYPE_CONDITION][0];
            menu->callback = HandleConditionMenuInput;
            return POKENAV_MENU_FUNC_OPEN_CONDITION;
        case POKENAV_MENUITEM_MATCH_CALL:
            menu->helpBarIndex = HELPBAR_MC_TRAINER_LIST;
            SetMenuIdAndCB(menu, POKENAV_MATCH_CALL);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        case POKENAV_MENUITEM_RIBBONS:
            if (CanViewRibbonsMenu())
            {
                menu->helpBarIndex = HELPBAR_RIBBONS_MON_LIST;
                SetMenuIdAndCB(menu, POKENAV_RIBBONS_MON_LIST);
                return POKENAV_MENU_FUNC_OPEN_FEATURE;
            }
            else
            {
                menu->callback = HandleCantOpenRibbonsInput;
                return POKENAV_MENU_FUNC_NO_RIBBON_WINNERS;
            }
#if IS_HNS
        case POKENAV_MENUITEM_RADIO:
            menu->helpBarIndex = HELPBAR_NONE;
            SetMenuIdAndCB(menu, POKENAV_RADIO);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        case POKENAV_MENUITEM_JOURNAL:
            menu->helpBarIndex = HELPBAR_JOURNAL;
            SetMenuIdAndCB(menu, POKENAV_JOURNAL);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
#endif
        case POKENAV_MENUITEM_SWITCH_OFF:
            return POKENAV_MENU_FUNC_EXIT;
        }
    }

    if (JOY_NEW(B_BUTTON))
        return POKENAV_MENU_FUNC_EXIT;

    return POKENAV_MENU_FUNC_NONE;
}

// Force the player to select Match Call during the call Mr. Stone PokéNav tutorial
static u32 HandleMainMenuInputTutorial(struct Pokenav_Menu *menu)
{
    if (UpdateMenuCursorPos(menu))
        return POKENAV_MENU_FUNC_MOVE_CURSOR;

    if (JOY_NEW(A_BUTTON))
    {
        if (MenuItemAt(menu, menu->cursorPos) == POKENAV_MENUITEM_MATCH_CALL)
        {
            menu->helpBarIndex = HELPBAR_MC_TRAINER_LIST;
            SetMenuIdAndCB(menu, POKENAV_MATCH_CALL);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        }
        else
        {
            PlaySE(SE_FAILURE);
            return POKENAV_MENU_FUNC_NONE;
        }
    }

    if (JOY_NEW(B_BUTTON))
    {
        PlaySE(SE_FAILURE);
        return POKENAV_MENU_FUNC_NONE;
    }

    return POKENAV_MENU_FUNC_NONE;
}

// After calling Mr. Stone during the PokéNav tutorial, force player to exit or use Match Call again
static u32 HandleMainMenuInputEndTutorial(struct Pokenav_Menu *menu)
{
    if (UpdateMenuCursorPos(menu))
        return POKENAV_MENU_FUNC_MOVE_CURSOR;

    if (JOY_NEW(A_BUTTON))
    {
        u32 menuItem = MenuItemAt(menu, menu->cursorPos);
        if (menuItem != POKENAV_MENUITEM_MATCH_CALL && menuItem != POKENAV_MENUITEM_SWITCH_OFF)
        {
            PlaySE(SE_FAILURE);
            return POKENAV_MENU_FUNC_NONE;
        }
        else if (menuItem == POKENAV_MENUITEM_MATCH_CALL)
        {
            menu->helpBarIndex = HELPBAR_MC_TRAINER_LIST;
            SetMenuIdAndCB(menu, POKENAV_MATCH_CALL);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        }
        else
        {
            return -1;
        }
    }
    else if (JOY_NEW(B_BUTTON))
    {
        return -1;
    }
    return POKENAV_MENU_FUNC_NONE;
}

// Handles input after selecting Ribbons when there are no ribbon winners left
// Selecting it again just reprints the Ribbon description to replace the "No Ribbon winners" message
static u32 HandleCantOpenRibbonsInput(struct Pokenav_Menu *menu)
{
    if (UpdateMenuCursorPos(menu))
    {
        menu->callback = GetMainMenuInputHandler();
        return POKENAV_MENU_FUNC_MOVE_CURSOR;
    }

    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        menu->callback = GetMainMenuInputHandler();
        return POKENAV_MENU_FUNC_RESHOW_DESCRIPTION;
    }

    return POKENAV_MENU_FUNC_NONE;
}

static u32 HandleConditionMenuInput(struct Pokenav_Menu *menu)
{
    if (UpdateMenuCursorPos(menu))
        return POKENAV_MENU_FUNC_MOVE_CURSOR;

    if (JOY_NEW(A_BUTTON))
    {
        switch (MenuItemAt(menu, menu->cursorPos))
        {
        case POKENAV_MENUITEM_CONDITION_SEARCH:
            menu->menuType = POKENAV_MENU_TYPE_CONDITION_SEARCH;
            menu->cursorPos = 0;
            menu->currMenuItem = sMenuItems[POKENAV_MENU_TYPE_CONDITION_SEARCH][0];
            menu->callback = HandleConditionSearchMenuInput;
            return POKENAV_MENU_FUNC_OPEN_CONDITION_SEARCH;
        case POKENAV_MENUITEM_CONDITION_PARTY:
            menu->helpBarIndex = 0;
            SetMenuIdAndCB(menu, POKENAV_CONDITION_GRAPH_PARTY);
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        case POKENAV_MENUITEM_CONDITION_CANCEL:
            PlaySE(SE_SELECT);
            ReturnToMainMenu(menu);
            return POKENAV_MENU_FUNC_RETURN_TO_MAIN;
        }
    }
    if (JOY_NEW(B_BUTTON))
    {
        if (menu->cursorPos != LastCursorPos(menu))
        {
            menu->cursorPos = LastCursorPos(menu);
            menu->callback = CB2_ReturnToMainMenu;
            return POKENAV_MENU_FUNC_MOVE_CURSOR;
        }
        else
        {
            PlaySE(SE_SELECT);
            ReturnToMainMenu(menu);
            return POKENAV_MENU_FUNC_RETURN_TO_MAIN;
        }
    }

    return POKENAV_MENU_FUNC_NONE;
}

static u32 HandleConditionSearchMenuInput(struct Pokenav_Menu *menu)
{
    if (UpdateMenuCursorPos(menu))
        return POKENAV_MENU_FUNC_MOVE_CURSOR;

    if (JOY_NEW(A_BUTTON))
    {
        u8 menuItem = MenuItemAt(menu, menu->cursorPos);
        if (menuItem != POKENAV_MENUITEM_CONDITION_SEARCH_CANCEL)
        {
            SetSelectedConditionSearch(menuItem - POKENAV_MENUITEM_CONDITION_SEARCH_COOL);
            SetMenuIdAndCB(menu, POKENAV_CONDITION_SEARCH_RESULTS);
            menu->helpBarIndex = HELPBAR_CONDITION_MON_LIST;
            return POKENAV_MENU_FUNC_OPEN_FEATURE;
        }
        else
        {
            PlaySE(SE_SELECT);
            ReturnToConditionMenu(menu);
            return POKENAV_MENU_FUNC_RETURN_TO_CONDITION;
        }
    }
    if (JOY_NEW(B_BUTTON))
    {
        if (menu->cursorPos != LastCursorPos(menu))
        {
            menu->cursorPos = LastCursorPos(menu);
            menu->callback = CB2_ReturnToConditionMenu;
            return POKENAV_MENU_FUNC_MOVE_CURSOR;
        }
        else
        {
            PlaySE(SE_SELECT);
            ReturnToConditionMenu(menu);
            return POKENAV_MENU_FUNC_RETURN_TO_CONDITION;
        }
    }
    return POKENAV_MENU_FUNC_NONE;
}

static u32 CB2_ReturnToMainMenu(struct Pokenav_Menu *menu)
{
    ReturnToMainMenu(menu);
    return POKENAV_MENU_FUNC_RETURN_TO_MAIN;
}

static u32 CB2_ReturnToConditionMenu(struct Pokenav_Menu *menu)
{
    ReturnToConditionMenu(menu);
    return POKENAV_MENU_FUNC_RETURN_TO_CONDITION;
}

static void SetMenuIdAndCB(struct Pokenav_Menu *menu, u32 menuId)
{
    menu->menuId = menuId;
    menu->callback = GetMenuId;
}

static u32 GetMenuId(struct Pokenav_Menu *menu)
{
    return menu->menuId;
}

static void ReturnToMainMenu(struct Pokenav_Menu *menu)
{
    SetMainMenuType(menu);
    menu->cursorPos = GetCursorPosOfMenuItem(menu, POKENAV_MENUITEM_CONDITION);
    menu->currMenuItem = MenuItemAt(menu, menu->cursorPos);
    menu->callback = HandleMainMenuInput;
}

static void ReturnToConditionMenu(struct Pokenav_Menu *menu)
{
    menu->menuType = POKENAV_MENU_TYPE_CONDITION;
    menu->cursorPos = 1;
    menu->currMenuItem = sMenuItems[POKENAV_MENU_TYPE_CONDITION][1];
    menu->callback = HandleConditionMenuInput;
}

static bool32 UpdateMenuCursorPos(struct Pokenav_Menu *menu)
{
    if (JOY_NEW(DPAD_UP))
    {
        if (--menu->cursorPos < 0)
            menu->cursorPos = LastCursorPos(menu);

        menu->currMenuItem = MenuItemAt(menu, menu->cursorPos);
        return TRUE;
    }
    else if (JOY_NEW(DPAD_DOWN))
    {
        menu->cursorPos++;
        if (menu->cursorPos > LastCursorPos(menu))
            menu->cursorPos = 0;

        menu->currMenuItem = MenuItemAt(menu, menu->cursorPos);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

int GetPokenavMenuType(void)
{
    struct Pokenav_Menu *menu = GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
    return menu->menuType;
}

// Position of cursor relative to number of current menu options
int GetPokenavCursorPos(void)
{
    struct Pokenav_Menu *menu = GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
    return menu->cursorPos;
}

// ID of menu item the cursor is currently on
int GetCurrentMenuItemId(void)
{
    struct Pokenav_Menu *menu = GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
    return menu->currMenuItem;
}

#if IS_HNS
// For the menu's graphics (pokenav_menu_handler_gfx.c): the item on row `pos`
// of the current menu, and how many rows it has.
int GetPokenavMenuItemAt(int pos)
{
    return MenuItemAt(GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER), pos);
}

int GetPokenavMenuItemCount(void)
{
    return LastCursorPos(GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER)) + 1;
}
#endif

u16 GetHelpBarTextId(void)
{
    struct Pokenav_Menu *menu = GetSubstructPtr(POKENAV_SUBSTRUCT_MAIN_MENU_HANDLER);
    return menu->helpBarIndex;
}
