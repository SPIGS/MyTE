#include "renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "freetype/freetype.h"
#include "freetype/fttypes.h"
#include "putils/defines.h"
#include "putils/log.h"
#include "putils/phashmap.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "putils/unicode.h"
#include "putils/file.h"
#include <grapheme.h>

#define FONT_PATH "./IosevkaTermNerdFontMono-Regular.ttf"
#define FALLBACK_FONT_PATH_1 "/usr/share/fonts/noto-cjk/NotoSansCJK-Regular.ttc"
#define FALLBACK_FONT_PATH_2 "/usr/share/fonts/noto/NotoSansRunic-Regular.ttf"
#define FONT_SIZE 24

#define GL_CALL(x) glClearError();\
    x;\
    glLogCall()

static void glClearError(void) {
    while(glGetError() != GL_NO_ERROR);
}

static void glLogCall(void) {
    GLenum error = 0;
    while((error = glGetError())) {
	LOG_ERROR("OpenGL error %#08x", error);
	printf("OpenGL error %#08x", error);
    }
}

static GLuint compileShader(GLenum type, const char *src) {
    GLuint shader = glCreateShader(type);
    i32 src_len = strlen(src);
    GL_CALL(glShaderSource(shader, 1, &src, &src_len));
    GL_CALL(glCompileShader(shader));
    int success;
    GL_CALL(glGetShaderiv(shader, GL_COMPILE_STATUS, &success));
    if (!success) {
        char log[512];
        GL_CALL(glGetShaderInfoLog(shader, 512, NULL, log));
	LOG_ERROR("Shader Compilation Error: %s", log);
	// FIX: handle error
	exit(1);
    }
    return shader;
}

static void setupShaders(Renderer *r) {
    r->shader_program = glCreateProgram();

    char *vert_code = readFile("./shaders/glyph_vert.glsl");
    char *frag_code = readFile("./shaders/glyph_frag.glsl");

    GLuint vert_shader_pgm = compileShader(GL_VERTEX_SHADER, vert_code);
    GLuint frag_shader_pgm = compileShader(GL_FRAGMENT_SHADER, frag_code);

    GL_CALL(glAttachShader(r->shader_program, vert_shader_pgm));
    GL_CALL(glAttachShader(r->shader_program, frag_shader_pgm));
    GL_CALL(glLinkProgram(r->shader_program));

    int success;
    GL_CALL(glGetProgramiv(r->shader_program, GL_LINK_STATUS, &success));
    if (!success) {
	char log[512];
	glGetProgramInfoLog(r->shader_program, 512, NULL, log);
	LOG_ERROR("Shader Linking Error: %s", log);
	// FIX: handle error
	exit(1);
    }

    glDeleteShader(vert_shader_pgm);
    glDeleteShader(frag_shader_pgm);
}

static void setupBuffers(Renderer *r) {

    glGenVertexArrays(1, &r->vao);
    glBindVertexArray(r->vao);

    glGenBuffers(1, &r->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Render_Vertex), NULL, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void*) offsetof(Render_Vertex, pos));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void*) offsetof(Render_Vertex, color));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void*) offsetof(Render_Vertex, uv));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Render_Vertex), (void*) offsetof(Render_Vertex, tex_index));
    glEnableVertexAttribArray(3);

    /* Index Buffer stuff */
    u32 indices[MAX_INDICES];
    u32 offset = 0;
    for (size_t i = 0; i < MAX_INDICES; i += 6) {
	indices[i + 0] = 0 + offset;
	indices[i + 1] = 1 + offset;
	indices[i + 2] = 2 + offset;

	indices[i + 3] = 2 + offset;
	indices[i + 4] = 3 + offset;
	indices[i + 5] = 0 + offset;

	offset += 4;
    }

    glGenBuffers(1, &r->ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, r->ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // FIX: handle errors
}

//~ Helper stuff
u32 _cached_white = 4096;
u32 rendererGetWhiteTexture(void) {
    if (_cached_white == 4096) {
	u32 tex;
	u8 image[4] = { 255, 255, 255, 255 };
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	_cached_white = tex;
    }
    return _cached_white;
}

