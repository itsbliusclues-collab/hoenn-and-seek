# Key-item pocket audit (2.1-04)

**Result (Nov 28): the key pocket is now 80 slots.** HnS's was 60, and 2.0 already hands out 63 different key items. A player who keeps everything (fossils unrevived, Abandoned Ship keys unused, every ticket and feather) could hold about 60 at once. That leaves no room for 2.1's own key items: the Seeker's Journal, and whatever the chains, friends and the second rival add. When the pocket is full, `giveitem` says the bag is full and a story item can be lost.

## How many a player can hold at once
- **63** key items are given by scripts Hoenn & Seek builds (table below).
- **Can't all be held together:** the Mach and Acro Bikes (Mauville's shop swaps them), the LOST ITEM and the PASS (one is traded for the other), the SINGING SAND and the SAND CHIME (the sand becomes the chime). That's 3 fewer, so **about 60 at once in the worst case**, which is exactly HnS's 60.
- Most players hold fewer, because the fossils get revived, the ship's keys get used and the quest items (MYSTERY EGG, RED SCALE, MACHINE PART, SECRET POTION, SCANNER, METEORITE, STORAGE KEY) go back. But the pocket has to fit the player who keeps everything.
- ESCAPE ROPE is a key item in HnS (reusable), so it counts too.

## What changed
- `BAG_KEYITEMS_COUNT` is 80 for HnS (`include/constants/global.h`). SaveBlock1 keeps its 60 slots in `struct Bag` (`BAG_KEYITEMS_SB1_COUNT`), because moving them would shift every field after the bag. Those 60 slots are unused from save version 8 on.
- The pocket itself is `hoennKeyItems[80]` at the end of SaveBlock3 (offset 0x474, 320 B). `SetBagItemsPointers` (`src/item.c`) points the pocket there, and `ClearBag` clears it.
- **Save version 8** (`src/save.c`) runs `HoennMoveKeyItemsToSaveBlock3()` (`src/hoenn_2_1_a.c`) once on an older save. It copies the 60 slots as they are (quantities stay encrypted with the same key) and zeroes the old ones.
- SaveBlock3 is now 1,460 of 1,624 B (164 B left). SaveBlock1 is unchanged at 15,760 of 15,872 B. The 2.1 plan expected SaveBlock1 to be full; it has 112 B free, but not in one place the pocket could use.
- Bag code reads pockets through `gBagPockets`, so nothing else needed changing. The bag menu's list buffer is sized by the largest pocket (Items, 236).

## Re-running
`tools/hoenn/key_item_audit.py` prints the counts, and `--write` refreshes the table below. Run it when a part adds key items. If "given" passes about 75, grow the pocket again: SaveBlock3 has room for about 40 more slots.

## The table
<!-- generated: key_item_audit.py -->
64 of 110 key items are given by a built script.

