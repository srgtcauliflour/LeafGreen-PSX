#include "lg/save_game.h"
#include <assert.h>
#include <string.h>

int main(void) {
    LGSaveGamePayload payload;
    memset(&payload, 0, sizeof payload);
    payload.player_x = 5;
    payload.player_y = -6;
    payload.player_facing = 2;
    payload.flags[0] = 0x81; /* bits 0 and 7 set */

    uint8_t buffer[sizeof(LGSaveHeader) + sizeof(LGSaveGamePayload)];
    size_t written = lg_save_game_write(&payload, 0x1234567890ULL, buffer, sizeof buffer);
    assert(written == sizeof buffer);

    LGSaveGamePayload round_trip;
    memset(&round_trip, 0xff, sizeof round_trip); /* poison before read */
    uint64_t save_id = 0;
    assert(lg_save_game_read(buffer, written, &round_trip, &save_id) == 1);
    assert(save_id == 0x1234567890ULL);
    assert(round_trip.player_x == 5 && round_trip.player_y == -6);
    assert(round_trip.player_facing == 2);
    assert(round_trip.flags[0] == 0x81);
    assert(round_trip.flags[1] == 0);

    /* out_save_id is optional. */
    assert(lg_save_game_read(buffer, written, &round_trip, 0) == 1);

    /* A too-small output buffer is rejected, not silently truncated. */
    uint8_t small[4];
    assert(lg_save_game_write(&payload, 1, small, sizeof small) == 0);

    /* A too-small input, corrupted payload, or mismatched payload_size
       are all rejected -- corruption is never silently accepted. */
    assert(lg_save_game_read(buffer, sizeof(LGSaveHeader) - 1, &round_trip, 0) == 0);
    uint8_t corrupted[sizeof buffer];
    memcpy(corrupted, buffer, sizeof buffer);
    corrupted[sizeof(LGSaveHeader)] ^= 0xff; /* flip a payload byte */
    assert(lg_save_game_read(corrupted, sizeof corrupted, &round_trip, 0) == 0);

    LGSaveHeader bad_size_header;
    memcpy(&bad_size_header, buffer, sizeof bad_size_header);
    bad_size_header.payload_size = sizeof(LGSaveGamePayload) - 1;
    uint8_t bad_size[sizeof buffer];
    memcpy(bad_size, &bad_size_header, sizeof bad_size_header);
    memcpy(bad_size + sizeof bad_size_header, buffer + sizeof bad_size_header,
           sizeof buffer - sizeof bad_size_header);
    assert(lg_save_game_read(bad_size, sizeof bad_size, &round_trip, 0) == 0);

    /* Invalid arguments are rejected. */
    assert(lg_save_game_write(0, 1, buffer, sizeof buffer) == 0);
    assert(lg_save_game_write(&payload, 1, 0, sizeof buffer) == 0);
    assert(lg_save_game_read(0, sizeof buffer, &round_trip, 0) == 0);
    assert(lg_save_game_read(buffer, written, 0, 0) == 0);

    /* Capture/apply round-trip real LGPlayer/LGScriptVM/LGInventory state,
       not just a hand-filled payload struct. */
    LGPlayer player = {7, -3, 1, 0};
    LGScriptVM vm;
    const uint8_t flag_script[] = {7, 40, 1, 0}; /* OP_FLAG_SET 40=1, OP_END */
    lg_script_init(&vm, flag_script, sizeof flag_script);
    assert(lg_script_step(&vm) == LG_SCRIPT_RUNNING);

    LGInventorySlot inv_slots[LG_SAVE_INVENTORY_SLOTS];
    memset(inv_slots, 0, sizeof inv_slots);
    LGInventory inventory = {inv_slots, LG_SAVE_INVENTORY_SLOTS};
    assert(lg_inventory_add(&inventory, 5, 3));

    LGSaveGamePayload captured;
    memset(&captured, 0xff, sizeof captured); /* poison before capture */
    lg_save_game_capture(&player, &vm, &inventory, &captured);
    assert(captured.player_x == 7 && captured.player_y == -3 && captured.player_facing == 1);
    assert((captured.flags[40 / 8] >> (40 % 8)) & 1);
    assert(captured.inventory[0].item_id == 5 && captured.inventory[0].quantity == 3);

    uint8_t save_buffer[sizeof(LGSaveHeader) + sizeof(LGSaveGamePayload)];
    size_t save_written = lg_save_game_write(&captured, 99, save_buffer, sizeof save_buffer);
    assert(save_written == sizeof save_buffer);

    LGPlayer restored_player = {0, 0, 0, 0};
    LGScriptVM restored_vm;
    lg_script_init(&restored_vm, flag_script, sizeof flag_script); /* flags start clear */
    LGInventorySlot restored_slots[LG_SAVE_INVENTORY_SLOTS];
    memset(restored_slots, 0, sizeof restored_slots);
    LGInventory restored_inventory = {restored_slots, LG_SAVE_INVENTORY_SLOTS};
    LGSaveGamePayload loaded;
    assert(lg_save_game_read(save_buffer, save_written, &loaded, 0) == 1);
    lg_save_game_apply(&loaded, &restored_player, &restored_vm, &restored_inventory);
    assert(restored_player.x == 7 && restored_player.y == -3 && restored_player.facing == 1);
    assert((restored_vm.flags[40 / 8] >> (40 % 8)) & 1);
    assert(lg_inventory_count(&restored_inventory, 5) == 3);
    /* Only x/y/facing/flags/inventory are touched -- execution state is untouched. */
    assert(restored_vm.status == LG_SCRIPT_RUNNING);

    /* Invalid arguments are no-ops, not partial writes. */
    LGSaveGamePayload untouched;
    memset(&untouched, 0x42, sizeof untouched);
    lg_save_game_capture(0, &vm, &inventory, &untouched);
    lg_save_game_capture(&player, 0, &inventory, &untouched);
    lg_save_game_capture(&player, &vm, 0, &untouched);
    assert(untouched.player_x == (int16_t)0x4242);
    LGPlayer unchanged_player = {1, 2, 3, 0};
    lg_save_game_apply(&loaded, 0, &restored_vm, &restored_inventory);
    lg_save_game_apply(0, &unchanged_player, &restored_vm, &restored_inventory);
    lg_save_game_apply(&loaded, &unchanged_player, &restored_vm, 0);
    assert(unchanged_player.x == 1 && unchanged_player.y == 2 && unchanged_player.facing == 3);

    /* A mismatched slot_count is a no-op too -- never a partial copy. */
    LGInventorySlot wrong_size_slots[LG_SAVE_INVENTORY_SLOTS - 1];
    memset(wrong_size_slots, 0, sizeof wrong_size_slots);
    LGInventory wrong_size_inventory = {wrong_size_slots, LG_SAVE_INVENTORY_SLOTS - 1};
    LGSaveGamePayload wrong_size_captured;
    memset(&wrong_size_captured, 0x42, sizeof wrong_size_captured);
    lg_save_game_capture(&player, &vm, &wrong_size_inventory, &wrong_size_captured);
    assert(wrong_size_captured.player_x == (int16_t)0x4242);
    lg_save_game_apply(&loaded, &unchanged_player, &restored_vm, &wrong_size_inventory);
    assert(unchanged_player.x == 1 && unchanged_player.y == 2 && unchanged_player.facing == 3);

    return 0;
}
