// Hoenn & Seek 2.1-09: SILVER's starter follows him (2.1 plan section 4; lane A).
//
// Bryan (round 4): "both halves". In his cutscenes the starter walks behind
// him, and wherever he stands still (Indigo, Mt. Moon, Ever Grande) it stands
// beside him. Both come from one per-frame update, so none of HnS's ~15 Silver
// scenes or their movement scripts change:
//
// - Silver is any visible object with OBJ_EVENT_GFX_SILVER_HNS on the map
//   (New Bark and Route 22 are left out: before the theft, and the young
//   Silver of the Celebi flashback).
// - The starter is a spawned object with no template, local ID
//   LOCALID_HOENN_SILVER_STARTER (IDs from 0xF0 up are looked up without a
//   map, like the follower's). It appears on a free tile behind Silver (then
//   beside, then in front) when he appears, and goes when he does.
// - Each time Silver starts a step, the tile he left is queued. The starter
//   walks to the queued tiles one by one at Silver's own speed (walk slow,
//   normal, fast, faster, run, ledge jump), so it walks the same path a step
//   behind. A tile it can't reach in one step (it spawned off to one side)
//   is a cut to it; if a script moves Silver away, it reappears beside him.
// - Stage: the starter's form is the one Silver used in the last battle you
//   fought him (his teams _1: base, _2-_3: middle, _4 on: final), the base
//   form before any. Species line from VAR_STARTER_MON (0 = you took
//   Chikorita, so his is Cyndaquil; 1 Totodile; 2 Chikorita). After a battle
//   that evolves it, the next frame swaps the object.
// - Talking to it: HoennSeek_EventScript_SilverStarter (cry + a line).
//
// Hooks: one line in OverworldBasic (src/overworld.c) calls
// HoennSilverStarterUpdate every frame, and one in
// GetObjectEventScriptPointerByLocalIdAndMap (src/event_object_movement.c)
// gives the talk script (the object has no template).
#include "global.h"
#include "battle_setup.h"
#include "event_data.h"
#include "event_object_movement.h"
#include "fieldmap.h"
#include "hoenn.h"
#include "metatile_behavior.h"
#include "string_util.h"
#include "constants/event_object_movement.h"
#include "constants/event_objects.h"
#include "constants/map_groups.h"
#include "constants/opponents.h"
#include "constants/species.h"

#if IS_HNS

#define QUEUE_SIZE 8

enum
{
    SPEED_SLOW,
    SPEED_NORMAL,
    SPEED_FAST,
    SPEED_FASTER,
    SPEED_RUN,
    SPEED_JUMP,
};

struct SilverStarterStep
{
    s16 x, y;
    u8 speed;
};

static EWRAM_DATA struct
{
    s16 silverX, silverY;        // Silver's tile last frame
    u8 head, count;
    struct SilverStarterStep queue[QUEUE_SIZE];
} sStarter = {0};

// [VAR_STARTER_MON][stage]
static const u16 sStarterLines[3][3] =
{
    {SPECIES_CYNDAQUIL, SPECIES_QUILAVA, SPECIES_TYPHLOSION},
    {SPECIES_TOTODILE, SPECIES_CROCONAW, SPECIES_FERALIGATR},
    {SPECIES_CHIKORITA, SPECIES_BAYLEEF, SPECIES_MEGANIUM},
};

// HnS's Silver battles in order (one row per battle, one ID per team).
static const u16 sSilverBattles[][3] =
{
    {TRAINER_RIVAL_CHIKORITA_1_HNS, TRAINER_RIVAL_CYNDAQUIL_1_HNS, TRAINER_RIVAL_TOTODILE_1_HNS},
    {TRAINER_RIVAL_CHIKORITA_2_HNS, TRAINER_RIVAL_CYNDAQUIL_2_HNS, TRAINER_RIVAL_TOTODILE_2_HNS},
    {TRAINER_RIVAL_CHIKORITA_3_HNS, TRAINER_RIVAL_CYNDAQUIL_3_HNS, TRAINER_RIVAL_TOTODILE_3_HNS},
    {TRAINER_RIVAL_CHIKORITA_4_HNS, TRAINER_RIVAL_CYNDAQUIL_4_HNS, TRAINER_RIVAL_TOTODILE_4_HNS},
    {TRAINER_RIVAL_CHIKORITA_5_HNS, TRAINER_RIVAL_CYNDAQUIL_5_HNS, TRAINER_RIVAL_TOTODILE_5_HNS},
    {TRAINER_RIVAL_CHIKORITA_6_HNS, TRAINER_RIVAL_CYNDAQUIL_6_HNS, TRAINER_RIVAL_TOTODILE_6_HNS},
    {TRAINER_RIVAL_CHIKORITA_7_HNS, TRAINER_RIVAL_CYNDAQUIL_7_HNS, TRAINER_RIVAL_TOTODILE_7_HNS},
};
// The starter's stage in each of those battles.
static const u8 sBattleStage[ARRAY_COUNT(sSilverBattles)] = {0, 1, 1, 2, 2, 2, 2};

