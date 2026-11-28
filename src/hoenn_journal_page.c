// Hoenn & Seek 2.1-06b: the SEEKER'S JOURNAL as a POKéGEAR page (2.1 lane A;
// 2.1 plan section 15, "The journal moves into the POKéGEAR").
//
// What's on the page comes from src/hoenn_journal.c (pages, states, texts,
// the TODAY rules); this file only draws it, on the same screen pieces as
// Match Call (src/pokenav_match_call_gfx.c): its frame on BG2, the shared list
// engine (src/pokenav_list.c) on BG3, its message-box border for reading.
//
//   - Tabs: TODAY, JOHTO, KANTO, HOENN, switched with L/R (TODAY first, like
//     2.1-06's menu). The left column shows the tab's name, then "Found X of
//     Y" on a region's tab, or the weekday and how many are on today.
//   - Rows: ??? until heard, then the rumor's short question, then the found
//     title with a check (an icon drawn on BG3 like Match Call's rematch
//     Poké Ball, at the row's right end). An empty TODAY shows one row,
//     "Nothing on today.", that A doesn't open.
//   - A: a panel over the list with the entry's name, its "when" line on
//     TODAY, then its rumor or found line (re-flowed: the message-box line
//     breaks become plain ones, so nothing waits for a button). B: back.
//   - B on the list goes back to the main menu with the cursor on JOURNAL.
//
// Substructs: POKENAV_SUBSTRUCT_JOURNAL (the tab, the rows, the input
// handler) and POKENAV_SUBSTRUCT_JOURNAL_GFX (windows and tilemaps), freed by
// FreeJournalSubstruct1/2 like every other PokéGear page.
#include "global.h"
#include "bg.h"
#include "decompress.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_time.h"
#include "international_string_util.h"
#include "match_call.h"
#include "menu.h"
#include "palette.h"
#include "pokenav.h"
#include "sound.h"
#include "string_util.h"
#include "strings.h"
#include "text.h"
#include "window.h"
#include "constants/songs.h"
#include "constants/hoenn_journal.h"

#if IS_HNS

#define TAB_COUNT       (HOENN_JOURNAL_PAGE_COUNT + 1)
#define ROW_NOTHING     0xFF // the empty TODAY's one row
#define CHECK_TILE_TOP  0x5000 // BG3 palette 5, tiles 0 and 1 (check.png)
#define CHECK_TILE_NONE 0x5002 // the list's own blank fill tile

enum
{
    JOURNAL_FUNC_NONE,
    JOURNAL_FUNC_UP,
    JOURNAL_FUNC_DOWN,
    JOURNAL_FUNC_PAGE_UP,
    JOURNAL_FUNC_PAGE_DOWN,
    JOURNAL_FUNC_TAB,
    JOURNAL_FUNC_READ,
    JOURNAL_FUNC_CLOSE_ENTRY,
    JOURNAL_FUNC_EXIT,
};

struct Pokenav_Journal
{
    u32 (*callback)(struct Pokenav_Journal *);
    u8 tab;        // index into sTabs
    u16 rowCount;
    struct PokenavListItem rows[HOENN_JOURNAL_ENTRIES]; // .item.mon.data = entry id (or ROW_NOTHING)
};

struct Pokenav_JournalGfx
{
    bool32 (*isTaskActiveCB)(void);
    u32 loopedTaskId;
    u16 tabWindowId;
    u16 infoWindowId;
    u16 entryWindowId;
    u8 bgTilemapBuffer1[BG_SCREEN_SIZE];
    u8 bgTilemapBuffer2[BG_SCREEN_SIZE];
};

