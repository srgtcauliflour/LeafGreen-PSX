# Save format foundation

The PS1 memory-card transport and logical LeafGreen save serialization are separate layers. A small `LGSV` versioned envelope is reserved now with stable save identity, payload size and CRC32.

This early boundary supports later transactional/backup writes and gives M10 memory-card trading a stable save identity without making trade code depend on card-sector details. Do not interpret the current envelope as a complete PS1 memory-card file format; card directory/icon/header handling belongs to the PS1 backend.

## M0 gameplay payload (`lg/save_game.h`)

`LGSaveGamePayload` (`player_x`, `player_y`, `player_facing`, and the 256
script flags mirroring `LGScriptVM.flags`) is the first concrete schema
behind the `LGSV` envelope, now that those game-state models exist to
serialize. `lg_save_game_write()` builds a header (magic/version/save
id/payload size/CRC32) plus the payload into a caller-supplied buffer;
`lg_save_game_read()` validates that header (rejecting a truncated
buffer, wrong magic/version, a payload_size that doesn't match
`sizeof(LGSaveGamePayload)`, or a bad CRC32) before copying the payload
out. See `tests/host/test_save_game.c` for the round-trip and every
rejection case.

`lg_save_game_capture(player, vm, out)`/`lg_save_game_apply(payload,
player, vm)` connect the payload to the live `LGPlayer`/`LGScriptVM`
structs directly, rather than leaving the caller to copy fields by hand:
capture fills a payload from a player's x/y/facing and a VM's 256 flags;
apply writes them back. Apply only touches x/y/facing/flags -- it never
resets `vm->pc`/`status`/`code`, since resuming mid-script from a save
isn't part of this schema. Both are no-ops (never a partial
read/write) if any argument is NULL.

This is still only the M0 slice: inventory, party and other LeafGreen
save state aren't ported yet, so `LGSaveGamePayload` will grow (bumping
`LG_SAVE_VERSION`) as they are. No PS1 memory-card I/O exists yet either
-- this only covers the in-memory envelope/payload, not writing it to a
card.
