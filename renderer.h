#pragma once
#include "util.h"
#include "font.h"

#define INITIAL_SCREEN_WIDTH 1080
#define INITIAL_SCREEN_HEIGHT 720


#define MAX_TRIANGLES 2048
#define MAX_VERTICES MAX_TRIANGLES * 3

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
	u32 shader;
	
	mat4 projection;
	
	// Tightly packed triangle data. This is a cpu side mirror of the buffer
	Render_Vertex triangle_data[MAX_VERTICES];
	u32 triangle_count;
	
	// Texture stuff
	u32 textures[8];
	u32 texture_count;

	// Misc
	Color clear_color;

	// Fonts
	FT_Library ft;
	GlyphAtlas font_atlases[8];
	u32 font_atlas_count;

} Renderer;

void Render_Init(Renderer* r, Color clear_color);
void Render_Free(Renderer* r);
void Render_Begin_Frame(Renderer* r);
void Render_End_Frame(Renderer* r);
void Render_Resize_Window (Renderer* r, i32 width, i32 height);
void Render_Push_Triangle(Renderer* r,
						  vec2 a, vec2 b, vec2 c,
						  Color a_color, Color b_color, Color c_color,
						  vec2 a_uv, vec2 b_uv, vec2 c_uv,
						  u32 texture);

// Helpers and Extensions
u32  Render_GetWhiteTexture();
u32 Render_Load_Font(Renderer *r, char *path, u32 size_px);

void Render_Push_Quad_C(Renderer* r, rect quad, Color color);
void Render_Push_Quad_T(Renderer* r, rect quad, Color tint, u32 texture);
void Render_Push_Char(Renderer* r, u32 font_id, i8* text, vec2 *pos, Color tint);
void Render_Push_Buffer(Renderer* r, u32 font_id, char *data, size_t cursor_pos, vec2 *pos, Color tint);