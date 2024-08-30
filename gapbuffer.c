#include "gapbuffer.h"


GapBuffer *create_gap_buffer(size_t inital_size) {
    GapBuffer *gb = (GapBuffer*)malloc(sizeof(GapBuffer));
    gb->buffer = (char*)malloc(inital_size * sizeof(char));
    gb->gap_start = 0;
    gb->gap_end = inital_size;
    gb->buffer_size = inital_size;
    return gb;
}

void destroy_gap_buffer(GapBuffer *gb) {
    free(gb->buffer);
    free(gb);
}

void shift_gap (GapBuffer *gb, size_t cursor_position) {
    size_t gap_size = gb->gap_end - gb->gap_start;
    size_t cursor = MIN(cursor_position, gb->buffer_size - gap_size);
    if (cursor == gb->gap_start) { return; }

    if (gb->gap_start > cursor) {
        size_t move_size = gb->gap_start - cursor;
        memmove(gb->buffer + gb->gap_end - move_size, gb->buffer + cursor, move_size);
        gb->gap_start = cursor;
        gb->gap_end -= move_size;
    } else if (gb->gap_start < cursor) {
        size_t move_size = cursor - gb->gap_start;
        memmove(gb->buffer + gb->gap_start, gb->buffer + gb->gap_end, move_size);
        gb->gap_start += move_size;
        gb->gap_end += move_size;
    }
}

void resize_gap(GapBuffer *gb, size_t required_space) {
    size_t gap_size = gb->gap_end - gb->gap_start;
    size_t new_buffer_size = gb->buffer_size * 2;

    if (gap_size < required_space) {
        shift_gap(gb, gb->gap_start);
        gb->buffer = (char *)realloc(gb->buffer, new_buffer_size * sizeof(char));

        gb->buffer_size = new_buffer_size;
        gb->gap_end = gb->buffer_size;
        gb->gap_start = (new_buffer_size / 2);        
    }
}

void insert_char (GapBuffer *gb, size_t cursor_position, char c) {
    if (gb->gap_start == gb->gap_end) {
        resize_gap(gb, 1);
    }
    shift_gap(gb, cursor_position);

    gb->buffer[gb->gap_start] = c;
    gb->gap_start += 1;
}

void remove_char_before_buffer (GapBuffer *gb, size_t cursor_position) {
    shift_gap(gb, cursor_position);

    if (cursor_position != 0) {
        gb->gap_start -= 1;
    }
}

void remove_char_after_buffer (GapBuffer *gb, size_t cursor_position) {
    shift_gap(gb, cursor_position);

    if (gb->gap_end != gb->buffer_size) {
        gb->gap_end += 1;
    }
}

char *buffer_string (GapBuffer *gb) {
    size_t string_len = gb->gap_start + (gb->buffer_size - gb->gap_end);

    char *content = (char *)malloc((string_len + 1) * sizeof (char));

    // handle memory allocation failure?

    memcpy(content, gb->buffer, gb->gap_start);
    memcpy(content + gb->gap_start, gb->buffer + gb->gap_end, gb->buffer_size - gb->gap_end);

    content[string_len] = '\0';
    return content;
}