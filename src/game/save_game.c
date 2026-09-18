#include "lg/save_game.h"
#include <string.h>

size_t lg_save_game_write(const LGSaveGamePayload *payload, uint64_t save_id,
                           uint8_t *out, size_t capacity) {
    if (!payload || !out) return 0;
    size_t total = sizeof(LGSaveHeader) + sizeof(LGSaveGamePayload);
    if (capacity < total) return 0;

    LGSaveHeader header;
    header.magic = LG_SAVE_MAGIC;
    header.version = LG_SAVE_VERSION;
    header.flags = 0;
    header.save_id = save_id;
    header.payload_size = (uint32_t)sizeof(LGSaveGamePayload);
    header.payload_crc32 = lg_save_crc32(payload, sizeof(LGSaveGamePayload));

    memcpy(out, &header, sizeof header);
    memcpy(out + sizeof header, payload, sizeof(LGSaveGamePayload));
    return total;
}

int lg_save_game_read(const uint8_t *data, size_t size,
                       LGSaveGamePayload *out_payload, uint64_t *out_save_id) {
    if (!data || !out_payload || size < sizeof(LGSaveHeader)) return 0;

    LGSaveHeader header;
    memcpy(&header, data, sizeof header);
    if (header.payload_size != sizeof(LGSaveGamePayload)) return 0;

    const uint8_t *payload_bytes = data + sizeof header;
    size_t available = size - sizeof header;
    if (!lg_save_header_valid(&header, payload_bytes, available)) return 0;

    memcpy(out_payload, payload_bytes, sizeof(LGSaveGamePayload));
    if (out_save_id) *out_save_id = header.save_id;
    return 1;
}

void lg_save_game_capture(const LGPlayer *player, const LGScriptVM *vm,
                           LGSaveGamePayload *out) {
    if (!player || !vm || !out) return;
    out->player_x = player->x;
    out->player_y = player->y;
    out->player_facing = player->facing;
    memcpy(out->flags, vm->flags, sizeof out->flags);
}

void lg_save_game_apply(const LGSaveGamePayload *payload, LGPlayer *player,
                         LGScriptVM *vm) {
    if (!payload || !player || !vm) return;
    player->x = payload->player_x;
    player->y = payload->player_y;
    player->facing = payload->player_facing;
    memcpy(vm->flags, payload->flags, sizeof vm->flags);
}
