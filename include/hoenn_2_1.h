#ifndef GUARD_HOENN_2_1_H
#define GUARD_HOENN_2_1_H

// Hoenn & Seek 2.1: declarations for 2.1's C code. Included at the end of
// hoenn.h, so any file that includes hoenn.h sees these.
//
// Two builders add to this file in parallel (hoenn-docs/2.1-tracks.md), so it
// has one section per track, far apart: add your part's lines at the end of
// your own track's section, never in the other's.
//
// Flags and vars: constants/hoenn_events_2_1.h. Phone call IDs:
// constants/hoenn_phone.h ("2.1" sections).

#if IS_HNS

// ---- Shared systems (2.1 part 02, track A) ---------------------------------

// The radio's news segments (src/hoenn_radio.c, data/hoenn/radio_segments.json).
// HOENN_RADIO_* mirror pokenav_radio.c's private station enum; that file
// checks they match at build time.
#define HOENN_RADIO_POKEMON_TALK      1  // PROF.OAK's POKéMON TALK (segments go in its intro)
#define HOENN_RADIO_POKEDEX_SHOW      2
#define HOENN_RADIO_POKEMON_MUSIC     3
#define HOENN_RADIO_LUCKY_CHANNEL     4
#define HOENN_RADIO_BUENAS_PASSWORD   5
#define HOENN_RADIO_UNOWN             6
#define HOENN_RADIO_PLACES_AND_PEOPLE 7
#define HOENN_RADIO_LETS_ALL_SING     8
#define HOENN_RADIO_POKE_FLUTE        9
#define HOENN_RADIO_EVOLUTION         10
#define HOENN_RADIO_ROCKET            11
#define HOENN_RADIO_HOENN_SOUND       12
#define HOENN_RADIO_STATION_COUNT     13
#define HOENN_RADIO_MAX_LINES         15 // pokenav_radio.c's MAX_RADIO_LINES
#define HOENN_RADIO_LINE_BUF_SIZE     40 // its RADIO_LINE_BUF_SIZE
#define HOENN_RADIO_SEGMENT_BUFFERS   2  // line buffers a segment may write

// A segment that builds its lines in C (a "builder" in the JSON). Writes up
// to HOENN_RADIO_SEGMENT_BUILDER_LINES pointers into lines[]; line1/line2 are
// HOENN_RADIO_LINE_BUF_SIZE-byte buffers it may fill and point at. Returns
// how many lines it wrote (0 = nothing to say today).
#define HOENN_RADIO_SEGMENT_BUILDER_LINES 3
typedef u32 (*HoennRadioBuilder)(const u8 **lines, u8 *line1, u8 *line2);

// Called by pokenav_radio.c before a station's own broadcast (for OAK's
// POKéMON TALK, in its intro). Writes the segments that are on today into
// lines[] and returns how many lines; sets *bufsUsed to the line buffers it
// took (the station's own lines start after them).
u32 HoennRadioSegments(u32 station, const u8 **lines, u8 *line1, u8 *line2, u32 *bufsUsed);

// The scaled-opponent ladder (src/hoenn_scaling.c). Values match
// HOENN_LADDER_* in constants/hoenn_events_2_1.h (for scripts).
enum HoennLadder
{
    LADDER_PROGRESS, // DAMIAN: your progress cap; 85 after Wallace
    LADDER_RIVAL,    // Silver, the second rival, the three friends: post-game up to 90
    LADDER_ELITE,    // the ~15 elite Hoenn trainers: the next leader's ace - 1
    LADDER_COUNT,
};
u32 HoennProgressCap(void);
u32 HoennScaledAce(enum HoennLadder ladder);
u16 HoennScaledAceFor(void); // specialvar: VAR_0x8004 = HOENN_LADDER_*
// A trainer on a ladder: its strongest Pokémon is set to the ladder's ace and
// the rest keep their gaps (HoennScaleTrainerMonLevel). LADDER_COUNT = none.
u32 HoennTrainerLadder(u32 trainerId);
void HoennDebugBufferLadders(void); // debug menu: H&S time & weather… → LADDERS

// ============================================================================
// ==== Track A: systems and trainers =========================================
// ============================================================================
// src/hoenn_2_1_a.c
u16 Hoenn21ADueCall(void);
// 2.1-04: save version 8 moves SaveBlock1's 60 key-item slots into SaveBlock3's 80.
void HoennMoveKeyItemsToSaveBlock3(void);

// 2.1-05: the SEEKER'S JOURNAL (src/hoenn_journal.c).
void HoennJournalHeardCall(u16 call); // an event call rang (HoennTryEventCall)
void HoennJournalPushPages(void);     // special
void HoennJournalPushEntries(void);   // special, page in VAR_0x8004
struct ScriptContext;
void HoennSeek_JournalEntryText(struct ScriptContext *ctx);
// 2.1-06: the TODAY page, the field notes, the professors' rewards.
void HoennSeek_JournalWhenText(struct ScriptContext *ctx);
u32 HoennJournalCountFound(u32 *total);
const u8 *HoennJournalFieldNote(void);
bool32 HoennJournalPageComplete(u32 page);
u16 HoennJournalPageDone(void); // specialvar, page in VAR_0x8004

// (track A's declarations go here)

// ============================================================================
// ==== Track B: the world and its people =====================================
// ============================================================================
// src/hoenn_2_1_b.c
u16 Hoenn21BDueCall(void);

// (track B's declarations go here)

#endif // IS_HNS

#endif // GUARD_HOENN_2_1_H
