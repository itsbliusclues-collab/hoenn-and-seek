// Hoenn & Seek 2.1, lane B (hoenn-docs/lanes/2.1-lanes.md; status in 2.1-lane-b.md).
//
// Each 2.1 lane keeps its C code in its own file (or a new src/hoenn_*.c of its
// own) so the two lanes never edit the same lines. Scripts and texts:
// data/scripts/hoenn_2_1_lane_b.inc. Flags and vars: lane B's section of
// constants/hoenn_events_2_1.h.
//
// - Hoenn21LaneBDueCall(): lane B's PokéGear event calls. The chain is
//   GetDueEventCall (src/hoenn_phone.c) → ... → HoennLaneBDueCall (2.0's last)
//   → Hoenn21LaneADueCall → Hoenn21LaneBDueCall. Add one `if` per call, in the
//   order they should ring; IDs come from lane B's 2.1 range in constants/hoenn_phone.h.
// - 2.1-34: Teleport to your secret base (below; plan section 17).
// - 2.1-21 to 23: fan-favorite chains: src/hoenn_fan_chains.c (run from the weekend update).
// - 2.1-18 to 20: gym leaders' weekends (below; plan section 6). A leader you've
//   beaten takes Saturdays and Sundays off. tools/hoenn/gen_hoenn_weekends.py
// turns data/hoenn/leader_weekends.json into three flags per leader, the table
// below, the weekend spots' and gym guides' scripts and the map.json edits:
//   - the gym object (and its follower) takes FLAG_HOENN_WEEKEND_GYM_<KEY>,
//     set while HnS's own story flag hides the leader, or while they're away;
//   - the weekend spot object (and its follower) takes FLAG_HOENN_WEEKEND_SPOT_<KEY>,
//     clear only while they're away;
//   - the Dojo copy (Saffron, HnS's post-game rematch) and its Pokémon take
//     FLAG_HOENN_WEEKEND_DOJO_<KEY>, set while HnS's FLAG_HIDE_DOJO_* is set or
//     they're away. The weekend spot offers that same rematch once it's open.
// HoennUpdateLeaderWeekends() sets them on every map load (one hook in each of
// overworld.c's two map loaders, before the OnTransition script), so a leader
// leaves or comes back with the next warp or map edge, never in front of you.
// It reads the calendar weekday (GetDayOfWeek), so changing the day at a wall
// clock moves them too (Bryan, round 4: calendar, not progress).
#include "global.h"
#include "event_data.h"
#include "field_effect.h"
#include "hoenn.h"
#include "overworld.h"
#include "party_menu.h"
#include "region_map.h"
#include "script.h"
#include "string_util.h"
#include "rtc.h"
#include "constants/field_effects.h"
#include "constants/flags.h"
#include "constants/hoenn_events.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/hoenn_phone.h"

#if IS_HNS

struct HoennLeaderWeekend
{
    u16 beaten;    // set once you have the badge
    u16 storyHide; // HnS's own flag for the gym object, or 0
    u16 dojoHide;  // HnS's FLAG_HIDE_DOJO_*, or 0
    u16 gymFlag;
    u16 spotFlag;
    u16 dojoFlag;  // 0 if the leader has no Dojo copy
    u16 cameo;     // HnS's flag for the leader's one-time cameo (clear while it's out), or 0
};

#include "data/hoenn_leader_weekends.h"

static void FlagSetTo(u16 flag, bool32 set)
{
    if (set)
        FlagSet(flag);
    else
        FlagClear(flag);
}

static bool32 IsWeekend(void)
{
    u32 day = GetDayOfWeek();
    return day == HOENN_SATURDAY || day == HOENN_SUNDAY;
}

void HoennUpdateLeaderWeekends(void)
{
    bool32 weekend = IsWeekend();
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLeaderWeekends); i++)
    {
        const struct HoennLeaderWeekend *l = &sLeaderWeekends[i];
        bool32 storyHidden = l->storyHide != 0 && FlagGet(l->storyHide);
        // While HnS's cameo is out (the leader waits somewhere to open their Dojo
        // rematch), they keep their usual days, so they're never in two places.
        bool32 cameoOut = l->cameo != 0 && !FlagGet(l->cameo);
        bool32 away = weekend && FlagGet(l->beaten) && !storyHidden && !cameoOut;

        FlagSetTo(l->gymFlag, storyHidden || away);
        FlagSetTo(l->spotFlag, !away);
        if (l->dojoFlag != 0)
            FlagSetTo(l->dojoFlag, FlagGet(l->dojoHide) || away);
    }
    HoennUpdateFanChains(); // 2.1-21: a chain step can move a leader off their spot
}

// TRUE while any leader is at their weekend spot (May's call).
static bool32 AnyLeaderAway(void)
{
    u32 i;

    for (i = 0; i < ARRAY_COUNT(sLeaderWeekends); i++)
    {
        if (!FlagGet(sLeaderWeekends[i].spotFlag))
            return TRUE;
    }
    return FALSE;
}

u16 Hoenn21LaneBDueCall(void)
{
    // 2.1-18: May's dad reads the LEAGUE's memos; she calls the first weekend a
    // leader you've beaten is away.
    if (FlagGet(FLAG_HOENN_BM_PHONE) && !FlagGet(FLAG_HOENN_CALL_WEEKENDS) && AnyLeaderAway())
        return HOENN_CALL_WEEKENDS;
    // 2.1-21: Brendan heard about Brock's ONIX DOLL at the PEWTER MUSEUM.
    if (FlagGet(FLAG_HOENN_BM_PHONE) && HoennBrockChainCallDue())
        return HOENN_CALL_BROCK_CHAIN;
    return HOENN_CALL_NONE; // the end of the chain
}

