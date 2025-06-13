#pragma once
#include "buffer.h"
#include "putils/pstring.h"
#include "putils/unicode.h"

// Token types
typedef enum {
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    UTF8String text;
    TokenType type;
} Token;

Token tokenNew(void);
void tokenDestroy(Token *token);
void tokenPushChar(Token *token, UnicodeChar c);

typedef struct {
    Token *tokens;
    size_t token_count;
    size_t capacity;
} Lexer;

void lexerInit(Lexer *lexer);
void lexerDestroy(Lexer *lexer);
void lexerClearTokens(Lexer *lexer);

void lex(Lexer *lexer,  GapBuffer *buf);
