#include "../include/lexer.h"
#include <ctype.h>

// Keyword table
typedef struct {
    const char* name;
    TokenType type;
    int length;
} Keyword;

// Keyword lookup table
static Keyword keywords[] = {
    {"let", TOKEN_LET, 3},
    {"true", TOKEN_TRUE, 4},
    {"false", TOKEN_FALSE, 5},
    {"null", TOKEN_NULL, 4},
    {"int", TOKEN_TYPE_INT, 3},
    {"float", TOKEN_TYPE_FLOAT, 5},
    {"bool", TOKEN_TYPE_BOOL, 4},
    {"string", TOKEN_TYPE_STRING, 6},
};

#define KEYWORDS_COUNT (sizeof(keywords) / sizeof(keywords[0]))

// Character class table for fast lookup
typedef enum {
    CHAR_WHITESPACE,
    CHAR_ALPHA,
    CHAR_DIGIT,
    CHAR_SPECIAL,
    CHAR_QUOTE,
    CHAR_NEWLINE,
    CHAR_EOF
} CharClass;

static CharClass char_classes[256];

// Initialize character class table
static void init_char_classes(void) {
    // Default all to special
    for (int i = 0; i < 256; i++) {
        char_classes[i] = CHAR_SPECIAL;
    }
    
    // Set specific classes
    for (int i = 0; i < 256; i++) {
        if (isspace(i) && i != '\n') char_classes[i] = CHAR_WHITESPACE;
        if (isalpha(i) || i == '_') char_classes[i] = CHAR_ALPHA;
        if (isdigit(i)) char_classes[i] = CHAR_DIGIT;
        if (i == '"') char_classes[i] = CHAR_QUOTE;
        if (i == '\n') char_classes[i] = CHAR_NEWLINE;
        if (i == 0) char_classes[i] = CHAR_EOF;
    }
}

// Initialize lexer with source code
void init_lexer(Lexer* lexer, const char* source) {
    static bool classes_initialized = false;
    
    if (!classes_initialized) {
        init_char_classes();
        classes_initialized = true;
    }
    
    lexer->source = source;
    lexer->current = source;
    lexer->start = source;
    lexer->line = 1;
    lexer->column = 1;
    lexer->had_error = false;
}

// Check if we're at the end of the source
bool is_at_end(Lexer* lexer) {
    return *lexer->current == '\0';
}

// Advance the current pointer and return the character
static char advance(Lexer* lexer) {
    lexer->column++;
    return *lexer->current++;
}

// Peek at the current character without advancing
static char peek(Lexer* lexer) {
    return *lexer->current;
}

// Peek at the next character
static char peek_next(Lexer* lexer) {
    if (is_at_end(lexer)) return '\0';
    return lexer->current[1];
}

// Skip whitespace
static void skip_whitespace(Lexer* lexer) {
    while (1) {
        char c = peek(lexer);
        
        switch (char_classes[(unsigned char)c]) {
            case CHAR_WHITESPACE:
                advance(lexer);
                break;
            case CHAR_NEWLINE:
                lexer->line++;
                lexer->column = 1;
                advance(lexer);
                break;
            default:
                return;
        }
    }
}

// Make a token
static Token make_token(Lexer* lexer, TokenType type) {
    Token token;
    token.type = type;
    token.start = lexer->start;
    token.length = (int)(lexer->current - lexer->start);
    token.line = lexer->line;
    token.column = lexer->column - token.length;
    return token;
}

// Make an error token
static Token error_token(Lexer* lexer, const char* message) {
    lexer->had_error = true;
    
    set_error(ERROR_SYNTAX, lexer->line, lexer->column, message, 
              lexer->source, (int)(lexer->start - lexer->source), 
              (int)(lexer->current - lexer->start));
    
    Token token;
    token.type = TOKEN_EOF;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = lexer->line;
    token.column = lexer->column;
    return token;
}

// Check if the current lexeme matches a keyword
static TokenType check_keyword(Lexer* lexer) {
    size_t length = lexer->current - lexer->start;
    
    for (size_t i = 0; i < KEYWORDS_COUNT; i++) {
        if (length == (size_t)keywords[i].length && 
            memcmp(lexer->start, keywords[i].name, length) == 0) {
            return keywords[i].type;
        }
    }
    
    return TOKEN_IDENTIFIER;
}

