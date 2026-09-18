#include "lg/input_control.h"
#include <assert.h>

int main(void) {
    /* 3x3 map, (2,1) blocked. Player starts centered at (1,1). */
    LGMapCell cells[9] = {0};
    cells[1 * 3 + 2].collision = 1;
    LGMap map = {3, 3, cells};
    LGPlayer player = {1, 1, 0, 0};
    LgInputState input = {0, 0, 0};

    /* No directional button pressed: no step, returns false. */
    assert(!lg_overworld_input_step(&player, &map, &input));
    assert(player.x == 1 && player.y == 1);

    /* A held (not pressed) direction doesn't move either. */
    input.held = LG_BUTTON_UP;
    assert(!lg_overworld_input_step(&player, &map, &input));
    assert(player.y == 1);

    /* A newly pressed direction takes exactly one step. */
    input.held = 0;
    input.pressed = LG_BUTTON_DOWN;
    assert(lg_overworld_input_step(&player, &map, &input));
    assert(player.x == 1 && player.y == 2 && player.facing == 2);

    /* Multiple directions pressed at once: up/down/left/right priority. */
    input.pressed = LG_BUTTON_UP | LG_BUTTON_RIGHT;
    assert(lg_overworld_input_step(&player, &map, &input));
    assert(player.x == 1 && player.y == 1); /* up won, back to start */

    /* A blocked direction still counts as "attempted" (returns true) but
       doesn't move the player, matching lg_player_step()'s own contract --
       it does turn to face it. */
    input.pressed = LG_BUTTON_RIGHT;
    assert(lg_overworld_input_step(&player, &map, &input));
    assert(player.x == 1 && player.y == 1 && player.facing == 1);

    /* Holding B (Running Shoes) uses the faster lg_player_run_step()
       presentation hint for the same grid step. Player is at (1,1). */
    input.held = LG_BUTTON_B;
    input.pressed = LG_BUTTON_DOWN;
    assert(lg_overworld_input_step(&player, &map, &input));
    assert(player.x == 1 && player.y == 2);
    assert(player.moving == LG_PLAYER_RUN_SLIDE_FRAMES);

    /* NULL arguments are rejected. */
    assert(!lg_overworld_input_step(0, &map, &input));
    assert(!lg_overworld_input_step(&player, 0, &input));
    assert(!lg_overworld_input_step(&player, &map, 0));

    return 0;
}
