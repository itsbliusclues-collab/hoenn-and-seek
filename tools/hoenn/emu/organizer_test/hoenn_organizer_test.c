// TEST-ONLY (2.1-32): never committed. Fills boxes, sorts, dumps for the harness.
#include "global.h"
#include "event_data.h"
#include "malloc.h"
#include "pokemon.h"
#include "pokemon_storage_system.h"
#include "random.h"
#include "constants/hoenn_events_2_1.h"
u16 HoennOrganizeBoxes(void);
struct OrgTestRec { u16 species, dex; u8 level, egg, type, has; u32 pers; };
struct OrgTest { u32 seed; u8 mode, skip, fill, done; u16 result, pad; struct OrgTestRec *pre, *post; };
EWRAM_DATA struct OrgTest gHoennOrgTest = {0};
static u32 sSeed;
static u32 Rnd(void) { sSeed = sSeed * 1103515245 + 12345; return sSeed >> 8; }
static void Dump(struct OrgTestRec *d)
{
    u32 i;
    for (i = 0; i < TOTAL_BOXES_COUNT * IN_BOX_COUNT; i++)
    {
        struct BoxPokemon *m = GetBoxedMonPtr(i / IN_BOX_COUNT, i % IN_BOX_COUNT);
        u16 sp = GetBoxMonData(m, MON_DATA_SPECIES);
        d[i].has = GetBoxMonData(m, MON_DATA_SANITY_HAS_SPECIES) != 0;
        d[i].species = sp;
        d[i].dex = SpeciesToNationalPokedexNum(SanitizeSpeciesId(sp));
        d[i].level = d[i].has ? GetLevelFromBoxMonExp(m) : 0;
        d[i].egg = GetBoxMonData(m, MON_DATA_IS_EGG);
        d[i].type = GetSpeciesType(SanitizeSpeciesId(sp), 0);
        d[i].pers = GetBoxMonData(m, MON_DATA_PERSONALITY);
    }
}
static void Fill(void)
{
    u32 i;
    sSeed = gHoennOrgTest.seed;
    for (i = 0; i < TOTAL_BOXES_COUNT * IN_BOX_COUNT; i++)
    {
        struct BoxPokemon *m = GetBoxedMonPtr(i / IN_BOX_COUNT, i % IN_BOX_COUNT);
        u16 sp;
        if (Rnd() % 100 >= gHoennOrgTest.fill) { ZeroBoxMonData(m); continue; }
        do { sp = 1 + Rnd() % (NUM_SPECIES - 1); } while (!IsSpeciesEnabled(sp) || SpeciesToNationalPokedexNum(sp) == 0);
        if (Rnd() % 4 == 0) sp = 1 + Rnd() % 30; // plenty of duplicates
        CreateBoxMon(m, sp, 1 + Rnd() % 100, Rnd() ^ (Rnd() << 16), OTID_STRUCT_PLAYER_ID);
        if (Rnd() % 20 == 0) { u8 one = 1; SetBoxMonData(m, MON_DATA_IS_EGG, &one); }
    }
}
void HoennOrganizerTestRun(void)
{
    if (gHoennOrgTest.pre == NULL) { gHoennOrgTest.pre = Alloc(420 * sizeof(struct OrgTestRec)); gHoennOrgTest.post = Alloc(420 * sizeof(struct OrgTestRec)); }
    if (gHoennOrgTest.fill) Fill();
    Dump(gHoennOrgTest.pre);
    if (gHoennOrgTest.mode < 3) { gSpecialVar_0x8004 = gHoennOrgTest.mode; gSpecialVar_0x8005 = gHoennOrgTest.skip; gHoennOrgTest.result = HoennOrganizeBoxes(); }
    Dump(gHoennOrgTest.post);
    gHoennOrgTest.done = 1;
}