static u32 HandleJournalInput_WaitOpen(struct Pokenav_Journal *);
static u32 HandleJournalInput(struct Pokenav_Journal *);
static u32 HandleJournalEntryInput(struct Pokenav_Journal *);
static u32 GetExitJournalMenuId(struct Pokenav_Journal *);
static void BuildTabRows(struct Pokenav_Journal *);
static bool32 GetCurrentJournalLoopedTaskActive(void);
static u32 LoopedTask_OpenJournal(s32);
static u32 LoopedTask_JournalMoveUp(s32);
static u32 LoopedTask_JournalMoveDown(s32);
static u32 LoopedTask_JournalPageUp(s32);
static u32 LoopedTask_JournalPageDown(s32);
static u32 LoopedTask_JournalSwitchTab(s32);
static u32 LoopedTask_JournalRead(s32);
static u32 LoopedTask_JournalCloseEntry(s32);
static u32 LoopedTask_ExitJournal(s32);
static void CreateJournalList(void);
static void PrintTabInfo(struct Pokenav_JournalGfx *);
static void PrintEntry(struct Pokenav_JournalGfx *);
static void BufferJournalRow(struct PokenavListItem *, u8 *);
static void DrawJournalRowCheck(u16, u32, u32);

// Match Call's frame and list colors (the page looks like its neighbours).
static const u16 sJournalUI_Pal[] = INCBIN_U16("graphics/pokenav/match_call/ui.gbapal");
static const u32 sJournalUI_Gfx[] = INCBIN_U32("graphics/pokenav/match_call/ui.4bpp.smol");
static const u32 sJournalUI_Tilemap[] = INCBIN_U32("graphics/pokenav/match_call/ui.bin.smolTM");
static const u16 sJournalWindow_Pal[] = INCBIN_U16("graphics/pokenav/match_call/call_window.gbapal");
static const u16 sJournalList_Pal[] = INCBIN_U16("graphics/pokenav/match_call/list_window.gbapal");
static const u16 sJournalCheck_Pal[] = INCBIN_U16("graphics/pokenav/hns/journal/check.gbapal");
static const u32 sJournalCheck_Gfx[] = INCBIN_U32("graphics/pokenav/hns/journal/check.4bpp.smol");

// TODAY first, then the regions in the journal's own order.
static const u8 sTabs[TAB_COUNT] =
{
    HOENN_JOURNAL_PAGE_TODAY,
    HOENN_JOURNAL_PAGE_JOHTO,
    HOENN_JOURNAL_PAGE_KANTO,
    HOENN_JOURNAL_PAGE_HOENN,
};

static const u8 sText_Found[] = _("Found {STR_VAR_1} of {STR_VAR_2}");
static const u8 sText_OnToday[] = _("On today: {STR_VAR_2}");
static const u8 sText_Tabs[] = _("{L_BUTTON}{R_BUTTON} Next tab");
static const u8 sText_NothingToday[] = _("Nothing on today.");
static const u8 sText_Unread[] = _("? ? ? ? ?");
static const u8 sEntryNameColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_BLUE, TEXT_COLOR_LIGHT_BLUE};
static const u8 sEntryWhenColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_GREEN, TEXT_COLOR_LIGHT_GREEN};
static const u8 sEntryTextColors[] = {TEXT_COLOR_WHITE, TEXT_COLOR_DARK_GRAY, TEXT_COLOR_LIGHT_GRAY};

static const struct BgTemplate sJournalBgTemplates[3] =
{
    {
        .bg = 1,
        .charBaseIndex = 3,
        .mapBaseIndex = 0x1F,
        .priority = 1,
    },
    {
        .bg = 2,
        .charBaseIndex = 2,
        .mapBaseIndex = 0x06,
        .priority = 2,
        .baseTile = 0x80
    },
    {
        .bg = 3,
        .charBaseIndex = 1,
        .mapBaseIndex = 0x07,
        .priority = 3,
    }
};

// The left column: Match Call's location box, then its info box.
static const struct WindowTemplate sTabWindowTemplate =
{
    .bg = 2,
    .tilemapLeft = 0,
    .tilemapTop = 5,
    .width = 11,
    .height = 2,
    .paletteNum = 2,
    .baseBlock = 16
};

static const struct WindowTemplate sInfoWindowTemplate =
{
    .bg = 2,
    .tilemapLeft = 0,
    .tilemapTop = 9,
    .width = 11,
    .height = 8,
    .paletteNum = 2,
    .baseBlock = 38
};

