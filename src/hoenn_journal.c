// Hoenn & Seek 2.1-05: the SEEKER'S JOURNAL (track A; 2.1 plan section 15).
//
// A key item that lists 2.0's (and later 2.1's) hidden, daily and weekly
// events by region. Data: data/hoenn/journal.json →
// tools/hoenn/gen_hoenn_journal.py → src/data/hoenn_journal.h (included below),
// the heard flags in include/constants/hoenn_journal.h and the texts in
// data/scripts/hoenn_journal_texts.inc. The script is
// HoennSeek_EventScript_SeekersJournal (data/scripts/hoenn_2_1_a.inc), run
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
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_hints.h"
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

struct HoennJournalEntry
{
    const struct HoennHintCond *found;
    u8 foundCount;
    const struct HoennHintCond *heardIf; // any one of them holding counts as heard
    u8 heardIfCount;
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

static bool32 IsFound(const struct HoennJournalEntry *e)
{
    return HoennHintCondsHold(e->found, e->foundCount);
}

static bool32 IsHeard(const struct HoennJournalEntry *e)
{
    u32 i;

    if (FlagGet(e->heardFlag) || IsFound(e))
        return TRUE;
    for (i = 0; i < e->heardIfCount; i++)
        if (HoennHintCondsHold(&e->heardIf[i], 1))
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

// The entry at place `slot` on `page`, or NULL.
static const struct HoennJournalEntry *EntryAt(u32 page, u32 slot)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sJournalEntries); i++)
        if (sJournalEntries[i].page == page && slot-- == 0)
            return &sJournalEntries[i];
    return NULL;
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

// special HoennJournalPushPages: "JOHTO 3/22" per page, then CLOSE
// (id HOENN_JOURNAL_PAGE_COUNT).
void HoennJournalPushPages(void)
{
    u8 buf[24];
    u32 page, found, total;
    u8 *end;

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
    PushName(sText_Close, HOENN_JOURNAL_PAGE_COUNT);
}

// special HoennJournalPushEntries: page VAR_0x8004's entries (id = place on
// the page), then BACK (HOENN_JOURNAL_MENU_BACK). STR_VAR_1 = the page's
// name, STR_VAR_2 = found, STR_VAR_3 = total.
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

// The heard flags are one block at the start of track A's flags.
STATIC_ASSERT(HOENN_JOURNAL_FLAGS + HOENN_JOURNAL_FLAGS_COUNT <= HOENN_TRACK_A_FLAGS + HOENN_TRACK_A_FLAGS_COUNT, JournalFlagsFit);
STATIC_ASSERT(FLAG_HOENN_GOT_SEEKERS_JOURNAL >= HOENN_JOURNAL_FLAGS + HOENN_JOURNAL_FLAGS_COUNT, JournalFlagsBeforeTrackA);

#endif // IS_HNS