// Handle identifiers and keywords
static Token identifier(Lexer* lexer) {
    while (char_classes[(unsigned char)peek(lexer)] == CHAR_ALPHA || 
           char_classes[(unsigned char)peek(lexer)] == CHAR_DIGIT) {
        advance(lexer);
    }
    
    return make_token(lexer, check_keyword(lexer));
}

// Handle numbers (integers and floats)
static Token number(Lexer* lexer) {
    TokenType type = TOKEN_INT;
    
    while (char_classes[(unsigned char)peek(lexer)] == CHAR_DIGIT) {
        advance(lexer);
    }
    
    // Look for a decimal point
    if (peek(lexer) == '.' && char_classes[(unsigned char)peek_next(lexer)] == CHAR_DIGIT) {
        type = TOKEN_FLOAT;
        
        // Consume the '.'
        advance(lexer);
        
        // Consume the fractional part
        while (char_classes[(unsigned char)peek(lexer)] == CHAR_DIGIT) {
            advance(lexer);
        }
    }
    
    Token token = make_token(lexer, type);
    
    // Parse the value
    char* end;
    if (type == TOKEN_INT) {
        token.value.as_int = strtoll(lexer->start, &end, 10);
    } else {
        token.value.as_float = strtod(lexer->start, &end);
    }
    
    return token;
}

// Handle strings
static Token string(Lexer* lexer) {
    // Skip the opening quote
    advance(lexer);
    
    // Mark the start of the string content
    const char* start = lexer->current;
    
    // Consume characters until closing quote or end of file
    while (peek(lexer) != '"' && !is_at_end(lexer)) {
        if (peek(lexer) == '\n') {
            lexer->line++;
            lexer->column = 1;
        }
        advance(lexer);
    }
    
    // Check for unterminated string
    if (is_at_end(lexer)) {
        return error_token(lexer, "Unterminated string.");
    }
    
    // Calculate string length
    int length = (int)(lexer->current - start);
    
    // Create the token
    Token token = make_token(lexer, TOKEN_STRING);
    
    // Copy the string value (without quotes)
    token.value.as_string = malloc(length + 1);
    memcpy(token.value.as_string, start, length);
    token.value.as_string[length] = '\0';
    
    // Skip the closing quote
    advance(lexer);
    
    return token;
}

// Scan the next token
Token scan_token(Lexer* lexer) {
    skip_whitespace(lexer);
    
    lexer->start = lexer->current;
    
    if (is_at_end(lexer)) {
        return make_token(lexer, TOKEN_EOF);
    }
    
    char c = advance(lexer);
    CharClass char_class = char_classes[(unsigned char)c];
    
    // Use character class for fast dispatch
    switch (char_class) {
        case CHAR_ALPHA: 
            return identifier(lexer);
        case CHAR_DIGIT: 
            return number(lexer);
        case CHAR_QUOTE: 
            return string(lexer);
        case CHAR_WHITESPACE:
        case CHAR_NEWLINE:
        case CHAR_EOF:
        case CHAR_SPECIAL:
            break; // Fall through to handle special characters below
    }
    
    // Handle single-character tokens
    switch (c) {
        case ':': return make_token(lexer, TOKEN_COLON);
        case '=': return make_token(lexer, TOKEN_EQUAL);
        case ';': return make_token(lexer, TOKEN_SEMICOLON);
    }
    
    // Unrecognized character
    char message[64];
    snprintf(message, sizeof(message), "Unexpected character: '%c'", c);
    return error_token(lexer, message);
}

// Convert token type to string
const char* token_type_to_string(TokenType type) {
    static const char* token_names[] = {
        "EOF",
        "IDENTIFIER",
        "INT",
        "FLOAT",
        "STRING",
        "LET",
        "COLON",
        "EQUAL",
        "SEMICOLON",
        "TRUE",
        "FALSE",
        "NULL",
        "TYPE_INT",
        "TYPE_FLOAT",
        "TYPE_BOOL",
        "TYPE_STRING",
        "TYPE_NULL"
    };
    
    return token_names[type];
}

// Print token for debugging
void print_token(Token token) {
    printf("Token: %s, Line: %d, Column: %d, Lexeme: '",
           token_type_to_string(token.type), token.line, token.column);
    
    for (int i = 0; i < token.length; i++) {
        putchar(token.start[i]);
    }
    
    printf("'\n");
}
