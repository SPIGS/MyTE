#include "gapbuffer.h"
#include<stdio.h>
#include <assert.h>


GapBuffer *create_gap_buffer(size_t inital_size) {
    GapBuffer *gb = (GapBuffer*)malloc(sizeof(GapBuffer));
    gb->buffer = (char*)malloc(inital_size * sizeof(char));
    gb->gap_start = 0;
    gb->gap_end = inital_size;
    gb->end = inital_size;
    return gb;
}

static void AssertBufferInvariants(GapBuffer* buffer) {
	assert(buffer->buffer);
	assert(buffer->gap_start <= buffer->gap_end);
	assert(buffer->gap_end <= buffer->end);
}

static void AssertCursorInvariants(GapBuffer* buffer, size_t cursor) {
	assert(cursor <= get_buffer_length(buffer));
}

void destroy_gap_buffer(GapBuffer *gb) {
    free(gb->buffer);
    free(gb);
}

size_t get_buffer_gap_size(GapBuffer *gb) {
    return gb->gap_end - gb->gap_start;
}

size_t get_buffer_length(GapBuffer *gb) {
    return gb->end - get_buffer_gap_size(gb);
}

size_t get_cursor_idx(GapBuffer *gb, size_t cursor_pos) {
    return ((cursor_pos <= gb->gap_start) ? cursor_pos : cursor_pos + get_buffer_gap_size(gb));
}

char get_char(GapBuffer *gb, size_t cursor) {
    return gb->buffer[get_cursor_idx(gb, cursor)];
}

void shift_gap (GapBuffer *gb, size_t cursor) {
    size_t gap_size = get_buffer_gap_size(gb);
    if (cursor < gb->gap_start) {
        size_t move_size = gb->gap_start - cursor;
        gb->gap_start -= move_size;
        gb->gap_end -= move_size;
        memmove(gb->buffer + gb->gap_end, gb->buffer + gb->gap_start, move_size);
    } else if (cursor > gb->gap_start) {
        size_t move_size = cursor - gb->gap_start;
        memmove(gb->buffer + gb->gap_start, gb->buffer + gb->gap_end, move_size);
        gb->gap_start += move_size;
        gb->gap_end += move_size;
    }
    assert(get_buffer_gap_size(gb) == gap_size);
	AssertBufferInvariants(gb);
}

void resize_gap(GapBuffer *gb, size_t required_space) {
    if (get_buffer_gap_size(gb) < required_space) {
        shift_gap(gb, get_buffer_length(gb));
        size_t new_end = MAX(2 * gb->end, gb->end + required_space) - get_buffer_gap_size(gb);
        gb->buffer = (char *)realloc(gb->buffer, new_end * sizeof(char));
        gb->end = new_end;
        gb->gap_end = gb->end;       
    }
    assert(get_buffer_gap_size(gb) >= required_space);
}

void insert_char (GapBuffer *gb, size_t cursor, char c) {
    AssertCursorInvariants(gb, cursor);
    resize_gap(gb, 1);
    shift_gap(gb, cursor);
    gb->buffer[gb->gap_start] = c;
    gb->gap_start++;
}

void remove_char_before_gap (GapBuffer *gb, size_t cursor) {
    if (cursor > 0) {
        shift_gap(gb, cursor);
        gb->gap_start--;
    }
}

void remove_char_after_gap(GapBuffer *gb, size_t cursor) {
    if (cursor < get_buffer_length(gb)) {
        shift_gap(gb, cursor);
        gb->gap_end++;
    }
}

char *get_buffer_string (GapBuffer *gb) {
    size_t length = get_buffer_length(gb);
    char *temp = (char *)malloc((length + 1) * sizeof(char)); // +1 for null-terminator
    if (temp == NULL) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    memcpy(temp, gb->buffer, gb->gap_start); // Copy before the gap
    memcpy(temp + gb->gap_start, gb->buffer + gb->gap_end, gb->end - gb->gap_end); // Copy after the gap

    temp[length] = '\0'; // Null-terminate the string
    return temp;
}

void output_buffer_string (GapBuffer *gb, size_t cursor) {
    char *temp = get_buffer_string(gb);
    printf("%d, %d\n", cursor, get_cursor_idx(gb, cursor));
    printf("%s\n", temp);
    free(temp);
}

size_t get_next_character_cursor(GapBuffer *gb, size_t cursor) {
    AssertCursorInvariants(gb, cursor);
    if (cursor < get_buffer_length(gb)) {
        return cursor + 1;
    } else {
        return cursor;
    }
}

size_t get_prev_character_cursor(GapBuffer *gb, size_t cursor) {
    if (cursor > 0) {
        return cursor - 1;
    } else {
        return cursor;
    }
}

size_t get_beginning_of_line_cursor(GapBuffer *gb, size_t cursor) {
    cursor = get_prev_character_cursor(gb, cursor);
    while (cursor > 0) {
        char character = get_char(gb, cursor);
        if (character == '\n') {
            return get_next_character_cursor(gb, cursor);
        }
        cursor = get_prev_character_cursor(gb, cursor);
    }
    return 0;
}

size_t get_end_of_line_cursor(GapBuffer *gb, size_t cursor) {
    while (cursor < get_buffer_length(gb)) {
        char character = get_char(gb, cursor);
        if (character == '\n') {
            return cursor;
        }
        cursor = get_next_character_cursor(gb, cursor);
    }
    return get_buffer_length(gb);
}

size_t get_beginning_of_next_line_cursor(GapBuffer *gb, size_t cursor) {
    return get_next_character_cursor(gb, get_end_of_line_cursor(gb, cursor));
}

size_t get_end_of_prev_line_cursor(GapBuffer *gb, size_t cursor) {
    return get_prev_character_cursor(gb, get_beginning_of_line_cursor(gb, cursor));
}

size_t get_beginning_of_prev_line_cursor(GapBuffer *gb, size_t cursor) {
    return get_beginning_of_line_cursor(gb, get_prev_character_cursor(gb, get_beginning_of_line_cursor(gb, cursor)));
}

size_t get_column(GapBuffer *gb, size_t cursor) {
    return cursor - get_beginning_of_line_cursor(gb, cursor);
}

size_t get_buffer_line_length(GapBuffer* gb, size_t cursor) {
    size_t end = get_end_of_line_cursor(gb, cursor);
    size_t beg = get_beginning_of_line_cursor(gb, cursor);
	return end - beg;
}