#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Token types
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_SYMBOL,
    TOKEN_STRING_LITERAL_DOUBLE,
    TOKEN_STRING_LITERAL_SINGLE,
    TOKEN_NUMBER,
    TOKEN_BUILT_IN_TYPE,
    TOKEN_TYPE,
    TOKEN_FUNCTION_NAME,
    TOKEN_WHITESPACE,
    TOKEN_COMMENT_SINGLE,
    TOKEN_COMMENT_MULTI,
    TOKEN_UNKNOWN
} TokenType;

// Token structure
typedef struct {
    char character;
    unsigned int color;
} Token;

Token *lex(const char *source, size_t *token_count);
