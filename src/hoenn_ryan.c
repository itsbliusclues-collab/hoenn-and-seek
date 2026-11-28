// Hoenn & Seek 2.1-30: RYAN, the roguelike runner, and THE LIBRARIAN
// (2.1 plan section 14; an affectionate homage, written as an original character).
//
// Who they are
//   RYAN lives at the BATTLE FRONTIER and narrates every run, out loud, to an
//   audience at home. He's learning Mandarin and drops a phrase into most of
//   what he says, in pinyin with tone marks (the 14 toned vowels the font
//   lacked are drawn by tools/hoenn/gen_hoenn_pinyin_glyphs.py). He never
//   translates himself: THE LIBRARIAN, who stands near him wherever he is and
//   catalogues everything he says, does. That's the running gag.
//
// Where they are: the run of the day
//   RYAN runs a different facility each weekday, and stands in its lobby with
//   THE LIBRARIAN: SUN TOWER, MON FACTORY, TUE PIKE, WED ARENA, THU PALACE,
//   FRI DOME, SAT PYRAMID (the calendar weekday, so a wall clock moves him, as
//   with the leaders' weekends). Each of the seven lobbies has its own RYAN and
//   LIBRARIAN objects sharing one flag, FLAG_HOENN_RYAN_HIDE_<FACILITY>;
//   HoennUpdateRyan() sets all seven on every map load (from
//   HoennUpdateLeaderWeekends, before objects spawn), so he moves with the
//   next warp, never in front of you.
//
// What RYAN says (callnative HoennSeek_RyanLine; text left in data[0] for
// `callstd MSGBOX_DEFAULT`, like the hint NPCs)
//   - The first talk of each day (FLAG_HOENN_DAILY_RYAN_RUN): today's run, and
//     tomorrow's building, so you know where to find him next.
//   - After that, his pool: the lines whose condition holds take turns
//     (VAR_HOENN_RYAN_TURN). Conditions read your FRONTIER records (the best
//     record streak of any level mode and battle style, per facility), your
//     SYMBOLS and BATTLE POINTS.
//   - Every line carries the Mandarin phrase in it (sPhrases index, or 0 for
//     none); it's kept in VAR_HOENN_RYAN_PHRASE for THE LIBRARIAN.
//   The texts are all in data/scripts/hoenn_2_1_lane_b.inc's 2.1-30 section, in
//   one place for the dialogue pass with Bryan (2.1-36c).
//
// What THE LIBRARIAN says (callnative HoennSeek_LibrarianLine)
//   - With a phrase waiting: the catalogue entry (STR_VAR_1, numbered from
//     4,000 by VAR_HOENN_RYAN_ENTRY, one per thing RYAN says), the pinyin, its
//     meaning and a dry note. The phrase is then cleared.
//   - Otherwise one of their own lines, in turn (VAR_HOENN_LIBRARIAN_TURN); before
//     you've met RYAN, only that they don't look up from the notebook.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "rtc.h"
#include "script.h"
#include "string_util.h"
#include "constants/flags.h"
#include "constants/hoenn_events.h"
#include "constants/hoenn_events_2_1.h"

#if IS_HNS

enum RyanFacility
{
    RYAN_TOWER,
    RYAN_DOME,
    RYAN_PALACE,
    RYAN_ARENA,
    RYAN_FACTORY,
    RYAN_PIKE,
    RYAN_PYRAMID,
    RYAN_FACILITY_COUNT,
};

// The run of the day, by calendar weekday.
static const u8 sRunOfTheDay[7] =
{
    [HOENN_SUNDAY]    = RYAN_TOWER,
    [HOENN_MONDAY]    = RYAN_FACTORY,
    [HOENN_MONDAY + 1] = RYAN_PIKE,     // Tuesday
    [HOENN_MONDAY + 2] = RYAN_ARENA,    // Wednesday
    [HOENN_MONDAY + 3] = RYAN_PALACE,   // Thursday
    [HOENN_MONDAY + 4] = RYAN_DOME,     // Friday
    [HOENN_SATURDAY]  = RYAN_PYRAMID,
};

