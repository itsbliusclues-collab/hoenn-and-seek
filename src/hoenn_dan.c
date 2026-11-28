// Hoenn & Seek 2.1-31: DAN, the history broadcaster (a lane B part, built by
// lane A, so it uses lane A's IDs).
//
// DAN stands on the PEWTER MUSEUM's second floor and tells long "episodes" of
// POKéMON history, one installment each time you talk to him. The episodes are
// data: data/hoenn/dan_episodes.json -> tools/hoenn/gen_hoenn_dan.py ->
// src/data/hoenn_dan.h + data/scripts/hoenn_dan_texts.inc. The scene is
// data/scripts/hoenn_dan.inc.
//
// State, two vars:
//   VAR_HOENN_DAN_EPISODE  the episode he's on (index into sDanEpisodes)
//   VAR_HOENN_DAN_PART     installments of it you've heard (numParts = finished)
// Finishing an episode sets FLAG_HOENN_WEEKLY_DAN, so the next one starts once a
// new week does (the weekly reset, src/hoenn_time.c). Changing the day at a wall
// clock counts, as with every weekly event (2.1 plan, "calendar, not progress").
// Finishing the last one sets FLAG_HOENN_DAN_ALL_TOLD. Any finished episode can be
// heard again from his replay menu.
//
// His PokéGear show, THE DEEP PAST, is a radio segment on PLACES AND PEOPLE
// (data/hoenn/radio_segments.json, DAN_SHOW): HoennRadioDan says which episode
// is on this week.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_radio.h"
#include "malloc.h"
#include "script.h"
#include "script_menu.h"
#include "string_util.h"
#include "list_menu.h"
#include "rtc.h"
#include "constants/hoenn_events_2_1.h"

#if IS_HNS

#define HOENN_DAN_MAX_PARTS 4 // gen_hoenn_dan.py's MAX_PARTS
#define HOENN_DAN_MAX_RADIO 4 // and MAX_RADIO

struct HoennDanEpisode
{
    const u8 *title;
    const u8 *parts[HOENN_DAN_MAX_PARTS];
    u8 numParts;
    const u8 *teaser;      // said at the end of the episode before this one (NULL for the first)
    const u8 *radio[HOENN_DAN_MAX_RADIO];
    u8 numRadio;
};

#include "data/hoenn_dan.h"

extern const u8 HoennSeek_Text_Dan_Outro[];

#define EPISODE_COUNT ARRAY_COUNT(sDanEpisodes)

static u32 Episode(void)
{
    u32 ep = VarGet(VAR_HOENN_DAN_EPISODE);
    return ep < EPISODE_COUNT ? ep : EPISODE_COUNT - 1;
}

static bool32 EpisodeFinished(u32 ep)
{
    return VarGet(VAR_HOENN_DAN_PART) >= sDanEpisodes[ep].numParts;
}

// The episode a new talk (or this week's show) is about, without changing
// anything: the current one while it has installments left, else the next one
// once the week has turned. *state gets HOENN_DAN_* (see hoenn_events_2_1.h).
static u32 UpcomingEpisode(u32 *state)
{
    u32 ep = Episode();

    if (!EpisodeFinished(ep))
        *state = VarGet(VAR_HOENN_DAN_PART) == 0 ? HOENN_DAN_NEW : HOENN_DAN_CONTINUE;
    else if (ep + 1 >= EPISODE_COUNT)
        *state = HOENN_DAN_DONE;
    else if (FlagGet(FLAG_HOENN_WEEKLY_DAN))
        *state = HOENN_DAN_WAIT;
    else
        *state = HOENN_DAN_NEW, ep++;
    return ep;
}