u16 HoennSilverStarterSpecies(void)
{
    u32 line = VarGet(VAR_STARTER_MON);
    s32 i, j;

    if (line >= ARRAY_COUNT(sStarterLines))
        line = 0;
    for (i = ARRAY_COUNT(sSilverBattles) - 1; i >= 0; i--)
    {
        for (j = 0; j < 3; j++)
        {
            if (HasTrainerBeenFought(sSilverBattles[i][j]))
                return sStarterLines[line][sBattleStage[i]];
        }
    }
    return sStarterLines[line][0];
}

// specialvar: the species in VAR_RESULT (for the cry), its name in STR_VAR_1.
u16 HoennSilverStarterForScript(void)
{
    u16 species = HoennSilverStarterSpecies();
    StringCopy(gStringVar1, GetSpeciesName(species));
    return species;
}

static bool32 SilverStarterMapAllowed(void)
{
    u16 map = (gSaveBlock1Ptr->location.mapGroup << 8) | gSaveBlock1Ptr->location.mapNum;
    return map != MAP_NEW_BARK_TOWN_HNS && map != MAP_ROUTE22_HNS && map != MAP_TEST_MAP1_HNS;
}

static struct ObjectEvent *FindSilver(void)
{
    u32 i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && !obj->isPlayer && !obj->invisible && obj->graphicsId == OBJ_EVENT_GFX_SILVER_HNS)
            return obj;
    }
    return NULL;
}

static struct ObjectEvent *FindStarter(void)
{
    u32 i;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        if (gObjectEvents[i].active && gObjectEvents[i].localId == LOCALID_HOENN_SILVER_STARTER)
            return &gObjectEvents[i];
    }
    return NULL;
}

// A tile the starter can stand on next to Silver: open, on his level, not
// water, no one on it.
static bool32 IsFreeTileNear(struct ObjectEvent *silver, s16 x, s16 y)
{
    u32 i;
    u8 elevation = MapGridGetElevationAt(x, y);

    if (MapGridGetCollisionAt(x, y))
        return FALSE;
    if (elevation != silver->currentElevation && elevation != 0 && elevation != 15)
        return FALSE;
    if (MetatileBehavior_IsSurfableWaterOrUnderwater(MapGridGetMetatileBehaviorAt(x, y)))
        return FALSE;
    for (i = 0; i < OBJECT_EVENTS_COUNT; i++)
    {
        struct ObjectEvent *obj = &gObjectEvents[i];
        if (obj->active && ((obj->currentCoords.x == x && obj->currentCoords.y == y)
                         || (obj->previousCoords.x == x && obj->previousCoords.y == y)))
            return FALSE;
    }
    return TRUE;
}

static void SpawnStarter(struct ObjectEvent *silver, u16 species)
{
    enum Direction facing = silver->facingDirection;
    enum Direction tries[4];
    u32 i;

    // Behind him first, then his sides, then in front.
    tries[0] = GetOppositeDirection(facing);
    tries[1] = (facing == DIR_NORTH || facing == DIR_SOUTH) ? DIR_WEST : DIR_NORTH;
    tries[2] = GetOppositeDirection(tries[1]);
    tries[3] = facing;
    for (i = 0; i < ARRAY_COUNT(tries); i++)
    {
        s16 x = silver->currentCoords.x, y = silver->currentCoords.y;
        u8 id;

        MoveCoords(tries[i], &x, &y);
        if (!IsFreeTileNear(silver, x, y))
            continue;
        id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_MON_BASE + species, MOVEMENT_TYPE_NONE,
                                                  LOCALID_HOENN_SILVER_STARTER, x, y, silver->currentElevation);
        if (id < OBJECT_EVENTS_COUNT)
            SetObjectEventDirection(&gObjectEvents[id], facing);
        break;
    }
    sStarter.silverX = silver->currentCoords.x;
    sStarter.silverY = silver->currentCoords.y;
    sStarter.count = 0;
}

static s16 Abs16(s16 v)
{
    return v < 0 ? -v : v;
}

