#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_KEYWORDS 100
#define MAX_SYMBOLS 50
#define MAX_TYPES 50

typedef enum {
    TOKEN_KEYWORD,
    TOKEN_SYMBOL,
    TOKEN_STRING_LITERAL,
    TOKEN_NUMBER,
    TOKEN_TYPE,
    TOKEN_WHITESPACE,
    TOKEN_UNKNOWN
} TokenType;

typedef struct {
    char character;
    unsigned int color;
    TokenType type;
} Token;

// Configuration for syntax highlighting
#define NUM_KEYWORDS 20
#define NUM_SYMBOLS 9
#define NUM_TYPES 9

// Keywords and their color
static const char *KEYWORDS[NUM_KEYWORDS] = {
    "return","if",
    "else","void", 
    "auto","break",
    "const","case",
    "for", "do",
    "typedef", "struct",
    "sizeof", "goto",
    "switch", "do",
    "while", "true",
    "false", "NULL"
};

#define KEYWORD_COLOR 0x5CCC96FF  // Green

// Types and their color
static const char *TYPES[NUM_TYPES] = {
    "int","char", "float", "double", "long", "size_t", "bool", "short", "signed",
};

#define TYPE_COLOR 0xF2CE00FF  // Yellow

// Symbols and their color
static const char SYMBOLS[NUM_SYMBOLS] = {
    '=', '*',
    '+','-',
    '|', '&',
    '!', '/',
    '?'
};

#define SYMBOL_COLOR 0xCE6F8FFF  // Purple

// String literals and their color
#define STRING_LITERAL_COLOR 0x00A3CCFF  // Cyan

// Numbers and their color
#define NUMBER_COLOR 0xF2CE00FF // Yellow

// Whitespace color (may not be visible depending on how it's rendered)
#define WHITESPACE_COLOR 0xFFFFFFFF  // White

#define UNKNOWN_COLOR 0xFFFFFFFF

Token *lex(const char *source, size_t *token_count);

#endif // LEXER_H
