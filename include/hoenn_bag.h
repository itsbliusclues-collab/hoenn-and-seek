#ifndef GUARD_HOENN_BAG_H
#define GUARD_HOENN_BAG_H

// Hoenn & Seek 2.1-04: a bigger key-item pocket.
//
// The audit (tools/hoenn/key_item_audit.py, hoenn-docs/key-items.md) found
// that a player can hold up to 57 key items at once, and the pocket has 60
// slots (BAG_KEYITEMS_COUNT, in SaveBlock1, which is full). 2.1 adds more key
// items, so the pocket now has HOENN_KEY_ITEMS_COUNT slots: SaveBlock1's 60,
// then HOENN_KEY_ITEMS_EXTRA more at the end of SaveBlock3
// (gSaveBlock3Ptr->hoennKeyItemsExtra).
//
// The two arrays aren't next to each other, so every access to a key-item
// slot goes through HoennBagSlot() (src/item.c's four slot accessors and the
// encryption-key loop are the only places that index a pocket's slots). Other
// pockets and the PC are unchanged.

#include "item.h"

#define HOENN_KEY_ITEMS_COUNT (BAG_KEYITEMS_COUNT + HOENN_KEY_ITEMS_EXTRA) // 80

#if IS_HNS
static inline struct ItemSlot *HoennBagSlot(struct BagPocket *pocket, u32 pos)
{
    if (pocket->id == POCKET_KEY_ITEMS && pos >= BAG_KEYITEMS_COUNT)
        return (struct ItemSlot *)&gSaveBlock3Ptr->hoennKeyItemsExtra[pos - BAG_KEYITEMS_COUNT];
    return &pocket->itemSlots[pos];
}
#define HOENN_KEY_POCKET_CAPACITY HOENN_KEY_ITEMS_COUNT
#else
#define HoennBagSlot(pocket, pos) (&(pocket)->itemSlots[(pos)])
#define HOENN_KEY_POCKET_CAPACITY BAG_KEYITEMS_COUNT
#endif

#include "hoenn_flags.h" // HoennClearKeyItemsExtra()

#endif // GUARD_HOENN_BAG_H
