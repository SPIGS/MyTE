#pragma once
#include <stddef.h>

typedef struct {
    size_t buffer_idx;
} Cursor;

Cursor cursorNew(void);


