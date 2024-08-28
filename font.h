#pragma once
#include <GL/glew.h>

#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
    float ax; // advance.x
    float ay; // advance.y

    float bw; // bitmap.width;
    float bh; // bitmap.rows;

    float bl; // bitmap_left;
    float bt; // bitmap_top;

    float tx; // x offset of glyph in texture coordinates
} Glyph_Metric;

#define GLYPH_METRICS_CAPACITY 128

typedef struct {
    FT_UInt atlas_width;
    FT_UInt atlas_height;
    GLuint glyphs_texture;
    Glyph_Metric metrics[GLYPH_METRICS_CAPACITY];
} Free_Glyph_Atlas;

void free_glyph_atlas_init(Free_Glyph_Atlas *atlas, FT_Face face);