// Hoenn & Seek 2.1 (part 02): news segments on the PokéGear radio.
//
// WHY A TABLE
//   2.0 hooked the radio once per feature: part 28 put the Unown Cipher's
//   signal on the Ruins' station, part 35b put EUSINE's tracking notes in the
//   intro of PROF.OAK's POKéMON TALK, each with its own line in
//   src/pokenav_radio.c. 2.1 has more to announce (the daily swarm, Johto and
//   Kanto weather forecasts, Dan's history show, news bulletins), so instead of
//   one hook each, every announcement is a row of one table, like the hint
//   engine's lines (src/hoenn_hints.c):
//
//     data/hoenn/radio_segments.json  ->  tools/hoenn/gen_hoenn_radio.py
//                                      ->  src/data/hoenn_radio_segments.h
//
//   A row says which station it plays on, the conditions it needs (the hint
//   engine's: flags, vars, badges, caught...), and either up to three fixed
//   lines or a C "builder" that writes lines of its own (a roamer's location,
//   today's swarm). Parts 28 and 35b are now rows with builders.
//
// WHERE IT PLAYS
//   pokenav_radio.c calls HoennRadioSegments() once per broadcast: in the
//   intro of OAK's show (after "With me, MARY!"), and before the station's own
//   lines on every other station. The segments that are on, in table order,
//   come first; then the station's usual broadcast.
//
// LIMITS (so no station ever overflows its 15 lines)
//   - Each station has a number of lines its own broadcast may need
//   (sStationLines below, counted from pokenav_radio.c). Segments only get what
//   is left, and never more than HOENN_RADIO_MAX_SEGMENT_LINES in all: a row
//   that doesn't fit is skipped this time.
//   - Only one builder plays per broadcast: builders share the two line
//   buffers. Later builders on the same station wait for the next broadcast in
//   which the first has nothing to say. Fixed-line rows always play if they fit.
//
// HEARD FLAGS
//   A row may name a "heard" flag, set the first time it plays. The Seeker's
//   Journal (2.1-05) reads these to count a rumor as heard on the radio.
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_hints.h"
#include "constants/flags.h"

#if IS_HNS

#define HOENN_RADIO_MAX_SEGMENT_LINES 6 // a segment block stays short: it's news, not the show

struct HoennRadioSegment
{
    u8 station;                         // HOENN_RADIO_*
    const struct HoennHintCond *conds;  // all must hold (NULL, 0 = always)
    u8 condCount;
    u8 lineCount;                       // fixed lines (0 when built)
    const u8 *const *lines;
    HoennRadioBuilder build;            // NULL for fixed lines
    u16 heardFlag;                      // set when it plays (0 = none)
};

#include "data/hoenn_radio_segments.h"

// The most lines each station's own broadcast writes (pokenav_radio.c's
// GenerateStationContent), so the segments in front of it leave room. Stations
// that stop at the line limit by themselves (POKéDEX SHOW's text, PLACES AND
// PEOPLE's list) only count their fixed lines.
static const u8 sStationLines[HOENN_RADIO_STATION_COUNT] =
{
    [HOENN_RADIO_POKEMON_TALK]      = 2,  // the intro: MARY's two lines
    [HOENN_RADIO_POKEDEX_SHOW]      = 2,  // + the entry, wrapped to fit
    [HOENN_RADIO_POKEMON_MUSIC]     = 6,
    [HOENN_RADIO_LUCKY_CHANNEL]     = 13,
    [HOENN_RADIO_BUENAS_PASSWORD]   = 6,
    [HOENN_RADIO_UNOWN]             = 0,  // all of it is segments (part 28's signal)
    [HOENN_RADIO_PLACES_AND_PEOPLE] = 4,  // + names, up to the limit
    [HOENN_RADIO_LETS_ALL_SING]     = 6,
    [HOENN_RADIO_POKE_FLUTE]        = 0,
    [HOENN_RADIO_EVOLUTION]         = 0,
    [HOENN_RADIO_ROCKET]            = 10,
    [HOENN_RADIO_HOENN_SOUND]       = 4,
};

u32 HoennRadioSegments(u32 station, const u8 **lines, u8 *line1, u8 *line2, u32 *bufsUsed)
{
    u32 i, j, n = 0, room;
    bool32 built = FALSE;

    *bufsUsed = 0;
    if (station >= HOENN_RADIO_STATION_COUNT)
        return 0;
    room = HOENN_RADIO_MAX_LINES - sStationLines[station];
    if (room > HOENN_RADIO_MAX_SEGMENT_LINES)
        room = HOENN_RADIO_MAX_SEGMENT_LINES;

    for (i = 0; i < ARRAY_COUNT(sHoennRadioSegments); i++)
    {
        const struct HoennRadioSegment *seg = &sHoennRadioSegments[i];
        u32 added = 0;

        if (seg->station != station || !HoennHintCondsHold(seg->conds, seg->condCount))
            continue;
        if (seg->build != NULL)
        {
            // A builder may write up to HOENN_RADIO_SEGMENT_BUILDER_LINES lines.
            if (built || n + HOENN_RADIO_SEGMENT_BUILDER_LINES > room)
                continue;
            added = seg->build(&lines[n], line1, line2);
            if (added != 0)
            {
                built = TRUE;
                *bufsUsed = HOENN_RADIO_SEGMENT_BUFFERS;
            }
        }
        else
        {
            if (n + seg->lineCount > room)
                continue;
            for (j = 0; j < seg->lineCount; j++)
                lines[n + j] = seg->lines[j];
            added = seg->lineCount;
        }
        if (added != 0 && seg->heardFlag != 0)
            FlagSet(seg->heardFlag);
        n += added;
    }
    return n;
}

#endif // IS_HNS
