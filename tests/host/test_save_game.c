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

    return 0;
}
