// Hoenn & Seek 2.1-07: the daily swarm (track A; 2.1 plan section 3).
//
// HnS has no HGSS swarms; the only swarm code was Emerald's TV "mass outbreak"
// (src/tv.c), which stores map group 0 + an Emerald map number, so in HnS its
// five Hoenn outbreaks landed on Johto and Kanto maps (a Lv 3 Seedot "swarm"
// on Route 35). Hoenn & Seek turns that show off (one line at the top of
// TryStartRandomMassOutbreak, src/tv.c) and replaces the encounter with this:
//
//   - One swarm a day: a row of sHoennSwarms (data/hoenn/swarms.json →
//     tools/hoenn/gen_hoenn_swarms.py → src/data/hoenn_swarms.h), rolled at
//     the daily reset (HoennClearDailyFlags, src/hoenn_flags.c) and never the
//     same as yesterday's. VAR_HOENN_SWARM_TODAY holds the row + 1; 0 means
//     "not rolled yet" (a new game, or a save from before 2.1-07), and the
//     first question about today's swarm rolls it.
//   - The encounter: wild_encounter.c's two outbreak functions hand over to
//     HoennSwarmHere / HoennSwarmPick (one hook each). On the swarm's map,
//     rate % of land encounters are its species, at a level from the row's
//     range (the map's own land table by default). Repel and Keen Eye checks
//     stay the engine's. Land only.
//   - The news: every TV (HoennSeek_EventScript_TVSwarm, before the forecast),
//     the PokéGear radio (the "daily_swarm" segment, HoennRadioSwarm) and the
//     SEEKER'S JOURNAL's TODAY page (its DAILY_SWARM entry's "when" line reads
//     STR_VAR_1/2, filled by HoennSwarmBufferNames). The TV and the radio set
//     the entry's heard flag; meeting a swarm Pokémon sets FLAG_HOENN_SWARM_MET.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "overworld.h"
#include "random.h"
#include "region_map.h"
#include "string_util.h"
#include "pokemon.h"
#include "constants/characters.h"
#include "constants/hoenn_events_2_1.h"
#include "constants/species.h"

#if IS_HNS

struct HoennSwarm
{
    u8 mapGroup;
    u8 mapNum;
    u16 species;
    u8 minLevel;
    u8 maxLevel;
    u8 rate; // % of land encounters on the map
};

#include "data/hoenn_swarms.h"

// Roll a new swarm (the daily reset): any row but yesterday's.
void HoennSwarmRoll(void)
{
    u32 n = ARRAY_COUNT(sHoennSwarms), old = VarGet(VAR_HOENN_SWARM_TODAY), pick;

    pick = Random() % n;
    if (n > 1 && pick + 1 == old)
        pick = (pick + 1 + Random() % (n - 1)) % n;
    VarSet(VAR_HOENN_SWARM_TODAY, pick + 1);
}

static const struct HoennSwarm *Today(void)
{
    u32 i = VarGet(VAR_HOENN_SWARM_TODAY);

    if (i == 0 || i > ARRAY_COUNT(sHoennSwarms))
    {
        HoennSwarmRoll();
        i = VarGet(VAR_HOENN_SWARM_TODAY);
    }
    return &sHoennSwarms[i - 1];
}

// DoMassOutbreakEncounterTest's hook: is this land encounter the swarm's?
bool32 HoennSwarmHere(void)
{
    const struct HoennSwarm *s = Today();

    return gSaveBlock1Ptr->location.mapGroup == s->mapGroup
        && gSaveBlock1Ptr->location.mapNum == s->mapNum
        && Random() % 100 < s->rate;
}

// SetUpMassOutbreakEncounter's hook: the swarm Pokémon to create.
void HoennSwarmPick(u16 *species, u8 *level)
{
    const struct HoennSwarm *s = Today();

    *species = s->species;
    *level = s->minLevel + Random() % (s->maxLevel - s->minLevel + 1);
}

void HoennSwarmMet(void)
{
    FlagSet(FLAG_HOENN_SWARM_MET);
}

// STR_VAR_1 = today's species, STR_VAR_2 = its map's name.
void HoennSwarmBufferNames(void)
{
    const struct HoennSwarm *s = Today();

    StringCopy(gStringVar1, GetSpeciesName(s->species));
    GetMapName(gStringVar2, Overworld_GetMapHeaderByGroupAndId(s->mapGroup, s->mapNum)->regionMapSectionId, 0);
}

// The radio's "daily_swarm" segment: "Today's swarm news!" /
// "YANMA swarming" / "on ROUTE 35!"
u32 HoennRadioSwarm(const u8 **lines, u8 *line1, u8 *line2)
{
    const struct HoennSwarm *s = Today();
    u8 *end;

    end = StringCopy(line1, GetSpeciesName(s->species));
    StringCopy(end, COMPOUND_STRING(" are swarming"));
    end = StringCopy(line2, COMPOUND_STRING("on "));
    end = GetMapName(end, Overworld_GetMapHeaderByGroupAndId(s->mapGroup, s->mapNum)->regionMapSectionId, 0);
    *end++ = CHAR_EXCL_MARK;
    *end = EOS;
    lines[0] = COMPOUND_STRING("Here's today's swarm report!");
    lines[1] = line1;
    lines[2] = line2;
    return 3;
}

#endif // IS_HNS
