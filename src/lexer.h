#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "util.h"
#include "toml.h"
#include "config.h"

// Token types
typedef enum {
    TOKEN_KEYWORD,
    TOKEN_BUILT_IN_TYPE,
    TOKEN_SYMBOL,
    TOKEN_IDENTIFER,
    TOKEN_STRING_LITERAL_DOUBLE,
    TOKEN_STRING_LITERAL_SINGLE,
    TOKEN_ESCAPE_SEQUENCE,
    TOKEN_NUMBER,
    TOKEN_TYPE_NAME,
    TOKEN_FUNCTION_NAME,
    TOKEN_WHITESPACE,
    TOKEN_NEW_LINE,
    TOKEN_COMMENT_SINGLE,
    TOKEN_COMMENT_MULTI,
    TOKEN_PREPROCESSOR_DIRECTIVE,
    TOKEN_SECONDARY_KEYWORD,
    TOKEN_UNKNOWN
} TokenType;

// Token structure
typedef struct {
    char *text;
    TokenType type;
    //unsigned int color;
} Token;

Token createToken();
void tokenDestroy(Token * token);
void tokenPushChar(Token *token, char c);

typedef struct {
    Token *tokens;
    size_t token_count;
    size_t capacity;

    // The file type association for this lexer
    FileType file_type;

    // sizes of data buffers
    size_t keywords_count;
    size_t symbols_count;
    size_t built_in_types_count;
    size_t preproc_directives_count;
    size_t secondary_keywords_count;

    // data buffers
    char **keywords;
    char **symbols;
    char **built_in_types;
    char **preproc_directives;
    char **secondary_keywords;

    // Comments
    toml_datum_t comment_single_prefix;
    toml_datum_t comment_multi_begin;
    toml_datum_t comment_multi_end;

    //Other lexer settings
    bool id_heuristics;
} Lexer;

void lexerInit(Lexer *lexer);
void lexerDestroy(Lexer *lexer);
void lexerClearTokens(Lexer *lexer);
void lexerUpdateFileType(Lexer *lexer, FileType file_type);

void lex(Lexer *lexer, const char *source);
