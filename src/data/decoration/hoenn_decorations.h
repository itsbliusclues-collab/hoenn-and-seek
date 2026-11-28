// Hoenn & Seek (2.0 part 13a): data for the decorations in constants/hoenn_decorations.h.
// Included at the end of description.h; HOENN_DECORATION_HEADERS ends gDecorations (header.h)
// and HOENN_DECORATION_ICONS ends gDecorIconTable (icon.h: NULL icons, so the menus draw
// the object sprite, like Emerald's small dolls).

static const u8 DecorDesc_SUDOWOODO_DOLL[] = _(
    "A doll of the\n"
        "POKéMON that\n"
        "poses as a tree.");
static const u16 DecorGfx_SUDOWOODO_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_SUDOWOODO};

static const u8 DecorDesc_SLOWPOKE_DOLL[] = _(
    "A doll of a\n"
        "POKéMON from the\n"
        "SLOWPOKE WELL.");
static const u16 DecorGfx_SLOWPOKE_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_SLOWPOKE};

static const u8 DecorDesc_MAREEP_DOLL[] = _(
    "A fluffy doll.\n"
        "Its wool feels\n"
        "slightly static.");
static const u16 DecorGfx_MAREEP_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_MAREEP};

static const u8 DecorDesc_UNOWN_DOLL[] = _(
    "A doll of a\n"
        "letter from the\n"
        "RUINS OF ALPH.");
static const u16 DecorGfx_UNOWN_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_UNOWN};

static const u8 DecorDesc_VULPIX_DOLL[] = _(
    "A doll of a\n"
        "VULPIX with six\n"
        "curly tails.");
static const u16 DecorGfx_VULPIX_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_VULPIX};

static const u8 DecorDesc_EEVEE_DOLL[] = _(
    "A doll of a\n"
        "POKéMON that can\n"
        "evolve many ways.");
static const u16 DecorGfx_EEVEE_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_EEVEE};

// 2.1-06: the professors' rewards (Elm, Oak, Birch) for a complete SEEKER'S
// JOURNAL page. Price 0: nobody sells them, and they can't be sold back.
static const u8 DecorDesc_CELEBI_DOLL[] = _(
    "A gift from\n"
        "PROF. ELM for a\n"
        "full JOHTO page.");
static const u16 DecorGfx_CELEBI_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_CELEBI};

static const u8 DecorDesc_MEW_DOLL[] = _(
    "A gift from\n"
        "PROF. OAK for a\n"
        "full KANTO page.");
static const u16 DecorGfx_MEW_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_MEW};

static const u8 DecorDesc_JIRACHI_DOLL[] = _(
    "A gift from\n"
        "PROF. BIRCH for a\n"
        "full HOENN page.");
static const u16 DecorGfx_JIRACHI_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_JIRACHI};

// 2.1-21: BROCK's reward at the end of his weekend chain.
static const u8 DecorDesc_ONIX_DOLL[] = _(
    "Stitched by\n"
        "BROCK's brothers\n"
        "and sisters.");
static const u16 DecorGfx_ONIX_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_ONIX};

// 2.1-22: MISTY's reward at the end of her weekend chain.
static const u8 DecorDesc_STARMIE_DOLL[] = _(
    "Sewn by MISTY's\n"
        "three sisters for\n"
        "her water show.");
static const u16 DecorGfx_STARMIE_DOLL[] = {OBJ_EVENT_GFX_MON_BASE + SPECIES_STARMIE};

#define HOENN_DECORATION_HEADERS \
    [DECOR_SUDOWOODO_DOLL] = {.id = DECOR_SUDOWOODO_DOLL, .name = _("SUDOWOODO DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 6000, \
        .description = DecorDesc_SUDOWOODO_DOLL, .tiles = DecorGfx_SUDOWOODO_DOLL}, \
    [DECOR_SLOWPOKE_DOLL] = {.id = DECOR_SLOWPOKE_DOLL, .name = _("SLOWPOKE DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 6000, \
        .description = DecorDesc_SLOWPOKE_DOLL, .tiles = DecorGfx_SLOWPOKE_DOLL}, \
    [DECOR_MAREEP_DOLL] = {.id = DECOR_MAREEP_DOLL, .name = _("MAREEP DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 6000, \
        .description = DecorDesc_MAREEP_DOLL, .tiles = DecorGfx_MAREEP_DOLL}, \
    [DECOR_UNOWN_DOLL] = {.id = DECOR_UNOWN_DOLL, .name = _("UNOWN DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 6000, \
        .description = DecorDesc_UNOWN_DOLL, .tiles = DecorGfx_UNOWN_DOLL}, \
    [DECOR_VULPIX_DOLL] = {.id = DECOR_VULPIX_DOLL, .name = _("VULPIX DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 6000, \
        .description = DecorDesc_VULPIX_DOLL, .tiles = DecorGfx_VULPIX_DOLL}, \
    [DECOR_EEVEE_DOLL] = {.id = DECOR_EEVEE_DOLL, .name = _("EEVEE DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 6000, \
        .description = DecorDesc_EEVEE_DOLL, .tiles = DecorGfx_EEVEE_DOLL}, \
    [DECOR_CELEBI_DOLL] = {.id = DECOR_CELEBI_DOLL, .name = _("CELEBI DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 0, \
        .description = DecorDesc_CELEBI_DOLL, .tiles = DecorGfx_CELEBI_DOLL}, \
    [DECOR_MEW_DOLL] = {.id = DECOR_MEW_DOLL, .name = _("MEW DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 0, \
        .description = DecorDesc_MEW_DOLL, .tiles = DecorGfx_MEW_DOLL}, \
    [DECOR_JIRACHI_DOLL] = {.id = DECOR_JIRACHI_DOLL, .name = _("JIRACHI DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 0, \
        .description = DecorDesc_JIRACHI_DOLL, .tiles = DecorGfx_JIRACHI_DOLL}, \
    [DECOR_ONIX_DOLL] = {.id = DECOR_ONIX_DOLL, .name = _("ONIX DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 0, \
        .description = DecorDesc_ONIX_DOLL, .tiles = DecorGfx_ONIX_DOLL}, \
    [DECOR_STARMIE_DOLL] = {.id = DECOR_STARMIE_DOLL, .name = _("STARMIE DOLL"), .permission = DECORPERM_SPRITE, \
        .shape = DECORSHAPE_1x1, .category = DECORCAT_DOLL, .price = 0, \
        .description = DecorDesc_STARMIE_DOLL, .tiles = DecorGfx_STARMIE_DOLL}, \

#define HOENN_DECORATION_ICONS \
    [DECOR_SUDOWOODO_DOLL] = {NULL, NULL}, \
    [DECOR_SLOWPOKE_DOLL] = {NULL, NULL}, \
    [DECOR_MAREEP_DOLL] = {NULL, NULL}, \
    [DECOR_UNOWN_DOLL] = {NULL, NULL}, \
    [DECOR_VULPIX_DOLL] = {NULL, NULL}, \
    [DECOR_EEVEE_DOLL] = {NULL, NULL}, \
    [DECOR_CELEBI_DOLL] = {NULL, NULL}, \
    [DECOR_MEW_DOLL] = {NULL, NULL}, \
    [DECOR_JIRACHI_DOLL] = {NULL, NULL}, \
    [DECOR_ONIX_DOLL] = {NULL, NULL}, \
    [DECOR_STARMIE_DOLL] = {NULL, NULL}, \