// One flag per lobby, shared by its RYAN and LIBRARIAN objects (set = hidden).
static const u16 sHideFlags[RYAN_FACILITY_COUNT] =
{
    [RYAN_TOWER]   = FLAG_HOENN_RYAN_HIDE_TOWER,
    [RYAN_DOME]    = FLAG_HOENN_RYAN_HIDE_DOME,
    [RYAN_PALACE]  = FLAG_HOENN_RYAN_HIDE_PALACE,
    [RYAN_ARENA]   = FLAG_HOENN_RYAN_HIDE_ARENA,
    [RYAN_FACTORY] = FLAG_HOENN_RYAN_HIDE_FACTORY,
    [RYAN_PIKE]    = FLAG_HOENN_RYAN_HIDE_PIKE,
    [RYAN_PYRAMID] = FLAG_HOENN_RYAN_HIDE_PYRAMID,
};

// RYAN's Mandarin phrases, by number (VAR_HOENN_RYAN_PHRASE); THE LIBRARIAN's
// entry for each one (pinyin, meaning, note) is the text of the same number.
enum RyanPhrase
{
    PHRASE_NONE,
    PHRASE_WELCOME,     // Nǚshìmen, xiānshengmen, huānyíng!
    PHRASE_BAD_LUCK,    // Wǒ de yùnqì tài chà le.
    PHRASE_STRONG,      // Zhè ge hěn qiáng.
    PHRASE_ADD_OIL,     // Jiāyóu!
    PHRASE_FINISHED,    // Wǒ wán le.
    PHRASE_NO_PROBLEM,  // Méi wèntí.
    PHRASE_IMPOSSIBLE,  // Bù kěnéng!
    PHRASE_AGAIN,       // Zài lái yí cì.
    PHRASE_RANDOM,      // Suíjīshù bù xǐhuan wǒ.
    PHRASE_FATE,        // Zhè shì mìngyùn.
    PHRASE_CAREFUL,     // Xiǎoxīn!
    PHRASE_GENIUS,      // Wǒ zhēn shì tiāncái.
    PHRASE_TOO_HARD,    // Tài nán le.
    PHRASE_DOORS,       // Hóng mén háishi lǜ mén?
    PHRASE_DONKEY,      // Wǒ shì lǘ.
    PHRASE_COUNT,
};

extern const u8 HoennSeek_Text_LibrarianPhraseWelcome[];
extern const u8 HoennSeek_Text_LibrarianPhraseBadLuck[];
extern const u8 HoennSeek_Text_LibrarianPhraseStrong[];
extern const u8 HoennSeek_Text_LibrarianPhraseAddOil[];
extern const u8 HoennSeek_Text_LibrarianPhraseFinished[];
extern const u8 HoennSeek_Text_LibrarianPhraseNoProblem[];
extern const u8 HoennSeek_Text_LibrarianPhraseImpossible[];
extern const u8 HoennSeek_Text_LibrarianPhraseAgain[];
extern const u8 HoennSeek_Text_LibrarianPhraseRandom[];
extern const u8 HoennSeek_Text_LibrarianPhraseFate[];
extern const u8 HoennSeek_Text_LibrarianPhraseCareful[];
extern const u8 HoennSeek_Text_LibrarianPhraseGenius[];
extern const u8 HoennSeek_Text_LibrarianPhraseTooHard[];
extern const u8 HoennSeek_Text_LibrarianPhraseDoors[];
extern const u8 HoennSeek_Text_LibrarianPhraseDonkey[];

static const u8 *const sPhrases[PHRASE_COUNT] =
{
    [PHRASE_WELCOME]    = HoennSeek_Text_LibrarianPhraseWelcome,
    [PHRASE_BAD_LUCK]   = HoennSeek_Text_LibrarianPhraseBadLuck,
    [PHRASE_STRONG]     = HoennSeek_Text_LibrarianPhraseStrong,
    [PHRASE_ADD_OIL]    = HoennSeek_Text_LibrarianPhraseAddOil,
    [PHRASE_FINISHED]   = HoennSeek_Text_LibrarianPhraseFinished,
    [PHRASE_NO_PROBLEM] = HoennSeek_Text_LibrarianPhraseNoProblem,
    [PHRASE_IMPOSSIBLE] = HoennSeek_Text_LibrarianPhraseImpossible,
    [PHRASE_AGAIN]      = HoennSeek_Text_LibrarianPhraseAgain,
    [PHRASE_RANDOM]     = HoennSeek_Text_LibrarianPhraseRandom,
    [PHRASE_FATE]       = HoennSeek_Text_LibrarianPhraseFate,
    [PHRASE_CAREFUL]    = HoennSeek_Text_LibrarianPhraseCareful,
    [PHRASE_GENIUS]     = HoennSeek_Text_LibrarianPhraseGenius,
    [PHRASE_TOO_HARD]   = HoennSeek_Text_LibrarianPhraseTooHard,
    [PHRASE_DOORS]      = HoennSeek_Text_LibrarianPhraseDoors,
    [PHRASE_DONKEY]     = HoennSeek_Text_LibrarianPhraseDonkey,
};