Renderer *rendererNew(Color clear_color) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Renderer *r = (Renderer *)malloc(sizeof(Renderer));

    r->clear_color = clear_color;
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

    setupBuffers(r);
    setupShaders(r);
    r->projection = orthoProj(0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, 0, -0.01, 1.0);
    r->screen_width = INITIAL_SCREEN_WIDTH;
    r->screen_height = INITIAL_SCREEN_HEIGHT;
    GL_CALL(glUseProgram(r->shader_program));
    r->proj_loc = glGetUniformLocation(r->shader_program, "u_proj");
    GL_CALL(glUniformMatrix4fv(r->proj_loc, 1, GL_FALSE, r->projection.a));
    u32 tex_loc = glGetUniformLocation(r->shader_program, "u_tex");
    i32 textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
    glUniform1iv(tex_loc, 8, textures);

    // Set up the white texture which will be used to render solid-color quads
    // this should set the texture id to 0.
    u32 tex = rendererGetWhiteTexture();
    UNUSED(tex);

    r->atlas.width = 1024;
    r->atlas.height = 1024;
    r->atlas.x = 0;
    r->atlas.y = 0;
    r->atlas.rowHeight = 0;

    GL_CALL(glGenTextures(1, &r->atlas.texture_id));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, r->atlas.texture_id));

    // for zero initializing the buffer
    u8* blank_buffer = (u8*)malloc(sizeof(u8) * (size_t)r->atlas.width * (size_t)r->atlas.height);

    GL_CALL(
	glTexImage2D(
	    GL_TEXTURE_2D, 
	    0, 
	    GL_RED, 
	    r->atlas.width, 
	    r->atlas.height, 
	    0, 
	    GL_RED, 
	    GL_UNSIGNED_BYTE, 
	    blank_buffer
	)
    );

    free(blank_buffer);

    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    r->glyphs = hashmapNew();

    // Setup fonts
    if (FT_Init_FreeType(&r->ft)) {
	LOG_ERROR("Could not init FreeType", "");
	return false;
    }

    // Load fonts
    r->font_collection = fontCollectionNew(4);
    fontCollectionAddFace(&r->ft, r->font_collection, FONT_PATH, 0, FONT_SIZE);
    fontCollectionAddFace(&r->ft, r->font_collection, FALLBACK_FONT_PATH_1, 0, FONT_SIZE);
    fontCollectionAddFace(&r->ft, r->font_collection, FALLBACK_FONT_PATH_2, 0, FONT_SIZE);

    return r;
}

void rendererDestroy(Renderer* r) {
    hashmapFree(r->glyphs);
    glDeleteBuffers(1, &r->vbo);
    glDeleteVertexArrays(1, &r->vao);
    glDeleteProgram(r->shader_program);
    fontcollectionDestroy(r->font_collection);
    FT_Done_FreeType(r->ft);
    free(r);
}

void rendererBegin(Renderer* r) {
    glClear(GL_COLOR_BUFFER_BIT);
    r->vert_count = 0;
    r->texture_count = 0;
    r->indices_count = 0;
}

void rendererEnd(Renderer* r) {
    for (u32 i = 0; i < r->texture_count; i++) {
	glActiveTexture(GL_TEXTURE0 + i);
	glBindTexture(GL_TEXTURE_2D, r->textures[i]);
    }

    glUseProgram(r->shader_program);
    glBindVertexArray(r->vao);
    glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, r->vert_count * sizeof(Render_Vertex), r->vertices);
    glDrawElements(GL_TRIANGLES, r->indices_count, GL_UNSIGNED_INT, NULL);
}

