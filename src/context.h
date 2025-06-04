#pragma once
#include "putils/phashmap.h"
#include "font.h"

typedef enum {
    FOCUS_EDITOR,
    FOCUS_COMMAND_MODAL,
    FOCUS_SAVE_MODAL,
} Focus;

// Stuff that will need to be passed around a lot
typedef struct {
    hashmap *glyph_cache;
    FontCollection *font_collection;
    TextureAtlas *atlas;
    f32 screen_width;
    f32 screen_height;
    f32 glyph_width;
} AppContext;