// The run of the day's texts, by weekday (each ends with tomorrow's building).
extern const u8 HoennSeek_Text_RyanRunSun[];
extern const u8 HoennSeek_Text_RyanRunMon[];
extern const u8 HoennSeek_Text_RyanRunTue[];
extern const u8 HoennSeek_Text_RyanRunWed[];
extern const u8 HoennSeek_Text_RyanRunThu[];
extern const u8 HoennSeek_Text_RyanRunFri[];
extern const u8 HoennSeek_Text_RyanRunSat[];

struct RyanText
{
    const u8 *text;
    u8 phrase;
};

static const struct RyanText sRunTexts[7] =
{
    [HOENN_SUNDAY]     = {HoennSeek_Text_RyanRunSun, PHRASE_FATE},
    [HOENN_MONDAY]     = {HoennSeek_Text_RyanRunMon, PHRASE_STRONG},
    [HOENN_MONDAY + 1] = {HoennSeek_Text_RyanRunTue, PHRASE_DOORS},
    [HOENN_MONDAY + 2] = {HoennSeek_Text_RyanRunWed, PHRASE_DONKEY},
    [HOENN_MONDAY + 3] = {HoennSeek_Text_RyanRunThu, PHRASE_IMPOSSIBLE},
    [HOENN_MONDAY + 4] = {HoennSeek_Text_RyanRunFri, PHRASE_FINISHED},
    [HOENN_SATURDAY]   = {HoennSeek_Text_RyanRunSat, PHRASE_ADD_OIL},
};

// The pool. A line is said only while its condition holds; the holding lines
// take turns, in this order.
enum RyanIf
{
    RYAN_IF_ALWAYS,
    RYAN_IF_NO_RUNS,   // no record at any facility yet
    RYAN_IF_RECORD,    // your record at `facility` is at least `value`
    RYAN_IF_FLAG,      // flag `value` is set
    RYAN_IF_ANY_GOLD,  // any of the seven GOLD SYMBOLS
    RYAN_IF_BP,        // you hold at least `value` BATTLE POINTS
};

struct RyanLine
{
    u8 type;
    u8 facility;
    u16 value;
    struct RyanText say;
};

extern const u8 HoennSeek_Text_RyanItemLuck[];
extern const u8 HoennSeek_Text_RyanMiddle[];
extern const u8 HoennSeek_Text_RyanTheOne[];
extern const u8 HoennSeek_Text_RyanRandom[];
extern const u8 HoennSeek_Text_RyanNotebook[];
extern const u8 HoennSeek_Text_RyanStop[];
extern const u8 HoennSeek_Text_RyanTones[];
extern const u8 HoennSeek_Text_RyanNoRuns[];
extern const u8 HoennSeek_Text_RyanFactoryRecord[];
extern const u8 HoennSeek_Text_RyanPyramidRecord[];
extern const u8 HoennSeek_Text_RyanPikeRecord[];
extern const u8 HoennSeek_Text_RyanTowerRecord[];
extern const u8 HoennSeek_Text_RyanSilverSymbols[];
extern const u8 HoennSeek_Text_RyanGoldSymbol[];
extern const u8 HoennSeek_Text_RyanPoints[];

