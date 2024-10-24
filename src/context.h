#pragma once
#include "util.h"

// Stuff that will need passed around a lot
typedef struct {
    u32 font_id;
    f32 glyph_adv;
    f32 line_height;
    f32 descender;
    f32 screen_width;
    f32 screen_height;
} AppContext;