#pragma once
#include "putils/phashmap.h"

// Stuff that will need to be passed around a lot
typedef struct {
    hashmap *glyph_cache;
} AppContext;
