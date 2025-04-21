#include "font.h"
#include "freetype/freetype.h"
#include "putils/log.h"
#include "putils/pmath.h"
#include "putils/pstring.h"
#include "putils/unicode.h"
#include <GL/glew.h>
#include <grapheme.h>

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

FontCollection *fontCollectionNew(size_t initial_capacity) {
    FontCollection *collection = (FontCollection *)malloc(sizeof(FontCollection));
    if (!collection) return NULL;

    collection->faces = (FontFace *)malloc(sizeof(FontFace) * initial_capacity);
    if (!collection->faces) return NULL;

    collection->count = 0;
    collection->capacity = initial_capacity;
    return collection;
}

void fontcollectionDestroy(FontCollection *collection) {
    for (size_t i = 0; i < collection->count; i++) {
        hb_font_destroy(collection->faces[i].hb_font);
        FT_Done_Face(collection->faces[i].face);
}
    free(collection->faces);
    free(collection);
}

void fontCollectionAddFace(FT_Library *ft,FontCollection *collection, const char *font_path, i32 face_idx, u32 font_size) {
    if (collection->count >= collection->capacity) {
        size_t new_capacity = collection->capacity * 2;
        FontFace *new_faces = realloc(collection->faces, sizeof(FontFace) * new_capacity);

        collection->faces = new_faces;
        collection->capacity = new_capacity;
    }

    FT_Face face;
    if (FT_New_Face(*ft, font_path, face_idx, &face)) {
        LOG_ERROR("Failed to load font: %s", font_path);
        return;
    }

    FT_Set_Pixel_Sizes(face, 0, font_size);

    hb_font_t *hb_font = hb_ft_font_create(face, NULL);

    // Get line height
    // Get max width
    f32 line_height = 0;
    f32 max_glyph_width = 0;
    FT_GlyphSlot g = face->glyph;
    for (u8 i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            continue;
        }

        line_height = MAX(line_height, g->bitmap.rows);
        max_glyph_width = MAX(max_glyph_width, g->bitmap.width);
    }

    collection->faces[collection->count].face = face;
    collection->faces[collection->count].hb_font = hb_font;
    collection->faces[collection->count].line_height = line_height;
    collection->faces[collection->count].max_glyph_width = max_glyph_width;
    collection->count++;
}

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

static GlyphTexture addGlyphToAtlas(TextureAtlas *atlas, FT_Bitmap *bitmap, FT_GlyphSlot slot) {
    GlyphTexture glyph;

    // Check if we need to move to a new row
    if (atlas->x + bitmap->width >= atlas->width) {
        atlas->x = 0;
        atlas->y += atlas->rowHeight;
        atlas->rowHeight = 0;
    }

    // Check if we need to resize the atlas
    if (atlas->y + bitmap->rows >= atlas->height) {
        // Save old atlas ID
        GLuint old_texture = atlas->texture_id;
        int old_width = atlas->width;
        int old_height = atlas->height;

        // Double the size of the atlas
        atlas->width *= 2;
        atlas->height *= 2;

        // Create new texture
        GL_CALL(glGenTextures(1, &atlas->texture_id));
        GL_CALL(glBindTexture(GL_TEXTURE_2D, atlas->texture_id));
        GL_CALL(glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, atlas->width, atlas->height, 0, GL_RED, GL_UNSIGNED_BYTE, NULL));

        // Set texture parameters
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
        GL_CALL(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

        // Copy old texture data to new texture
        GLubyte *old_data = (GLubyte *)malloc(old_width * old_height);
        GL_CALL(glBindTexture(GL_TEXTURE_2D, old_texture));
        GL_CALL(glGetTexImage(GL_TEXTURE_2D, 0, GL_RED, GL_UNSIGNED_BYTE, old_data));

        GL_CALL(glBindTexture(GL_TEXTURE_2D, atlas->texture_id));
        GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, old_width, old_height, GL_RED, GL_UNSIGNED_BYTE, old_data));

        // Clean up
        free(old_data);
        GL_CALL(glDeleteTextures(1, &old_texture));
    }

    // Add the glyph bitmap to the atlas
    GL_CALL(glBindTexture(GL_TEXTURE_2D, atlas->texture_id));
    GL_CALL(glPixelStorei(GL_UNPACK_ALIGNMENT, 1));
    GL_CALL(glTexSubImage2D(GL_TEXTURE_2D, 0, atlas->x, atlas->y,
			   bitmap->width, bitmap->rows, 
			   GL_RED, GL_UNSIGNED_BYTE, bitmap->buffer));

    // Calculate UV coordinates
    glyph.uv_min.x = (f32)atlas->x / atlas->width;
    glyph.uv_max.y = (f32)atlas->y / atlas->height;
    glyph.uv_max.x = (f32)(atlas->x + bitmap->width) / atlas->width;
    glyph.uv_min.y = (f32)(atlas->y + bitmap->rows) / atlas->height;

    // Store glyph metrics
    glyph.width = bitmap->width;
    glyph.height = bitmap->rows;
    glyph.bearingX = slot->bitmap_left;
    glyph.bearingY = slot->bitmap_top;
    glyph.advance = slot->advance.x >> 6;

    // Update atlas current position
    atlas->x += bitmap->width + 1;  // Add 1 pixel padding
    atlas->rowHeight = MAX(atlas->rowHeight, bitmap->rows);

    return glyph;
}

