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
	f32 tex_index;
} Render_Vertex;

// OpenGL Texture structure for storing glyphs
typedef struct{
    Vector2 uv_min, uv_max;
    i32 width, height;
    i32 bearingX, bearingY;
    i32 advance;
} GlyphTexture;

typedef struct {
    u32 texture_id;
    u32 width, height;
    u32 x,y; // Current position in the atlas
    u32 rowHeight; // Height of the current row
} TextureAtlas;

typedef struct {
    u32 vao;
    u32 vbo;
    u32 ibo;
    u32 shader_program;

    Matrix4 projection;
    u32 proj_loc;

    Render_Vertex vertices[MAX_VERTICES];
    u32 vert_count;
    u32 indices_count;

    Color clear_color;

    TextureAtlas atlas;

    u32 textures[8];
    u32 texture_count;

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
void renderGrapheme(Renderer *r, UnicodeChar grapheme, f32 *x, f32 y, f32 scale, Color color);
void renderQuad(Renderer *r, f32 x, f32 y, f32 w, f32 h, Color color);
void rendererText(Renderer *r, const char *str, f32 x, f32 y, Color color);

u32 rendererLoadFont(Renderer *r, const char *path, u32 size_px);
