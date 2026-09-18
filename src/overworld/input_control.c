#include "lg/input_control.h"

bool lg_overworld_input_step(LGPlayer *player, const LGMap *map,
                              const LgInputState *input) {
    if (!player || !map || !input) return false;
    int dx = 0, dy = 0;
    if (input->pressed & LG_BUTTON_UP) dy = -1;
    else if (input->pressed & LG_BUTTON_DOWN) dy = 1;
    else if (input->pressed & LG_BUTTON_LEFT) dx = -1;
    else if (input->pressed & LG_BUTTON_RIGHT) dx = 1;
    else return false;
    if (input->held & LG_BUTTON_B) lg_player_run_step(player, map, dx, dy);
    else lg_player_step(player, map, dx, dy);
    return true;
}
