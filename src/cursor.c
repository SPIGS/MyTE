#include "cursor.h"

Cursor cursorInit(vec2 init_screen_pos, f32 blink_rate) {
    return (Cursor) { 
        .buffer_pos = 0, 
        .prev_buffer_pos = 0, 
        .screen_pos = init_screen_pos, 
        .prev_screen_pos = init_screen_pos, 
        .target_screen_pos = init_screen_pos,
        .pos_anim_time = 0.0f,
        .disp_column = 1,
        .disp_row = 1,
        .blinkable = true,
        .moved_last_frame = false,
        .blink_time = 0.0,
        .blink_rate = blink_rate,
        .alpha = 1.0,
        .target_alpha = 0.0,
        .selection_size = 0,
        .width = 3.0f,
        .prev_width = 1.0f,
        .target_width = 1.0f,
        .size_anim_time = 0.0f
    };
}

void cursorUpdate(Cursor *c, vec2 adj_cursor_pos, f64 delta_time) {
    
    setCursorTargetScreenPos(c, adj_cursor_pos);

    // increment the animation timer
    //ed->cursor.anim_time += (f32)delta_time * ed->cursor_speed;
    c->pos_anim_time += (f32)delta_time;
    if (c->pos_anim_time >= 1.0f) {
        c->pos_anim_time = 1.0f;
    }

    c->size_anim_time += (f32)delta_time;
    if (c->size_anim_time >= 1.0f) {
        c->size_anim_time = 1.0f;
    }
    lerpCursorScreenPos(c);
    lerpCursorWidth(c);

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
            c->alpha = ease_out(c->alpha, c->target_alpha, c->blink_time);
        }
    } 
}

void resetAnimTime(Cursor *c) {
    c->pos_anim_time = 0.0f;
    c->size_anim_time = 0.0f;
}

void setCursorTargetScreenPos(Cursor *c, vec2 new_target) {
    c->target_screen_pos = new_target;
    c->prev_screen_pos = c->screen_pos;
}

void setCursorTargetWidth(Cursor *c, f32 new_target) {
    c->target_width = new_target;
    c->prev_width = c->width;
}

void lerpCursorScreenPos(Cursor *c) {
    c->screen_pos = vec2_ease_out(c->prev_screen_pos, c->target_screen_pos, c->pos_anim_time);
}

void lerpCursorWidth(Cursor *c) {
    vec2 prev = vec2_init(c->prev_width, 0);
    vec2 target = vec2_init(c->target_width, 0);
    c->width = vec2_ease_out(prev, target, c->size_anim_time).x;
}