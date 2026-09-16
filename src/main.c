#include "lg/platform.h"

int main(void) {
    LgInputState input;
    if (!lg_platform_init()) {
        return 1;
    }
    lg_platform_text_init();
    for (;;) {
        lg_platform_begin_frame();
        lg_platform_poll_input(&input);
        lg_platform_draw_debug_hud(&input);
        lg_platform_draw_text_demo();
        lg_platform_end_frame();
    }
}
