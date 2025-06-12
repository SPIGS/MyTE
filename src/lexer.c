#include "lexer.h"
#include "putils/pstring.h"
#include <stddef.h>

Token tokenNew() {
    Token token;
    token.text = NULL;
    token.type = TOKEN_UNKNOWN;
    return token;
}

void tokenDestroy(Token *token) {
    if (token->text)
        stringFree(token->text);
}

void tokenPushChar(Token *token, char c) {
    if (!token->text) {
        token->text = stringNew("");
    }
    token->text = stringCatChar(token->text, c);
}

void lexerInit(Lexer *lexer) {
    lexer->tokens = NULL;
    lexer->token_count = 0;
    lexer->capacity = 8;
}

void lexerDestroy(Lexer *lexer) {
    if (lexer->tokens) {
        lexerClearTokens(lexer);
        free(lexer->tokens);
    }
}

void lexerClearTokens(Lexer *lexer) {
    if (lexer->tokens) {
        for(size_t i = 0; i < lexer->token_count; i++) {
            tokenDestroy(&lexer->tokens[i]);
        }
    }
    lexer->token_count = 0;
}

static void lexerPushToken(Lexer *lexer, Token token) {
    if (!token.text || stringLength(token.text) == 0) {
        return;
    }

    if (!lexer->tokens) {
        lexer->tokens = (Token *)malloc(lexer->capacity * sizeof(Token));
    }

    if (lexer->token_count >= lexer->capacity) {
        lexer->capacity *= 2;
        lexer->tokens = (Token *)realloc(lexer->tokens, lexer->capacity * sizeof(Token));
    }

    lexer->tokens[lexer->token_count] = token;
    lexer->token_count++;
}

void lex(Lexer *lexer, string source) {
    if (lexer->tokens) {
        lexerClearTokens(lexer);
    }

    // TODO: Add back filetypes
    
    // If we don't know what file type it is, don't do anything.
    Token cur_tok = tokenNew();
    size_t len = stringLength(source);
    for (size_t i = 0; i < len; i++) {
        tokenPushChar(&cur_tok, source[i]);

        // If we encounter a newline, start a new token.
        // this doesn't change highlighting, it helps with
        // visualizing the user selection (we don't have to worry about multi-
        // line tokens).
        if (source[i] == '\n') {
            cur_tok.type = TOKEN_UNKNOWN;
            lexerPushToken(lexer, cur_tok);
            cur_tok = tokenNew();
        }
    }
    if (cur_tok.text) {
        lexerPushToken(lexer, cur_tok);
    }
}
