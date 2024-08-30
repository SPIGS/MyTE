#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include "renderer.h"

void Render_Init(Renderer* r, Color clear_color) {
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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
	
	// @hardcoded @resize
	r->projection = mat4_ortho(0, INITIAL_SCREEN_WIDTH, INITIAL_SCREEN_HEIGHT, 0, -0.01, 1.0);
	
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
}

void Render_Free(Renderer* r) {
	glDeleteBuffers(1, &r->vbo);
	glDeleteVertexArrays(1, &r->vao);
	
	glDeleteProgram(r->shader);
}

void Render_Begin_Frame(Renderer* r) {
	glClear(GL_COLOR_BUFFER_BIT);
	
	r->triangle_count = 0;
	r->texture_count = 0;
}

void Render_End_Frame(Renderer* r) {
	
	for (u32 i = 0; i < r->texture_count; i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, r->textures[i]);
	}
	
	glUseProgram(r->shader);
	glBindVertexArray(r->vao);
	glBindBuffer(GL_ARRAY_BUFFER, r->vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, r->triangle_count * 3 * sizeof(Render_Vertex), r->triangle_data);
	
	glDrawArrays(GL_TRIANGLES, 0, r->triangle_count * 3);
}

void Render_Resize_Window (Renderer* r, i32 width, i32 height) {
	// Adjust the viewport for opengl
	glViewport(0,0, width, height);

	// adjust the projection for the renderer
	r->projection = mat4_ortho(0, (f32)width, (f32)height, 0, -0.01, 1.0);

	// pass the updated projection to the shader
	u32 proj_loc = glGetUniformLocation(r->shader, "u_proj");
    glUniformMatrix4fv(proj_loc, 1, GL_FALSE, r->projection.a);
}

void Render_Push_Triangle(Renderer* r,
						  vec2 a, vec2 b, vec2 c,
						  Color a_color, Color b_color, Color c_color,
						  vec2 a_uv, vec2 b_uv, vec2 c_uv,
						  u32 texture) {
	
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
	if (r->triangle_count == MAX_TRIANGLES || tex_index == 1248) {
		Render_End_Frame(r);
		Render_Begin_Frame(r);
	}
	
	r->triangle_data[r->triangle_count * 3 + 0].pos = a;
	r->triangle_data[r->triangle_count * 3 + 0].color = a_color;
	r->triangle_data[r->triangle_count * 3 + 1].pos = b;
	r->triangle_data[r->triangle_count * 3 + 1].color = b_color;
	r->triangle_data[r->triangle_count * 3 + 2].pos = c;
	r->triangle_data[r->triangle_count * 3 + 2].color = c_color;
	
	r->triangle_data[r->triangle_count * 3 + 0].uv = a_uv;
	r->triangle_data[r->triangle_count * 3 + 0].tex_index = tex_index;
	r->triangle_data[r->triangle_count * 3 + 1].uv = b_uv;
	r->triangle_data[r->triangle_count * 3 + 1].tex_index = tex_index;
	r->triangle_data[r->triangle_count * 3 + 2].uv = c_uv;
	r->triangle_data[r->triangle_count * 3 + 2].tex_index = tex_index;
	
	r->triangle_count++;
}

//~ Helper stuff
u32 _cached_white = 4096;

