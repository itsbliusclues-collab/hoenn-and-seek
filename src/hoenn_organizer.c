// Hoenn & Seek 2.1-32: ANAMARIA, the professional organizer in Cherrygrove
// (hoenn-docs/2.1-plan.md, "Anamaria, the professional organizer").
//
// She sorts the PC boxes three ways. One special does all of it:
//
//   setvar VAR_0x8004, HOENN_ORGANIZE_BY_NUMBER / _BY_TYPE / _BY_LEVEL
//   copyvar VAR_0x8005, <boxes to leave alone, 0-13>
//   specialvar VAR_RESULT, HoennOrganizeBoxes   @ HOENN_ORGANIZE_* below
//
// The sorted region is every box from the first one not left alone to the last.
// The party and the fusion storage are never touched.
//
// How it moves Pokémon (nothing is ever copied or lost):
//  1. One sort key per occupied slot (u64, on the heap while it runs: 420 keys
//     plus three u16 tables and two u8 ones per slot, ~6.7 KB, freed at the end).
//     NUMBER: dex number, then species ID (so forms follow their base form);
//     TYPE: our type order, then dex number; LEVEL: highest first, then dex
//     number. Eggs go last in every mode. Ties keep their old order (the sort is
//     a stable insertion sort; 420 keys take a few frames).
//  2. Where each Pokémon goes: packed from the start of the region (NUMBER,
//     LEVEL), or for TYPE each type starts a new box unless it fits in what is
//     left of the current box (then they share it: "ICE +"). If those breaks
//     don't fit in the region, TYPE packs tightly instead (HOENN_ORGANIZE_PACKED,
//     and she says so). Empty slots fill the rest, in their old order.
//  3. Checks before writing: the plan is a permutation of the region (every slot
//     used exactly once), and every occupied slot lands on a planned spot.
//  4. The permutation is applied cycle by cycle with one spare BoxPokemon (80 B
//     on the stack), so each Pokémon is moved once and the region never needs a
//     second copy (EWRAM has no room for one).
//  5. After writing, the occupied count and a checksum of every Pokémon's
//     personality, OT ID and checksum must match the ones taken before.
//  6. Box names and wallpapers last. TYPE names each box it fills for its first
//     type ("WATER", "WATER 2", "ICE +", "EGGS") and gives it a wallpaper to
//     match; VAR_HOENN_ORGANIZER_NAMED remembers which boxes she named, so a
//     later NUMBER or LEVEL sort (or an emptied box) puts back "BOX<n>" and the
//     default wallpaper only on those, never on names you chose yourself.
#include "global.h"
#include "battle_main.h"
#include "event_data.h"
#include "hoenn.h"
#include "malloc.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "string_util.h"
#include "constants/characters.h"
#include "constants/hoenn_events_2_1.h"

#if IS_HNS

#define REGION_MAX (TOTAL_BOXES_COUNT * IN_BOX_COUNT)
#define NO_SLOT    0xFFFF

// Group order for BY TYPE (the modern Pokédex's type order), then eggs.
static const u8 sTypeOrder[] =
{
    TYPE_NORMAL, TYPE_FIRE, TYPE_WATER, TYPE_GRASS, TYPE_ELECTRIC, TYPE_ICE,
    TYPE_FIGHTING, TYPE_POISON, TYPE_GROUND, TYPE_FLYING, TYPE_PSYCHIC, TYPE_BUG,
    TYPE_ROCK, TYPE_GHOST, TYPE_DRAGON, TYPE_DARK, TYPE_STEEL, TYPE_FAIRY,
};
#define GROUP_EGGS  ARRAY_COUNT(sTypeOrder)
#define GROUP_OTHER (GROUP_EGGS - 1) // a type not in the list (???, Stellar) sorts with Fairy's neighbours, last

// Wallpaper IDs from src/data/wallpapers.h (that enum is private to
// pokemon_storage_system.c; these are its values).
enum
{
    WP_FOREST, WP_CITY, WP_DESERT, WP_SAVANNA, WP_CRAG, WP_VOLCANO, WP_SNOW,
    WP_CAVE, WP_BEACH, WP_SEAFLOOR, WP_RIVER, WP_SKY, WP_POLKADOT, WP_POKECENTER,
    WP_MACHINE, WP_PLAIN,
};
#define DEFAULT_WALLPAPER_COUNT 4 // MAX_DEFAULT_WALLPAPER (SAVANNA) + 1: new boxes cycle FOREST, CITY, DESERT, SAVANNA

