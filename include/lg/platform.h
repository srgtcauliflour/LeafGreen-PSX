#ifndef LG_PLATFORM_H
#define LG_PLATFORM_H

#include <stdbool.h>
#include <stdint.h>

typedef enum LgButton {
    LG_BUTTON_UP    = 1u << 0,
    LG_BUTTON_DOWN  = 1u << 1,
    LG_BUTTON_LEFT  = 1u << 2,
    LG_BUTTON_RIGHT = 1u << 3,
    LG_BUTTON_A     = 1u << 4,
    LG_BUTTON_B     = 1u << 5,
    LG_BUTTON_START = 1u << 6,
    LG_BUTTON_SELECT= 1u << 7,
    LG_BUTTON_L     = 1u << 8,
    LG_BUTTON_R     = 1u << 9
} LgButton;

typedef struct LgInputState {
    uint16_t held;
    uint16_t pressed;
    uint16_t released;
} LgInputState;

bool lg_platform_init(void);
void lg_platform_begin_frame(void);
void lg_platform_end_frame(void);
void lg_platform_poll_input(LgInputState *state);
uint32_t lg_platform_frame_counter(void);
/* Temporary SDK-font bring-up display; not the LeafGreen text renderer. */
void lg_platform_draw_debug_hud(const LgInputState *input);

#endif
