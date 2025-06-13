#include "lexer.h"
#include "putils/log.h"
#include "buffer.h"
#include <stddef.h>
#include <string.h>


Token tokenNew(void) {
    Token token;
    token.text = UTF8StringNew();
    token.type = TOKEN_UNKNOWN;
    return token;
}

void tokenDestroy(Token *token) {
    if (token->text.s)
        UTF8StringDestroy(&token->text);
}

void tokenPushChar(Token *token, UnicodeChar c) {
    UTF8StringPushChar(&token->text, c);
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
    if (token.text.size == 0) {
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

void lex(Lexer *lexer, GapBuffer *buf) {
    if (lexer->tokens) {
        lexerClearTokens(lexer);
    }

    // TODO: Add back filetypes
    
    // If we don't know what file type it is, don't do anything.
    Token cur_tok = tokenNew();
    size_t len = getBufLength(buf);
    size_t cursor_idx = 0;
    while (cursor_idx < len) {
        UnicodeChar c = getBufChar(buf, cursor_idx);
        tokenPushChar(&cur_tok, c);


        // If we encounter a newline, start a new token.
        // this doesn't change highlighting, it helps with
        // visualizing the user selection (we don't have to worry about multi-
        // line tokens).
        if (c == '\n') {
            cur_tok.type = TOKEN_UNKNOWN;
            lexerPushToken(lexer, cur_tok);
            cur_tok = tokenNew();
        }
        cursor_idx = getNextGraphemeCursor(buf, cursor_idx);
    }
    // for (size_t i = 0; i < len; i++) {
    //     tokenPushChar(&cur_tok, source[i]);
    //
    //     if (source[i] == '\n') {
    //         cur_tok.type = TOKEN_UNKNOWN;
    //         lexerPushToken(lexer, cur_tok);
    //         cur_tok = tokenNew();
    //     }
    // }
    if (cur_tok.text.size > 0) {
        lexerPushToken(lexer, cur_tok);
    }
}
