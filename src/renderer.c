#include "renderer.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "freetype/freetype.h"
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "putils/defines.h"
#include "putils/log.h"
#include "putils/phashmap.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "putils/unicode.h"
#include "putils/file.h"

#define FONT_PATH "./iosevka-firamono.ttf"
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
    }

    glDeleteShader(vert_shader_pgm);
    glDeleteShader(frag_shader_pgm);
}

static void setupBuffers(Renderer *r) {
    GL_CALL(glGenVertexArrays(1, &r->vao));
    GL_CALL(glGenBuffers(1, &r->vbo));

    GL_CALL(glBindVertexArray(r->vao));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r->vbo));
    GL_CALL(glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW));
    GL_CALL(glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0));
    GL_CALL(glEnableVertexAttribArray(0));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CALL(glBindVertexArray(0));
    
    // FIX: handle errors
}

static bool loadFont(Renderer *r) {
    if (FT_Init_FreeType(&r->ft)) {
	LOG_ERROR("Could not init FreeType", "");
	return false;
    }

    if (FT_New_Face(r->ft, FONT_PATH, 0, &r->face)) {
	LOG_ERROR("Could not load font", "");
	return false;
    }

    FT_Set_Pixel_Sizes(r->face, 0, FONT_SIZE);
    return true;
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
    r->proj_loc = glGetUniformLocation(r->shader_program, "projection");
    GL_CALL(glUniformMatrix4fv(r->proj_loc, 1, GL_FALSE, r->projection.a));

    // Set up the white texture which will be used to render solid-color quads
    // this should set the texture id to 0.
    u32 tex = rendererGetWhiteTexture();
    UNUSED(tex);

    r->glyphs = hashmapNew();

    // Load font
    if (!loadFont(r)) return NULL;

    return r;
}

void rendererDestroy(Renderer* r) {
    hashmapFree(r->glyphs);
    free(r);
}
void rendererBegin(Renderer* r) {
    glClear(GL_COLOR_BUFFER_BIT);
}

void rendererEnd(Renderer* r) {
    // TODO:
}



// HarfBuzz: Shape text using FreeType font
static hb_glyph_info_t* shapeText(hb_buffer_t* hb_buffer, FT_Face face, const char* text, unsigned int* glyph_count) {
    // Create a HarfBuzz font object from the FreeType face
    hb_font_t* hb_font = hb_ft_font_create(face, NULL);

    // Add UTF-8 text to the buffer
    hb_buffer_add_utf8(hb_buffer, text, -1, 0, -1);

    // Guess script/language/direction properties
    hb_buffer_guess_segment_properties(hb_buffer);

    // Shape the text (perform glyph substitution and positioning)
    hb_shape(hb_font, hb_buffer, NULL, 0);
    
    // Get shaped glyph information
    hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(hb_buffer, glyph_count);

    // Cleanup HarfBuzz font
    hb_font_destroy(hb_font);

    return glyph_info;
}
//
// OpenGL: Create texture from FreeType glyph
GlyphTexture createGlyphTexture(FT_Bitmap *bitmap, FT_GlyphSlot slot) {
    GlyphTexture glyph;
    GL_CALL(glGenTextures(1, &glyph.textureID));
    GL_CALL(glBindTexture(GL_TEXTURE_2D, glyph.textureID));
    
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, bitmap->width, bitmap->rows, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap->buffer));
    glyph.width = bitmap->width;
    glyph.height = bitmap->rows;
    glyph.bearingX = slot->bitmap_left;
    glyph.bearingY = slot->bitmap_top;
    glyph.advance = slot->advance.x >> 6; // Convert 1/64th pixel to integer pixels
    return glyph;
}

