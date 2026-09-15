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

    PutDispEnv(&s_disp[0]);
    PutDrawEnv(&s_draw[0]);
    SetDispMask(1);
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

        /* LGPSX-001..006 smoke test: PS1 runtime, video, input, timing,
           and the portable platform boundary. Rendering LeafGreen assets
           starts in the next milestone. */

        lg_platform_end_frame();
        s_buffer ^= 1;
        PutDispEnv(&s_disp[s_buffer]);
        PutDrawEnv(&s_draw[s_buffer]);
    }
}