static u8 SilverSpeed(u8 action)
{
    enum Direction d;
    for (d = DIR_SOUTH; d <= DIR_EAST; d++)
    {
        if (action == GetWalkSlowMovementAction(d))
            return SPEED_SLOW;
        if (action == GetWalkFastMovementAction(d))
            return SPEED_FAST;
        if (action == GetWalkFasterMovementAction(d))
            return SPEED_FASTER;
        if (action == GetPlayerRunMovementAction(d))
            return SPEED_RUN;
        if (action == GetJump2MovementAction(d))
            return SPEED_JUMP;
    }
    return SPEED_NORMAL;
}

static u8 StepAction(u8 speed, enum Direction d)
{
    switch (speed)
    {
    case SPEED_SLOW:   return GetWalkSlowMovementAction(d);
    case SPEED_FAST:   return GetWalkFastMovementAction(d);
    case SPEED_FASTER: return GetWalkFasterMovementAction(d);
    case SPEED_RUN:    return GetPlayerRunMovementAction(d);
    default:           return GetWalkNormalMovementAction(d);
    }
}

static void TakeNextStep(struct ObjectEvent *starter, struct ObjectEvent *silver)
{
    struct SilverStarterStep step;
    s16 dx, dy;
    enum Direction d;

    if (starter->heldMovementActive)
    {
        if (!starter->heldMovementFinished)
            return;
        ObjectEventClearHeldMovement(starter);
    }
    if (sStarter.count == 0)
    {
        // Standing again: match Silver's freeze, so a message box stops both.
        if (silver->frozen && !starter->frozen)
            FreezeObjectEvent(starter);
        return;
    }

    step = sStarter.queue[sStarter.head];
    sStarter.head = (sStarter.head + 1) % QUEUE_SIZE;
    sStarter.count--;

    dx = step.x - starter->currentCoords.x;
    dy = step.y - starter->currentCoords.y;
    if (dx == 0 && dy == 0)
        return;
    d = GetDirectionToFace(starter->currentCoords.x, starter->currentCoords.y, step.x, step.y);
    if (starter->frozen)
        UnfreezeObjectEvent(starter);
    if (Abs16(dx) + Abs16(dy) == 1)
        ObjectEventSetHeldMovement(starter, StepAction(step.speed, d));
    else if (Abs16(dx) + Abs16(dy) == 2 && (dx == 0 || dy == 0))
        ObjectEventSetHeldMovement(starter, GetJump2MovementAction(d));
    else
    {
        MoveObjectEventToMapCoords(starter, step.x, step.y);
        SetObjectEventDirection(starter, d);
    }
}

void HoennSilverStarterUpdate(void)
{
    struct ObjectEvent *silver, *starter;
    u16 species;

    starter = FindStarter();
    silver = SilverStarterMapAllowed() ? FindSilver() : NULL;
    if (silver == NULL)
    {
        if (starter != NULL)
            RemoveObjectEvent(starter);
        return;
    }

    species = HoennSilverStarterSpecies();
    if (starter != NULL && starter->graphicsId != OBJ_EVENT_GFX_MON_BASE + species)
    {
        // It evolved in the battle just fought: swap it on the same tile.
        s16 x = starter->currentCoords.x, y = starter->currentCoords.y;
        enum Direction d = starter->facingDirection;
        u8 id;

        RemoveObjectEvent(starter);
        id = SpawnSpecialObjectEventParameterized(OBJ_EVENT_GFX_MON_BASE + species, MOVEMENT_TYPE_NONE,
                                                  LOCALID_HOENN_SILVER_STARTER, x, y, silver->currentElevation);
        starter = id < OBJECT_EVENTS_COUNT ? &gObjectEvents[id] : NULL;
        if (starter != NULL)
            SetObjectEventDirection(starter, d);
        return;
    }
    if (starter == NULL)
    {
        SpawnStarter(silver, species);
        return;
    }

    if (Abs16(silver->currentCoords.x - sStarter.silverX) + Abs16(silver->currentCoords.y - sStarter.silverY) > 2)
    {
        // A script put him somewhere else: start again beside him next frame.
        RemoveObjectEvent(starter);
        return;
    }
    if (silver->currentCoords.x != sStarter.silverX || silver->currentCoords.y != sStarter.silverY)
    {
        // Silver started a step (or a script moved him): the tile he left is next.
        if (sStarter.count < QUEUE_SIZE)
        {
            struct SilverStarterStep *step = &sStarter.queue[(sStarter.head + sStarter.count) % QUEUE_SIZE];
            step->x = sStarter.silverX;
            step->y = sStarter.silverY;
            step->speed = SilverSpeed(silver->movementActionId);
            sStarter.count++;
        }
        sStarter.silverX = silver->currentCoords.x;
        sStarter.silverY = silver->currentCoords.y;
    }
    TakeNextStep(starter, silver);
}

#endif // IS_HNS