// Per group, in sTypeOrder order, then eggs.
static const u8 sGroupWallpaper[] =
{
    WP_SAVANNA,  // NORMAL
    WP_VOLCANO,  // FIRE
    WP_RIVER,    // WATER
    WP_FOREST,   // GRASS
    WP_MACHINE,  // ELECTRIC
    WP_SNOW,     // ICE
    WP_CITY,     // FIGHTING
    WP_SEAFLOOR, // POISON
    WP_DESERT,   // GROUND
    WP_SKY,      // FLYING
    WP_PLAIN,    // PSYCHIC
    WP_FOREST,   // BUG
    WP_CRAG,     // ROCK
    WP_CAVE,     // GHOST
    WP_BEACH,    // DRAGON
    WP_CAVE,     // DARK
    WP_MACHINE,  // STEEL
    WP_POLKADOT, // FAIRY
    WP_POKECENTER, // EGGS
};
STATIC_ASSERT(ARRAY_COUNT(sGroupWallpaper) == GROUP_EGGS + 1, OrganizerWallpaperPerGroup);

static const u8 sText_Eggs[] = _("EGGS");
static const u8 sText_Box[] = _("BOX"); // as ResetPokemonStorageSystem names them

struct OrganizerWork
{
    u64 key[REGION_MAX];     // by region slot
    u16 order[REGION_MAX];   // occupied region slots, sorted
    u16 newSlot[REGION_MAX]; // newSlot[i] = where order[i] goes
    u16 target[REGION_MAX];  // target[newSlot] = old region slot
    u8 groupOf[REGION_MAX];  // by region slot (TYPE)
    u8 used[REGION_MAX];     // scratch: "slot taken" / "slot written"
};

static struct BoxPokemon *RegionMon(u16 first, u16 r)
{
    u16 g = first + r;
    return GetBoxedMonPtr(g / IN_BOX_COUNT, g % IN_BOX_COUNT);
}

static bool32 IsOccupied(struct BoxPokemon *mon)
{
    return GetBoxMonData(mon, MON_DATA_SANITY_HAS_SPECIES) != 0;
}

static bool32 IsEggSlot(struct BoxPokemon *mon)
{
    return GetBoxMonData(mon, MON_DATA_IS_EGG) || GetBoxMonData(mon, MON_DATA_SANITY_IS_BAD_EGG);
}

static u8 TypeGroup(u16 species)
{
    u32 i;
    enum Type type = GetSpeciesType(species, 0);

    for (i = 0; i < ARRAY_COUNT(sTypeOrder); i++)
    {
        if (sTypeOrder[i] == type)
            return i;
    }
    return GROUP_OTHER;
}

static u64 MonKey(struct BoxPokemon *mon, u16 mode, u8 *group)
{
    u16 species = SanitizeSpeciesId(GetBoxMonData(mon, MON_DATA_SPECIES));
    u64 dex = SpeciesToNationalPokedexNum(species);
    u64 egg = IsEggSlot(mon) ? 1 : 0;
    u64 key = (dex << 16) | species;

    *group = egg ? GROUP_EGGS : TypeGroup(species);
    switch (mode)
    {
    case HOENN_ORGANIZE_BY_TYPE:
        key |= (u64)*group << 40;
        break;
    case HOENN_ORGANIZE_BY_LEVEL:
        if (!egg)
            key |= (u64)(MAX_LEVEL - GetLevelFromBoxMonExp(mon)) << 32;
        break;
    }
    return key | (egg << 48);
}

static u32 MonSum(struct BoxPokemon *mon)
{
    return GetBoxMonData(mon, MON_DATA_PERSONALITY)
         + GetBoxMonData(mon, MON_DATA_OT_ID) * 3
         + mon->checksum;
}

