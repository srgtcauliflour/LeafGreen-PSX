#include "lg/runner.h"
#include <assert.h>
#include <string.h>

typedef struct { unsigned count; } Seen;
static bool capture(void *context, uint8_t glyph, int x, int y, uint8_t width) {
    (void)glyph; (void)x; (void)y; (void)width;
    ((Seen *)context)->count++;
    return true;
}

static bool lookup_text(void *context, uint8_t id, const uint8_t **text, size_t *size) {
    (void)context;
    static const uint8_t hi[] = {0xbb, 0xbc, 0xff};
    if (id != 3) return false;
    *text = hi;
    *size = sizeof hi;
    return true;
}

static bool lookup_script(void *context, uint8_t script_id, const uint8_t **code, size_t *size) {
    (void)context;
    static const uint8_t greet[] = {4, 3, 0}; /* OP_TEXT 3, OP_END */
    if (script_id != 5) return false;
    *code = greet;
    *size = sizeof greet;
    return true;
}

int main(void) {
    /* 3x3 map; (1,1) is both blocked and where the NPC stands, so the
       player can face it from (1,0) without ever stepping onto it. */
    LGMapCell cells[9] = {0};
    cells[1 * 3 + 1].collision = 1;
    LGMap map = {3, 3, cells};
    LGPlayer player = {1, 0, 0, 0};
    LGGameService svc;
    LGScriptVM vm;
    lg_game_service_init(&svc, &player, &map, 0, 0);

    uint8_t widths[256];
    memset(widths, 6, sizeof widths);
    Seen seen = {0};
    LGGameLoop loop;
    lg_game_loop_init(&loop, &vm, &svc, widths, capture, &seen, lookup_text, 0, 10, 20, 0, 0);

    LGObjectEvent events[] = {{1, 1, 5}};
    LGOverworldRunner runner;
    lg_overworld_runner_init(&runner, &player, &svc, &vm, &loop, events, 1, lookup_script, 0);

    LgInputState input = {0, 0, 0};

    /* Idle: no input, nothing happens. */
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_IDLE);
    assert(player.x == 1 && player.y == 0);

    /* Idle: pressing down turns to face the NPC's (blocked) tile without
       stepping onto it -- lg_player_step()'s own contract. */
    input.pressed = LG_BUTTON_DOWN;
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_IDLE);
    assert(player.x == 1 && player.y == 0 && player.facing == 2);

    /* Pressing A while facing the NPC starts its script. */
    input.pressed = LG_BUTTON_A;
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_SCRIPT_STARTED);

    /* The runner now drives the script via LGGameLoop each frame. */
    input.pressed = 0;
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_SCRIPT_RUNNING); /* opens window */
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_SCRIPT_RUNNING); /* draws + closes window, unblocks */
    assert(seen.count == 2);
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_SCRIPT_DONE); /* OP_END */

    /* Back to idle: free-roam works again. */
    input.pressed = LG_BUTTON_RIGHT;
    assert(lg_overworld_runner_step(&runner, &input, false) == LG_RUNNER_STEP_IDLE);
    assert(player.x == 2 && player.y == 0);

    /* Facing an event whose script id the lookup doesn't know is not an
       error -- it's simply nothing to interact with, so A alone (not a
       directional button) leaves the runner idle with no movement. */
    LGPlayer player2 = {1, 0, 2, 0}; /* already facing down */
    LGGameService svc2;
    LGScriptVM vm2;
    lg_game_service_init(&svc2, &player2, &map, 0, 0);
    LGGameLoop loop2;
    lg_game_loop_init(&loop2, &vm2, &svc2, widths, capture, &seen, lookup_text, 0, 10, 20, 0, 0);
    LGObjectEvent events2[] = {{1, 1, 9}}; /* script_id 9 isn't in lookup_script */
    LGOverworldRunner runner2;
    lg_overworld_runner_init(&runner2, &player2, &svc2, &vm2, &loop2, events2, 1, lookup_script, 0);
    LgInputState press_a = {0, LG_BUTTON_A, 0};
    assert(lg_overworld_runner_step(&runner2, &press_a, false) == LG_RUNNER_STEP_IDLE);
    assert(player2.x == 1 && player2.y == 0);

    /* A runner with no object events at all never requires script_lookup_fn. */
    LGOverworldRunner runner3;
    lg_overworld_runner_init(&runner3, &player2, &svc2, &vm2, &loop2, 0, 0, 0, 0);
    assert(lg_overworld_runner_step(&runner3, &press_a, false) == LG_RUNNER_STEP_IDLE);

    /* Invalid arguments are rejected. */
    assert(lg_overworld_runner_step(0, &input, false) == LG_RUNNER_STEP_ERROR);
    LGOverworldRunner bad;
    lg_overworld_runner_init(&bad, 0, 0, 0, 0, 0, 0, 0, 0);
    assert(lg_overworld_runner_step(&bad, &input, false) == LG_RUNNER_STEP_ERROR);

    return 0;
}
