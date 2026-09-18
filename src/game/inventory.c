#include "lg/inventory.h"

bool lg_inventory_add(LGInventory *inv, uint8_t item_id, uint16_t quantity) {
    if (!inv || !inv->slots || item_id == 0 || quantity == 0) return false;
    LGInventorySlot *empty = 0;
    for (size_t i = 0; i < inv->slot_count; ++i) {
        if (inv->slots[i].item_id == item_id) {
            if ((uint32_t)inv->slots[i].quantity + quantity > 0xffffu) return false;
            inv->slots[i].quantity = (uint16_t)(inv->slots[i].quantity + quantity);
            return true;
        }
        if (!empty && inv->slots[i].item_id == 0) empty = &inv->slots[i];
    }
    if (!empty) return false;
    empty->item_id = item_id;
    empty->quantity = quantity;
    return true;
}

bool lg_inventory_remove(LGInventory *inv, uint8_t item_id, uint16_t quantity) {
    if (!inv || !inv->slots || item_id == 0 || quantity == 0) return false;
    for (size_t i = 0; i < inv->slot_count; ++i) {
        if (inv->slots[i].item_id != item_id) continue;
        if (inv->slots[i].quantity < quantity) return false;
        inv->slots[i].quantity = (uint16_t)(inv->slots[i].quantity - quantity);
        if (inv->slots[i].quantity == 0) inv->slots[i].item_id = 0;
        return true;
    }
    return false;
}

uint16_t lg_inventory_count(const LGInventory *inv, uint8_t item_id) {
    if (!inv || !inv->slots || item_id == 0) return 0;
    for (size_t i = 0; i < inv->slot_count; ++i)
        if (inv->slots[i].item_id == item_id) return inv->slots[i].quantity;
    return 0;
}