// Where each sorted Pokémon goes: newSlot[i] for order[i]. Returns FALSE if
// TYPE's box breaks don't fit (the caller packs tightly).
static bool32 PlanTypeBreaks(struct OrganizerWork *w, u16 count, u16 size, u16 *newSlot)
{
    u16 i = 0, pos = 0;

    while (i < count)
    {
        u8 g = w->groupOf[w->order[i]];
        u16 n = 0, inBox = pos % IN_BOX_COUNT;

        while (i + n < count && w->groupOf[w->order[i + n]] == g)
            n++;
        if (inBox != 0 && n > IN_BOX_COUNT - inBox)
            pos += IN_BOX_COUNT - inBox; // too big to share: start a new box
        if (pos + n > size)
            return FALSE;
        while (n--)
            newSlot[i++] = pos++;
    }
    return TRUE;
}

static void DefaultBoxName(u8 box)
{
    u8 *dest = StringCopy(GetBoxNamePtr(box), sText_Box);
    ConvertIntToDecimalStringN(dest, box + 1, STR_CONV_MODE_LEFT_ALIGN, 2);
}

static void SetWallpaper(u8 box, u8 wallpaper)
{
    gPokemonStoragePtr->boxWallpapers[box] = wallpaper;
}

// "WATER", "WATER 2", "ICE +", "FIGHTI 2+": the group's name cut to fit
// BOX_NAME_LENGTH with its suffix.
static void TypeBoxName(u8 box, u8 group, u8 nth, bool32 shared)
{
    u8 suffix[6], *s = suffix, *dest = GetBoxNamePtr(box);
    const u8 *name = group == GROUP_EGGS ? sText_Eggs : gTypesInfo[sTypeOrder[group]].name;
    u32 room, len;

    if (nth > 1)
    {
        *s++ = CHAR_SPACE;
        s = ConvertIntToDecimalStringN(s, nth, STR_CONV_MODE_LEFT_ALIGN, 2);
        if (shared)
            *s++ = CHAR_PLUS;
    }
    else if (shared)
    {
        *s++ = CHAR_SPACE;
        *s++ = CHAR_PLUS;
    }
    *s = EOS;
    len = s - suffix;
    room = BOX_NAME_LENGTH - len;
    s = dest;
    while (*name != EOS && (u32)(s - dest) < room)
        *s++ = *name++;
    StringCopy(s, suffix);
}

static void RelabelBoxes(struct OrganizerWork *w, u16 mode, u8 skip)
{
    u16 named = VarGet(VAR_HOENN_ORGANIZER_NAMED);
    u8 box, starts[GROUP_EGGS + 1] = {0};

    for (box = skip; box < TOTAL_BOXES_COUNT; box++)
    {
        struct BoxPokemon *first = GetBoxedMonPtr(box, 0);
        u8 slot, group = 0;
        bool32 shared = FALSE;

        if (mode == HOENN_ORGANIZE_BY_TYPE && IsOccupied(first))
        {
            MonKey(first, mode, &group);
            for (slot = 1; slot < IN_BOX_COUNT; slot++)
            {
                struct BoxPokemon *mon = GetBoxedMonPtr(box, slot);
                u8 g;
                if (!IsOccupied(mon))
                    break;
                MonKey(mon, mode, &g);
                if (g != group)
                    shared = TRUE;
            }
            TypeBoxName(box, group, ++starts[group], shared);
            SetWallpaper(box, sGroupWallpaper[group]);
            named |= 1 << box;
        }
        else if (named & (1 << box))
        {
            DefaultBoxName(box);
            SetWallpaper(box, box % DEFAULT_WALLPAPER_COUNT);
            named &= ~(1 << box);
        }
    }
    VarSet(VAR_HOENN_ORGANIZER_NAMED, named);
}