// Reading an entry: Match Call's message box, made tall enough for a name,
// a two-line "when" and a five-line rumor in FONT_SHORT (14 px lines).
#define ENTRY_LINE_HEIGHT 14
static const struct WindowTemplate sEntryWindowTemplate =
{
    .bg = 1,
    .tilemapLeft = 1,
    .tilemapTop = 3,
    .width = 28,
    .height = 14,
    .paletteNum = 1,
    .baseBlock = 10
};

static const LoopedTask sJournalLoopTaskFuncs[] =
{
    [JOURNAL_FUNC_NONE]        = NULL,
    [JOURNAL_FUNC_UP]          = LoopedTask_JournalMoveUp,
    [JOURNAL_FUNC_DOWN]        = LoopedTask_JournalMoveDown,
    [JOURNAL_FUNC_PAGE_UP]     = LoopedTask_JournalPageUp,
    [JOURNAL_FUNC_PAGE_DOWN]   = LoopedTask_JournalPageDown,
    [JOURNAL_FUNC_TAB]         = LoopedTask_JournalSwitchTab,
    [JOURNAL_FUNC_READ]        = LoopedTask_JournalRead,
    [JOURNAL_FUNC_CLOSE_ENTRY] = LoopedTask_JournalCloseEntry,
    [JOURNAL_FUNC_EXIT]        = LoopedTask_ExitJournal,
};

// ---- State and input -------------------------------------------------------

bool32 PokenavCallback_Init_Journal(void)
{
    struct Pokenav_Journal *journal = AllocSubstruct(POKENAV_SUBSTRUCT_JOURNAL, sizeof(struct Pokenav_Journal));
    if (journal == NULL)
        return FALSE;

    journal->tab = 0;
    BuildTabRows(journal);
    journal->callback = HandleJournalInput_WaitOpen;
    return TRUE;
}

u32 GetJournalCallback(void)
{
    struct Pokenav_Journal *journal = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL);
    return journal->callback(journal);
}

void FreeJournalSubstruct1(void)
{
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_JOURNAL);
}

static void BuildTabRows(struct Pokenav_Journal *journal)
{
    u8 entries[HOENN_JOURNAL_ENTRIES];
    u32 i, n = HoennJournalPageEntries(sTabs[journal->tab], entries);

    for (i = 0; i < n; i++)
        journal->rows[i].item.mon.data = entries[i];
    if (n == 0)
    {
        journal->rows[0].item.mon.data = ROW_NOTHING;
        n = 1;
    }
    journal->rowCount = n;
}

static u32 HandleJournalInput_WaitOpen(struct Pokenav_Journal *journal)
{
    if (!IsJournalLoopedTaskActive())
        journal->callback = HandleJournalInput;
    return JOURNAL_FUNC_NONE;
}

static u32 HandleJournalInput(struct Pokenav_Journal *journal)
{
    if (JOY_REPEAT(DPAD_UP))
        return JOURNAL_FUNC_UP;
    if (JOY_REPEAT(DPAD_DOWN))
        return JOURNAL_FUNC_DOWN;
    if (JOY_NEW(DPAD_LEFT))
        return JOURNAL_FUNC_PAGE_UP;
    if (JOY_NEW(DPAD_RIGHT))
        return JOURNAL_FUNC_PAGE_DOWN;
    if (JOY_NEW(L_BUTTON | R_BUTTON))
    {
        if (JOY_NEW(R_BUTTON))
            journal->tab = (journal->tab + 1) % TAB_COUNT;
        else
            journal->tab = (journal->tab + TAB_COUNT - 1) % TAB_COUNT;
        BuildTabRows(journal);
        return JOURNAL_FUNC_TAB;
    }
    if (JOY_NEW(A_BUTTON)
     && journal->rows[PokenavList_GetSelectedIndex()].item.mon.data != ROW_NOTHING)
    {
        journal->callback = HandleJournalEntryInput;
        return JOURNAL_FUNC_READ;
    }
    if (JOY_NEW(B_BUTTON))
    {
        journal->callback = GetExitJournalMenuId;
        return JOURNAL_FUNC_EXIT;
    }
    return JOURNAL_FUNC_NONE;
}