static const struct RyanLine sPool[] =
{
    {RYAN_IF_NO_RUNS,  0,             0,   {HoennSeek_Text_RyanNoRuns,        PHRASE_ADD_OIL}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanItemLuck,      PHRASE_BAD_LUCK}},
    {RYAN_IF_RECORD,   RYAN_FACTORY,  21,  {HoennSeek_Text_RyanFactoryRecord, PHRASE_IMPOSSIBLE}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanMiddle,        PHRASE_NO_PROBLEM}},
    {RYAN_IF_RECORD,   RYAN_PYRAMID,  21,  {HoennSeek_Text_RyanPyramidRecord, PHRASE_GENIUS}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanTheOne,        PHRASE_AGAIN}},
    {RYAN_IF_RECORD,   RYAN_PIKE,     28,  {HoennSeek_Text_RyanPikeRecord,    PHRASE_DOORS}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanRandom,        PHRASE_RANDOM}},
    {RYAN_IF_RECORD,   RYAN_TOWER,    35,  {HoennSeek_Text_RyanTowerRecord,   PHRASE_STRONG}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanNotebook,      PHRASE_CAREFUL}},
    {RYAN_IF_FLAG,     0, FLAG_COLLECTED_ALL_SILVER_SYMBOLS, {HoennSeek_Text_RyanSilverSymbols, PHRASE_FINISHED}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanStop,          PHRASE_TOO_HARD}},
    {RYAN_IF_ANY_GOLD, 0,             0,   {HoennSeek_Text_RyanGoldSymbol,    PHRASE_IMPOSSIBLE}},
    {RYAN_IF_ALWAYS,   0,             0,   {HoennSeek_Text_RyanTones,         PHRASE_GENIUS}},
    {RYAN_IF_BP,       0,             100, {HoennSeek_Text_RyanPoints,        PHRASE_AGAIN}},
};

// THE LIBRARIAN's own lines, when there's nothing to translate.
extern const u8 HoennSeek_Text_LibrarianNotMet[];
extern const u8 HoennSeek_Text_LibrarianVolume[];
extern const u8 HoennSeek_Text_LibrarianTheOne[];
extern const u8 HoennSeek_Text_LibrarianTones[];
extern const u8 HoennSeek_Text_LibrarianBorrow[];

static const u8 *const sLibrarianLines[] =
{
    HoennSeek_Text_LibrarianVolume,
    HoennSeek_Text_LibrarianTheOne,
    HoennSeek_Text_LibrarianTones,
    HoennSeek_Text_LibrarianBorrow,
};

// The best record streak at a facility: any level mode, any battle style.
// (DOME counts tournaments won, PIKE rooms and PYRAMID floors, as the game does.)
static u16 MaxOf(const u16 *values, u32 count)
{
    u32 i;
    u16 best = 0;

    for (i = 0; i < count; i++)
    {
        if (values[i] > best)
            best = values[i];
    }
    return best;
}

static u16 RyanRecord(u32 facility)
{
    struct BattleFrontier *f = &gSaveBlock2Ptr->frontier;

    switch (facility)
    {
    case RYAN_TOWER:   return MaxOf(&f->towerRecordWinStreaks[0][0], ARRAY_COUNT(f->towerRecordWinStreaks) * FRONTIER_LVL_MODE_COUNT);
    case RYAN_DOME:    return MaxOf(&f->domeRecordWinStreaks[0][0], ARRAY_COUNT(f->domeRecordWinStreaks) * FRONTIER_LVL_MODE_COUNT);
    case RYAN_PALACE:  return MaxOf(&f->palaceRecordWinStreaks[0][0], ARRAY_COUNT(f->palaceRecordWinStreaks) * FRONTIER_LVL_MODE_COUNT);
    case RYAN_ARENA:   return MaxOf(f->arenaRecordStreaks, FRONTIER_LVL_MODE_COUNT);
    case RYAN_FACTORY: return MaxOf(&f->factoryRecordWinStreaks[0][0], ARRAY_COUNT(f->factoryRecordWinStreaks) * FRONTIER_LVL_MODE_COUNT);
    case RYAN_PIKE:    return MaxOf(f->pikeRecordStreaks, FRONTIER_LVL_MODE_COUNT);
    case RYAN_PYRAMID: return MaxOf(f->pyramidRecordStreaks, FRONTIER_LVL_MODE_COUNT);
    }
    return 0;
}

