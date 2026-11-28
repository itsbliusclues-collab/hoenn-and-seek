#ifndef GUARD_HOENN_RADIO_H
#define GUARD_HOENN_RADIO_H

// Hoenn & Seek 2.1-02: radio segments. Hoenn & Seek's own radio lines come
// from one table (data/hoenn/radio_segments.json -> tools/hoenn/gen_hoenn_radio.py
// -> src/data/hoenn_radio_segments.h), played before a station's normal
// broadcast. Code and design: src/hoenn_radio.c.

#if IS_HNS

// Stations, in the same order as src/pokenav_radio.c's enum (checked there by
// HOENN_RADIO_STATION_CHECKS).
enum
{
    HOENN_RADIO_NONE,
    HOENN_RADIO_OAK,            // OAK's POKéMON TALK: segments play in its intro, after MARY's greeting
    HOENN_RADIO_POKEDEX,
    HOENN_RADIO_MUSIC,
    HOENN_RADIO_LUCKY,
    HOENN_RADIO_BUENA,
    HOENN_RADIO_UNOWN,          // only tunes in at the Ruins of Alph
    HOENN_RADIO_PLACES,
    HOENN_RADIO_SING,
    HOENN_RADIO_POKE_FLUTE,
    HOENN_RADIO_EVOLUTION,
    HOENN_RADIO_ROCKET,
    HOENN_RADIO_HOENN_SOUND,
    HOENN_RADIO_STATION_COUNT,
};

// A segment's lines that are built at runtime ("native" in the JSON) come from
// a function like this: it writes up to the entry's "lines" pointers into
// lines[] and may use the entry's "buffers" text buffers, buffer k at
// bufs + k * HOENN_RADIO_BUF_SIZE. Returns how many lines it wrote (0 = skip).
#define HOENN_RADIO_BUF_SIZE 40
typedef u32 (*HoennRadioBuilder)(const u8 **lines, u8 *bufs);

// Inserts every segment that applies to `station` into lines[at..], moving the
// station's own lines after them, as long as they fit (maxLines, buffers from
// *buf on). Returns the new line count.
u32 HoennRadioSegments(u32 station, const u8 **lines, u32 n, u32 at, u32 maxLines,
                       u8 *bufs, u32 *buf, u32 numBufs, u32 bufSize);

// How many lines the segments that apply to `station` may take (a station that
// fills every free line stops this short of the end).
u32 HoennRadioReservedLines(u32 station);

// The hooks in src/pokenav_radio.c (one line each), written as macros so they
// can use that file's own sizes.
#define HOENN_RADIO_SEGMENTS(station, radio, n, at, buf) \
    HoennRadioSegments((station), (radio)->lines, (n), (at), MAX_RADIO_LINES, \
                       (radio)->lineBuffers[0], &(buf), NUM_RADIO_LINE_BUFS, RADIO_LINE_BUF_SIZE)
#define HOENN_RADIO_STATION_CHECKS \
    STATIC_ASSERT((int)RADIO_STATION_POKEMON_TALK == (int)HOENN_RADIO_OAK && (int)RADIO_STATION_UNOWN == (int)HOENN_RADIO_UNOWN \
               && (int)RADIO_STATION_HOENN_SOUND == (int)HOENN_RADIO_HOENN_SOUND && RADIO_LINE_BUF_SIZE == HOENN_RADIO_BUF_SIZE, \
                  HoennRadioStationsMatch)

// Builders (the JSON's "native" names).
u32 HoennRadioUnownBroadcast(const u8 **lines, u8 *bufs); // src/hoenn_lane_a.c (2.0 part 28)
u32 HoennRadioEusineReport(const u8 **lines, u8 *bufs);   // src/hoenn_johto_legends.c (2.0 part 35b)
u32 HoennRadioSwarm(const u8 **lines, u8 *bufs);         // src/hoenn_swarm.c (2.1-07)
u32 HoennRadioDan(const u8 **lines, u8 *bufs);           // src/hoenn_dan.c (2.1-31)

#endif // IS_HNS

#endif // GUARD_HOENN_RADIO_H