static GlyphTexture addGlyphToAtlas(Renderer *r, FT_Bitmap *bitmap, FT_GlyphSlot slot) {
    GlyphTexture glyph;

    // Check if we need to move to a new row
    if (r->atlas.x + bitmap->width >= r->atlas.width) {
	r->atlas.x = 0;
	r->atlas.y += r->atlas.rowHeight;
	r->atlas.rowHeight = 0;
    }

    // Check if we need to resize the atlas
    if (r->atlas.y + bitmap->rows >= r->atlas.height) {
	// Save old atlas ID
	GLuint old_texture = r->atlas.texture_id;
	int old_width = r->atlas.width;
	int old_height = r->atlas.height;
	
	// Double the size of the atlas
	r->atlas.width *= 2;
	r->atlas.height *= 2;
	
	// Create new texture
	GL_CALL(glGenTextures(1, &r->atlas.texture_id));
	GL_CALL(glBindTexture(GL_TEXTURE_2D, r->atlas.texture_id));
	GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, r->atlas.width, r->atlas.height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));
	
	// Set texture parameters
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	
	// Copy old texture data to new texture
	GLubyte *old_data = (GLubyte *)malloc(old_width * old_height);
	GL_CALL(glBindTexture(GL_TEXTURE_2D, old_texture));
	GL_CALL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, old_data));
	
	GL_CALL(glBindTexture(GL_TEXTURE_2D, r->atlas.texture_id));
	GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, old_width, old_height, GL_RED, GL_UNSIGNED_BYTE, old_data));
	
	// Clean up
	free(old_data);
	GL_CALL(glDeleteTextures(1, &old_texture));
	
	// Update all existing glyph UV coordinates
	// Note: This would require iterating through the hashmap and updating all UVs
	// This is left as an exercise or could be implemented as a separate function
    }

    // Add the glyph bitmap to the atlas
    GL_CALL(glBindTexture(GL_TEXTURE_2D, r->atlas.texture_id));
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, r->atlas.x, r->atlas.y, 
			   bitmap->width, bitmap->rows, 
			   GL_RED, GL_UNSIGNED_BYTE, bitmap->buffer));

    // Calculate UV coordinates
    glyph.uv_min.x = (f32)r->atlas.x / r->atlas.width;
    glyph.uv_max.y = (f32)r->atlas.y / r->atlas.height;
    glyph.uv_max.x = (f32)(r->atlas.x + bitmap->width) / r->atlas.width;
    glyph.uv_min.y = (f32)(r->atlas.y + bitmap->rows) / r->atlas.height;

    // Store glyph metrics
    glyph.width = bitmap->width;
    glyph.height = bitmap->rows;
    glyph.bearingX = slot->bitmap_left;
    glyph.bearingY = slot->bitmap_top;
    glyph.advance = slot->advance.x >> 6;

    // Update atlas current position
    r->atlas.x += bitmap->width + 1;  // Add 1 pixel padding
    r->atlas.rowHeight = MAX(r->atlas.rowHeight, bitmap->rows);

    return glyph;
}

static void pushQuad (Renderer* r, Vector2 a, Vector2 b, Vector2 c, Vector2 d,
					Color a_color, Color b_color, Color c_color, Color d_color,
					Vector2 a_uv, Vector2 b_uv, Vector2 c_uv, Vector2 d_uv,
					u32 texture) {

    /*CULLING - This if statement causes textures to glitch out */
    // if (((b.x) < 0 || a.x > r->screen_width || a.y > r->screen_height)){
    // 	return;	
    // }

    // 1248 is just an invalid value since this is an unsigned number, -1 doesnt work
    u32 tex_index = 1248;
    for (u32 i = 0; i < r->texture_count; i++) {
	if (r->textures[i] == texture) {
	    tex_index = i;
	    break;
	}
    }

    // r->texture_count < 8 confirms we don't write more than the available
    // texture slots
    if (tex_index == 1248 && r->texture_count < 8) {
	r->textures[r->texture_count] = texture;
	tex_index = r->texture_count;
	r->texture_count += 1;
    }

    // Flush the batch if it is full. We don't like segfaults on this channel.
    if (r->vert_count == MAX_VERTICES || tex_index == 1248) {
	rendererEnd(r);
	r->vert_count = 0;
	r->indices_count = 0;
	r->texture_count = 0;
    }

    // Insert info for each vertex and increment the count
    r->vertices[r->vert_count].pos = a;
    r->vertices[r->vert_count].color = a_color;
    r->vertices[r->vert_count].uv = a_uv;
    r->vertices[r->vert_count].tex_index = tex_index;
    r->vert_count++;

    r->vertices[r->vert_count].pos = b;
    r->vertices[r->vert_count].color = b_color;
    r->vertices[r->vert_count].uv = b_uv;
    r->vertices[r->vert_count].tex_index = tex_index;
    r->vert_count++;

    r->vertices[r->vert_count].pos = c;
    r->vertices[r->vert_count].color = c_color;
    r->vertices[r->vert_count].uv = c_uv;
    r->vertices[r->vert_count].tex_index = tex_index;
    r->vert_count++;

    r->vertices[r->vert_count].pos = d;
    r->vertices[r->vert_count].color = d_color;
    r->vertices[r->vert_count].uv = d_uv;
    r->vertices[r->vert_count].tex_index = tex_index;
    r->vert_count++;

    r->indices_count += 6;
}

