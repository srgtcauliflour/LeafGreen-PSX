#include "lg/inventory.h"
#include <assert.h>

int main(void) {
    LGInventorySlot slots[2] = {0};
    LGInventory inv = {slots, 2};

    /* Adding a new item claims the first empty slot. */
    assert(lg_inventory_add(&inv, 5, 3));
    assert(lg_inventory_count(&inv, 5) == 3);

    /* Adding the same item again tops up the existing slot, not a new one. */
    assert(lg_inventory_add(&inv, 5, 2));
    assert(lg_inventory_count(&inv, 5) == 5);

    /* A different item claims the second (last empty) slot. */
    assert(lg_inventory_add(&inv, 9, 1));
    assert(lg_inventory_count(&inv, 9) == 1);

    /* Bag full: no empty slot and no matching one for a third item. */
    assert(!lg_inventory_add(&inv, 7, 1));
    assert(lg_inventory_count(&inv, 7) == 0);

    /* Removing partially, then fully (clearing the slot so a new item can
       claim it), then removing more than held all behave correctly. */
    assert(lg_inventory_remove(&inv, 5, 2));
    assert(lg_inventory_count(&inv, 5) == 3);
    assert(lg_inventory_remove(&inv, 5, 3));
    assert(lg_inventory_count(&inv, 5) == 0);
    assert(lg_inventory_add(&inv, 7, 1)); /* slot 5 vacated, now usable */
    assert(!lg_inventory_remove(&inv, 9, 5)); /* more than held */
    assert(lg_inventory_count(&inv, 9) == 1); /* unchanged, not partial */

    /* item_id 0 (reserved for "empty") and quantity 0 are always rejected. */
    assert(!lg_inventory_add(&inv, 0, 1));
    assert(!lg_inventory_add(&inv, 9, 0));
    assert(!lg_inventory_remove(&inv, 0, 1));
    assert(!lg_inventory_remove(&inv, 9, 0));
    assert(lg_inventory_count(&inv, 0) == 0);

    /* Overflowing a slot's uint16_t quantity is rejected, not wrapped. */
    LGInventorySlot near_max[1] = {{3, 0xfffe}};
    LGInventory tight = {near_max, 1};
    assert(!lg_inventory_add(&tight, 3, 3));
    assert(lg_inventory_count(&tight, 3) == 0xfffe);
    assert(lg_inventory_add(&tight, 3, 1));
    assert(lg_inventory_count(&tight, 3) == 0xffff);

    /* Invalid arguments are rejected. */
    assert(!lg_inventory_add(0, 1, 1));
    assert(!lg_inventory_remove(0, 1, 1));
    assert(lg_inventory_count(0, 1) == 0);
    LGInventory no_slots = {0, 0};
    assert(!lg_inventory_add(&no_slots, 1, 1));

    return 0;
}
