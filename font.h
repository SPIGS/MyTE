#pragma once
#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    f32 ax; // advance.x
    f32 ay; // advance.y

    f32 bw; // bitmap.width;
    f32 bh; // bitmap.rows;

    f32 bl; // bitmap_left;
    f32 bt; // bitmap_top;

    f32 tx; // x offset of glyph in texture coordinates
} GlyphMetric;

#define GLYPH_METRICS_CAPACITY 128

typedef struct {
    FT_UInt atlas_width;
    FT_UInt atlas_height;
    GLuint glyphs_texture;
    GlyphMetric metrics[GLYPH_METRICS_CAPACITY];
} GlyphAtlas;

void glyph_atlas_init(GlyphAtlas *atlas, FT_Face face);