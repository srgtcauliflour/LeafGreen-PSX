#ifndef LG_SAVE_GAME_H
#define LG_SAVE_GAME_H
#include "lg/save.h"
#include <stdint.h>

/* M0 gameplay save payload: player position/facing plus the 256 script
   flags (mirrors LGScriptVM.flags). This is the first concrete schema
   behind the versioned LGSV envelope reserved in save.h -- SAVE-FORMAT.md
   deferred exactly this until game-state models existed to serialize.
   Expect it to grow (and LG_SAVE_VERSION to bump) as more M0 state --
   inventory, party, more flags -- gets ported; this is not a complete
   LeafGreen save format. */
typedef struct {
    int16_t player_x, player_y;
    int8_t player_facing;
    uint8_t flags[32];
} LGSaveGamePayload;

/* Writes an LGSV-enveloped copy of payload into out (header immediately
   followed by the payload bytes). Returns the total bytes written, or 0
   if out is too small or args are invalid. */
size_t lg_save_game_write(const LGSaveGamePayload *payload, uint64_t save_id,
                           uint8_t *out, size_t capacity);
/* Validates the header (magic/version/size/CRC32) and, on success, copies
   the payload out and reports the save_id. Returns 1 on success, 0 on any
   corruption/mismatch (wrong magic/version, truncated data, bad CRC, or a
   payload_size that doesn't match sizeof(LGSaveGamePayload)). */
int lg_save_game_read(const uint8_t *data, size_t size,
                       LGSaveGamePayload *out_payload, uint64_t *out_save_id);
#endif
