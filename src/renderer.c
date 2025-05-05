#include "renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "buffer.h"
#include "editor.h"
#include "freetype/freetype.h"
#include "putils/color.h"
#include "putils/defines.h"
#include "putils/log.h"
#include "putils/phashmap.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "putils/unicode.h"
#include "putils/file.h"
#include <grapheme.h>
#include <stddef.h>

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

    r->glyph_width = r->font_collection->faces[0].face->max_advance_width >> 6;
    r->line_height = r->font_collection->faces[0].line_height;

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
	cacheGrapheme(r->font_collection, r->glyphs, &r->atlas,unpacked_grapheme);
    }
    glyph = (GlyphTexture *)hashmapGet(r->glyphs, unpacked_grapheme);
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

void rendererText(Renderer *r, const char *str, f32 *x, f32 y, Color color) {
    size_t grapheme_size, offset = 0;
    for (offset = 0; str[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(str + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(str + offset, grapheme_size);
	renderGrapheme(r, grapheme, x, y, 1.0, color);
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

static void renderCursor(Renderer *r, Cursor cursor) {
    Color cursor_color = COLOR_WHITE;
    cursor_color.a = cursor.alpha;
    renderQuad(r, cursor.screen_pos.x, cursor.screen_pos.y, 3, r->line_height, cursor_color);
}

void renderEditor(Renderer *r, Editor *ed, f64 delta_time) {
    // Render the background
    Rect frame = ed->frame;
    //renderQuad(r, frame.x, frame.y, frame.w, frame.h, COLOR_GRAY);

    size_t cur_line = ed->cursor.disp_row;

    // Draw the text
    f32 text_base_x = frame.x + ed->gutter.size.x + r->glyph_width;
    Vector2 text_pos = vec2(text_base_x, frame.y + frame.h - r->line_height);
    text_pos = vec2Add(text_pos, ed->scroll_pos);

    UnicodeChar *graphemes = getBufferString(ed->buf);
    size_t buf_len = getBufLength(ed->buf);

    // Draw the cursor
    renderCursor(r, ed->cursor);

    for (size_t i = 0; i < buf_len; i++) {
        if (graphemes[i] == 10) { // newline
            text_pos.y -= r->line_height;
            text_pos.x = text_base_x + ed->scroll_pos.x;
            continue;
        } else if (graphemes[i] == '\t') {
	    for (size_t k = 0; k < TAB_WIDTH; k++) {
		renderGrapheme(r, 32, &text_pos.x, text_pos.y, 1.0, COLOR_WHITE);
	    }
	    continue;
	}
        renderGrapheme(r, graphemes[i], &text_pos.x, text_pos.y, 1.0, COLOR_WHITE);
    }
    free(graphemes);

    // Draw the gutter
    renderQuad(r, frame.x, 0, ed->gutter.size.x, r->screen_height, COLOR_BLACK);
    for (size_t i = 1; i <= ed->line_count; i++) {
	string num = stringNew("");
	num = stringFmt(num, "%*d", ed->gutter.padding, i);
	rendererText(r, num, &ed->gutter.txt_pos.x, ed->gutter.txt_pos.y, cur_line == i ? COLOR_WHITE : COLOR_SILVER);
	ed->gutter.txt_pos.x = frame.x;
	ed->gutter.txt_pos.y -= r->line_height;
	stringFree(num);
    }

    // Draw the gutter divider
    renderQuad(r, frame.x + ed->gutter.size.x, 0, 1, r->screen_height, COLOR_SILVER);
}

