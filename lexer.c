#include "lexer.h"

#define KEYWORDS_COUNT 10
#define SYMBOLS_COUNT 16
#define BUILT_IN_TYPES_COUNT 10

// Keywords and their color
static const char *KEYWORDS[KEYWORDS_COUNT] = {
    "return","if",
    "else", 
    "auto","break",
    "const","case",
    "for", "do",
    "typedef", "struct",
    "sizeof", "goto",
    "switch", "do",
    "while", "true",
    "false", "NULL"
};

static const char *SYMBOLS[SYMBOLS_COUNT] = {
    "(", ")", 
    "{", "}", 
    "*", "=",
    "|", "&",
    "!", "%",
    "-", "+",
    "/", "?",
    "<", ">"
};

static const char *BUILT_IN_TYPES[BUILT_IN_TYPES_COUNT] = {
    "int","char", 
    "float", "double", 
    "long", "size_t", 
    "bool", "short", "signed", "void"
};

#define COMMENT_SINGLE_PREFIX "//"
#define COMMENT_MULTI_BEGIN "/*"
#define COMMENT_MULTI_END "*/"

// Colors - the theme is Space Duck (https://github.com/pineapplegiant/spaceduck)
#define KEYWORD_COLOR               0x7A5CCCFF  // Purple
#define SYMBOL_COLOR                0xCE6F8FFF  // Magenta
#define STRING_LITERAL_DOUBLE_COLOR 0x00A3CCFF  // Cyan
#define STRING_LITERAL_SINGLE_COLOR 0xF2CE00FF  // Yellow
#define NUMBER_COLOR                0xF2CE00FF  // Yellow
#define BUILT_IN_TYPE_COLOR         0x5CCC96FF  // Green
#define TYPE_COLOR                  0xF2CE00FF  // Yellow
#define FUNCTION_NAME_COLOR         0x5CCC96FF  // Green
#define WHITESPACE_COLOR            0xFFFFFFFF  // White
#define COMMENT_SINGLE_COLOR        0x686f9aFF  // Dark purple
#define COMMENT_MULTI_COLOR         0x686f9aFF  // dark purple
#define UNKNOWN_COLOR               0xFFFFFFFF  // white

static int is_keyword(const char *word) {
    for (size_t i = 0; i < KEYWORDS_COUNT; i++) {
        if (strcmp(word, KEYWORDS[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_symbol(char character) {
    for (size_t i = 0; i < SYMBOLS_COUNT; i++) {
        if (character == SYMBOLS[i][0]) {
            return 1;
        }
    }
    return 0;
}

static int is_builtin_type(const char *word) {
    for (size_t i = 0; i < BUILT_IN_TYPES_COUNT; i++) {
        if (strcmp(word, BUILT_IN_TYPES[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_type(const char *source, size_t start, size_t length) {
    // Check if the word is a type by looking for patterns of variable or function declarations
    size_t i = start + length;

    // Skip whitespace
    while (isspace((unsigned char)source[i])) {
        i++;
    }

    // Check for type followed by variable declaration or function definition
    if (source[i] == '*' || isalpha((unsigned char)source[i]) || source[i] == '(') {
        return 1;
    }
    return 0;
}

static int is_function_name(const char *source, size_t start, size_t length) {
    size_t i = start + length;

    // Skip whitespace
    while (isspace((unsigned char)source[i])) {
        i++;
    }

    // Check for function declaration or usage (name followed by '(')
    if (source[i] == '(') {
        return 1;
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

        // Handle single-line comments
        if (strncmp(&source[i], COMMENT_SINGLE_PREFIX, strlen(COMMENT_SINGLE_PREFIX)) == 0) {
            while (i < length && source[i] != '\n') {
                i++;
            }
            color = COMMENT_SINGLE_COLOR;
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
        }
        // Handle double-quote string literals
        else if (source[i] == '"') {
            i++;
            while (i < length && source[i] != '"') {
                if (source[i] == '\\' && i + 1 < length) {
                    i++;  // Skip escaped characters
                }
                i++;
            }
            if (i < length && source[i] == '"') {
                i++;
            }
            color = STRING_LITERAL_DOUBLE_COLOR;
        }
        // Handle single-quote string literals
        else if (source[i] == '\'') {
            i++;
            while (i < length && source[i] != '\'') {
                if (source[i] == '\\' && i + 1 < length) {
                    i++;  // Skip escaped characters
                }
                i++;
            }
            if (i < length && source[i] == '\'') {
                i++;
            }
            color = STRING_LITERAL_SINGLE_COLOR;
        }
        // Handle digits/numbers
        else if (isdigit((unsigned char)source[i])) {
            while (i < length && isdigit((unsigned char)source[i])) {
                i++;
            }
            color = NUMBER_COLOR;
        }
        // Handle built-in types and user-defined types in specific contexts
        else if (isalpha((unsigned char)source[i])) {
            while (i < length && (isalnum((unsigned char)source[i]) || source[i] == '_')) {
                i++;
            }
            char *word = (char *)malloc((i - start + 1) * sizeof(char));
            if (word) {
                strncpy(word, source + start, i - start);
                word[i - start] = '\0';

                // Check in order of precedence: built-in types > keywords > types > function names
                if (is_builtin_type(word)) {
                    color = BUILT_IN_TYPE_COLOR;
                } else if (is_keyword(word)) {
                    color = KEYWORD_COLOR;
                }else if (is_function_name(source, start, i - start)) {
                    color = FUNCTION_NAME_COLOR;
                } 
                else if (is_type(source, start, i - start)) {
                    color = TYPE_COLOR;
                } 

                free(word);
            }
        }
        // Handle symbols
        else if (is_symbol(source[i])) {
            i++;
            color = SYMBOL_COLOR;
        }
        // Handle whitespace
        else if (isspace((unsigned char)source[i])) {
            color = WHITESPACE_COLOR;
            i++;
        }
        // Handle unknown characters
        else {
            color = UNKNOWN_COLOR;
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
