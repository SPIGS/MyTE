#include "putils/unicode.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include "putils/defines.h"
#include "putils/phashmap.h"
#include "putils/pmath.h"

#define TAB_WIDTH 4
typedef struct {
    FT_Face face;
    hb_font_t *hb_font;
    f32 line_height;
    f32 max_glyph_width;
} FontFace;

typedef struct {
    FontFace *faces;
    size_t count;
    size_t capacity;
} FontCollection;

// OpenGL Texture structure for storing glyphs
typedef struct{
    Vector2 uv_min, uv_max;
    i32 width, height;
    i32 bearingX, bearingY;
    i32 advance;
} GlyphTexture;

typedef struct {
    u32 texture_id;
    u32 width, height;
    u32 x,y; // Current position in the atlas
    u32 rowHeight; // Height of the current row
} TextureAtlas;

FontCollection *fontCollectionNew(size_t initial_capacity);
void fontcollectionDestroy(FontCollection *collection);
void fontCollectionAddFace(FT_Library *ft,FontCollection *collection, const char *font_path, i32 face_idx, u32 font_size);

void cacheGrapheme(FontCollection *collection, hashmap *glyph_map, TextureAtlas *atlas, char *unpacked_grapheme);
f32 getSizeOfText(FontCollection *collection, hashmap *glyph_map, TextureAtlas *atlas, char *text, f32 scale);
