#ifndef GUARD_HOENN_HINTS_H
#define GUARD_HOENN_HINTS_H

// Hoenn & Seek 2.0 (part 7): NPC hint lines, rumors, field notes and forecasts.
// Code and design: src/hoenn_hints.c. Data: data/hoenn/npc_hints.json.

#if IS_HNS
// Conditions, shared by the hint tables and the radio segment table (2.1-02):
// the generators write them from the JSON's "if" lists (gen_hoenn_hints.py's
// cond_c, which gen_hoenn_radio.py reuses).
enum HoennHintCondType
{
    HINT_IF_FLAG,
    HINT_IF_NOT_FLAG,
    HINT_IF_VAR_MIN,
    HINT_IF_VAR_BELOW,
    HINT_IF_STAMPS_MIN,   // Hoenn gym badges (stamp flags)
    HINT_IF_STAMPS_BELOW,
    HINT_IF_BADGES_MIN,   // HnS's 16 Johto + Kanto badges
    HINT_IF_BADGES_BELOW,
    HINT_IF_CAUGHT,       // Pokédex caught flag
    HINT_IF_NOT_CAUGHT,
    HINT_IF_LEVEL_CAP,    // HnS's LEVEL CAP challenge setting is on
};

struct HoennHintCond
{
    u8 type;
    u16 id;     // flag, var or species
    u16 value;  // var / badge threshold
};

bool32 HoennHintConditionsHold(const struct HoennHintCond *conds, u32 count);

struct ScriptContext;
// callnatives used by data/scripts/hoenn_hints.inc (generated)
void HoennSeek_NpcHint(struct ScriptContext *ctx);
void HoennSeek_FieldNotesCount(struct ScriptContext *ctx);
void HoennSeek_FieldNotesHint(struct ScriptContext *ctx);
void HoennSeek_Forecast(struct ScriptContext *ctx);
#endif

#endif // GUARD_HOENN_HINTS_H
