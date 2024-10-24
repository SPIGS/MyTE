#pragma once
#include <stdlib.h>
#include "util.h"

typedef struct {
    size_t buffer_pos;
    size_t prev_buffer_pos;
    vec2 screen_pos;
    vec2 prev_screen_pos;
    vec2 target_screen_pos;
    f32 pos_anim_time;

    // display coordinates
    size_t disp_column;
    size_t disp_row;

    // Blinking
    bool blinkable;
    bool moved_last_frame;
    f32 blink_time;
    f32 blink_rate;
    f32 alpha;
    f32 target_alpha;

    // Selection
    i32 selection_size;
    vec2 screen_pos_beg_selection;

    // width (for browser)
    f32 size_anim_time;
    f32 width;
    f32 prev_width;
    f32 target_width;
} Cursor;

Cursor cursorInit(vec2 init_screen_pos, f32 blink_rate);
void cursorUpdate(Cursor *c, vec2 adj_cursor_pos, f64 delta_time);

void resetAnimTime(Cursor *c);
void setCursorTargetScreenPos(Cursor *c, vec2 new_target);
void lerpCursorScreenPos(Cursor *c);

void setCursorTargetWidth(Cursor *c, f32 new_target);
void lerpCursorWidth(Cursor *c);