| Item | Given by | Taken back by |
|---|---|---|
| `ACRO_BIKE` | MauvilleCity_BikeShop | MauvilleCity_BikeShop |
| `AURORA_TICKET` | OlivineCity_hns, gift_aurora_ticket.inc, hoenn_lane_a.inc |  |
| `AZURE_FLUTE` | MtSilver_2F_hns |  |
| `BASEMENT_KEY` | GoldenrodCity_RadioTower_5F_hns, MauvilleCity, hoenn_gyms.inc … |  |
| `BICYCLE` | GoldenrodCity_BikeShop_hns |  |
| `CARD_KEY` | GoldenrodCity_UndergroundStorage_hns |  |
| `CLAW_FOSSIL` | DesertUnderpass, MirageTower_4F, PewterCity_Museum_1F_hns | RuinsOfAlph_Lab_hns, RustboroCity_DevonCorp_2F |
| `CLEAR_BELL` | EcruteakCity_Theater_hns, hoenn_johto_legends.inc |  |
| `COIN_CASE` | GoldenrodCity_UndergroundTunnel_hns, MauvilleCity_House2 |  |
| `CONTEST_PASS` | LilycoveCity_ContestLobby_hns |  |
| `DEVON_PARTS` | RusturfTunnel |  |
| `DEVON_SCOPE` | hoenn_story.inc |  |
| `DOME_FOSSIL` | RuinsOfAlph_B1F_hns | RuinsOfAlph_Lab_hns |
| `DOWSING_MACHINE` | EcruteakCity_House2_hns, Route110 |  |
| `ENIGMA_STONE` | hoenn_lane_b.inc |  |
| `EON_TICKET` | FuchsiaCity_SafariZoneEntrance_hns, cable_club.inc |  |
| `ESCAPE_ROPE` | AbandonedShip_Rooms_B1F, GraniteCave_1F, MagmaHideout_3F_3R … |  |
| `EXP_SHARE` | RustboroCity_DevonCorp_3F, new_game.inc |  |
| `FLAME_FEATHER` | hoenn_lane_b.inc |  |
| `FROST_FEATHER` | hoenn_hidden.inc |  |
| `GB_PLAYER` | new_game.inc |  |
| `GLASS_FLUTE` | hoenn_hidden.inc |  |
| `GOOD_ROD` | OlivineCity_House3_hns, Route118 |  |
| `GO_GOGGLES` | LavaridgeTown, hoenn_story.inc |  |
| `HELIX_FOSSIL` | RuinsOfAlph_B1F_hns | RuinsOfAlph_Lab_hns |
| `JADE_ORB` | hoenn_lane_b.inc |  |
| `KEY_TO_ROOM_1` | AbandonedShip_HiddenFloorRooms | AbandonedShip_HiddenFloorCorridors |
| `KEY_TO_ROOM_2` | AbandonedShip_HiddenFloorRooms | AbandonedShip_HiddenFloorCorridors |
| `KEY_TO_ROOM_4` | AbandonedShip_HiddenFloorRooms | AbandonedShip_HiddenFloorCorridors |
| `KEY_TO_ROOM_6` | AbandonedShip_HiddenFloorRooms | AbandonedShip_HiddenFloorCorridors |
| `LETTER` | RustboroCity_DevonCorp_3F |  |
| `LOST_ITEM` | VermilionCity_FanClub_hns | SaffronCity_CopyCatsHouse_2F_hns |
| `MACHINE_PART` | CeruleanCity_Gym_hns | Route10_PowerPlantBackRoom_hns |
| `MACH_BIKE` | MauvilleCity_BikeShop | MauvilleCity_BikeShop |
| `MAGMA_EMBLEM` | MtPyre_Summit |  |
| `METEORITE` | BirthIsland_Exterior_hns, MtChimney, hoenn_gyms.inc | hoenn_gyms.inc |
| `MYSTERY_EGG` | Route30_MrPokemonsHouse_hns | NewBarkTown_Lab_hns |
| `MYSTIC_TICKET` | gift_mystic_ticket.inc |  |
| `OLD_AMBER` | RuinsOfAlph_B1F_hns | RuinsOfAlph_Lab_hns |
| `OLD_ROD` | DewfordTown, Route32_PokemonCenter_hns |  |
| `OLD_SEA_MAP` | VermilionCity_hns, gift_old_sea_map.inc, hoenn_lane_b.inc |  |
| `OVAL_CHARM` | LilycoveCity_CoveLilyMotel_2F |  |
| `PASS` | SaffronCity_CopyCatsHouse_2F_hns |  |
| `POKEBLOCK_CASE` | FuchsiaCity_SafariZoneEntrance_hns, SafariZoneGate_SafariZoneEntrance_hns, contest_hall.inc |  |
| `POWDER_JAR` | SlateportCity |  |
| `RAINBOW_WING` | GoldenrodCity_RadioTower_5F_hns, PewterCity_hns |  |
| `RED_SCALE` | LakeOfRage_hns | Route30_MrPokemonsHouse_hns |
| `ROOT_FOSSIL` | DesertUnderpass, MirageTower_4F, PewterCity_Museum_1F_hns | RuinsOfAlph_Lab_hns, RustboroCity_DevonCorp_2F |
| `SAND_CHIME` | hoenn_hidden.inc |  |
| `SCANNER` | AbandonedShip_HiddenFloorRooms | SlateportCity_Harbor, hoenn_teams.inc |
| `SECRET_POTION` | CianwoodShop_hns | OlivineCity_Lighthouse_hns |
| `SEEKERS_JOURNAL` | hoenn_2_1_a.inc |  |
| `SHINY_CHARM` | LilycoveCity_CoveLilyMotel_2F |  |
| `SILVER_WING` | GoldenrodCity_RadioTower_5F_hns, PewterCity_hns |  |
| `SINGING_SAND` | hoenn_hidden.inc | hoenn_hidden.inc |
| `SOOT_SACK` | Route113_GlassWorkshop |  |
| `SQUIRT_BOTTLE` | GoldenrodCity_FlowerShop_hns |  |
| `SS_TICKET` | NewBarkTown_Lab_hns, players_house.inc |  |
| `STORAGE_KEY` | AbandonedShip_CaptainsOffice | AbandonedShip_Corridors_B1F |
| `SUPER_ROD` | MossdeepCity_House3, Route12_House_hns |  |
| `THUNDER_FEATHER` | hoenn_lane_b.inc |  |
| `TIDAL_BELL` | EcruteakCity_Theater_hns |  |
| `TIDAL_TICKET` | hoenn_ferry.inc |  |
| `WAILMER_PAIL` | Route104_PrettyPetalFlowerShop |  |

Never given (debug, other games, or unused): `BERRY_POUCH`, `BIKE_VOUCHER`, `CATCHING_CHARM`, `DNA_SPLICERS`, `DYNAMAX_BAND`, `EXP_CHARM`, `FAME_CHECKER`, `GLIMMERING_CHARM`, `GOLD_TEETH`, `GRACIDEA`, `HEALING_HEART`, `INFINITE_RARE_CANDIES`, `INFINITE_REPEL`, `LIFT_KEY`, `MEGA_RING`, `N_LUNARIZER`, `N_SOLARIZER`, `PARCEL`, `POKEMON_BOX_LINK`, `POKE_FLUTE`, `POKE_RADAR`, `PRISON_BOTTLE`, `RADIO`, `RAINBOW_PASS`, `REINS_OF_UNITY`, `REVEAL_GLASS`, `ROOM_1_KEY`, `ROOM_2_KEY`, `ROOM_4_KEY`, `ROOM_6_KEY`, `ROTOM_CATALOG`, `RUBY`, `SAPPHIRE`, `SCROLL_OF_DARKNESS`, `SCROLL_OF_WATERS`, `SECRET_KEY`, `SILPH_SCOPE`, `TEA`, `TEACHY_TV`, `TERA_ORB`, `TM_CASE`, `TOWN_MAP`, `TRI_PASS`, `VS_SEEKER`, `ZYGARDE_CUBE`, `Z_POWER_RING`.
<!-- end generated -->