static u16 Organize(struct OrganizerWork *w, u16 mode, u8 skip)
{
    u16 first = skip * IN_BOX_COUNT, size = REGION_MAX - first;
    u16 count = 0, r, i, j, packedResult = HOENN_ORGANIZE_DONE;
    u32 sumBefore = 0, sumAfter = 0, countAfter = 0;
    u16 *newSlot = w->newSlot;
    u8 *used = w->used;

    // 1. Keys.
    for (r = 0; r < size; r++)
    {
        struct BoxPokemon *mon = RegionMon(first, r);
        if (!IsOccupied(mon))
            continue;
        w->key[r] = MonKey(mon, mode, &w->groupOf[r]);
        sumBefore += MonSum(mon);
        // Stable insertion sort by key: equal keys keep their slot order.
        for (j = count; j > 0 && w->key[w->order[j - 1]] > w->key[r]; j--)
            w->order[j] = w->order[j - 1];
        w->order[j] = r;
        count++;
    }
    if (count == 0)
        return HOENN_ORGANIZE_EMPTY;

    // 2. Where they go.
    if (mode != HOENN_ORGANIZE_BY_TYPE || !PlanTypeBreaks(w, count, size, newSlot))
    {
        if (mode == HOENN_ORGANIZE_BY_TYPE)
            packedResult = HOENN_ORGANIZE_PACKED;
        for (i = 0; i < count; i++)
            newSlot[i] = i;
    }

    // No two Pokémon may share a new slot, and every one must fit.
    memset(used, 0, size);
    for (i = 0; i < count; i++)
    {
        if (newSlot[i] >= size || used[newSlot[i]])
            return HOENN_ORGANIZE_FAILED;
        used[newSlot[i]] = 1;
    }
    for (r = 0; r < size; r++)
        w->target[r] = NO_SLOT;
    for (i = 0; i < count; i++)
        w->target[newSlot[i]] = w->order[i];
    // Empty old slots fill the unplanned new slots, in order.
    for (r = 0, j = 0; r < size; r++)
    {
        if (IsOccupied(RegionMon(first, r)))
            continue;
        while (j < size && w->target[j] != NO_SLOT)
            j++;
        if (j >= size)
            return HOENN_ORGANIZE_FAILED;
        w->target[j++] = r;
    }

    // 3. The plan must be a permutation of the region.
    memset(used, 0, size);
    for (r = 0; r < size; r++)
    {
        if (w->target[r] >= size || used[w->target[r]])
            return HOENN_ORGANIZE_FAILED;
        used[w->target[r]] = 1;
    }

    // 4. Apply it, cycle by cycle: new[r] = old[target[r]]. used[] now marks
    // slots already written.
    memset(used, 0, size);
    for (r = 0; r < size; r++)
    {
        struct BoxPokemon spare;
        u16 at = r;

        if (used[r] || w->target[r] == r)
            continue;
        spare = *RegionMon(first, r);
        while (w->target[at] != r)
        {
            *RegionMon(first, at) = *RegionMon(first, w->target[at]);
            used[at] = 1;
            at = w->target[at];
        }
        *RegionMon(first, at) = spare;
        used[at] = 1;
    }

    // 5. Same Pokémon as before.
    for (r = 0; r < size; r++)
    {
        struct BoxPokemon *mon = RegionMon(first, r);
        if (IsOccupied(mon))
        {
            sumAfter += MonSum(mon);
            countAfter++;
        }
    }
    if (countAfter != count || sumAfter != sumBefore)
        return HOENN_ORGANIZE_FAILED;

    // 6. Names and wallpapers.
    RelabelBoxes(w, mode, skip);
    ConvertIntToDecimalStringN(gStringVar1, count, STR_CONV_MODE_LEFT_ALIGN, 3);
    return packedResult;
}

// specialvar VAR_RESULT, HoennOrganizeBoxes: VAR_0x8004 mode, VAR_0x8005 boxes
// to leave alone. STR_VAR_1 = how many Pokémon were sorted.
u16 HoennOrganizeBoxes(void)
{
    u16 mode = gSpecialVar_0x8004, result;
    u8 skip = gSpecialVar_0x8005;
    struct OrganizerWork *w;

    if (mode > HOENN_ORGANIZE_BY_LEVEL || skip >= TOTAL_BOXES_COUNT)
        return HOENN_ORGANIZE_FAILED;
    w = Alloc(sizeof(*w));
    if (w == NULL)
        return HOENN_ORGANIZE_FAILED;
    result = Organize(w, mode, skip);
    Free(w);
    return result;
}

#endif // IS_HNS