void cacheGrapheme(FontCollection *collection, hashmap *glyph_map, TextureAtlas *atlas, char *unpacked_grapheme) {
    FT_Face current_face = collection->faces[0].face;

    // Initialize HarfBuzz buffer
    hb_buffer_t* hb_buffer = hb_buffer_create();
    u32 glyph_count;
    hb_glyph_info_t* glyph_info = shapeText(hb_buffer, current_face, unpacked_grapheme, &glyph_count);

    if (glyph_info->codepoint == 0) {
        for (size_t i = 0; i< collection->count; i++) {
            if (collection->faces[i].face == current_face)
                continue;

            hb_buffer_destroy(hb_buffer);
            hb_buffer = hb_buffer_create();
            glyph_info = shapeText(hb_buffer, collection->faces[i].face, unpacked_grapheme, &glyph_count);
            if (glyph_info->codepoint != 0) {
                current_face = collection->faces[i].face;
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
        *new_glyph = addGlyphToAtlas(atlas, &current_face->glyph->bitmap, current_face->glyph);
        codepoint_key = (char *)hashmapPush(glyph_map, codepoint_key, new_glyph);
        hb_buffer_destroy(hb_buffer);
    }

}

f32 getSizeOfText(FontCollection *collection, hashmap *glyph_map, TextureAtlas *atlas, char *text, f32 scale) {
    
    // f32 size = 0.0;
    // for (size_t i = 0; i < length; i++) {
    //     UnicodeChar grapheme = text[i];
    //     char *unpacked_grapheme = unpackUTF8(grapheme);
    //     GlyphTexture *glyph = (GlyphTexture *)hashmapGet(glyph_map, unpacked_grapheme);
    //
    //     if (glyph == NULL) {
    //         cacheGrapheme(collection, glyph_map, atlas, unpacked_grapheme);
    //     }
    //     glyph = (GlyphTexture *)hashmapGet(glyph_map, unpacked_grapheme);
    //     free(unpacked_grapheme);
    //
    //     size += (glyph->width + glyph->advance) * scale;
    // }
    // return size;

    f32 size = 0.0;
    size_t grapheme_size, offset = 0;
    for (offset = 0; text[offset] != '\0'; offset += grapheme_size) {
        grapheme_size = grapheme_next_character_break_utf8(text + offset, SIZE_MAX);
        UnicodeChar grapheme = packUTF8(text + offset, grapheme_size);
        char *unpacked_grapheme = unpackUTF8(grapheme);
        GlyphTexture *glyph = (GlyphTexture *)hashmapGet(glyph_map, unpacked_grapheme);

        if (glyph == NULL) {
            cacheGrapheme(collection, glyph_map, atlas, unpacked_grapheme);
        }
        glyph = (GlyphTexture *)hashmapGet(glyph_map, unpacked_grapheme);
        free(unpacked_grapheme);

        if (grapheme == '\t') {
            glyph = (GlyphTexture *)hashmapGet(glyph_map, " ");
            size += (TAB_WIDTH * glyph->advance) * scale;
        } else {
            size += (glyph->advance) * scale;
        }
    }
    return size;
}
