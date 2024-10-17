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

static int is_secondary_keyword(Lexer *lexer, const char *word) {
    for (size_t i = 0; i < lexer->secondary_keywords_count; i++) {
        char *sec_keyword = lexer->secondary_keywords[i];
        if (strcmp(word, sec_keyword) == 0) {
            return 1;
        }
    }
    return 0;
}

static int is_preproc_directive(Lexer *lexer, const char *word) {
    for (size_t i = 0; i < lexer->preproc_directives_count; i++) {
        char *preprocessor = lexer->preproc_directives[i];
        if (strcmp(word, preprocessor) == 0) {
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

static int isHexNumber(char source) {
    const char *matches = "abcedfABCEDF";
    if (isdigit(source) || strchr(matches, source) != NULL) {
        return 1;
    }
    return 0;
}

static int isOctalNumber(char source) {
    const char *matches = "01234567";
    if (strchr(matches, source) != NULL) {
        return 1;
    }
    return 0;
}

static int is_built_in_type(Lexer *lexer, const char *word) {
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

Token createToken() {
    Token token;
    token.text = (char *)malloc(sizeof(char) );
    token.text[0] = '\0';
    token.type = TOKEN_UNKNOWN;
    return token;
}
void tokenDestroy(Token * token) {
    if (token->text)
        free(token->text);
}

void tokenPushChar(Token *token, char c) {
    if (!token->text) {
        token->text = (char *)malloc(2 * sizeof(char));
        token->text[0] = c;
        token->text[1] = '\0';
    } else {
        size_t len = strlen(token->text);
        token->text = (char *)realloc(token->text, (len + 2) * sizeof(char));
        token->text[len] = c;
        token->text[len + 1] = '\0';
    }
}

void lexerInit(Lexer* lexer) {
    lexer->tokens = NULL;
    lexer->token_count = 0;
    lexer->capacity = 10;
    lexer->file_type = FILE_TYPE_UNKNOWN;
        // sizes of data buffers
    lexer->keywords_count = 0;
    lexer->symbols_count = 0;
    lexer->built_in_types_count = 0;
    lexer->preproc_directives_count = 0;
    lexer->secondary_keywords_count = 0;

    // data buffers
    lexer->keywords = NULL;
    lexer->symbols = NULL;
    lexer->built_in_types = NULL;
    lexer->preproc_directives = NULL;
    lexer->secondary_keywords = NULL;

    // Comment stuff
    lexer->comment_single_prefix.u.s = NULL;
    lexer->comment_single_prefix.u.ts = NULL;
    lexer->comment_multi_begin.u.s = NULL;
    lexer->comment_multi_begin.u.ts = NULL;
    lexer->comment_multi_end.u.s = NULL;
    lexer->comment_multi_end.u.ts = NULL;

    lexer->id_heuristics = false;
}

void lexerDestroy(Lexer *lexer) {
    if (lexer->tokens) {
        for (size_t i = 0; i < lexer->token_count; i++) {
            tokenDestroy(&lexer->tokens[i]);
        }
        free(lexer->tokens);
    }
        
    if (lexer->keywords) {
        for (size_t i = 0; i< lexer->keywords_count; i++) {
            free(lexer->keywords[i]);
        }
        free(lexer->keywords);
    }
    
    if (lexer->symbols) {
        for (size_t i = 0; i< lexer->symbols_count; i++) {
            free(lexer->symbols[i]);
        }
        free(lexer->symbols);
    }

    if (lexer->built_in_types) {
        for (size_t i = 0; i< lexer->built_in_types_count; i++) {
            free(lexer->built_in_types[i]);
        }
        free(lexer->built_in_types);
    }

    if (lexer->secondary_keywords) {
        for (size_t i = 0; i< lexer->secondary_keywords_count; i++) {
            free(lexer->secondary_keywords[i]);
        }
        free(lexer->secondary_keywords);
    }

    if (lexer->preproc_directives) {
        for (size_t i = 0; i< lexer->preproc_directives_count; i++) {
            free(lexer->preproc_directives[i]);
        }
        free(lexer->preproc_directives);
    }

    if (lexer->comment_single_prefix.u.s)
        free(lexer->comment_single_prefix.u.s);

    if (lexer->comment_multi_begin.u.s)
        free(lexer->comment_multi_begin.u.s);

    if (lexer->comment_multi_end.u.s)
        free(lexer->comment_multi_end.u.s);
}

void lexerClearTokens(Lexer *lexer) {
    if (lexer->tokens) {
        for (size_t i = 0; i < lexer->token_count; i++) {
            tokenDestroy(&lexer->tokens[i]);
        }
    }
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
    LOAD_TOML_STR(hl_table, comment_single_prefix);
    LOAD_TOML_STR(hl_table, comment_multi_begin);
    LOAD_TOML_STR(hl_table, comment_multi_end);
    LOAD_TOML_STR_ARRAY(hl_table, "keywords", keywords_array, keywords_array_len, keywords, keywords_count);
    LOAD_TOML_STR_ARRAY(hl_table, "symbols", symbols_array, symbols_array_len, symbols, symbols_count);
    LOAD_TOML_STR_ARRAY(hl_table, "built_in_types", built_in_types_array, built_in_types_array_len, built_in_types, built_in_types_count);
    LOAD_TOML_STR_ARRAY(hl_table, "preprocessor_directives", preproc_directives_array, preproc_directives_array_len, preproc_directives, preproc_directives_count);
    LOAD_TOML_STR_ARRAY(hl_table, "secondary_keywords", secondary_keywords_array, secondary_keywords_array_len, secondary_keywords, secondary_keywords_count);
    LOAD_TOML_BOOL(hl_table, identifier_heuristics);

    lexer->comment_single_prefix = comment_single_prefix;  
    lexer->comment_multi_begin = comment_multi_begin;   
    lexer->comment_multi_end = comment_multi_end;
    lexer->keywords_count = keywords_count;
    lexer->keywords = keywords;
    lexer->symbols_count = symbols_count;
    lexer->symbols = symbols;
    lexer->built_in_types_count = built_in_types_count;
    lexer->built_in_types = built_in_types;
    lexer->preproc_directives_count = preproc_directives_count;
    lexer->preproc_directives = preproc_directives;
    lexer->secondary_keywords_count = secondary_keywords_count;
    lexer->secondary_keywords = secondary_keywords;
    lexer->id_heuristics = identifier_heuristics.u.b;

    lexer->file_type = file_type;

    toml_free(hl_conf);
}

static void pushToken(Lexer *lexer, Token token) {
    if (strlen(token.text) == 0) {
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

void lexerUpdateFileType(Lexer *lexer, FileType file_type) {
    lexer->file_type = file_type;

    lexerDestroy(lexer);
    lexerInit(lexer);

    if (file_type != FILE_TYPE_UNKNOWN) {
        loadHighlightingInfo(lexer, file_type);
    }
}

static void refreshToken(Lexer *lexer, Token *curToken, TokenType new_type) {
    if (curToken->type == new_type && strlen(curToken->text) != 0) {
        pushToken(lexer, *curToken);
        *curToken = createToken();
    }
}

void lex (Lexer *lexer, const char *source) {
    size_t length = strlen(source);
    bool preprocessor = false;

    switch (lexer->file_type) {
        case FILE_TYPE_C:
            preprocessor = true;
        break;
        default:
            preprocessor = false;
        break;
    }

    if (lexer->tokens) {
        lexerClearTokens(lexer);
    }
    lexer->token_count = 0;
    
    if (lexer->file_type == FILE_TYPE_UNKNOWN) {
        // If we don't know what file type it is, don't do anything.
        Token curToken = createToken();
        for (size_t i = 0; i < length; i++) {
            tokenPushChar(&curToken, source[i]);

            // If we encounter a newline, start a new token.
            // this doesn't change highlighting, it helps with
            // visualizing the user selection (we don't have to worry about multi-
            // line tokens). 
            if (source[i] == '\n') {
                curToken.type = TOKEN_UNKNOWN;
                pushToken(lexer, curToken);
                curToken = createToken();
                refreshToken(lexer, &curToken, TOKEN_UNKNOWN);
            }
        }
        if (strlen(curToken.text) != 0) {
            pushToken(lexer, curToken);
        }
        LOG_DEBUG("Lexing done", "");
    } else {
        size_t i = 0;
        Token curToken = createToken();
        while (i < length) {
            // Handle single-line comments
            if (lexer->comment_single_prefix.ok && strncmp(&source[i], lexer->comment_single_prefix.u.s, strlen(lexer->comment_single_prefix.u.s)) == 0) {
                refreshToken(lexer, &curToken, TOKEN_COMMENT_SINGLE);

                while (i < length && source[i] != '\n') {
                    tokenPushChar(&curToken, source[i]);
                    i++;
                }
                curToken.type = TOKEN_COMMENT_SINGLE;
                pushToken(lexer, curToken);
                curToken = createToken();
            } 
            // Handle multiline comments
            else if (lexer->comment_multi_begin.ok && strncmp(&source[i], lexer->comment_multi_begin.u.s, strlen(lexer->comment_multi_begin.u.s)) == 0) {
                refreshToken(lexer, &curToken, TOKEN_COMMENT_MULTI);

                // Add the characters in the multiline comment.
                while (i < length && strncmp(&source[i], lexer->comment_multi_end.u.s, strlen(lexer->comment_multi_end.u.s)) != 0) {
                    tokenPushChar(&curToken, source[i]);

                    // If we encounter a newline, start a new token.
                    // this doesn't change highlighting, it helps with
                    // visualizing the user selection (we don't have to worry about multi-
                    // line tokens). 
                    if (source[i] == '\n') {
                        curToken.type = TOKEN_COMMENT_MULTI;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                        refreshToken(lexer, &curToken, TOKEN_COMMENT_MULTI);
                    }
                    i++;
                }
                
                if (strncmp(&source[i], lexer->comment_multi_end.u.s, strlen(lexer->comment_multi_end.u.s)) == 0) {
                    tokenPushChar(&curToken, source[i]);
                    tokenPushChar(&curToken, source[i+1]);
                    i+=2;
                }

                curToken.type = TOKEN_COMMENT_MULTI;
                pushToken(lexer, curToken);
                curToken = createToken();
            } 
            // Handle double-quote string literals
            else if (source[i] == '"') {
                refreshToken(lexer, &curToken, TOKEN_STRING_LITERAL_DOUBLE);

                // Push the first "
                tokenPushChar(&curToken, source[i]);
                i++;

                while (i < length && source[i] != '"') {
                    //Escaped characters
                    if (source[i] == '\\' && i + 1 < length) {
                        curToken.type = TOKEN_STRING_LITERAL_DOUBLE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                        
                        tokenPushChar(&curToken, source[i]);
                        i++;

                         if (source[i] == 'a' || source[i] == 'b' || source[i] == 'e' || source[i] == 'f'
                            || source[i] == 'n' || source[i] == 'n' || source[i] == 'r' || source[i] == 't'
                            || source[i] == 'v' || source[i] == '\\' || source[i] == '\'' || source[i] == '\"'
                            || source[i] == '\?') {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        }

                        curToken.type = TOKEN_ESCAPE_SEQUENCE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else if (source[i] == '\n'){
                        curToken.type = TOKEN_STRING_LITERAL_DOUBLE;
                        pushToken(lexer, curToken);
                        curToken = createToken();

                        tokenPushChar(&curToken, source[i]);
                        i++;

                        curToken.type = TOKEN_NEW_LINE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else {
                        tokenPushChar(&curToken, source[i]);
                        i++;
                    }
                }

                // Push the last "
                if (i < length && source[i] == '"') {
                    tokenPushChar(&curToken, source[i]);
                    i++;
                }
                
                curToken.type = TOKEN_STRING_LITERAL_DOUBLE;
                pushToken(lexer, curToken);
                curToken = createToken();
            }
            // Handle single-quote string literals
            else if (source[i] == '\'') {
                refreshToken(lexer, &curToken, TOKEN_STRING_LITERAL_SINGLE);

                // Push the first '
                tokenPushChar(&curToken, source[i]);
                i++;

                while (i < length && source[i] != '\'') {
                    //Escaped characters
                    if (source[i] == '\\' && i + 1 < length) {
                        curToken.type = TOKEN_STRING_LITERAL_SINGLE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                        
                        tokenPushChar(&curToken, source[i]);
                        i++;

                         if (source[i] == 'a' || source[i] == 'b' || source[i] == 'e' || source[i] == 'f'
                            || source[i] == 'n' || source[i] == 'n' || source[i] == 'r' || source[i] == 't'
                            || source[i] == 'v' || source[i] == '\\' || source[i] == '\'' || source[i] == '\"'
                            || source[i] == '\?') {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        }

                        curToken.type = TOKEN_ESCAPE_SEQUENCE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else if (source[i] == '\n'){
                        curToken.type = TOKEN_STRING_LITERAL_DOUBLE;
                        pushToken(lexer, curToken);
                        curToken = createToken();

                        tokenPushChar(&curToken, source[i]);
                        i++;

                        curToken.type = TOKEN_NEW_LINE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else {
                        tokenPushChar(&curToken, source[i]);
                        i++;
                    }
                }

                // Push the last '
                if (i < length && source[i] == '\'') {
                    tokenPushChar(&curToken, source[i]);
                    i++;
                }
                curToken.type = TOKEN_STRING_LITERAL_SINGLE;
                pushToken(lexer, curToken);
                curToken = createToken();
            }
            // Handles digits/numbers
            else if (isdigit((unsigned char)source[i])) {
                refreshToken(lexer, &curToken, TOKEN_NUMBER);

                // Handle hex, octal, and binary numbers
                if (source[i] == '0') {
                    tokenPushChar(&curToken, source[i]);
                    i++;

                    if (source[i] == 'x' || source[i] == 'X') {
                        // Handle hex numbers
                        tokenPushChar(&curToken, source[i]);
                        i++;
                        while (i < length && isHexNumber((unsigned char)source[i])) {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        }
                    } else if (source[i] == 'b' || source[i] == 'B') {
                        // Handle binary numbers
                        tokenPushChar(&curToken, source[i]);
                        i++;

                        while (i < length && (source[i] == '0' || source[i] == '1')) {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        }
                    } else if (isOctalNumber(source[i])) {
                        // Handle octal numbers
                        tokenPushChar(&curToken, source[i]);
                        i++;

                        while (i < length && isOctalNumber(source[i])) {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        }
                    } else if (source[i] == '.') {
                        // Handle decimal numbers
                        tokenPushChar(&curToken, source[i]);
                        i++;

                        while (i < length && isdigit(source[i])) {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        }
                    }
                } else {
                    bool hit_decimal = false;
                    while (i < length) {
                        if (isdigit((unsigned char)source[i])) {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                        } else if (source[i] == '.' && !hit_decimal) {
                            tokenPushChar(&curToken, source[i]);
                            i++;
                            hit_decimal = true;
                        } else {
                            break;
                        }
                        
                    }
                }
                curToken.type = TOKEN_NUMBER;
                pushToken(lexer, curToken);
                curToken = createToken();
            }
            // Handle preprocessor directives
            else if (preprocessor && source[i] == '#') {
                refreshToken(lexer, &curToken, TOKEN_PREPROCESSOR_DIRECTIVE);

                // consume the # character
                tokenPushChar(&curToken, source[i]);
                i++;

                while (i < length && isalpha(source[i])) {
                    tokenPushChar(&curToken, source[i]);
                    i++;
                }

                if (is_preproc_directive(lexer, curToken.text)) {
                    curToken.type = TOKEN_PREPROCESSOR_DIRECTIVE;
                    pushToken(lexer, curToken);
                    curToken = createToken();
                } else {
                    curToken.type = TOKEN_UNKNOWN;
                    pushToken(lexer, curToken);
                    curToken = createToken();
                }
            }
            // Handle symbols
            else if (is_symbol(lexer, source[i])) {
                if (curToken.type != TOKEN_SYMBOL && strlen(curToken.text) != 0) {
                    pushToken(lexer, curToken);
                    curToken = createToken();
                }

                tokenPushChar(&curToken, source[i]);
                i++;
                curToken.type = TOKEN_SYMBOL;
                pushToken(lexer, curToken);
                curToken = createToken();
            }
            else {
                // Handle new lines
                if (source[i] == '\n') {
                    refreshToken(lexer, &curToken, TOKEN_NEW_LINE);
                    tokenPushChar(&curToken, source[i]);
                    i++;
                    curToken.type = TOKEN_NEW_LINE;
                    pushToken(lexer, curToken);
                    curToken = createToken();

                // Handle whitespace
                } else if (source[i] == ' ' || source[i] == '\t') {
                    refreshToken(lexer, &curToken, TOKEN_WHITESPACE);
                    tokenPushChar(&curToken, source[i]);
                    i++;
                    curToken.type = TOKEN_WHITESPACE;
                    pushToken(lexer, curToken);
                    curToken = createToken();
                } else if (!is_symbol(lexer, source[i]) && !isspace(source[i]) && source[i] != ',' && source[i] != '.' && source[i] != ';'){
                    refreshToken(lexer, &curToken, TOKEN_IDENTIFER);
                    size_t start = i;
                    while (i < length && !is_symbol(lexer, source[i]) && !isspace(source[i]) && source[i] != ',' && source[i] != '.' && source[i] != ';') {
                        tokenPushChar(&curToken, source[i]);
                        if (ispunct(source[i]) && source[i] != '_') {
                            i++;
                            break;   
                        }
                        i++;
                    }

                    if (is_keyword(lexer, curToken.text)) {
                        curToken.type = TOKEN_KEYWORD;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else if (is_secondary_keyword(lexer, curToken.text)) {
                        curToken.type = TOKEN_SECONDARY_KEYWORD;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else if (is_built_in_type(lexer, curToken.text)) {
                        curToken.type = TOKEN_BUILT_IN_TYPE;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else if (lexer->id_heuristics && is_function_name(source, start, i - start)) {
                        curToken.type = TOKEN_FUNCTION_NAME;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else if (lexer->id_heuristics && is_type(source, start, i-start)) {
                        curToken.type = TOKEN_TYPE_NAME;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    } else {
                        curToken.type = TOKEN_UNKNOWN;
                        pushToken(lexer, curToken);
                        curToken = createToken();
                    }
                } else {
                    refreshToken(lexer, &curToken, TOKEN_UNKNOWN);
                    tokenPushChar(&curToken, source[i]);
                    i++;
                    curToken.type = TOKEN_UNKNOWN;
                    pushToken(lexer, curToken);
                    curToken = createToken();
                }
            }
        }
        if (strlen(curToken.text) != 0) {
            pushToken(lexer, curToken);
        }
        LOG_DEBUG("Lexing done", "");
    }
}
