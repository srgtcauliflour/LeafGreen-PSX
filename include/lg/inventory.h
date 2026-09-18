#ifndef LG_INVENTORY_H
#define LG_INVENTORY_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* A generic fixed-slot bag: item_id 0 marks an empty slot. This is not
   verified LeafGreen inventory data -- real bag pockets, capacity, key
   items and stacking rules all need ROM evidence. It exists so OP_ITEM
   (script.h) has something real to wire to. */
typedef struct { uint8_t item_id; uint16_t quantity; } LGInventorySlot;
typedef struct { LGInventorySlot *slots; size_t slot_count; } LGInventory;

/* Adds quantity of item_id: tops up an existing slot for that id, or
   claims the first empty slot. Returns false, changing nothing, if
   quantity is 0, item_id is 0 (reserved for "empty"), adding would
   overflow the slot's uint16_t quantity, or no matching/empty slot
   exists (bag full for this new item) -- never a partial add. */
bool lg_inventory_add(LGInventory *inv, uint8_t item_id, uint16_t quantity);
/* Removes quantity of item_id. Returns false, changing nothing, if
   quantity is 0, item_id is 0, or the held quantity is less than
   requested -- never a partial removal. Clears the slot (item_id=0)
   once its quantity reaches exactly 0. */
bool lg_inventory_remove(LGInventory *inv, uint8_t item_id, uint16_t quantity);
/* Returns the quantity held of item_id (0 if none, or if item_id is 0). */
uint16_t lg_inventory_count(const LGInventory *inv, uint8_t item_id);
#endif
