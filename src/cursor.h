#pragma once
#include <stddef.h>
#include "putils/defines.h"
#include "putils/pmath.h"

typedef struct {
    size_t buffer_idx;
    size_t prev_buffer_idx;

    // Screen Coordinates
    Vector2 screen_pos;
    Vector2 prev_screen_pos;
    Vector2 target_screen_pos;
    f32 pos_anim_time;

    // Animation State
    bool blinkable;
    bool moved_last_frame;
    f32 blink_time;
    f32 blink_rate;
    f32 alpha;
    f32 target_alpha;

    // Cardinal Display coordinates
    size_t disp_col;
    size_t disp_row;
    size_t prev_disp_row;
} Cursor;

Cursor cursorNew(Vector2 pos);
void cursorUpdate(Cursor *c, Vector2 adj_cursor_pos, f64 delta_time);


