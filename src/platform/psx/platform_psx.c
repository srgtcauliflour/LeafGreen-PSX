#include "lg/platform.h"

#include <psxapi.h>
#include <psxetc.h>
#include <psxgpu.h>
#include <psxpad.h>

static DISPENV s_disp[2];
static DRAWENV s_draw[2];
static int s_buffer;

static void init_video(void) {
    SetDefDispEnv(&s_disp[0], 0, 0, 320, 240);
    SetDefDrawEnv(&s_draw[0], 0, 240, 320, 240);
    SetDefDispEnv(&s_disp[1], 0, 240, 320, 240);
    SetDefDrawEnv(&s_draw[1], 0, 0, 320, 240);

    setRGB0(&s_draw[0], 20, 28, 40);
    setRGB0(&s_draw[1], 20, 28, 40);
    s_draw[0].isbg = 1;
    s_draw[1].isbg = 1;
    s_draw[0].dtd = 1;
    s_draw[1].dtd = 1;

    PutDispEnv(&s_disp[0]);
    PutDrawEnv(&s_draw[0]);
    SetDispMask(1);

    /* Temporary SDK font is only a bring-up HUD. LeafGreen's real generated
       font renderer remains LGPSX-010 and must not depend on FntPrint(). */
    FntLoad(960, 0);
    FntOpen(16, 16, 288, 208, 0, 256);
}

static uint32_t s_frame_counter;
static uint16_t s_previous_buttons;
static uint8_t s_pad_buffer[2][34];

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

    /* BIOS pad polling writes a PADTYPE response into each persistent buffer. */
    EnterCriticalSection();
    InitPAD(s_pad_buffer[0], sizeof(s_pad_buffer[0]),
            s_pad_buffer[1], sizeof(s_pad_buffer[1]));
    StartPAD();
    ChangeClearPAD(0);
    ExitCriticalSection();

    init_video();
    s_buffer = 0;
    s_frame_counter = 0;
    s_previous_buttons = 0;
    return true;
}

void lg_platform_begin_frame(void) {
}

void lg_platform_end_frame(void) {
    DrawSync(0);
    VSync(0);
    ++s_frame_counter;
    s_buffer ^= 1;
    PutDispEnv(&s_disp[s_buffer]);
    PutDrawEnv(&s_draw[s_buffer]);
    SetDispMask(1);
}

void lg_platform_poll_input(LgInputState *state) {
    const PADTYPE *pad = (const PADTYPE *)s_pad_buffer[0];
    uint16_t held = 0;

    if (pad->stat == 0 && pad->type != PAD_ID_NONE) {
        held = translate_buttons(pad->btn);
    }

    state->held = held;
    state->pressed = held & (uint16_t)~s_previous_buttons;
    state->released = s_previous_buttons & (uint16_t)~held;
    s_previous_buttons = held;
}

uint32_t lg_platform_frame_counter(void) {
    return s_frame_counter;
}

void lg_platform_draw_debug_hud(const LgInputState *input) {
    FntPrint(-1, "LeafGreen-PSX native PS1 bring-up\n\n");
    FntPrint(-1, "frame: %lu\n", (unsigned long)lg_platform_frame_counter());
    FntPrint(-1, "held:     %04x\n", input->held);
    FntPrint(-1, "pressed:  %04x\n", input->pressed);
    FntPrint(-1, "released: %04x\n\n", input->released);
    FntPrint(-1, "D-pad = movement\nX = A   O = B\nSTART/SELECT, L1/R1 mapped\n");
    FntFlush(-1);
}
