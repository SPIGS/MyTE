#pragma once
#include "putils/color.h"
#include "putils/pmath.h"
#include "putils/phashmap.h"
#include "putils/unicode.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#define MAX_QUADS 1024
#define MAX_VERTICES MAX_QUADS * 4
#define MAX_INDICES MAX_VERTICES * 6
#define INITIAL_SCREEN_WIDTH 1280
#define INITIAL_SCREEN_HEIGHT 720

typedef struct {
	Vector2 pos;
	Color color;
	Vector2 uv;
	float tex_index;
} Render_Vertex;

// OpenGL Texture structure for storing glyphs
typedef struct{
    u32 textureID;
    int width, height;
    int bearingX, bearingY;
    int advance;
} GlyphTexture;

typedef struct {
    u32 vao;
    u32 vbo;
    u32 shader_program;

    Matrix4 projection;
    u32 proj_loc;

    Render_Vertex vertices[MAX_VERTICES];
    u32 vert_count;
    u32 indices_count;

    Color clear_color;

    // grapheme to gl texture mapping
    hashmap *glyphs;

    FT_Library ft;
    FT_Face face;

    // Screen size info
    f32 screen_width;
    f32 screen_height;
} Renderer;

Renderer *rendererNew(Color clear_color);
void rendererDestroy(Renderer* r);
void rendererBegin(Renderer* r);
void rendererEnd(Renderer* r);
void rendererResizeWindow (Renderer* r, i32 width, i32 height);
void renderGrapheme(Renderer *r, UnicodeChar grapheme, float *x, float y, float scale, Color color);
void renderQuad(Renderer *r, float x, float y, float w, float h, Color color);

u32 rendererLoadFont(Renderer *r, const char *path, u32 size_px);
