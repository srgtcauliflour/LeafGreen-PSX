#include "lg/text.h"
/*
 * Platform-neutral text entry point. The first native renderer intentionally
 * accepts ASCII debug strings; LeafGreen character-map decoding will sit above
 * this boundary so game text is not coupled to PS1 GPU code.
 */
void lg_text_init(void) {}
void lg_text_draw(const char *ascii, int x, int y) {
    (void)ascii; (void)x; (void)y;
    /* LGPSX-010: PS1 backend submits glyph quads from generated font texture. */
}
