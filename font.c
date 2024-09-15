#include "util.h"
#include "font.h"

void glyphAtlasInit(GlyphAtlas *atlas, FT_Face face, f32 *glyph_adv) {
    	//Creating a texture atlas
	FT_GlyphSlot g = face->glyph;
	u32 w = 0;
	u32 h = 0;

	for (u32 i = 32; i < 128; i++) {
		if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
			fprintf(stderr, "Loading character %c failed!\n", i);
    		continue;
		}

		w += g->bitmap.width;
		*glyph_adv = MAX(*glyph_adv, g->bitmap.width);
		h = MAX(h, g->bitmap.rows);
	}

    atlas->atlas_width = w;
    atlas->atlas_height = h;

	// for zero initializing the buffer
	char* blank_buffer = (char*)malloc(sizeof(char) * (size_t)w * (size_t)h);

	// Make a blank texture the size needed for the atlas
	glActiveTexture(GL_TEXTURE0);
	glGenTextures(1, &(atlas->glyphs_texture));
	glBindTexture(GL_TEXTURE_2D, atlas->glyphs_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		GL_RED,  // Single channel format for storing the alpha
		w,
		h,
		0,
		GL_RED,  // Using the red channel for alpha
		GL_UNSIGNED_BYTE,
		blank_buffer
	);

	free(blank_buffer);

	// Add the glyphs to the atlas
    int x = 0;
    for (int i = 32; i < 128; ++i) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            fprintf(stderr, "ERROR: could not load glyph of a character with code %d\n", i);
            continue;
        }

        atlas->metrics[i].ax = face->glyph->advance.x >> 6;
        atlas->metrics[i].ay = face->glyph->advance.y >> 6;
        atlas->metrics[i].bw = face->glyph->bitmap.width;
        atlas->metrics[i].bh = face->glyph->bitmap.rows;
        atlas->metrics[i].bl = face->glyph->bitmap_left;
        atlas->metrics[i].bt = face->glyph->bitmap_top;
        atlas->metrics[i].tx = (float) x / (float) atlas->atlas_width;

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexSubImage2D(
            GL_TEXTURE_2D,
            0,
            x,
            0,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
		);
        x += face->glyph->bitmap.width;
    }
}