static u32 HandleJournalEntryInput(struct Pokenav_Journal *journal)
{
    if (JOY_NEW(A_BUTTON | B_BUTTON))
    {
        journal->callback = HandleJournalInput;
        return JOURNAL_FUNC_CLOSE_ENTRY;
    }
    return JOURNAL_FUNC_NONE;
}

static u32 GetExitJournalMenuId(struct Pokenav_Journal *journal)
{
    return POKENAV_MAIN_MENU_CURSOR_ON_JOURNAL;
}

static u32 SelectedEntry(void)
{
    struct Pokenav_Journal *journal = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL);
    return journal->rows[PokenavList_GetSelectedIndex()].item.mon.data;
}

// ---- Graphics --------------------------------------------------------------

bool32 OpenPokenavJournal(void)
{
    struct Pokenav_JournalGfx *gfx = AllocSubstruct(POKENAV_SUBSTRUCT_JOURNAL_GFX, sizeof(struct Pokenav_JournalGfx));
    if (gfx == NULL)
        return FALSE;

    gfx->loopedTaskId = CreateLoopedTask(LoopedTask_OpenJournal, 1);
    gfx->isTaskActiveCB = GetCurrentJournalLoopedTaskActive;
    return TRUE;
}

void CreateJournalLoopedTask(s32 index)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);
    gfx->loopedTaskId = CreateLoopedTask(sJournalLoopTaskFuncs[index], 1);
    gfx->isTaskActiveCB = GetCurrentJournalLoopedTaskActive;
}

bool32 IsJournalLoopedTaskActive(void)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);
    return gfx->isTaskActiveCB();
}

static bool32 GetCurrentJournalLoopedTaskActive(void)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);
    return IsLoopedTaskActive(gfx->loopedTaskId);
}

void FreeJournalSubstruct2(void)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);
    DestroyPokenavList();
    RemoveWindow(gfx->tabWindowId);
    RemoveWindow(gfx->infoWindowId);
    RemoveWindow(gfx->entryWindowId);
    FreePokenavSubstruct(POKENAV_SUBSTRUCT_JOURNAL_GFX);
}

