#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "renderer.h"
#include "lexer.h"
#include "browser.h"

#define GL_CALL(x) glClearError();\
	x;\
	glLogCall()

static void glClearError() {
	while(glGetError() != GL_NO_ERROR);
}

static void glLogCall() {
	GLenum error = 0;
	while((error = glGetError())) {
		LOG_ERROR("OpenGL error %#08x", error);
	}
}

void rendererInit(Renderer* r, Color clear_color) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	/* Vertex Buffer Stuff */
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
	
	r->projection = mat4_ortho(0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, 0, -0.01, 1.0);
	r->screen_width = INITIAL_SCREEN_WIDTH;
	r->screen_height = INITIAL_SCREEN_HEIGHT;
	
	r->shader = glCreateProgram();
    u32 vert_module = glCreateShader(GL_VERTEX_SHADER);
    u32 frag_module = glCreateShader(GL_FRAGMENT_SHADER);
	

    // Temperarily inlining shader code until I implement file loading/hot reloading
	i8 *vert_code =
        "#version 330 core\n"
        "layout (location = 0) in vec2  a_pos;"
        "layout (location = 1) in vec4  a_color;"
        "layout (location = 2) in vec2  a_uv;"
        "layout (location = 3) in float a_texindex;"
        "out vec4  v_color;"
        "out vec2  v_uv;"
        "out float v_texindex;"
        "uniform mat4 u_proj;"
        "void main() {"
            "gl_Position = u_proj * vec4(a_pos, 0.0, 1.0);"
            "v_texindex = a_texindex;"
            "v_uv = a_uv;"
            "v_color = a_color;"
        "}";

    GLint vert_src_length = (GLint)strlen(vert_code);
	glShaderSource(vert_module, 1, (const GLchar *const *)&vert_code, &vert_src_length);
	
	i8 *frag_code = 
    "#version 330 core\n"
    "in vec4  v_color;"
    "in vec2  v_uv;"
    "in float v_texindex;"
    "layout (location=0) out vec4 f_color;"
    "uniform sampler2D u_tex[8];"
    "void main() {"
	"vec4 sampled = texture(u_tex[int(v_texindex)], v_uv);"
    "f_color = vec4(v_color.rgb, v_color.a * sampled.r);"
    "}";

    GLint frag_src_length = (GLint)strlen(frag_code);
	glShaderSource(frag_module, 1, (const GLchar *const *)&frag_code, &frag_src_length);
	
	glCompileShader(vert_module);
	glCompileShader(frag_module);

    GLchar* vert_info;
    GLchar* frag_info;
    GLchar* prog_info;
	
	i32 error;
	glGetShaderiv(vert_module, GL_COMPILE_STATUS, &error);
	if (error == GL_FALSE) {
		printf("Vertex Shader Compilation failed!");
		i32 length = 0;
		glGetShaderiv(vert_module, GL_INFO_LOG_LENGTH, &length);
		
		vert_info = (GLchar *)malloc(length * sizeof(GLchar));
		glGetShaderInfoLog(vert_module, length * sizeof(GLchar), NULL, vert_info);
		printf("%s", vert_info);
	}
	
	glGetShaderiv(frag_module, GL_COMPILE_STATUS, &error);
	if (error == GL_FALSE) {
		printf("Fragment Shader Compilation failed!");
		i32 length = 0;
		glGetShaderiv(frag_module, GL_INFO_LOG_LENGTH, &length);
		
		frag_info = (GLchar *)malloc(length * sizeof(GLchar));
		glGetShaderInfoLog(frag_module, length * sizeof(GLchar), NULL, frag_info);
		printf("%s", frag_info);
	}
	
	glAttachShader(r->shader, vert_module);
	glAttachShader(r->shader, frag_module);
	
	glLinkProgram(r->shader);
	glGetProgramiv(r->shader, GL_LINK_STATUS, &error);
	if (error == GL_FALSE) {
		printf("Program Linking Failed:\n");
		i32 length = 0;
		glGetProgramiv(r->shader, GL_INFO_LOG_LENGTH, &length);
		
		prog_info = (GLchar *)malloc(length * sizeof(GLchar));
		glGetProgramInfoLog(r->shader, length, NULL, prog_info);
		printf("%s", prog_info);
	}
	
	glDetachShader(r->shader, vert_module);
	glDetachShader(r->shader, frag_module);
	glDeleteShader(vert_module);
	glDeleteShader(frag_module);
	
	glUseProgram(r->shader);
	u32 proj_loc = glGetUniformLocation(r->shader, "u_proj");
	// the member a here is the underlying array. mat4 is defined as so:
	//    typedef struct mat4 { f32 a[4*4]; } mat4;
	// Arrays implicitly become pointers, so this works nicely
	glUniformMatrix4fv(proj_loc, 1, GL_FALSE, r->projection.a);
	
	
	u32 tex_loc = glGetUniformLocation(r->shader, "u_tex");
	i32 textures[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	glUniform1iv(tex_loc, 8, textures);

	
    r->clear_color = clear_color;
    glClearColor(clear_color.r, clear_color.g, clear_color.b, clear_color.a);

	// Initaliaze Freetype
	if (FT_Init_FreeType(&r->ft)) {
		printf("ERROR: Couldn't louad freetype library\n");
		exit(1);
	}
	r->font_atlas_count = 0;
	r->glyph_adv = 0;
}

