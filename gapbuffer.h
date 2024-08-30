#pragma once
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define INITIAL_SIZE 3

typedef struct
{
    char *buffer;
    size_t gap_start;
    size_t gap_end;
    size_t buffer_size;
} GapBuffer;

GapBuffer *create_gap_buffer(size_t inital_size);
void destroy_gap_buffer(GapBuffer *gb);
void shift_gap (GapBuffer *gb, size_t cursor_position);
void resize_gap(GapBuffer *gb, size_t required_space);
void insert_char (GapBuffer *gb, size_t cursor_position, char c);
void remove_char_before_buffer (GapBuffer *gb, size_t cursor_position);
void remove_char_after_buffer (GapBuffer *gb, size_t cursor_position);
void calculate_lines (GapBuffer *gb);
char *buffer_string (GapBuffer *gb);