static u32 LoopedTask_OpenJournal(s32 state)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);

    switch (state)
    {
    case 0:
        InitBgTemplates(sJournalBgTemplates, ARRAY_COUNT(sJournalBgTemplates));
        ChangeBgX(2, 0, BG_COORD_SET);
        ChangeBgY(2, 0, BG_COORD_SET);
        DecompressAndCopyTileDataToVram(2, sJournalUI_Gfx, 0, 0, 0);
        SetBgTilemapBuffer(2, gfx->bgTilemapBuffer2);
        CopyToBgTilemapBuffer(2, sJournalUI_Tilemap, 0, 0);
        CopyBgTilemapBufferToVram(2);
        CopyPaletteIntoBufferUnfaded(sJournalUI_Pal, BG_PLTT_ID(2), sizeof(sJournalUI_Pal));
        return LT_INC_AND_PAUSE;
    case 1:
        if (FreeTempTileDataBuffersIfPossible())
            return LT_PAUSE;
        BgDmaFill(1, 0, 0, 1);
        SetBgTilemapBuffer(1, gfx->bgTilemapBuffer1);
        FillBgTilemapBufferRect_Palette0(1, 0x1000, 0, 0, 32, 20);
        CopyPaletteIntoBufferUnfaded(sJournalWindow_Pal, BG_PLTT_ID(1), sizeof(sJournalWindow_Pal));
        CopyBgTilemapBufferToVram(1);
        return LT_INC_AND_PAUSE;
    case 2:
        if (FreeTempTileDataBuffersIfPossible())
            return LT_PAUSE;
        gfx->entryWindowId = AddWindow(&sEntryWindowTemplate);
        LoadMatchCallWindowGfx(gfx->entryWindowId, 1, 4);
        DecompressAndCopyTileDataToVram(3, sJournalCheck_Gfx, 0, 0, 0);
        CopyPaletteIntoBufferUnfaded(sJournalList_Pal, BG_PLTT_ID(3), sizeof(sJournalList_Pal));
        CopyPaletteIntoBufferUnfaded(sJournalCheck_Pal, BG_PLTT_ID(5), PLTT_SIZE_4BPP);
        return LT_INC_AND_PAUSE;
    case 3:
        if (FreeTempTileDataBuffersIfPossible())
            return LT_PAUSE;
        CreateJournalList();
        return LT_INC_AND_PAUSE;
    case 4:
        if (IsCreatePokenavListTaskActive())
            return LT_PAUSE;
        gfx->tabWindowId = AddWindow(&sTabWindowTemplate);
        gfx->infoWindowId = AddWindow(&sInfoWindowTemplate);
        PutWindowTilemap(gfx->tabWindowId);
        PutWindowTilemap(gfx->infoWindowId);
        PrintTabInfo(gfx);
        return LT_INC_AND_PAUSE;
    case 5:
        ChangeBgX(1, 0, BG_COORD_SET);
        ChangeBgY(1, 0, BG_COORD_SET);
        ShowBg(2);
        ShowBg(3);
        ShowBg(1);
        LoadLeftHeaderGfxForIndex(POKENAV_GFX_JOURNAL_MENU);
        ShowLeftHeaderGfx(POKENAV_GFX_JOURNAL_MENU, TRUE, FALSE);
        PrintHelpBarText(HELPBAR_JOURNAL);
        PokenavFadeScreen(POKENAV_FADE_FROM_BLACK);
        return LT_INC_AND_PAUSE;
    case 6:
        if (IsPaletteFadeActive() || AreLeftHeaderSpritesMoving())
            return LT_PAUSE;
        break;
    }
    return LT_FINISH;
}

static void CreateJournalList(void)
{
    struct Pokenav_Journal *journal = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL);
    struct PokenavListTemplate template;

    template.list = journal->rows;
    template.count = journal->rowCount;
    template.itemSize = sizeof(struct PokenavListItem);
    template.startIndex = 0;
    template.item_X = 13;
    template.windowWidth = 16;
    template.listTop = 1;
    template.maxShowed = 8;
    template.fillValue = 3;
    template.fontId = FONT_NARROW;
    template.bufferItemFunc = BufferJournalRow;
    template.iconDrawFunc = DrawJournalRowCheck;
    CreatePokenavList(&sJournalBgTemplates[2], &template, 2);
}

static void BufferJournalRow(struct PokenavListItem *item, u8 *dest)
{
    u32 entry = item->item.mon.data;

    if (entry == ROW_NOTHING)
        StringCopy(dest, sText_NothingToday);
    else if (HoennJournalEntryState(entry) == HOENN_JOURNAL_NOT_HEARD)
        StringCopy(dest, sText_Unread);
    else
        StringCopy(dest, HoennJournalEntryName(entry));
}

// The list engine calls this with the row's list index (the same argument
// Match Call gets as its entry index) and its place in the window.
static void DrawJournalRowCheck(u16 windowId, u32 index, u32 tileOffset)
{
    struct Pokenav_Journal *journal = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL);
    u16 *tilemap = GetBgTilemapBuffer(GetWindowAttribute(windowId, WINDOW_BG));
    u32 entry = journal->rows[index].item.mon.data;
    bool32 found = (entry != ROW_NOTHING && HoennJournalEntryState(entry) == HOENN_JOURNAL_FOUND);

    tilemap += tileOffset * 64 + 0x1D;
    tilemap[0] = found ? CHECK_TILE_TOP : CHECK_TILE_NONE;
    tilemap[0x20] = found ? CHECK_TILE_TOP + 1 : CHECK_TILE_NONE;
}

