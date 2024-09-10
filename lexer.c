#include "lexer.h"

static int is_keyword(const char *word) {
    for (int i = 0; i < NUM_KEYWORDS; i++) {
        if (strcmp(word, KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_type(const char *word) {
    for (int i = 0; i < NUM_TYPES; i++) {
        if (strcmp(word, TYPES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_symbol(char ch) {
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        if (ch == SYMBOLS[i]) {
            return 1;
        }
    }
    return 0;
}

Token *lex(const char *source, size_t *token_count) {
    size_t length = strlen(source);
    size_t capacity = 10;
    *token_count = 0;
    Token *tokens = (Token *)malloc(capacity * sizeof(Token));

    if (!tokens) {
        return NULL;
    }

    size_t i = 0;
    while (i < length) {
        size_t start = i;
        unsigned int color = UNKNOWN_COLOR;
        TokenType type = TOKEN_UNKNOWN;

        // Handle single-line comments
        if (strncmp(&source[i], COMMENT_SINGLE_PREFIX, strlen(COMMENT_SINGLE_PREFIX)) == 0) {
            while (i < length && source[i] != '\n') {
                i++;
            }
            color = COMMENT_SINGLE_COLOR;
            type = TOKEN_COMMENT_SINGLE;
        }
        // Handle multi-line comments
        else if (strncmp(&source[i], COMMENT_MULTI_BEGIN, strlen(COMMENT_MULTI_BEGIN)) == 0) {
            i += strlen(COMMENT_MULTI_BEGIN);
            while (i < length && strncmp(&source[i], COMMENT_MULTI_END, strlen(COMMENT_MULTI_END)) != 0) {
                i++;
            }
            if (i < length) {
                i += strlen(COMMENT_MULTI_END);
            }
            color = COMMENT_MULTI_COLOR;
            type = TOKEN_COMMENT_MULTI;
        }
        // Handle double-quote string literals
        else if (source[i] == '"') {
            char quote = source[i];
            i++;
            while (i < length && source[i] != quote) {
                if (source[i] == '\\' && i + 1 < length) {
                    i++;  // Skip escaped characters
                }
                i++;
            }
            if (i < length && source[i] == quote) {
                i++;
            }
            color = STRING_LITERAL_DOUBLE_COLOR;
            type = TOKEN_STRING_LITERAL_DOUBLE;
        }
        // Handle single-quote string literals
        else if (source[i] == '\'') {
            char quote = source[i];
            i++;
            while (i < length && source[i] != quote) {
                if (source[i] == '\\' && i + 1 < length) {
                    i++;  // Skip escaped characters
                }
                i++;
            }
            if (i < length && source[i] == quote) {
                i++;
            }
            color = STRING_LITERAL_SINGLE_COLOR;
            type = TOKEN_STRING_LITERAL_SINGLE;
        }
        // Handle digits/numbers
        else if (isdigit((unsigned char)source[i])) {
            while (i < length && isdigit((unsigned char)source[i])) {
                i++;
            }
            color = NUMBER_COLOR;
            type = TOKEN_NUMBER;
        }
        // Handle types, keywords, and unknown words
        else if (isalpha((unsigned char)source[i])) {
            while (i < length && (isalnum((unsigned char)source[i]) || source[i] == '_')) {
                i++;
            }
            char *word = (char *)malloc((i - start + 1) * sizeof(char));
            if (word) {
                strncpy(word, source + start, i - start);
                word[i - start] = '\0';

                if (is_type(word)) {
                    color = TYPE_COLOR;
                    type = TOKEN_TYPE;
                } else if (is_keyword(word)) {
                    color = KEYWORD_COLOR;
                    type = TOKEN_KEYWORD;
                }
                free(word);
            }
        }
        // Handle symbols
        else if (is_symbol(source[i])) {
            i++;
            color = SYMBOL_COLOR;
            type = TOKEN_SYMBOL;
        }
        // Handle whitespace
        else if (isspace((unsigned char)source[i])) {
            color = WHITESPACE_COLOR;
            type = TOKEN_WHITESPACE;
            i++;
        }
        // Handle unknown characters
        else {
            color = UNKNOWN_COLOR;
            type = TOKEN_UNKNOWN;
            i++;
        }

        // Add tokens character by character
        for (size_t j = start; j < i; j++) {
            if (*token_count >= capacity) {
                capacity *= 2;
                Token *new_tokens = (Token *)realloc(tokens, capacity * sizeof(Token));
                if (!new_tokens) {
                    free(tokens);
                    return NULL;
                }
                tokens = new_tokens;
            }
            tokens[*token_count].character = source[j];
            tokens[*token_count].color = color;
            (*token_count)++;
        }
    }

    return tokens;
}