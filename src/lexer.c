#include <errno.h>
#include "lexer.h"

// Highlighting files (hardcoded for now)
#define C_HIGHLIGHTING_FILE "./config/syntaxes/c.toml"
#define MAKEFILE_HIGHLIGHTING_FILE "./config/syntaxes/makefile.toml"
#define TOML_HIGHLIGHTING_FILE "./config/syntaxes/toml.toml"
#define PYTHON_HIGHLIGHTING_FILE "./config/syntaxes/python.toml"

static int is_keyword(Lexer *lexer, const char *word) {
    for (size_t i = 0; i < lexer->keywords_count; i++) {
        char *keyword = lexer->keywords[i];
        if (strcmp(word, keyword) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_symbol(Lexer *lexer, char character) {
    for (size_t i = 0; i < lexer->symbols_count; i++) {
        char *symbol = lexer->symbols[i];
        if (symbol && character == symbol[0]) {
            return 1;
        }
    }
    return 0;
}

static int is_builtin_type(Lexer *lexer, const char *word) {
    for (size_t i = 0; i < lexer->built_in_types_count; i++) {
        char *built_in_type = lexer->built_in_types[i];
        if (strcmp(word, built_in_type) == 0) {
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

void lexerInit(Lexer* lexer) {
    lexer->tokens = NULL;
    lexer->token_count = 0;
    lexer->file_type = FILE_TYPE_UNKNOWN;
        // sizes of data buffers
    lexer->keywords_count = 0;
    lexer->symbols_count = 0;
    lexer->built_in_types_count = 0;

    // data buffers
    lexer->keywords = NULL;
    lexer->symbols = NULL;
    lexer->built_in_types = NULL;

    // Comment stuff
    lexer->comment_single_prefix.u.s = NULL;
    lexer->comment_single_prefix.u.ts = NULL;
    lexer->comment_multi_begin.u.s = NULL;
    lexer->comment_multi_begin.u.ts = NULL;
    lexer->comment_multi_end.u.s = NULL;
    lexer->comment_multi_end.u.ts = NULL;

    lexer->additional_colors = false;
}

void lexerDestroy(Lexer *lexer) {
    if (lexer->tokens)
        free(lexer->tokens);

    if (lexer->keywords) {
        for (size_t i = 0; i< lexer->keywords_count; i++) {
            free(lexer->keywords[i]);
        }
    }
    
    if (lexer->symbols) {
        for (size_t i = 0; i< lexer->symbols_count; i++) {
            free(lexer->symbols[i]);
        }
    }

    if (lexer->built_in_types) {
        for (size_t i = 0; i< lexer->built_in_types_count; i++) {
            free(lexer->built_in_types[i]);
        }
    }

    if (lexer->comment_single_prefix.u.s)
        free(lexer->comment_single_prefix.u.s);

    if (lexer->comment_multi_begin.u.s)
        free(lexer->comment_multi_begin.u.s);

    if (lexer->comment_multi_end.u.s)
        free(lexer->comment_multi_end.u.s);
}

static void loadHighlightingInfo (Lexer *lexer, FileType file_type) {
    FILE *fp;
    char errbuf[200];

    switch (file_type)
    {
    case FILE_TYPE_C:
        fp = fopen(C_HIGHLIGHTING_FILE, "r");
        break;

    case FILE_TYPE_MAKEFILE:
        fp = fopen(MAKEFILE_HIGHLIGHTING_FILE, "r");
        break;

    case FILE_TYPE_TOML:
        fp = fopen(TOML_HIGHLIGHTING_FILE, "r");
        break;

    case FILE_TYPE_PYTHON:
        fp = fopen(PYTHON_HIGHLIGHTING_FILE, "r");
        break;
    
    default:
        return;
        break;
    }

    toml_table_t *hl_conf = toml_parse_file(fp, errbuf, sizeof(errbuf));
    fclose(fp);

    if (!hl_conf) {
        LOG_ERROR("cannot open highlighting file - ", strerror(errno));
        return;
    }

    toml_table_t* hl_table = toml_table_in(hl_conf, "highlighting");
    if (!hl_table) {
        LOG_ERROR("Highlight file missing [highlighting]", "");
        return;
    }

    // Get data
    LOAD_TOML_STR(hl_table, "comment_single_prefix", comment_single_prefix);
    LOAD_TOML_STR(hl_table, "comment_multi_begin", comment_multi_begin);
    LOAD_TOML_STR(hl_table, "comment_multi_end", comment_multi_end);
    LOAD_TOML_BOOL(hl_table, "additional_colors", additional_colors);
    LOAD_TOML_STR_ARRAY(hl_table, "keywords", keywords_array, keywords_array_len, keywords, keywords_count);
    LOAD_TOML_STR_ARRAY(hl_table, "symbols", symbols_array, symbols_array_len, symbols, symbols_count);
    LOAD_TOML_STR_ARRAY(hl_table, "built_in_types", built_in_types_array, built_in_types_array_len, built_in_types, built_in_types_count);

    lexer->comment_single_prefix = comment_single_prefix;  
    lexer->comment_multi_begin = comment_multi_begin;   
    lexer->comment_multi_end = comment_multi_end;
    lexer->additional_colors = additional_colors.u.b;
    lexer->keywords_count = keywords_count;
    lexer->keywords = keywords;
    lexer->symbols_count = symbols_count;
    lexer->symbols = symbols;
    lexer->built_in_types_count = built_in_types_count;
    lexer->built_in_types = built_in_types;

    lexer->file_type = file_type;

    toml_free(hl_conf);
}

void lexerUpdateFileType(Lexer *lexer, FileType file_type) {
    lexer->file_type = file_type;

    lexerDestroy(lexer);
    lexerInit(lexer);

    if (file_type != FILE_TYPE_UNKNOWN) {
        loadHighlightingInfo(lexer, file_type);
    }
}

Token *lex(Lexer *lexer, const char *source, ColorTheme theme) {
    size_t length = strlen(source);
    size_t capacity = 10;
    lexer->token_count = 0;
    lexer->tokens = (Token *)malloc(capacity * sizeof(Token));

    if (!lexer->tokens) {
        return NULL;
    }

    if (lexer->file_type == FILE_TYPE_UNKNOWN) {
        // If the file type is unknown, don't highlight anything.
        for (size_t j = 0; j < length; j++) {
            if (lexer->token_count >= capacity) {
                capacity *= 2;
                Token *new_tokens = (Token *)realloc(lexer->tokens, capacity * sizeof(Token));
                if (!new_tokens) {
                    free(lexer->tokens);
                    return NULL;
                }
                lexer->tokens = new_tokens;
            }
            lexer->tokens[lexer->token_count].character = source[j];
            lexer->tokens[lexer->token_count].color = theme.foreground_color;
            (lexer->token_count)++;
        }
    } else {
        size_t i = 0;
        while (i < length) {
            size_t start = i;
            unsigned int color = theme.foreground_color;

            // Handle single-line comments
            if (strncmp(&source[i], lexer->comment_single_prefix.u.s, strlen(lexer->comment_single_prefix.u.s)) == 0) {
                while (i < length && source[i] != '\n') {
                    i++;
                }
                color = theme.comment_single_color;
            }
            // Handle multi-line comments
            else if (strncmp(&source[i], lexer->comment_multi_begin.u.s, strlen(lexer->comment_multi_begin.u.s)) == 0) {
                i += strlen(lexer->comment_multi_begin.u.s);
                while (i < length && strncmp(&source[i], lexer->comment_multi_end.u.s, strlen(lexer->comment_multi_end.u.s)) != 0) {
                    i++;
                }
                if (i < length) {
                    i += strlen(lexer->comment_multi_end.u.s);
                }
                color = theme.comment_multi_color;
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
                color = theme.string_literal_double_color;
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
                color = theme.string_literal_single_color;
            }
            // Handle digits/numbers
            else if (isdigit((unsigned char)source[i])) {
                while (i < length && isdigit((unsigned char)source[i])) {
                    i++;
                }
                color = theme.number_color;
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
                    if (is_builtin_type(lexer, word)) {
                        color = theme.built_in_type_color;
                    } else if (is_keyword(lexer, word)) {
                        color = theme.keyword_color;
                    }else if (is_function_name(source, start, i - start)) {
                        color = theme.function_name_color;
                    } 
                    else if (is_type(source, start, i - start) && lexer->additional_colors) {
                        color = theme.type_color;
                    } else {
                        color = theme.foreground_color;
                    }

                    free(word);
                }
            }
            // Handle symbols
            else if (is_symbol(lexer, source[i])) {
                i++;
                color = theme.symbol_color;
            }
            // Handle whitespace
            else if (isspace((unsigned char)source[i])) {
                color = theme.foreground_color;
                i++;
            }
            // Handle unknown characters
            else {
                color = theme.foreground_color;
                i++;
            }

            // Add tokens character by character
            for (size_t j = start; j < i; j++) {
                if (lexer->token_count >= capacity) {
                    capacity *= 2;
                    Token *new_tokens = (Token *)realloc(lexer->tokens, capacity * sizeof(Token));
                    if (!new_tokens) {
                        free(lexer->tokens);
                        return NULL;
                    }
                    lexer->tokens = new_tokens;
                }
                lexer->tokens[lexer->token_count].character = source[j];
                lexer->tokens[lexer->token_count].color = color;
                (lexer->token_count)++;
            }
        }
    }
    return lexer->tokens;
}