// The left column's lines are centred, like the tab name above them (the
// box's frame covers the window's first pixels).
static void PrintInfoLine(u32 windowId, const u8 *text, u32 y)
{
    AddTextPrinterParameterized(windowId, FONT_NARROW, text, GetStringCenterAlignXOffset(FONT_NARROW, text, 88), y, TEXT_SKIP_DRAW, NULL);
}

static void PrintTabInfo(struct Pokenav_JournalGfx *gfx)
{
    struct Pokenav_Journal *journal = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL);
    u32 page = sTabs[journal->tab], found, total;
    const u8 *name = HoennJournalPageName(page);

    FillWindowPixelBuffer(gfx->tabWindowId, PIXEL_FILL(1));
    AddTextPrinterParameterized(gfx->tabWindowId, FONT_NARROW, name, GetStringCenterAlignXOffset(FONT_NARROW, name, 88), 1, TEXT_SKIP_DRAW, NULL);
    CopyWindowToVram(gfx->tabWindowId, COPYWIN_FULL);

    FillWindowPixelBuffer(gfx->infoWindowId, PIXEL_FILL(1));
    HoennJournalPageCounts(page, &found, &total);
    ConvertIntToDecimalStringN(gStringVar1, found, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar2, total, STR_CONV_MODE_LEFT_ALIGN, 3);
    if (page == HOENN_JOURNAL_PAGE_TODAY)
    {
        HoennBufferWeekdayName(); // STR_VAR_1 (TODAY is the calendar's day; night-only rows use the night's)
        PrintInfoLine(gfx->infoWindowId, gStringVar1, 1);
        ConvertIntToDecimalStringN(gStringVar2, total, STR_CONV_MODE_LEFT_ALIGN, 3);
        StringExpandPlaceholders(gStringVar4, sText_OnToday);
    }
    else
    {
        StringExpandPlaceholders(gStringVar4, sText_Found);
    }
    PrintInfoLine(gfx->infoWindowId, gStringVar4, 17);
    PrintInfoLine(gfx->infoWindowId, sText_Tabs, 49);
    CopyWindowToVram(gfx->infoWindowId, COPYWIN_FULL);
}

// Prints `text` (placeholders expanded) from line `line`, one printed line
// per text line; message-box breaks (\l, \p) become plain line breaks.
// Returns the line after the last one.
static u32 PrintEntryLines(u16 windowId, const u8 *text, u32 line, const u8 *colors)
{
    u8 *c;

    StringExpandPlaceholders(gStringVar4, text);
    for (c = gStringVar4; *c != EOS; c++)
    {
        if (*c == CHAR_PROMPT_SCROLL || *c == CHAR_PROMPT_CLEAR)
            *c = CHAR_NEWLINE;
    }
    AddTextPrinterParameterized4(windowId, FONT_SHORT, 2, 1 + line * ENTRY_LINE_HEIGHT, 0, 0, colors, TEXT_SKIP_DRAW, gStringVar4);
    for (c = gStringVar4; *c != EOS; c++)
    {
        if (*c == CHAR_NEWLINE)
            line++;
    }
    return line + 1;
}

static void PrintEntry(struct Pokenav_JournalGfx *gfx)
{
    struct Pokenav_Journal *journal = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL);
    u32 entry = SelectedEntry(), line = 0;
    const u8 *when = NULL;

    FillWindowPixelBuffer(gfx->entryWindowId, PIXEL_FILL(1));
    if (HoennJournalEntryState(entry) != HOENN_JOURNAL_NOT_HEARD)
        line = PrintEntryLines(gfx->entryWindowId, HoennJournalEntryName(entry), line, sEntryNameColors);
    if (sTabs[journal->tab] == HOENN_JOURNAL_PAGE_TODAY)
        when = HoennJournalEntryWhen(entry);
    if (when != NULL)
        line = PrintEntryLines(gfx->entryWindowId, when, line, sEntryWhenColors);
    PrintEntryLines(gfx->entryWindowId, HoennJournalEntryText(entry), line, sEntryTextColors);
    CopyWindowToVram(gfx->entryWindowId, COPYWIN_GFX);
}

