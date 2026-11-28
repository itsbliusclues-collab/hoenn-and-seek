// Hoenn & Seek 2.1-13: frame tables and graphics info of the friend trainers' sprites
// (pics in hoenn_friend_pics.h). Same shape as HnS's 16x32 trainers, own palette each.
static const struct SpriteFrameImage sPicTable_ExplorerBridgette[] = {
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 0),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 1),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 2),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 3),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 4),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 5),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 6),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 7),
    overworld_frame(gObjectEventPic_ExplorerBridgette, 2, 4, 8),
};
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_ExplorerBridgette = {TAG_NONE, OBJ_EVENT_PAL_TAG_EXPLORER_BRIDGETTE, OBJ_EVENT_PAL_TAG_NONE, 256, 16, 32, 5, SHADOW_SIZE_M, FALSE, FALSE, TRACKS_FOOT, &gObjectEventBaseOam_16x32, sOamTables_16x32, sAnimTable_Standard, sPicTable_ExplorerBridgette, gDummySpriteAffineAnimTable};

static const struct SpriteFrameImage sPicTable_SkydiverAidan[] = {
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 0),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 1),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 2),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 3),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 4),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 5),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 6),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 7),
    overworld_frame(gObjectEventPic_SkydiverAidan, 2, 4, 8),
};
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_SkydiverAidan = {TAG_NONE, OBJ_EVENT_PAL_TAG_SKYDIVER_AIDAN, OBJ_EVENT_PAL_TAG_NONE, 256, 16, 32, 5, SHADOW_SIZE_M, FALSE, FALSE, TRACKS_FOOT, &gObjectEventBaseOam_16x32, sOamTables_16x32, sAnimTable_Standard, sPicTable_SkydiverAidan, gDummySpriteAffineAnimTable};

static const struct SpriteFrameImage sPicTable_AthleteBrock[] = {
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 0),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 1),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 2),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 3),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 4),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 5),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 6),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 7),
    overworld_frame(gObjectEventPic_AthleteBrock, 2, 4, 8),
};
const struct ObjectEventGraphicsInfo gObjectEventGraphicsInfo_AthleteBrock = {TAG_NONE, OBJ_EVENT_PAL_TAG_ATHLETE_BROCK, OBJ_EVENT_PAL_TAG_NONE, 256, 16, 32, 5, SHADOW_SIZE_M, FALSE, FALSE, TRACKS_FOOT, &gObjectEventBaseOam_16x32, sOamTables_16x32, sAnimTable_Standard, sPicTable_AthleteBrock, gDummySpriteAffineAnimTable};