void renderGrapheme(Renderer *r, UnicodeChar grapheme, float *x, float y, float scale, Color color) {
    GL_CALL(glUseProgram(r->shader_program));
    // Pass the text color to the shader
    GL_CALL(glUniform3f(glGetUniformLocation(r->shader_program, "textColor"), color.r, color.g, color.b));

    GL_CALL(glBindVertexArray(r->vao));
    
    // Initialize HarfBuzz buffer
    hb_buffer_t* hb_buffer = hb_buffer_create();
    unsigned int glyph_count;
    char *unpacked_grapheme = unpackUTF8(grapheme);
    hb_glyph_info_t* glyph_info = shapeText(hb_buffer, r->face, unpacked_grapheme, &glyph_count);
    free(unpacked_grapheme);

    if (glyph_count) {
        int codepoint = glyph_info[0].codepoint;
	string codepoint_key = stringNew("");
	codepoint_key = stringFmt(codepoint_key, "%d", codepoint);
    

	if (hashmapGet(r->glyphs, codepoint_key) == NULL) {
            if (FT_Load_Glyph(r->face, codepoint, FT_LOAD_RENDER))
                printf("Loading glyph failed\n");

	    GlyphTexture *new_glyph = (GlyphTexture *)malloc(sizeof(GlyphTexture));
	    *new_glyph = createGlyphTexture(&r->face->glyph->bitmap, r->face->glyph);
	    codepoint_key = (char *)hashmapPush(r->glyphs, codepoint_key, new_glyph);

        }

        //GlyphTexture glyph = r->glyphs[codepoint];
	GlyphTexture *glyph = (GlyphTexture *)hashmapGet(r->glyphs, codepoint_key);
	//stringFree(codepoint_key);

        // Calculate quad position
        float xpos = *x + glyph->bearingX * scale;
        float ypos = y - (glyph->height - glyph->bearingY) * scale;
        float w = glyph->width * scale;
        float h = glyph->height * scale;

        // Vertex positions for the quad (x, y, tex_x, tex_y)
        float vertices[6][4] = {
            { xpos,     ypos + h, 0.0f, 0.0f }, // Top-left
            { xpos,     ypos,     0.0f, 1.0f }, // Bottom-left
            { xpos + w, ypos,     1.0f, 1.0f }, // Bottom-right

            { xpos,     ypos + h, 0.0f, 0.0f }, // Top-left
            { xpos + w, ypos,     1.0f, 1.0f }, // Bottom-right
            { xpos + w, ypos + h, 1.0f, 0.0f }  // Top-right
        };

       // Bind texture and update VBO with new vertices
        GL_CALL(glBindTexture(GL_TEXTURE_2D, glyph->textureID));
        GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r->vbo));
        GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));

        // Draw the quad
        GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

        // Advance to the next character position
        *x += glyph->advance * scale; // Advance is in 1/64 pixels, so shift right by 6
    }

    hb_buffer_destroy(hb_buffer);
    GL_CALL(glBindVertexArray(0));
}

void renderQuad(Renderer *r, float x, float y, float w, float h, Color color) {
    u32 texture = rendererGetWhiteTexture();
    GL_CALL(glUseProgram(r->shader_program));
    // Pass the text color to the shader
    GL_CALL(glUniform3f(glGetUniformLocation(r->shader_program, "textColor"), color.r, color.g, color.b));

    GL_CALL(glBindVertexArray(r->vao));
    // Vertex positions for the quad (x, y, tex_x, tex_y)
    float vertices[6][4] = {
	{ x,     y + h, 0.0f, 0.0f }, // Top-left
	{ x,     y,     0.0f, 1.0f }, // Bottom-left
	{ x + w, y,     1.0f, 1.0f }, // Bottom-right

	{ x,     y + h, 0.0f, 0.0f }, // Top-left
	{ x + w, y,     1.0f, 1.0f }, // Bottom-right
	{ x + w, y + h, 1.0f, 0.0f }  // Top-right
    };

   // Bind texture and update VBO with new vertices
    GL_CALL(glBindTexture(GL_TEXTURE_2D, texture));
    GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, r->vbo));
    GL_CALL(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));

    // Draw the quad
    GL_CALL(glDrawArrays(GL_TRIANGLES, 0, 6));

}

void rendererResizeWindow (Renderer* r, i32 width, i32 height) {
    // Adjust the viewport for opengl
    glViewport(0, 0, width, height);
    
    r->screen_width = (f32)width;
    r->screen_height = (f32)height;
}