static u32 MoveCursor(s32 state, int (*move)(void))
{
    switch (state)
    {
    case 0:
        switch (move())
        {
        case 0:
            return LT_FINISH;
        case 1:
            PlaySE(SE_SELECT);
            return LT_SET_STATE(2);
        case 2:
            PlaySE(SE_SELECT);
            break;
        }
        return LT_INC_AND_PAUSE;
    case 1:
        if (PokenavList_IsMoveWindowTaskActive())
            return LT_PAUSE;
        // fallthrough
    case 2:
        if (IsDma3ManagerBusyWithBgCopy())
            return LT_PAUSE;
        break;
    }
    return LT_FINISH;
}

static u32 LoopedTask_JournalMoveUp(s32 state)
{
    return MoveCursor(state, PokenavList_MoveCursorUp);
}

static u32 LoopedTask_JournalMoveDown(s32 state)
{
    return MoveCursor(state, PokenavList_MoveCursorDown);
}

static u32 LoopedTask_JournalPageUp(s32 state)
{
    return MoveCursor(state, PokenavList_PageUp);
}

static u32 LoopedTask_JournalPageDown(s32 state)
{
    return MoveCursor(state, PokenavList_PageDown);
}

// L/R: the rows were rebuilt for the new tab (HandleJournalInput); remake
// the list on them, from the top.
static u32 LoopedTask_JournalSwitchTab(s32 state)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);

    switch (state)
    {
    case 0:
        PlaySE(SE_SELECT);
        DestroyPokenavList();
        CreateJournalList();
        return LT_INC_AND_PAUSE;
    case 1:
        if (IsCreatePokenavListTaskActive())
            return LT_PAUSE;
        PrintTabInfo(gfx);
        return LT_INC_AND_PAUSE;
    case 2:
        if (IsDma3ManagerBusyWithBgCopy())
            return LT_PAUSE;
        break;
    }
    return LT_FINISH;
}

static u32 LoopedTask_JournalRead(s32 state)
{
    struct Pokenav_JournalGfx *gfx = GetSubstructPtr(POKENAV_SUBSTRUCT_JOURNAL_GFX);

    switch (state)
    {
    case 0:
        PlaySE(SE_SELECT);
        PokenavList_ToggleVerticalArrows(TRUE);
        DrawMatchCallTextBoxBorder(gfx->entryWindowId, 1, 4);
        PrintEntry(gfx);
        PutWindowTilemap(gfx->entryWindowId);
        CopyBgTilemapBufferToVram(1);
        PrintHelpBarText(HELPBAR_JOURNAL_ENTRY);
        return LT_INC_AND_PAUSE;
    case 1:
        if (IsDma3ManagerBusyWithBgCopy() || WaitForHelpBar())
            return LT_PAUSE;
        break;
    }
    return LT_FINISH;
}

static u32 LoopedTask_JournalCloseEntry(s32 state)
{
    switch (state)
    {
    case 0:
        PlaySE(SE_SELECT);
        FillBgTilemapBufferRect_Palette0(1, 0, 0, 0, 32, 20);
        CopyBgTilemapBufferToVram(1);
        PokenavList_ToggleVerticalArrows(FALSE);
        PrintHelpBarText(HELPBAR_JOURNAL);
        return LT_INC_AND_PAUSE;
    case 1:
        if (IsDma3ManagerBusyWithBgCopy() || WaitForHelpBar())
            return LT_PAUSE;
        break;
    }
    return LT_FINISH;
}

static u32 LoopedTask_ExitJournal(s32 state)
{
    switch (state)
    {
    case 0:
        PlaySE(SE_SELECT);
        PokenavFadeScreen(POKENAV_FADE_TO_BLACK);
        SlideMenuHeaderDown();
        return LT_INC_AND_PAUSE;
    case 1:
        if (IsPaletteFadeActive() || MainMenuLoopedTaskIsBusy())
            return LT_PAUSE;
        SetLeftHeaderSpritesInvisibility();
        break;
    }
    return LT_FINISH;
}

#endif // IS_HNS