// specialvar VAR_RESULT, HoennDanState: HOENN_DAN_NEW (a new episode starts:
// the vars move on to it), _CONTINUE (its next installment), _WAIT (finished,
// the next one comes with the new week) or _DONE (all told). STR_VAR_1 = the
// episode's number, STR_VAR_2 its title, STR_VAR_3 the installment's number.
// For _WAIT and _DONE they describe the episode just finished.
u16 HoennDanState(void)
{
    u32 state;
    u32 ep = UpcomingEpisode(&state);

    if (ep != VarGet(VAR_HOENN_DAN_EPISODE))
    {
        VarSet(VAR_HOENN_DAN_EPISODE, ep);
        VarSet(VAR_HOENN_DAN_PART, 0);
    }
    ConvertIntToDecimalStringN(gStringVar1, ep + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    StringCopy(gStringVar2, sDanEpisodes[ep].title);
    ConvertIntToDecimalStringN(gStringVar3, VarGet(VAR_HOENN_DAN_PART) + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
    return state;
}

// special HoennDanPartDone, after an installment: counts it, and on the
// episode's last one sets the weekly flag (and FLAG_HOENN_DAN_ALL_TOLD after
// the last episode). VAR_RESULT = TRUE if that finished the episode.
void HoennDanPartDone(void)
{
    u32 ep = Episode();
    u32 part = VarGet(VAR_HOENN_DAN_PART);

    if (part < sDanEpisodes[ep].numParts)
        VarSet(VAR_HOENN_DAN_PART, ++part);
    gSpecialVar_Result = part >= sDanEpisodes[ep].numParts;
    if (gSpecialVar_Result)
    {
        FlagSet(FLAG_HOENN_WEEKLY_DAN);
        if (ep + 1 >= EPISODE_COUNT)
            FlagSet(FLAG_HOENN_DAN_ALL_TOLD);
    }
}

// callnative HoennSeek_DanText, .byte HOENN_DAN_TEXT_*: ctx->data[0] = that
// text, for callstd MSGBOX_* (like the hint NPCs):
//   _PART    the current episode's next installment
//   _TEASER  the next episode's teaser, or the outro after the last one
//   _REPLAY  installment VAR_0x8006 of episode VAR_0x8005 (the replay menu)
void HoennSeek_DanText(struct ScriptContext *ctx)
{
    u32 which = ScriptReadByte(ctx);
    u32 ep = Episode(), part = VarGet(VAR_HOENN_DAN_PART);
    const u8 *text;

    switch (which)
    {
    case HOENN_DAN_TEXT_PART:
    default:
        text = sDanEpisodes[ep].parts[part < sDanEpisodes[ep].numParts ? part : sDanEpisodes[ep].numParts - 1];
        break;
    case HOENN_DAN_TEXT_TEASER:
        text = ep + 1 < EPISODE_COUNT ? sDanEpisodes[ep + 1].teaser : NULL;
        if (text == NULL)
            text = HoennSeek_Text_Dan_Outro;
        break;
    case HOENN_DAN_TEXT_REPLAY:
        ep = gSpecialVar_0x8005 < EPISODE_COUNT ? gSpecialVar_0x8005 : 0;
        part = gSpecialVar_0x8006 < sDanEpisodes[ep].numParts ? gSpecialVar_0x8006 : 0;
        text = sDanEpisodes[ep].parts[part];
        break;
    }
    ctx->data[0] = (uintptr_t)text;
}

// How many episodes you've heard to the end (the replay menu's length).
static u32 FinishedEpisodes(void)
{
    u32 ep = Episode();
    return EpisodeFinished(ep) ? ep + 1 : ep;
}

// specialvar VAR_RESULT, HoennDanFinishedCount
u16 HoennDanFinishedCount(void)
{
    return FinishedEpisodes();
}

// special HoennDanPushEpisodes: fills the dynmultistack list with the finished
// episodes' titles (ids = episode indexes). Dynamic menus free every name when
// they close, so each is an Alloc'ed copy (CLAUDE.md, part 14a).
void HoennDanPushEpisodes(void)
{
    u32 i, n = FinishedEpisodes();
    struct ListMenuItem entry;

    for (i = 0; i < n; i++)
    {
        u8 *name = Alloc(StringLength(sDanEpisodes[i].title) + 1);
        if (name == NULL)
            break;
        StringCopy(name, sDanEpisodes[i].title);
        entry.name = name;
        entry.id = i;
        MultichoiceDynamic_PushElement(entry);
    }
}

// specialvar VAR_RESULT, HoennDanReplayNext: moves VAR_0x8006 to the next
// installment of episode VAR_0x8005; FALSE once there isn't one.
u16 HoennDanReplayNext(void)
{
    u32 ep = gSpecialVar_0x8005 < EPISODE_COUNT ? gSpecialVar_0x8005 : 0;

    gSpecialVar_0x8006++;
    return gSpecialVar_0x8006 < sDanEpisodes[ep].numParts;
}

// The PokéGear show (radio segment DAN_SHOW on PLACES AND PEOPLE): this week's
// episode (the one a talk would start or continue; next week's if you've
// finished this one; a rerun picked by the weekday once all are told).
u32 HoennRadioDan(const u8 **lines, u8 *bufs)
{
    static const u8 sIntro[] = _("THE DEEP PAST, with DAN.");
    static const u8 sThisWeek[] = _("This week's episode:");
    static const u8 sNextWeek[] = _("Next week's episode:");
    static const u8 sRerun[] = _("Tonight, from the archive:");
    static const u8 sOutro1[] = _("Hear the whole story at the");
    static const u8 sOutro2[] = _("PEWTER MUSEUM, second floor.");
    u32 state, ep, n = 0, i;

    ep = UpcomingEpisode(&state);
    if (state == HOENN_DAN_WAIT)
        ep++;
    else if (state == HOENN_DAN_DONE)
        ep = GetDayOfWeek() % EPISODE_COUNT;

    lines[n++] = sIntro;
    if (state == HOENN_DAN_WAIT)
        lines[n++] = sNextWeek;
    else if (state == HOENN_DAN_DONE)
        lines[n++] = sRerun;
    else
        lines[n++] = sThisWeek;
    for (i = 0; i < sDanEpisodes[ep].numRadio && n < 6; i++)
        lines[n++] = sDanEpisodes[ep].radio[i];
    lines[n++] = sOutro1;
    lines[n++] = sOutro2;
    return n;
}

// Brendan's call (HOENN_CALL_DAN): once you've heard three episodes to the end.
bool32 HoennDanCallDue(void)
{
    return FinishedEpisodes() >= 3;
}

#endif // IS_HNS
