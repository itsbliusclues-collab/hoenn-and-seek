// Hoenn & Seek 2.1-13: overworld sprites of the three friend trainers.
// The PNGs in graphics/hoenn/friends/ come from tools/hoenn/install_friend_sprites.py
// (10 frames of 16x32 stacked vertically, each with its own 16-colour palette).
// They are 2.1-01's placeholder repaints until Bryan's designs replace them.
const u32 gObjectEventPic_ExplorerBridgette[] = INCBIN_U32("graphics/hoenn/friends/bridgette_overworld.4bpp");
const u32 gObjectEventPic_SkydiverAidan[] = INCBIN_U32("graphics/hoenn/friends/aidan_overworld.4bpp");
const u32 gObjectEventPic_AthleteBrock[] = INCBIN_U32("graphics/hoenn/friends/brock_overworld.4bpp");
const u16 gObjectEventPal_ExplorerBridgette[] = INCBIN_U16("graphics/hoenn/friends/bridgette_overworld.gbapal");
const u16 gObjectEventPal_SkydiverAidan[] = INCBIN_U16("graphics/hoenn/friends/aidan_overworld.gbapal");
const u16 gObjectEventPal_AthleteBrock[] = INCBIN_U16("graphics/hoenn/friends/brock_overworld.gbapal");
