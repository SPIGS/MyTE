#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "toml.h"
#include "theme.h"

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


typedef struct {
    Token *tokens;
    size_t token_count;

    // The file type association for this lexer
    FileType file_type;

    // sizes of data buffers
    size_t keywords_count;
    size_t symbols_count;
    size_t built_in_types_count;

    // data buffers
    char **keywords;
    char **symbols;
    char **built_in_types;

    // Comments
    toml_datum_t comment_single_prefix;
    toml_datum_t comment_multi_begin;
    toml_datum_t comment_multi_end;

    bool additional_colors;
} Lexer;

void lexerInit(Lexer *lexer);
void lexerDestroy(Lexer *lexer);

void lexerUpdateFileType(Lexer *lexer, FileType file_type);


Token *lex(Lexer *lexer, const char *source, ColorTheme theme);
