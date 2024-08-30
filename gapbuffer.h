#pragma once
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define INITIAL_SIZE 4

typedef struct {
    char *buffer;
    size_t gap_start;
    size_t gap_end;
    size_t end;
} GapBuffer;

GapBuffer *create_gap_buffer(size_t inital_size);
void destroy_gap_buffer(GapBuffer *gb);
size_t get_buffer_gap_size(GapBuffer *gb);
size_t get_buffer_length(GapBuffer *gb);
size_t get_cursor_idx(GapBuffer *gb, size_t cursor_pos);
char get_char(GapBuffer *gb, size_t cursor);
void shift_gap (GapBuffer *gb, size_t cursor);
void resize_gap(GapBuffer *gb, size_t required_space);
void insert_char (GapBuffer *gb, size_t cursor, char c);
void remove_char_before_gap (GapBuffer *gb, size_t cursor);
void remove_char_after_gap (GapBuffer *gb, size_t cursor);
void calculate_lines (GapBuffer *gb);
char *get_buffer_string (GapBuffer *gb);
void output_buffer_string (GapBuffer *gb, size_t);

size_t get_next_character_cursor(GapBuffer *gb, size_t cursor);
size_t get_prev_character_cursor(GapBuffer *gb, size_t cursor);
size_t get_beginning_of_line_cursor(GapBuffer *gb, size_t cursor);
size_t get_end_of_line_cursor(GapBuffer *gb, size_t cursor);
size_t get_beginning_of_next_line_cursor(GapBuffer *gb, size_t cursor);
size_t get_end_of_prev_line_cursor(GapBuffer *gb, size_t cursor);
size_t get_beginning_of_prev_line_cursor(GapBuffer *gb, size_t cursor);
size_t get_column(GapBuffer *gb, size_t cursor);
size_t get_buffer_line_length(GapBuffer* gb, size_t cursor);