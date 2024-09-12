#include <errno.h>
#include "lexer.h"

// Highlighting files (hardcoded for now)
#define C_HIGHLIGHTING_FILE "./config/syntaxes/c.toml"
#define MAKEFILE_HIGHLIGHTING_FILE "./config/syntaxes/makefile.toml"
#define TOML_HIGHLIGHTING_FILE "./config/syntaxes/toml.toml"
#define PYTHON_HIGHLIGHTING_FILE "./config/syntaxes/python.toml"

static void error(const char* msg, const char* msg1)
{
    fprintf(stderr, "ERROR: %s%s\n", msg, msg1?msg1:"");
}

static int is_keyword(Lexer *lexer, const char *word) {
    for (size_t i = 0; i < lexer->keywords_count; i++) {
        toml_datum_t keyword = toml_string_at(lexer->keywords, i);
        if (strcmp(word, keyword.u.s) == 0) {
            free(keyword.u.s);
            return 1;
        } else {
            free(keyword.u.s);
        }
    }
    return 0;
}

static int is_symbol(Lexer *lexer, char character) {
    for (size_t i = 0; i < lexer->symbols_count; i++) {
        toml_datum_t symbol = toml_string_at(lexer->symbols, i);
        if (character == symbol.u.s[0]) {
            free(symbol.u.s);
            return 1;
        } else {
            free(symbol.u.s);
        }
    }
    return 0;
}

static int is_builtin_type(Lexer *lexer, const char *word) {
    for (size_t i = 0; i < lexer->built_in_types_count; i++) {
        toml_datum_t built_in_type = toml_string_at(lexer->built_in_types, i);
        if (strcmp(word, built_in_type.u.s) == 0) {
            free(built_in_type.u.s);
            return 1;
        } else {
            free(built_in_type.u.s);
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

Lexer *lexerInit() {
    Lexer *lexer = (Lexer*)malloc(sizeof(Lexer));
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

    return lexer;
}

void lexerDestroy(Lexer *lexer) {
    if (lexer->keywords)
        free(lexer->keywords);
    
    if (lexer->symbols)
        free(lexer->symbols);

    if (lexer->built_in_types)
        free(lexer->built_in_types);

    if (lexer->comment_single_prefix.u.s)
        free(lexer->comment_single_prefix.u.s);

    if (lexer->comment_multi_begin.u.s)
        free(lexer->comment_multi_begin.u.s);

    if (lexer->comment_multi_end.u.s)
        free(lexer->comment_multi_end.u.s);

    free(lexer);
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
        error("cannot open highlighting file - ", strerror(errno));
        return;
    }

    toml_table_t* hl_table = toml_table_in(hl_conf, "highlighting");
    if (!hl_conf) {
        error("Highlight file missing [highlighting]", "");
        return;
    }

    // Get data
    toml_datum_t comment_single_prefix = toml_string_in(hl_table, "comment_single_prefix");
    if (!comment_single_prefix.ok) {
        error("cannot read highlighting.comment_single_prefix", "");
        return;
    }
    lexer->comment_single_prefix = comment_single_prefix;
    printf("comment_single_prefix: %s\n", comment_single_prefix.u.s);


    toml_datum_t comment_multi_begin = toml_string_in(hl_table, "comment_multi_begin");
    if (!comment_multi_begin.ok) {
        error("cannot read highlighting.comment_multi_begin", "");
        return;
    }
    lexer->comment_multi_begin = comment_multi_begin;
    printf("comment_multi_begin: %s\n", comment_multi_begin.u.s);


    toml_datum_t comment_multi_end = toml_string_in(hl_table, "comment_multi_end");
    if (!comment_multi_end.ok) {
        error("cannot read highlighting.comment_multi_end", "");
        return;
    }
    lexer->comment_multi_end = comment_multi_end;
    printf("comment_multi_end: %s\n", comment_multi_end.u.s);

    toml_datum_t additional_colors = toml_bool_in(hl_table, "additional_colors");
    if (!additional_colors.ok) {
        error("cannot read highlighting.additional_colors", "");
        return;
    }
    lexer->additional_colors = additional_colors.u.b;
    printf("additional_colors: %d\n", additional_colors.u.b);


    toml_array_t* keywords_array = toml_array_in(hl_table, "keywords");
    if (!keywords_array) {
        error("cannot read highlighting.keywords", "");
        return;
    }
    lexer->keywords_count = toml_array_nelem(keywords_array);
    lexer->keywords = keywords_array;


    toml_array_t* symbols_array = toml_array_in(hl_table, "symbols");
    if (!symbols_array) {
        error("cannot read highlighting.symbols", "");
        return;
    }
    lexer->symbols_count = toml_array_nelem(symbols_array);
    lexer->symbols = symbols_array;


    toml_array_t* built_in_types_array = toml_array_in(hl_table, "built_in_types");
    if (!built_in_types_array) {
        error("cannot read highlighting.built_in_types", "");
        return;
    }
    lexer->built_in_types_count = toml_array_nelem(built_in_types_array);
    lexer->built_in_types = built_in_types_array;

    lexer->file_type = file_type;

    free(hl_conf);
}

void lexerUpdateFileType(Lexer *lexer, FileType file_type) {
    lexer->file_type = file_type;

    lexerDestroy(lexer);
    lexerInit(lexer);

    if (file_type != FILE_TYPE_UNKNOWN) {
        loadHighlightingInfo(lexer, file_type);
    }
}

Token *lex(Lexer *lexer, const char *source, size_t *token_count, ColorTheme theme) {
    size_t length = strlen(source);
    size_t capacity = 10;
    *token_count = 0;
    Token *tokens = (Token *)malloc(capacity * sizeof(Token));

    if (!tokens) {
        return NULL;
    }

    if (lexer->file_type == FILE_TYPE_UNKNOWN) {
        // If the file type is unknown, don't highlight anything.
        for (size_t j = 0; j < length; j++) {
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
            tokens[*token_count].color = theme.foreground_color;
            (*token_count)++;
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
    }
    return tokens;
}
