// Hoenn & Seek 2.1-02: radio segments.
//
// 2.0 added its radio lines with one hook per feature in src/pokenav_radio.c
// (part 28's Unown signal, 35b's EUSINE tracking). 2.1 has more on the way
// (swarm reports, Johto/Kanto forecasts, Dan's show, news), so they all come
// from one table instead, like the hint engine's lines:
//
//   data/hoenn/radio_segments.json -> tools/hoenn/gen_hoenn_radio.py
//   -> src/data/hoenn_radio_segments.h (sRadioSegments, included below)
//
// Each segment names a station, conditions (the hint engine's, HoennHintCond)
// and either fixed lines or a builder function (HoennRadioBuilder) for lines
// made at runtime. When a station's broadcast is generated, every segment for
// it whose conditions hold is put in front of the station's own lines, in table
// order. On OAK's POKéMON TALK, which plays in phases, they go in its intro
// after MARY's greeting, where 35b's EUSINE notes always were.
//
// Room: src/pokenav_radio.c has 15 line slots and 8 text buffers per broadcast,
// and some stations fill most of them (LUCKY CHANNEL uses 13 lines). A segment
// that doesn't fit whole (its declared "lines" and "buffers") is skipped, so a
// station never loses its own lines. Two hooks in src/pokenav_radio.c call
// HoennRadioSegments (HOENN_RADIO_SEGMENTS, include/hoenn_radio.h).
#include "global.h"
#include "event_data.h"
#include "hoenn.h"
#include "hoenn_hints.h"
#include "hoenn_radio.h"
#include "constants/flags.h"
#include "constants/vars.h"
#include "constants/species.h"

#if IS_HNS

struct HoennRadioSegment
{
    u8 station;
    u8 maxLines;                    // fixed lines, or the most a builder writes
    u8 buffers;                     // text buffers a builder needs
    u16 heard;                      // 2.1-07: a journal heard flag set when it plays (0 = none)
    const struct HoennHintCond *conds;
    u8 condCount;
    const u8 *const *lines;         // fixed lines (NULL for a builder)
    HoennRadioBuilder builder;      // NULL for fixed lines
};

#include "data/hoenn_radio_segments.h"

#define MAX_SEGMENT_LINES 8 // the most one segment may have (the generator checks it)

// The lines the segments that apply to `station` right now may take (2.1-31).
// A station that fills every line it can (PLACES AND PEOPLE) stops this short,
// or no segment would ever fit in front of it.
u32 HoennRadioReservedLines(u32 station)
{
    u32 i, total = 0;

    for (i = 0; i < ARRAY_COUNT(sRadioSegments); i++)
        if (sRadioSegments[i].station == station
         && HoennHintConditionsHold(sRadioSegments[i].conds, sRadioSegments[i].condCount))
            total += sRadioSegments[i].maxLines;
    return total;
}

u32 HoennRadioSegments(u32 station, const u8 **lines, u32 n, u32 at, u32 maxLines,
                       u8 *bufs, u32 *buf, u32 numBufs, u32 bufSize)
{
    const u8 *seg[MAX_SEGMENT_LINES];
    u32 i, j, count;

    if (bufSize != HOENN_RADIO_BUF_SIZE)
        return n;
    for (i = 0; i < ARRAY_COUNT(sRadioSegments); i++)
    {
        const struct HoennRadioSegment *s = &sRadioSegments[i];

        if (s->station != station)
            continue;
        if (n + s->maxLines > maxLines || *buf + s->buffers > numBufs)
            continue; // no room: skip it whole
        if (!HoennHintConditionsHold(s->conds, s->condCount))
            continue;

        if (s->builder != NULL)
            count = s->builder(seg, bufs + *buf * bufSize);
        else
            for (count = 0; count < s->maxLines; count++)
                seg[count] = s->lines[count];
        if (count == 0)
            continue;
        if (count > s->maxLines)
            count = s->maxLines; // a builder must stay within its entry's "lines"
        *buf += s->buffers;
        if (s->heard != 0)
            FlagSet(s->heard); // the SEEKER'S JOURNAL heard it on the radio

        // Move the station's lines from `at` on down, then put the segment in.
        for (j = n; j > at; j--)
            lines[j - 1 + count] = lines[j - 1];
        for (j = 0; j < count; j++)
            lines[at + j] = seg[j];
        n += count;
        at += count;
    }
    return n;
}

#endif // IS_HNS