void rendererDestroy(Renderer* r) {
	glDeleteBuffers(1, &r->vbo);
	glDeleteVertexArrays(1, &r->vao);
	glDeleteProgram(r->shader);
	FT_Done_FreeType(r->ft);
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
	
	glUseProgram(r->shader);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, r->vert_count * sizeof(Render_Vertex), r->vertices);
	glDrawElements(GL_TRIANGLES, r->indices_count, GL_UNSIGNED_INT, NULL);
}

void rendererResizeWindow (Renderer* r, i32 width, i32 height) {
	// Adjust the viewport for opengl
	glViewport(0,0, width, height);

	// adjust the projection for the renderer
	r->projection = mat4_ortho(0, (f32)width, (f32)height, 0, -0.01, 1.0);
	r->screen_width = (f32)width;
	r->screen_height = (f32)height;

	// pass the updated projection to the shader
	u32 proj_loc = glGetUniformLocation(r->shader, "u_proj");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, r->projection.a);
}

static void pushQuad (Renderer* r, vec2 a, vec2 b, vec2 c, vec2 d,
					Color a_color, Color b_color, Color c_color, Color d_color,
					vec2 a_uv, vec2 b_uv, vec2 c_uv, vec2 d_uv,
					u32 texture) {
	
	/* 	CULLING
		for some reason adding this comparison ((c.y) < 0 ) causes some characters
		to glitch out. Not sure whats up with that. */
	if (((b.x) < 0 || a.x > r->screen_width || a.y > r->screen_height)){
		return;	
	}

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

void renderQuad(Renderer* r, rect quad, Color color) {
	u32 texture = rendererGetWhiteTexture();
	vec2 uv_min = vec2_init(0, 1);  // Bottom-left
    vec2 uv_max = vec2_init(1, 0);  // Top-right

	pushQuad (
		r,
		vec2_init(quad.x, quad.y), 
		vec2_init(quad.x + quad.w, quad.y), 
		vec2_init(quad.x + quad.w, quad.y + quad.h), 
		vec2_init(quad.x, quad.y + quad.h),
		color, color, color, color,
		uv_min, 
		vec2_init(uv_max.x, uv_min.y), 
		uv_max, 
		vec2_init(uv_min.x, uv_max.y),
		texture
	);
}

void renderTexturedQuad(Renderer* r, rect quad, Color tint, u32 texture) {
    vec2 uv_min = vec2_init(0, 1);  // Bottom-left
    vec2 uv_max = vec2_init(1, 0);  // Top-right

	pushQuad (
		r,
		vec2_init(quad.x, quad.y), 
		vec2_init(quad.x + quad.w, quad.y), 
		vec2_init(quad.x + quad.w, quad.y + quad.h), 
		vec2_init(quad.x, quad.y + quad.h),
		tint, tint, tint, tint,
		uv_min, 
		vec2_init(uv_max.x, uv_min.y), 
		uv_max, 
		vec2_init(uv_min.x, uv_max.y),
		texture
	);
}

void renderChar(Renderer* r, char character, vec2 *pos, GlyphAtlas *atlas, Color tint) {
	// If the character is a newline, don't do anything.
	if (character == '\n') {
		return;
	}

	size_t glyph_index = character;
	if (glyph_index >= GLYPH_METRICS_CAPACITY) {
		glyph_index = '?';
	}

	GlyphMetric metric = atlas->metrics[glyph_index];
	f32 x2 = pos->x + metric.bl;
	f32 y2 = pos->y - (metric.bh - metric.bt);
	f32 w  = metric.bw;
	f32 h  = metric.bh;

	f32 u0 = metric.tx;
	f32 v0 = 0.0f;
	f32 u1 = (metric.tx) + (w / (f32)atlas->atlas_width);
	f32 v1 = h / (f32)atlas->atlas_height;

	vec2 uv_min = vec2_init(u0, v1);
	vec2 uv_max = vec2_init(u1, v0);

	// advance the position by the width of the character
	pos->x += metric.ax;

	// cull
	pushQuad (
		r,
		vec2_init(x2, y2), 
		vec2_init(x2 + w, y2), 
		vec2_init(x2 + w, y2 + h), 
		vec2_init(x2, y2 + h),
		tint, tint, tint, tint,
		uv_min, 
		vec2_init(uv_max.x, uv_min.y), 
		uv_max, 
		vec2_init(uv_min.x, uv_max.y),
		atlas->glyphs_texture
	);
	
}

void renderText(Renderer* r, char *data, vec2 *pos, GlyphAtlas *atlas, Color tint) {
	vec2 init_pos = vec2_init(pos->x, pos->y);

	
	size_t len_data = strlen(data);
	for (size_t i = 0; i < len_data; i++) {
		// If the character is a newline, move down and back over to the left.
		if (data[i] == '\n') {
			pos->x = init_pos.x;
			pos->y -= atlas->atlas_height;
			continue;
		}

		// Render the character
		renderChar(r, data[i], pos, atlas, tint);
	}
}

//~ Helper stuff
u32 _cached_white = 4096;

u32 rendererGetWhiteTexture() {
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

void renderEditor(Renderer* r, u32 font_id, Editor *e, f64 delta_time, ColorTheme theme) {
	UNUSED(delta_time);

	f32 PADDING = 30.0f;
	renderQuad(r, e->frame, theme.background);
	GlyphAtlas atlas = r->font_atlases[font_id];

	
	if (e->mode != EDITOR_MODE_OPEN) {
		vec2 init_pos = vec2_init(((r->glyph_adv * 3) + PADDING), e->text_pos.y - e->line_height);
		vec2 adj_text_pos = vec2_add(init_pos, e->scroll_pos);
		
		// Render line highlight
		renderQuad(r, rect_init(e->frame.x, e->cursor.screen_pos.y, e->frame.w, atlas.atlas_height), theme.current_line);

		// Render the tokens
		for (size_t i = 0; i < e->lexer.token_count; i++) {
			Token curToken = e->lexer.tokens[i];
			switch(curToken.type) {
				case TOKEN_COMMENT_SINGLE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.single_line_comment);
				break;

				case TOKEN_COMMENT_MULTI:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.multiline_comment);
				break;

				case TOKEN_STRING_LITERAL_DOUBLE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.double_quote_string);
				break;

				case TOKEN_STRING_LITERAL_SINGLE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.single_quote_string);
				break;

				case TOKEN_ESCAPE_SEQUENCE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.number);
				break;

				case TOKEN_NUMBER:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.number);
				break;

				case TOKEN_SYMBOL:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.symbol);
				break;

				case TOKEN_NEW_LINE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.foreground);
					adj_text_pos.x = init_pos.x;
				break;

				case TOKEN_PREPROCESSOR_DIRECTIVE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.keyword);
				break;

				case TOKEN_KEYWORD:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.keyword);
				break;

				case TOKEN_SECONDARY_KEYWORD:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.secondary_keyword);
				break;

				case TOKEN_BUILT_IN_TYPE:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.built_in_type);
				break;

				case TOKEN_FUNCTION_NAME:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.function_name);
				break;

				case TOKEN_TYPE_NAME:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.type);
				break;
				
				default:
					renderText(r, e->lexer.tokens[i].text, &adj_text_pos, &atlas, theme.foreground);
				break;
			}
		}
		
		// Render cursor
		rect cursor_quad = rect_init(e->cursor.screen_pos.x, e->cursor.screen_pos.y, 3, atlas.atlas_height);
		renderQuad(r, cursor_quad, theme.foreground);
	} else {
		vec2 init_pos = vec2_init(((r->glyph_adv * 3) + PADDING), e->text_pos.y - e->line_height);

		// Render selection highlight
		rect selection_highlight = rect_init(e->browser.sel_screen_pos.x, e->browser.sel_screen_pos.y, e->browser.sel_size.x, e->browser.sel_size.y);
		renderQuad(r, selection_highlight, theme.user_selection);

		for (size_t i = 0; i < e->browser.num_paths; i++){
			if (e->browser.items[i].is_dir) {
				char *dir_with_slash = (char *)malloc(strlen(e->browser.items[i].name_ext) + 2);
				strcpy(dir_with_slash, e->browser.items[i].name_ext);
				strcat(dir_with_slash, "/");
				renderText(r, dir_with_slash, &init_pos, &atlas, theme.foreground);
				free(dir_with_slash);
			} else {
				renderText(r, e->browser.items[i].name_ext, &init_pos, &atlas, theme.foreground);
			}
			renderText(r, "\n", &init_pos, &atlas, theme.foreground);
			init_pos.x = ((r->glyph_adv * 3) + PADDING);
		}
	}
	

	// gutter
	PADDING = 10.0f;
	vec2 gutter_pos = vec2_init(0, e->frame.y + e->frame.h - e->line_height);
	renderQuad(r, rect_init((r->glyph_adv * 3) + PADDING, 0, 1, r->screen_height), theme.gutter_foreground);
	if (e->mode != EDITOR_MODE_OPEN) {
		gutter_pos = vec2_add(gutter_pos, e->scroll_pos);
		i32 cur_line = e->cursor.disp_row;
		for (i32 i = 1; i <= (i32)e->line_count; ++i) {
			char num[11];
			sprintf(num, "%3d", i);
			renderText(r, num, &gutter_pos, &atlas, cur_line == i ? theme.user_selection : theme.gutter_foreground);
			gutter_pos.x = 0;
			gutter_pos.y -= e->line_height;
		}
	} else {
		gutter_pos = vec2_add(gutter_pos, e->browser.scroll_pos);
		for (size_t i = 0; i <= e->browser.num_paths; i++) {
			char num[4];
			sprintf(num, "%3s", "~");
			renderText(r, num, &gutter_pos, &atlas, theme.gutter_foreground);
			gutter_pos.x = 0;
			gutter_pos.y -= e->line_height;
		}
	}

	// Status line
	renderQuad(r, rect_init(0, 0, r->screen_width, e->line_height), theme.gutter_foreground);
	
	char mode_text[7];
	Color mode_color;
	switch (e->mode)
	{
	case EDITOR_MODE_NORMAL:
		strcpy(mode_text, "NORMAL");
		mode_color = color_from_hex(0x2277FFFF);
		break;

	case EDITOR_MODE_OPEN:
		strcpy(mode_text, "OPEN");
		mode_color = color_from_hex(0xFAC748FF);
		break;

	case EDITOR_MODE_SAVE:
		strcpy(mode_text, "SAVE");
		mode_color = color_from_hex(0xD30C7BFF);
		break;
	
	default:
		strcpy(mode_text, "NORMAL");
		mode_color = color_from_hex(0x2277FFFF);
		break;
	}

	renderQuad(r, rect_init((r->glyph_adv * 3) + PADDING, 0, (strlen(mode_text) * r->glyph_adv) + PADDING, e->line_height), mode_color);
	vec2 mode_text_pos = vec2_init((r->glyph_adv * 3) + (PADDING * 1.5), (PADDING / 2));
	renderText(r, mode_text, &mode_text_pos, &atlas, COLOR_WHITE);
	
	char col_row_disp[24];
	sprintf(col_row_disp, "%lu,%lu", e->cursor.disp_row, e->cursor.disp_column);
	vec2 col_row_disp_pos = vec2_init(r->screen_width - (strlen(col_row_disp) * r->glyph_adv) - PADDING, 5);
	renderText(r, col_row_disp, &col_row_disp_pos, &atlas, COLOR_WHITE);

}

u32 rendererLoadFont(Renderer *r, const char *path, u32 size_px) {
	// Load a face
	FT_Face face;
	if(FT_New_Face(r->ft, path, 0, &face)) {
		fprintf(stderr, "Could not open font\n");
		exit(1);
	}

	// Set the size of the font in pixels
	FT_Set_Pixel_Sizes(face, 0, size_px);

	//Make the atlas
	GlyphAtlas atlas;
	glyphAtlasInit(&atlas, face, &r->glyph_adv, &r->descender);

	//Store atlas
	u32 font_id = r->font_atlas_count;
	r->font_atlases[r->font_atlas_count] = atlas;
	r->font_atlas_count++;
	return font_id;
}