u32 Render_GetWhiteTexture() {
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

void Render_Push_Quad_C(Renderer* r, rect quad, Color color) {
	u32 texture = Render_GetWhiteTexture();
	Render_Push_Triangle(r,
						 vec2_init(quad.x, quad.y),
						 vec2_init(quad.x + quad.w, quad.y),
						 vec2_init(quad.x + quad.w, quad.y + quad.h),
						 color, color, color,
						 vec2_init(0, 0), vec2_init(1, 0), vec2_init(1, 1), texture);
	Render_Push_Triangle(r,
						 vec2_init(quad.x, quad.y),
						 vec2_init(quad.x + quad.w, quad.y + quad.h),
						 vec2_init(quad.x, quad.y + quad.h),
						 color, color, color,
						 vec2_init(0, 0), vec2_init(1, 0), vec2_init(1, 1), texture);
}

void Render_Push_Quad_T(Renderer* r, rect quad, Color tint, u32 texture) {
    // Skip rect_uv_cull and directly use full texture coordinates
    vec2 uv_min = vec2_init(0, 1);  // Bottom-left
    vec2 uv_max = vec2_init(1, 0);  // Top-right

    Render_Push_Triangle(r,
                         vec2_init(quad.x, quad.y), 
                         vec2_init(quad.x + quad.w, quad.y),
                         vec2_init(quad.x + quad.w, quad.y + quad.h),
                         tint, tint, tint,
                         uv_min, vec2_init(uv_max.x, uv_min.y), uv_max, texture);
    Render_Push_Triangle(r,
                         vec2_init(quad.x, quad.y), 
                         vec2_init(quad.x + quad.w, quad.y + quad.h), 
                         vec2_init(quad.x, quad.y + quad.h),
                         tint, tint, tint,
                         uv_min, uv_max, vec2_init(uv_min.x, uv_max.y), texture);
}

void Render_Push_Char(Renderer* r, u32 font_id, i8* text, vec2 *pos, Color tint) {
	GlyphAtlas atlas = r->font_atlases[font_id];
	vec2 init_pos = vec2_init(pos->x, pos->y);

	for (size_t i = 0; i < strlen(text); i++) {
		
		// If the character is a newline, move down and back over to the left.
		if (text[i] == '\n') {
			pos->x = init_pos.x;
			pos->y -= atlas.atlas_height;
			continue;
		}

		size_t glyph_index = text[i];

		if (glyph_index >= GLYPH_METRICS_CAPACITY) {
            glyph_index = '?';
    	}

		GlyphMetric metric = atlas.metrics[glyph_index];
		f32 x2 = pos->x + metric.bl;
		f32 y2 = pos->y - (metric.bh - metric.bt);
		f32 w  = metric.bw;
		f32 h  = metric.bh;

		f32 u0 = metric.tx;
		f32 v0 = 0.0f;
		f32 u1 = (metric.tx) + (w / (f32)atlas.atlas_width);
		f32 v1 = h / (f32)atlas.atlas_height;

		vec2 uv_min = vec2_init(u0, v1);
		vec2 uv_max = vec2_init(u1, v0);

		pos->x += metric.ax;

		Render_Push_Triangle(r,
							vec2_init(x2, y2), 
							vec2_init(x2 + w, y2),
							vec2_init(x2 + w, y2 + h),
							tint, tint, tint,
							uv_min, vec2_init(uv_max.x, uv_min.y), uv_max, atlas.glyphs_texture);
		Render_Push_Triangle(r,
							vec2_init(x2, y2), 
							vec2_init(x2 + w, y2 + h), 
							vec2_init(x2, y2 + h),
							tint, tint, tint,
							uv_min, uv_max, vec2_init(uv_min.x, uv_max.y), atlas.glyphs_texture);
	}
}

void Render_Push_Buffer(Renderer* r, u32 font_id, char *data, size_t cursor_pos, vec2 *pos, Color tint) {
	GlyphAtlas atlas = r->font_atlases[font_id];
	vec2 init_pos = vec2_init(pos->x, pos->y);

	
	size_t len_data = strlen(data);
	for (size_t i = 0; i < len_data; i++) {

		if (i == cursor_pos) {
			rect cursor_quad = rect_init(pos->x, pos->y, 2, atlas.atlas_height);
			Render_Push_Quad_C(r, cursor_quad, tint);
		}

		// If the character is a newline, move down and back over to the left.
		if (data[i] == '\n') {
			pos->x = init_pos.x;
			pos->y -= atlas.atlas_height;
			continue;
		}

		size_t glyph_index = data[i];

		if (glyph_index >= GLYPH_METRICS_CAPACITY) {
            glyph_index = '?';
    	}

		GlyphMetric metric = atlas.metrics[glyph_index];
		f32 x2 = pos->x + metric.bl;
		f32 y2 = pos->y - (metric.bh - metric.bt);
		f32 w  = metric.bw;
		f32 h  = metric.bh;

		f32 u0 = metric.tx;
		f32 v0 = 0.0f;
		f32 u1 = (metric.tx) + (w / (f32)atlas.atlas_width);
		f32 v1 = h / (f32)atlas.atlas_height;

		vec2 uv_min = vec2_init(u0, v1);
		vec2 uv_max = vec2_init(u1, v0);

		pos->x += metric.ax;

		Render_Push_Triangle(r,
							vec2_init(x2, y2), 
							vec2_init(x2 + w, y2),
							vec2_init(x2 + w, y2 + h),
							tint, tint, tint,
							uv_min, vec2_init(uv_max.x, uv_min.y), uv_max, atlas.glyphs_texture);
		Render_Push_Triangle(r,
							vec2_init(x2, y2), 
							vec2_init(x2 + w, y2 + h), 
							vec2_init(x2, y2 + h),
							tint, tint, tint,
							uv_min, uv_max, vec2_init(uv_min.x, uv_max.y), atlas.glyphs_texture);
	}
	if (cursor_pos == len_data) {
		rect cursor_quad = rect_init(pos->x, pos->y, 2, atlas.atlas_height);
		Render_Push_Quad_C(r, cursor_quad, tint);
	}
}

u32 Render_Load_Font(Renderer *r, char *path, u32 size_px) {
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
	glyph_atlas_init(&atlas, face);

	//Store atlas
	u32 font_id = r->font_atlas_count;
	r->font_atlases[r->font_atlas_count] = atlas;
	r->font_atlas_count++;
	return font_id;
}
