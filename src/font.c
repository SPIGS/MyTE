#include "font.h"
#include "freetype/freetype.h"
#include "putils/log.h"
#include "putils/pmath.h"

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
    f32 line_height = 0;
    FT_GlyphSlot g = face->glyph;
    for (u8 i = 32; i < 128; i++) {
        if (FT_Load_Char(face, i, FT_LOAD_RENDER)) {
            continue;
        }

        line_height = MAX(line_height, g->bitmap.rows);
    }

    collection->faces[collection->count].face = face;
    collection->faces[collection->count].hb_font = hb_font;
    collection->faces[collection->count].line_height = line_height;
    collection->count++;
}

hb_glyph_info_t* shapeText(hb_buffer_t* hb_buffer, FT_Face face, const char* text, unsigned int* glyph_count) {
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
