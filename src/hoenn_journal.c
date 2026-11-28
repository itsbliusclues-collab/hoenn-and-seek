// Hoenn & Seek 2.1-05: the SEEKER'S JOURNAL (2.1 lane A; 2.1 plan section 15).
//
// A key item that lists 2.0's (and later 2.1's) hidden, daily and weekly
// events by region. Data: data/hoenn/journal.json →
// tools/hoenn/gen_hoenn_journal.py → src/data/hoenn_journal.h (included below),
// the heard flags in include/constants/hoenn_journal.h and the texts in
// data/scripts/hoenn_journal_texts.inc. The script is
// HoennSeek_EventScript_SeekersJournal (data/scripts/hoenn_2_1_lane_a.inc), run
// from the bag or SELECT like the Sand Chime (src/item_use.c).
//
// Each entry is in one of three states:
//   - ??? until you've heard of it;
//   - heard: an NPC said a hint line or rumor that names it (src/hoenn_hints.c
//     sets the flag, through src/data/hoenn_journal_hints.h), an event call
//     about it rang (HoennJournalHeardCall, hooked in HoennTryEventCall), a
//     scene set FLAG_HOENN_HEARD_<KEY> itself, or one of its "heard if"
//     conditions holds (something you've seen that points to it);
//   - found: its found conditions hold (the hint engine's conditions). Found
//     always counts as heard, so nothing found ever shows as ???.
// So the heard flags only record what can't be read from the game's state.
//
// The screen is two dynamic menus: the pages (JOHTO 3/22 …), then a page's
// entries (??? / the rumor's short question / the found title). Picking an
// entry shows its rumor or its found line in a message box, and the list
// opens again on the same row. Menu names are Alloc'ed copies: the dynamic
// menu frees every name when it closes (CLAUDE.md, part 14a).
//
// 2.1-06 adds:
//   - the TODAY page, listed first: every entry with a "when" rule that you've
//     heard of, isn't found yet (or comes back: repeat), and fits today: its
//     weekdays (a night-only event counts the night's weekday,
//     HoennNightWeekday) and its weather maps (one has its special-weather
//     day today, the forecast table's test). Picking one shows its "when"
//     line (times, tide) first, then its rumor or found line;
//   - Birch's aide's field notes read this table instead of the hint rumors:
//     found X of all entries, then the first unfound entry you haven't heard
//     of (Hoenn's page first), read out as its rumor, which marks it heard;
//   - the professors' rewards: when a region's page is all found, that
//     region's professor calls (Elm, Oak, Birch: Hoenn21LaneADueCall) and gives
//     a doll at the lab (CELEBI, MEW, JIRACHI; data/scripts/hoenn_2_1_lane_a.inc).
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_hints.h"
#include "hoenn_time.h"
#include "rtc.h"
#include "list_menu.h"
#include "malloc.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/hoenn_events.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/hoenn_phone.h"
#include "constants/species.h"

#if IS_HNS

// When an entry shows on the TODAY page (journal.json's "when").
struct HoennJournalWhen
{
    const u16 *weatherMaps; // MAP_* ids; any one on its special-weather day today
    u8 weatherCount;
    u8 days;                // bit per weekday (1 << HOENN_SUNDAY ...); 0 = every day
    u8 times;               // morning 1, day 2, evening 4, night 8 (only night: the night's weekday)
    u8 hour;                // only in its text
    u8 tide;                // only in its text
    bool8 repeat;           // shows even once found
    const u8 *text;
};

#define WHEN_NIGHT 8

struct HoennJournalEntry
{
    const struct HoennHintCond *found;
    u8 foundCount;
    const struct HoennHintCond *heardIf; // any one of them holding counts as heard
    u8 heardIfCount;
    const struct HoennJournalWhen *when; // NULL: never on the TODAY page
    const u8 *hint;      // list name once heard: a short question
    const u8 *title;     // list name once found
    const u8 *rumor;     // what you heard
    const u8 *foundText; // what you saw, in Birch's aide's voice
    u16 heardFlag;
    u8 page;
};

struct HoennJournalCall
{
    u16 call;
    u16 heardFlag;
};

#include "data/hoenn_journal.h"

static const u8 sText_Unknown[] = _("? ? ? ? ?");
static const u8 sText_Back[] = _("BACK");
static const u8 sText_Close[] = _("CLOSE");
static const u8 sText_NotHeard[] = _("Nothing written here yet.\nKeep your ears open!");
static const u8 sText_Today[] = _("TODAY");

static bool32 IsFound(const struct HoennJournalEntry *e)
{
    return HoennHintConditionsHold(e->found, e->foundCount);
}

static bool32 IsHeard(const struct HoennJournalEntry *e)
{
    u32 i;

    if (FlagGet(e->heardFlag) || IsFound(e))
        return TRUE;
    for (i = 0; i < e->heardIfCount; i++)
        if (HoennHintConditionsHold(&e->heardIf[i], 1))
            return TRUE;
    return FALSE;
}

