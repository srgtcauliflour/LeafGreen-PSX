#include "lg/platform.h"

#include <psxetc.h>
#include <psxgpu.h>

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

int main(void) {
    LgInputState input;

    if (!lg_platform_init()) {
        return 1;
    }

    init_video();

    for (;;) {
        lg_platform_begin_frame();
        lg_platform_poll_input(&input);

        FntPrint(-1, "LeafGreen-PSX native PS1 bring-up\n\n");
        FntPrint(-1, "frame: %lu\n", (unsigned long)lg_platform_frame_counter());
        FntPrint(-1, "held:     %04x\n", input.held);
        FntPrint(-1, "pressed:  %04x\n", input.pressed);
        FntPrint(-1, "released: %04x\n\n", input.released);
        FntPrint(-1, "D-pad = movement\nX = A   O = B\nSTART/SELECT, L1/R1 mapped\n");
        FntFlush(-1);

        DrawSync(0);
        lg_platform_end_frame();

        s_buffer ^= 1;
        PutDispEnv(&s_disp[s_buffer]);
        PutDrawEnv(&s_draw[s_buffer]);
        SetDispMask(1);
    }
}