// ---------------------------------------------------------------------------
// 2.1-34: Teleport to your secret base (plan section 17).
//
// Emerald keeps only your base's ID, not where its entrance is: leaving a base
// uses the dynamic warp that walking in set (the map and the tile you stood
// on). So every time you walk into your own base, EnterSecretBase
// (src/secret_base.c, one hook line) records that map and tile here, with the
// base's ID. Teleport (src/fldeff_teleport.c, one hook line) then asks
// POKéMON CENTER / SECRET BASE / CANCEL while the recorded base is still yours
// (Secret Power elsewhere makes a new base, which records itself when you walk
// in; moving out clears secretBases[0], so the IDs stop matching). Teleport
// itself is only allowed outdoors (Overworld_MapTypeAllowsTeleportAndFly), so
// never from inside a base. Unlike Fly it reaches your base from any region.
//
// SECRET BASE runs Teleport's own field effect; when it picks its destination
// (SetWarpDestinationForTeleport, src/overworld.c, one hook line) the base's
// state is set as walking in sets it, the dynamic warp points at the recorded
// tile (so walking out leaves by your base's door), and you land on the base's
// entrance warp with Teleport's spin.

static EWRAM_DATA bool8 sTeleportToBase = FALSE;
static EWRAM_DATA u8 sTeleportMon = 0;

extern const u8 HoennSeek_EventScript_TeleportChoice[];

// EnterSecretBase: you're walking into the base with this ID.
void HoennRecordBaseEntrance(u32 secretBaseId)
{
    if (secretBaseId == 0 || secretBaseId != gSaveBlock1Ptr->secretBases[0].secretBaseId)
        return; // someone else's base
    VarSet(VAR_HOENN_BASE_TELEPORT_ID, secretBaseId);
    VarSet(VAR_HOENN_BASE_TELEPORT_MAP, (u8)gSaveBlock1Ptr->location.mapGroup << 8 | (u8)gSaveBlock1Ptr->location.mapNum);
    VarSet(VAR_HOENN_BASE_TELEPORT_XY, (u8)gSaveBlock1Ptr->pos.x | (u8)gSaveBlock1Ptr->pos.y << 8);
}

// Teleport's party-menu question (src/party_menu.c) and FieldCallback_Teleport.
bool32 HoennCanTeleportToBase(void)
{
    u32 id = VarGet(VAR_HOENN_BASE_TELEPORT_ID);
    return id != 0 && id == gSaveBlock1Ptr->secretBases[0].secretBaseId;
}

// FieldCallback_Teleport: TRUE = the choice script took over. (The party menu
// skips its own "return to the healing spot?" question when this will ask.)
bool32 HoennTeleportOffersBase(void)
{
    if (!HoennCanTeleportToBase())
        return FALSE;
    sTeleportMon = GetCursorSelectionMonId();
    gSpecialVar_0x8004 = sTeleportMon; // for the mon's name in the question
    // The first choice names the healing spot, as HnS's own question does.
    GetMapNameGeneric(gStringVar2, Overworld_GetMapHeaderByGroupAndId(gSaveBlock1Ptr->lastHealLocation.mapGroup,
                                                                       gSaveBlock1Ptr->lastHealLocation.mapNum)->regionMapSectionId);
    ScriptContext_SetupScript(HoennSeek_EventScript_TeleportChoice);
    return TRUE;
}

// special HoennTeleportGo: Teleport to VAR_RESULT (HOENN_TELEPORT_TO_*), the
// same way FieldCallback_Teleport does.
void HoennTeleportGo(void)
{
    sTeleportToBase = (gSpecialVar_Result == HOENN_TELEPORT_TO_BASE && HoennCanTeleportToBase());
    LockPlayerFieldControls();
    Overworld_ResetStateAfterTeleport();
    gFieldEffectArguments[0] = sTeleportMon;
    FieldEffectStart(FLDEFF_USE_TELEPORT);
    gFieldEffectArguments[0] = sTeleportMon;
}

// SetWarpDestinationForTeleport: TRUE = the destination is your base.
bool32 HoennTeleportBaseDestination(void)
{
    u32 map, xy;

    if (!sTeleportToBase)
        return FALSE;
    sTeleportToBase = FALSE;
    map = VarGet(VAR_HOENN_BASE_TELEPORT_MAP);
    xy = VarGet(VAR_HOENN_BASE_TELEPORT_XY);
    // What SecretBase_EventScript_EnterPlayersBase does before the warp.
    VarSet(VAR_INIT_SECRET_BASE, 1);
    FlagSet(FLAG_HIDE_SECRET_BASE_TRAINER);
    VarSet(VAR_SECRET_BASE_INITIALIZED, 0);
    if (gSaveBlock1Ptr->secretBases[0].numTimesEntered < 255)
        gSaveBlock1Ptr->secretBases[0].numTimesEntered++;
    SetDynamicWarpWithCoords(0, map >> 8, map & 0xFF, WARP_ID_NONE, xy & 0xFF, xy >> 8);
    HoennPrepareOwnSecretBaseWarp();
    return TRUE;
}

#endif // IS_HNS
