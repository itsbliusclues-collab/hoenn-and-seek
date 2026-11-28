// Hoenn & Seek 2.1-21 to 23: fan-favorite chains (plan section 6, lane B).
//
// Brock, Misty and Blue each get a short story told one weekend at a time, four
// steps in all, ending with a secret base decoration; after that they have a pool
// of lines that follow what you've done (the plan's "deeper conversations").
// Data: data/hoenn/fan_chains.json → tools/hoenn/gen_hoenn_chains.py →
// src/data/hoenn_fan_chains.h (the table below) and data/scripts/hoenn_fan_chains.inc
// (the pool's texts). The scenes themselves are hand-written in
// data/scripts/hoenn_2_1_lane_b.inc.
//
// THE WEEKEND RULE
//   A chain is built on its leader's weekends (2.1-18 to 20): a step can only
//   happen while the leader is away from their gym, and only one step per
//   weekend. HoennChainWeekendId() names the weekend: today's day count minus
//   the days since Monday, so a Saturday and the Sunday after it share one ID
//   and the next Saturday has another. Like the leaders' weekends this reads the
//   calendar weekday, so changing the day at a wall clock counts as a new
//   weekend (Bryan, round 4: calendar, not progress; time travel is there for the
//   impatient). The ID of the weekend the last step happened on is saved in the
//   chain's weekend var; a step is "ready" on any other weekend day.
//
// WHERE THE LEADER IS
//   Each step happens at a place: the weekend spot itself (0 in the table) or a
//   chain object of its own, hidden by its own flag. HoennUpdateFanChains() runs
//   right after the weekend update on every map load: while the leader is away,
//   they're at the next step's place if it's ready, else at the place of the step
//   they last did (so they stay where you left them for the rest of that weekend),
//   and at the weekend spot before the first step and after the last. Away from
//   the spot, the spot object (and its follower) is hidden with the spot flag.
//
// THE POOL
//   callnative HoennSeek_ChainPoolLine + .2byte chain: the next line, in turn, of
//   the chain's lines whose conditions hold (the hint engine's conditions,
//   include/hoenn_hints.h). VAR_RESULT is FALSE if none holds; otherwise the
//   text is left in data[0] for `callstd MSGBOX_DEFAULT`, like the hint NPCs.
//   One turn counter (VAR_HOENN_CHAIN_POOL_TURN) serves every chain.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_hints.h"
#include "rtc.h"
#include "script.h"
#include "constants/hoenn_events.h"
#include "constants/hoenn_events_2_1.h"

#if IS_HNS

struct HoennChainLine
{
    const struct HoennHintCond *conds;
    u8 condCount;
    const u8 *text;
};

struct HoennFanChain
{
    u16 spotFlag;    // the leader's FLAG_HOENN_WEEKEND_SPOT_<KEY> (clear while away)
    u16 stageVar;    // steps done, 0-HOENN_CHAIN_DONE
    u16 weekendVar;  // HoennChainWeekendId() of the last step (0 = none yet)
    u16 places[HOENN_CHAIN_DONE]; // where each step happens: 0 = the weekend spot, else the chain object's flag
    const struct HoennChainLine *pool;
    u8 poolCount;
};

#include "data/hoenn_fan_chains.h"

static bool32 IsWeekend(void)
{
    u32 day = GetDayOfWeek();
    return day == HOENN_SATURDAY || day == HOENN_SUNDAY;
}

// One number per weekend (see the header): the day count of this week's Monday,
// + 7 so that it's never 0 (0 means "no step yet").
static u16 HoennChainWeekendId(void)
{
    RtcCalcLocalTime();
    return gLocalTime.days - (GetDayOfWeek() + WEEKDAY_COUNT - HOENN_MONDAY) % WEEKDAY_COUNT + WEEKDAY_COUNT;
}

static const struct HoennFanChain *GetChain(u32 id)
{
    return &sFanChains[id < ARRAY_COUNT(sFanChains) ? id : 0];
}

static bool32 StepReady(const struct HoennFanChain *c)
{
    return VarGet(c->stageVar) < HOENN_CHAIN_DONE && IsWeekend()
        && VarGet(c->weekendVar) != HoennChainWeekendId();
}

// The place (0 = weekend spot, else a chain object's flag) the leader is at this weekend.
static u16 ChainPlace(const struct HoennFanChain *c)
{
    u32 stage = VarGet(c->stageVar);

    if (stage >= HOENN_CHAIN_DONE)
        return 0;
    if (StepReady(c))
        return c->places[stage];
    if (stage == 0)
        return 0;
    return c->places[stage - 1];
}

// After HoennUpdateLeaderWeekends (src/hoenn_2_1_lane_b.c), on every map load.
void HoennUpdateFanChains(void)
{
    u32 i, j;

    for (i = 0; i < ARRAY_COUNT(sFanChains); i++)
    {
        const struct HoennFanChain *c = &sFanChains[i];
        u16 place;

        for (j = 0; j < HOENN_CHAIN_DONE; j++)
        {
            if (c->places[j] != 0)
                FlagSet(c->places[j]);
        }
        if (FlagGet(c->spotFlag)) // not away this weekend (or not a weekend)
            continue;
        place = ChainPlace(c);
        if (place != 0)
        {
            FlagSet(c->spotFlag);
            FlagClear(place);
        }
    }
}

// specialvar VAR_RESULT, HoennChainStepReady (VAR_0x8004 = HOENN_CHAIN_*):
// TRUE if the next step can happen now (a weekend other than the last step's).
u16 HoennChainStepReady(void)
{
    return StepReady(GetChain(gSpecialVar_0x8004));
}

// special HoennChainStepDone (VAR_0x8004 = HOENN_CHAIN_*): one step further,
// on this weekend.
void HoennChainStepDone(void)
{
    const struct HoennFanChain *c = GetChain(gSpecialVar_0x8004);

    if (VarGet(c->stageVar) < HOENN_CHAIN_DONE)
        VarSet(c->stageVar, VarGet(c->stageVar) + 1);
    VarSet(c->weekendVar, HoennChainWeekendId());
}

// callnative HoennSeek_ChainPoolLine, .2byte chain: see the header.
void HoennSeek_ChainPoolLine(struct ScriptContext *ctx)
{
    const struct HoennFanChain *c = GetChain(ScriptReadHalfword(ctx));
    u8 holding[32];
    u32 i, count = 0;
    u16 turn;

    for (i = 0; i < c->poolCount && count < ARRAY_COUNT(holding); i++)
    {
        if (HoennHintConditionsHold(c->pool[i].conds, c->pool[i].condCount))
            holding[count++] = i;
    }
    if (count == 0)
    {
        gSpecialVar_Result = FALSE;
        return;
    }
    turn = VarGet(VAR_HOENN_CHAIN_POOL_TURN);
    VarSet(VAR_HOENN_CHAIN_POOL_TURN, turn + 1);
    ctx->data[0] = (u32)c->pool[holding[turn % count]].text;
    gSpecialVar_Result = TRUE;
}

// Brendan's call once Brock's ONIX DOLL is yours (Hoenn21LaneBDueCall).
bool32 HoennBrockChainCallDue(void)
{
    return VarGet(VAR_HOENN_CHAIN_BROCK) >= HOENN_CHAIN_DONE && !FlagGet(FLAG_HOENN_CALL_BROCK_CHAIN);
}

#endif // IS_HNS
