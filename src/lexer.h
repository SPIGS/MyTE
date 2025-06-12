#pragma once
#include "putils/pstring.h"

// Token types
typedef enum {
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    string text;
    TokenType type;
} Token;

Token tokenNew();
void tokenDestroy(Token *token);
void tokenPushChar(Token *token, char c);

typedef struct {
    Token *tokens;
    size_t token_count;
    size_t capacity;
} Lexer;

void lexerInit(Lexer *lexer);
void lexerDestroy(Lexer *lexer);
void lexerClearTokens(Lexer *lexer);

void lex(Lexer *lexer, string source);
