// Hoenn & Seek (2.0 part 13a): Johto and Kanto dolls, after Emerald's 120 decorations.
// Each one shows the species' overworld sprite (OBJ_EVENT_GFX_MON_BASE + species), the
// same sprite HnS uses for following Pokémon. Data in src/data/decoration/hoenn_decorations.h;
// sold by the decoration clerks in Goldenrod's and Celadon's department stores.
// Included at the end of constants/decorations.h. IDs are u8 (saved), so 255 is the limit.
#ifndef GUARD_CONSTANTS_HOENN_DECORATIONS_H
#define GUARD_CONSTANTS_HOENN_DECORATIONS_H

#define DECOR_SUDOWOODO_DOLL 121
#define DECOR_SLOWPOKE_DOLL 122
#define DECOR_MAREEP_DOLL 123
#define DECOR_UNOWN_DOLL 124
#define DECOR_VULPIX_DOLL 125
#define DECOR_EEVEE_DOLL 126
// 2.1-06: the professors' rewards for a complete SEEKER'S JOURNAL page (not sold)
#define DECOR_CELEBI_DOLL 127
#define DECOR_MEW_DOLL 128
#define DECOR_JIRACHI_DOLL 129
// 2.1-21 to 23: the fan chains' rewards (not sold)
#define DECOR_ONIX_DOLL 130
#define DECOR_STARMIE_DOLL 131
#define DECOR_PIDGEOT_DOLL 132

#define HOENN_DECORATIONS_START DECOR_SUDOWOODO_DOLL
#define HOENN_DECORATIONS_END   DECOR_PIDGEOT_DOLL

#endif
