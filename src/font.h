#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "putils/defines.h"

typedef struct {
    FT_Face face;
    hb_font_t *hb_font;
} FontFace;

typedef struct {
    FontFace *faces;
    size_t count;
    size_t capacity;
} FontCollection;

FontCollection *fontCollectionNew(size_t initial_capacity);
void fontcollectionDestroy(FontCollection *collection);
void fontCollectionAddFace(FT_Library *ft,FontCollection *collection, const char *font_path, i32 face_idx, u32 font_size);

hb_glyph_info_t* shapeText(hb_buffer_t* hb_buffer, FT_Face face, const char* text, unsigned int* glyph_count);
