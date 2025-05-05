#include "cursor.h"
#include "putils/pmath.h"

#define BLINK_RATE 0.5

Cursor cursorNew(Vector2 pos) {
    return (Cursor) {
        .buffer_idx = 0,
        .prev_buffer_idx = 0,
        .screen_pos = vec2(pos.x, pos.y),
        .prev_screen_pos = vec2(pos.x, pos.y),
        .target_screen_pos = vec2(pos.x, pos.y),
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

void resetAnimTime(Cursor *c) {
    c->pos_anim_time = 0.0f;
    //c->size_anim_time = 0.0f;
}

void setCursorTargetScreenPos(Cursor *c, Vector2 new_target) {
    c->target_screen_pos = new_target;
    c->prev_screen_pos = c->screen_pos;
}

// void setCursorTargetWidth(Cursor *c, f32 new_target) {
//     c->target_width = new_target;
//     c->prev_width = c->width;
// }

void lerpCursorScreenPos(Cursor *c) {
    c->screen_pos = vec2EaseOut(c->prev_screen_pos, c->target_screen_pos, c->pos_anim_time);
}

void cursorUpdate(Cursor *c, Vector2 adj_cursor_pos, f64 delta_time) {
    setCursorTargetScreenPos(c, adj_cursor_pos);

    // increment the animation timer
    //ed->cursor.anim_time += (f32)delta_time * ed->cursor_speed;
    c->pos_anim_time += (f32)delta_time;
    if (c->pos_anim_time >= 1.0f) {
        c->pos_anim_time = 1.0f;
    }

    // c->size_anim_time += (f32)delta_time;
    // if (c->size_anim_time >= 1.0f) {
    //     c->size_anim_time = 1.0f;
    // }
    lerpCursorScreenPos(c);
    //lerpCursorWidth(c);

    // Blink cursor
    if (c->blinkable) {
        c->blink_time += (f32)delta_time;
        if (c->blink_time >= c->blink_rate) {
            c->blink_time = 0.0;
            c->target_alpha = c->target_alpha == 1.0 ? 0.0 : 1.0;
        }
        if (c->moved_last_frame) {
            c->alpha = 1.0;
            c->target_alpha = 1.0;
            c->blink_time = 0.0;
            c->moved_last_frame = false;
        } else {
            c->alpha = easeOutF(c->alpha, c->target_alpha, c->blink_time);
        }
    } 
}