// An event call about an event rang: you've heard of it.
void HoennJournalHeardCall(u16 call)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sJournalCalls); i++)
        if (sJournalCalls[i].call == call && sJournalCalls[i].heardFlag != 0)
            FlagSet(sJournalCalls[i].heardFlag);
}

// On the TODAY page: heard, not found (unless it comes back), and today fits.
static bool32 IsToday(const struct HoennJournalEntry *e)
{
    const struct HoennJournalWhen *w = e->when;
    u32 i;

    if (w == NULL || !IsHeard(e) || (!w->repeat && IsFound(e)))
        return FALSE;
    if (w->days != 0)
    {
        u32 day = (w->times == WHEN_NIGHT) ? HoennNightWeekday() : GetDayOfWeek();
        if (!(w->days & (1 << day)))
            return FALSE;
    }
    if (w->weatherCount == 0)
        return TRUE;
    for (i = 0; i < w->weatherCount; i++)
        if (HoennIsSpecialWeatherInDays(w->weatherMaps[i] >> 8, w->weatherMaps[i] & 0xFF, 0))
            return TRUE;
    return FALSE;
}

static bool32 OnPage(const struct HoennJournalEntry *e, u32 page)
{
    return (page == HOENN_JOURNAL_PAGE_TODAY) ? IsToday(e) : e->page == page;
}

// The entry at place `slot` on `page` (the TODAY page too), or NULL.
static const struct HoennJournalEntry *EntryAt(u32 page, u32 slot)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sJournalEntries); i++)
        if (OnPage(&sJournalEntries[i], page) && slot-- == 0)
            return &sJournalEntries[i];
    return NULL;
}

static u32 CountToday(void)
{
    u32 i, n = 0;

    for (i = 0; i < ARRAY_COUNT(sJournalEntries); i++)
        if (IsToday(&sJournalEntries[i]))
            n++;
    return n;
}

static void CountPage(u32 page, u32 *found, u32 *total)
{
    u32 i;

    *found = *total = 0;
    for (i = 0; i < ARRAY_COUNT(sJournalEntries); i++)
    {
        if (sJournalEntries[i].page != page)
            continue;
        (*total)++;
        if (IsFound(&sJournalEntries[i]))
            (*found)++;
    }
}

static void PushName(const u8 *text, u32 id)
{
    struct ListMenuItem item;
    u8 *name = Alloc(StringLength(text) + 1);

    StringCopy(name, text);
    item.name = name;
    item.id = id;
    MultichoiceDynamic_PushElement(item);
}

// special HoennJournalPushPages: "TODAY 3" (id HOENN_JOURNAL_PAGE_TODAY),
// "JOHTO 3/22" per region, then CLOSE (HOENN_JOURNAL_PAGE_TODAY + 1).
void HoennJournalPushPages(void)
{
    u8 buf[24];
    u32 page, found, total;
    u8 *end;

    end = StringCopy(buf, sText_Today);
    *end++ = CHAR_SPACE;
    ConvertIntToDecimalStringN(end, CountToday(), STR_CONV_MODE_LEFT_ALIGN, 3);
    PushName(buf, HOENN_JOURNAL_PAGE_TODAY);
    for (page = 0; page < HOENN_JOURNAL_PAGE_COUNT; page++)
    {
        CountPage(page, &found, &total);
        end = StringCopy(buf, sJournalPageNames[page]);
        *end++ = CHAR_SPACE;
        end = ConvertIntToDecimalStringN(end, found, STR_CONV_MODE_LEFT_ALIGN, 3);
        *end++ = CHAR_SLASH;
        ConvertIntToDecimalStringN(end, total, STR_CONV_MODE_LEFT_ALIGN, 3);
        PushName(buf, page);
    }
    PushName(sText_Close, HOENN_JOURNAL_PAGE_TODAY + 1);
}

// special HoennJournalPushEntries: page VAR_0x8004's entries (id = place on
// the page), then BACK (HOENN_JOURNAL_MENU_BACK). STR_VAR_1 = the page's
// name, STR_VAR_2 = found, STR_VAR_3 = total. On the TODAY page: STR_VAR_1 =
// the weekday, STR_VAR_2 = how many are on.
void HoennJournalPushEntries(void)
{
    u32 page = gSpecialVar_0x8004, slot, found, total;
    const struct HoennJournalEntry *e;

    for (slot = 0; (e = EntryAt(page, slot)) != NULL; slot++)
    {
        if (IsFound(e))
            PushName(e->title, slot);
        else if (IsHeard(e))
            PushName(e->hint, slot);
        else
            PushName(sText_Unknown, slot);
    }
    PushName(sText_Back, HOENN_JOURNAL_MENU_BACK);
    if (page == HOENN_JOURNAL_PAGE_TODAY)
    {
        HoennBufferWeekdayName(); // STR_VAR_1
        ConvertIntToDecimalStringN(gStringVar2, CountToday(), STR_CONV_MODE_LEFT_ALIGN, 3);
        return;
    }
    CountPage(page, &found, &total);
    StringCopy(gStringVar1, sJournalPageNames[page]);
    ConvertIntToDecimalStringN(gStringVar2, found, STR_CONV_MODE_LEFT_ALIGN, 3);
    ConvertIntToDecimalStringN(gStringVar3, total, STR_CONV_MODE_LEFT_ALIGN, 3);
}

