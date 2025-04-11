#pragma once
#include <stddef.h>

typedef struct {
    size_t buffer_idx;

    // Display coordinates
    size_t disp_col;
    size_t disp_row;
} Cursor;

Cursor cursorNew(void);


