#pragma once
#include "util.h"
#include "font.h"
#include "editor.h"

#define INITIAL_SCREEN_WIDTH 1080
#define INITIAL_SCREEN_HEIGHT 720


#define MAX_QUADS 1024
#define MAX_VERTICES MAX_QUADS * 4
#define MAX_INDICES MAX_VERTICES * 6

typedef struct {
	vec2 pos;
	Color color;
	vec2 uv;
	float tex_index;
} Render_Vertex;

typedef struct {
	// The required OpenGL objects
	u32 vao;
	u32 vbo;
	u32 ibo;
	u32 shader;
	
	mat4 projection;
	
	// Tightly packed triangle data. This is a cpu side mirror of the buffer
	Render_Vertex vertices[MAX_VERTICES];
	u32 vert_count;
	u32 indices_count;
	
	// Texture stuff
	u32 textures[8];
	u32 texture_count;

	// Misc
	Color clear_color;

	// Fonts
	FT_Library ft;
	GlyphAtlas font_atlases[8];
	u32 font_atlas_count;
	f32 glyph_adv;
	f32 descender;

	// Screen size info
	f32 screen_width;
	f32 screen_height;

} Renderer;

void rendererInit(Renderer* r, Color clear_color);
void rendererDestroy(Renderer* r);
void rendererBegin(Renderer* r);
void rendererEnd(Renderer* r);
void rendererResizeWindow (Renderer* r, i32 width, i32 height);

void renderTriangle(Renderer* r,
						  vec2 a, vec2 b, vec2 c,
						  Color a_color, Color b_color, Color c_color,
						  vec2 a_uv, vec2 b_uv, vec2 c_uv,
						  u32 texture);

u32  rendererGetWhiteTexture();
void renderQuad(Renderer* r, rect quad, Color color);
void renderTexturedQuad(Renderer* r, rect quad, Color tint, u32 texture);
void renderChar(Renderer* r, char character, vec2 *pos, GlyphAtlas *atlas, Color tint);
void renderText(Renderer* r, char *text, vec2 *pos, GlyphAtlas *atlas, Color tint);
void renderEditor(Renderer* r, u32 font_id, Editor *e, f64 delta_time, ColorTheme theme);

u32 rendererLoadFont(Renderer *r, const char *path, u32 size_px);