static bool32 RyanLineHolds(const struct RyanLine *line)
{
    u32 i;

    switch (line->type)
    {
    case RYAN_IF_ALWAYS:
        return TRUE;
    case RYAN_IF_NO_RUNS:
        for (i = 0; i < RYAN_FACILITY_COUNT; i++)
        {
            if (RyanRecord(i) != 0)
                return FALSE;
        }
        return TRUE;
    case RYAN_IF_RECORD:
        return RyanRecord(line->facility) >= line->value;
    case RYAN_IF_FLAG:
        return FlagGet(line->value);
    case RYAN_IF_ANY_GOLD:
        // The GOLD flags sit right after each SILVER one (FLAG_SYS_TOWER_SILVER, ..._GOLD, ...).
        for (i = 0; i < RYAN_FACILITY_COUNT; i++)
        {
            if (FlagGet(FLAG_SYS_TOWER_GOLD + 2 * i))
                return TRUE;
        }
        return FALSE;
    case RYAN_IF_BP:
        return gSaveBlock2Ptr->frontier.battlePoints >= line->value;
    }
    return FALSE;
}

static u32 Today(void)
{
    u32 day = GetDayOfWeek();
    return day < 7 ? day : HOENN_SUNDAY;
}

// After HoennUpdateFanChains, on every map load: RYAN and THE LIBRARIAN are in
// the lobby of today's run, and only there.
void HoennUpdateRyan(void)
{
    u32 i, today = sRunOfTheDay[Today()];

    for (i = 0; i < RYAN_FACILITY_COUNT; i++)
    {
        if (i == today)
            FlagClear(sHideFlags[i]);
        else
            FlagSet(sHideFlags[i]);
    }
}

static void Say(struct ScriptContext *ctx, const struct RyanText *say)
{
    ctx->data[0] = (u32)say->text;
    VarSet(VAR_HOENN_RYAN_PHRASE, say->phrase);
    VarSet(VAR_HOENN_RYAN_ENTRY, VarGet(VAR_HOENN_RYAN_ENTRY) + 1);
}

// callnative HoennSeek_RyanLine: see the header. Called once you've met him
// (the first meeting is its own scene, which sets the phrase itself).
void HoennSeek_RyanLine(struct ScriptContext *ctx)
{
    u8 holding[ARRAY_COUNT(sPool)];
    u32 i, count = 0;
    u16 turn;

    if (!FlagGet(FLAG_HOENN_DAILY_RYAN_RUN))
    {
        FlagSet(FLAG_HOENN_DAILY_RYAN_RUN);
        Say(ctx, &sRunTexts[Today()]);
        return;
    }
    for (i = 0; i < ARRAY_COUNT(sPool); i++)
    {
        if (RyanLineHolds(&sPool[i]))
            holding[count++] = i;
    }
    // count >= 1: the pool has RYAN_IF_ALWAYS lines.
    turn = VarGet(VAR_HOENN_RYAN_TURN);
    VarSet(VAR_HOENN_RYAN_TURN, turn + 1);
    Say(ctx, &sPool[holding[turn % count]].say);
}

// callnative HoennSeek_LibrarianLine: see the header.
void HoennSeek_LibrarianLine(struct ScriptContext *ctx)
{
    u16 phrase = VarGet(VAR_HOENN_RYAN_PHRASE);
    u16 turn;

    if (!FlagGet(FLAG_HOENN_RYAN_MET))
    {
        ctx->data[0] = (u32)HoennSeek_Text_LibrarianNotMet;
        return;
    }
    if (phrase != PHRASE_NONE && phrase < PHRASE_COUNT)
    {
        ConvertIntToDecimalStringN(gStringVar1, 4000 + VarGet(VAR_HOENN_RYAN_ENTRY), STR_CONV_MODE_LEFT_ALIGN, 5);
        ctx->data[0] = (u32)sPhrases[phrase];
        VarSet(VAR_HOENN_RYAN_PHRASE, PHRASE_NONE);
        return;
    }
    turn = VarGet(VAR_HOENN_LIBRARIAN_TURN);
    VarSet(VAR_HOENN_LIBRARIAN_TURN, turn + 1);
    ctx->data[0] = (u32)sLibrarianLines[turn % ARRAY_COUNT(sLibrarianLines)];
}

// special HoennRyanMet: the first meeting's end (its phrase is the welcome; the
// next talk is today's run).
void HoennRyanMet(void)
{
    FlagSet(FLAG_HOENN_RYAN_MET);
    VarSet(VAR_HOENN_RYAN_PHRASE, PHRASE_WELCOME);
    VarSet(VAR_HOENN_RYAN_ENTRY, VarGet(VAR_HOENN_RYAN_ENTRY) + 1);
}

#endif // IS_HNS