void renderGrapheme(Renderer *r, UnicodeChar grapheme, f32 *x, f32 y, f32 scale, Color color) {
    char *unpacked_grapheme = unpackUTF8(grapheme);
    GlyphTexture *glyph = (GlyphTexture *)hashmapGet(r->glyphs, unpacked_grapheme);

    if (glyph == NULL) {
	FT_Face current_face = r->font_collection->faces[0].face;

	// Initialize HarfBuzz buffer
	hb_buffer_t* hb_buffer = hb_buffer_create();
	u32 glyph_count;
	hb_glyph_info_t* glyph_info = shapeText(hb_buffer, current_face, unpacked_grapheme, &glyph_count);

	if (glyph_info->codepoint == 0) {
	    for (size_t i = 0; i< r->font_collection->count; i++) {
		if (r->font_collection->faces[i].face == current_face)
			continue;

		
		hb_buffer_destroy(hb_buffer);
		hb_buffer = hb_buffer_create();
		glyph_info = shapeText(hb_buffer, r->font_collection->faces[i].face, unpacked_grapheme, &glyph_count);
		if (glyph_info->codepoint != 0) {
		    current_face = r->font_collection->faces[i].face;
		    break;
		}
	    }
	}

	if (glyph_count == 0) {
	    hb_buffer_destroy(hb_buffer);
	    free(unpacked_grapheme);
	    return;
	} else {
	    i32 codepoint = glyph_info[0].codepoint;
	    string codepoint_key = stringNew("");
	    codepoint_key = stringFmt(codepoint_key, "%s", unpacked_grapheme);

	    if (FT_Load_Glyph(current_face, codepoint, FT_LOAD_RENDER))
		printf("Loading glyph failed\n");

	    GlyphTexture *new_glyph = (GlyphTexture *)malloc(sizeof(GlyphTexture));
	    *new_glyph = addGlyphToAtlas(r, &current_face->glyph->bitmap, current_face->glyph);
	    codepoint_key = (char *)hashmapPush(r->glyphs, codepoint_key, new_glyph);
	    glyph = new_glyph;
	    hb_buffer_destroy(hb_buffer);
	}
    }
    free(unpacked_grapheme);

    // Calculate quad position
    f32 xpos = *x + glyph->bearingX * scale;
    f32 ypos = y - (glyph->height - glyph->bearingY) * scale;
    f32 w = glyph->width * scale;
    f32 h = glyph->height * scale;

    pushQuad (
	r,
	vec2(xpos, ypos),
	vec2(xpos + w, ypos),
	vec2(xpos + w, ypos + h),
	vec2(xpos, ypos + h),
	color, color, color, color,
	glyph->uv_min,
	vec2(glyph->uv_max.x, glyph->uv_min.y),
	glyph->uv_max,
	vec2(glyph->uv_min.x, glyph->uv_max.y),
	r->atlas.texture_id
    );

    // Advance to the next character position
    *x += glyph->advance * scale; // Advance is in 1/64 pixels, so shift right by 6
}

void renderQuad(Renderer *r, f32 x, f32 y, f32 w, f32 h, Color color) {
    u32 texture = rendererGetWhiteTexture();
    pushQuad (
	r,
	vec2(x, y),
	vec2(x + w, y),
	vec2(x+ w, y+ h),
	vec2(x, y+ h),
	color, color, color, color,
	vec2(0.0, 0.0),
	vec2(1.0, 0.0),
	vec2(1.0, 1.0),
	vec2(0.0, 1.0),
	texture
    );
}

void rendererText(Renderer *r, const char *str, f32 x, f32 y, Color color) {
    size_t grapheme_size, offset = 0;
    for (offset = 0; str[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(str + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(str + offset, grapheme_size);
	renderGrapheme(r, grapheme, &x, y, 1.0, color);
    }
}

void rendererResizeWindow (Renderer* r, i32 width, i32 height) {
    // Adjust the viewport for opengl
    glViewport(0, 0, width, height);

    // adjust the projection for the renderer
    r->projection = orthoProj(0, (f32)width, (f32)height, 0, -0.01, 1.0);
    r->screen_width = (f32)width;
    r->screen_height = (f32)height;

    u32 proj_loc = glGetUniformLocation(r->shader_program, "u_proj");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, r->projection.a);
}
