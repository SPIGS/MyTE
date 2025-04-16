#include "cursor.h"
#include "putils/pmath.h"

#define BLINK_RATE 0.5

Cursor cursorNew(void) {
    return (Cursor) {
        .buffer_idx = 0,
        .prev_buffer_idx = 0,

        .screen_pos = vec2(0.0, 0.0),
        .prev_screen_pos = vec2(0.0, 0.0),
        .target_screen_pos = vec2(0.0, 0.0),
        .pos_anim_time = 0.0f,
        .disp_col = 1,
        .disp_row = 1,
        .prev_disp_row = 1,
        .moved_last_frame = true,
        .blinkable = true,
        .blink_time = 1.0,
        .blink_rate = BLINK_RATE,
        .alpha = 1.0,
        .target_alpha = 0.0,
    };
}

void cursorUpdate(Cursor *c, f64 delta_time) {
    //
    // Blink cursor
    // if (c->blinkable) {
    //     c->blink_time += (f32)delta_time;
    //     if (c->blink_time >= c->blink_rate) {
    //         c->blink_time = 0.0;
    //         c->target_alpha = c->target_alpha == 1.0 ? 0.0 : 1.0;
    //     }
    //     if (c->moved_last_frame) {
    //         c->alpha = 1.0;
    //         c->target_alpha = 1.0;
    //         c->blink_time = 0.0;
    //     } else {
    //         c->alpha = easeOutF(c->alpha, c->target_alpha, c->blink_time);
    //     }
    // }
}