// callnative HoennSeek_JournalEntryText: data[0] = the text of entry
// VAR_0x8005 on page VAR_0x8004 (for callstd MSGBOX_DEFAULT, like the hint
// NPCs).
void HoennSeek_JournalEntryText(struct ScriptContext *ctx)
{
    const struct HoennJournalEntry *e = EntryAt(gSpecialVar_0x8004, gSpecialVar_0x8005);

    if (e == NULL || !IsHeard(e))
        ctx->data[0] = (uintptr_t)sText_NotHeard;
    else if (IsFound(e))
        ctx->data[0] = (uintptr_t)e->foundText;
    else
        ctx->data[0] = (uintptr_t)e->rumor;
}

// The heard flags are 2.1's generated journal block (HOENN_21_JOURNAL_FLAGS).
STATIC_ASSERT(HOENN_JOURNAL_FLAGS + HOENN_JOURNAL_FLAGS_COUNT <= HOENN_21_JOURNAL_FLAGS + HOENN_21_JOURNAL_FLAGS_COUNT, JournalFlagsFit);
STATIC_ASSERT(FLAG_HOENN_GOT_SEEKERS_JOURNAL >= HOENN_21_LANE_A_FLAGS && FLAG_HOENN_GOT_SEEKERS_JOURNAL < HOENN_21_LANE_A_FLAGS + HOENN_21_LANE_A_FLAGS_COUNT, JournalGotFlagInLaneA);

#endif // IS_HNS

#if IS_HNS
// callnative HoennSeek_JournalWhenText: data[0] = the "when" line of TODAY's
// entry VAR_0x8005 (shown before its rumor or found line).
void HoennSeek_JournalWhenText(struct ScriptContext *ctx)
{
    const struct HoennJournalEntry *e = EntryAt(HOENN_JOURNAL_PAGE_TODAY, gSpecialVar_0x8005);

    ctx->data[0] = (uintptr_t)(e != NULL && e->when != NULL ? e->when->text : sText_NotHeard);
}

// ---- Birch's aide's field notes (src/hoenn_hints.c) -------------------------

// Found entries, and all entries, on every page.
u32 HoennJournalCountFound(u32 *total)
{
    u32 i, found = 0;

    for (i = 0; i < ARRAY_COUNT(sJournalEntries); i++)
        if (IsFound(&sJournalEntries[i]))
            found++;
    *total = ARRAY_COUNT(sJournalEntries);
    return found;
}

// The rumor the aide reads out: the first unfound entry you haven't heard of,
// Hoenn's page first (he's Birch's aide), else the first unfound one. Marks
// it heard. NULL when everything is found.
const u8 *HoennJournalFieldNote(void)
{
    static const u8 sOrder[] = {HOENN_JOURNAL_PAGE_HOENN, HOENN_JOURNAL_PAGE_JOHTO, HOENN_JOURNAL_PAGE_KANTO};
    const struct HoennJournalEntry *pick = NULL;
    u32 p, i;

    for (p = 0; p < ARRAY_COUNT(sOrder) && pick == NULL; p++)
        for (i = 0; i < ARRAY_COUNT(sJournalEntries) && pick == NULL; i++)
            if (sJournalEntries[i].page == sOrder[p] && !IsHeard(&sJournalEntries[i]))
                pick = &sJournalEntries[i];
    for (i = 0; i < ARRAY_COUNT(sJournalEntries) && pick == NULL; i++)
        if (!IsFound(&sJournalEntries[i]))
            pick = &sJournalEntries[i];
    if (pick == NULL)
        return NULL;
    FlagSet(pick->heardFlag);
    return pick->rumor;
}

// ---- The professors' rewards ------------------------------------------------

STATIC_ASSERT(HOENN_JOURNAL_PAGE_COUNT == 3, JournalRewardsArePerRegion);

bool32 HoennJournalPageComplete(u32 page)
{
    u32 found, total;

    CountPage(page, &found, &total);
    return total != 0 && found == total;
}

// specialvar VAR_RESULT, HoennJournalPageDone: VAR_0x8004 = page.
u16 HoennJournalPageDone(void)
{
    return HoennJournalPageComplete(gSpecialVar_0x8004);
}
#endif // IS_HNS
