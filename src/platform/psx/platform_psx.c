#include "lg/platform.h"

#include <psxetc.h>
#include <psxgpu.h>
#include <psxpad.h>

static uint32_t s_frame_counter;
static uint16_t s_previous_buttons;
static char s_pad_buffer[2][34];

static uint16_t translate_buttons(uint16_t pad) {
    uint16_t out = 0;
    if (!(pad & PAD_UP))     out |= LG_BUTTON_UP;
    if (!(pad & PAD_DOWN))   out |= LG_BUTTON_DOWN;
    if (!(pad & PAD_LEFT))   out |= LG_BUTTON_LEFT;
    if (!(pad & PAD_RIGHT))  out |= LG_BUTTON_RIGHT;
    if (!(pad & PAD_CROSS))  out |= LG_BUTTON_A;
    if (!(pad & PAD_CIRCLE)) out |= LG_BUTTON_B;
    if (!(pad & PAD_START))  out |= LG_BUTTON_START;
    if (!(pad & PAD_SELECT)) out |= LG_BUTTON_SELECT;
    if (!(pad & PAD_L1))     out |= LG_BUTTON_L;
    if (!(pad & PAD_R1))     out |= LG_BUTTON_R;
    return out;
}

bool lg_platform_init(void) {
    ResetGraph(0);

    /* PSn00bSDK's low-level pad API requires persistent 34-byte receive
       buffers for both controller ports. The BIOS pad driver writes into
       these asynchronously after StartPAD(). */
    EnterCriticalSection();
    InitPAD(s_pad_buffer[0], sizeof(s_pad_buffer[0]),
            s_pad_buffer[1], sizeof(s_pad_buffer[1]));
    StartPAD();
    ChangeClearPAD(0);
    ExitCriticalSection();

    s_frame_counter = 0;
    s_previous_buttons = 0;
    return true;
}

void lg_platform_begin_frame(void) {
}

void lg_platform_end_frame(void) {
    VSync(0);
    ++s_frame_counter;
}

void lg_platform_poll_input(LgInputState *state) {
    const uint16_t held = translate_buttons(PadRead(0));
    state->held = held;
    state->pressed = held & (uint16_t)~s_previous_buttons;
    state->released = s_previous_buttons & (uint16_t)~held;
    s_previous_buttons = held;
}

uint32_t lg_platform_frame_counter(void) {
    return s_frame_counter;
}
