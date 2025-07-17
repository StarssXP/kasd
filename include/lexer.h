#ifndef LEXER_H
#define LEXER_H

#include "common.h"

// Token types
typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_LET,
    TOKEN_COLON,
    TOKEN_EQUAL,
    TOKEN_SEMICOLON,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NULL,
    TOKEN_TYPE_INT,
    TOKEN_TYPE_FLOAT,
    TOKEN_TYPE_BOOL,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_NULL
} TokenType;

// Token structure
typedef struct {
    TokenType type;
    const char* start;
    int length;
    int line;
    int column;
    union {
        int64_t as_int;
        double as_float;
        char* as_string;
    } value;
} Token;

// Lexer structure
typedef struct {
    const char* source;
    const char* current;
    const char* start;
    int line;
    int column;
    Token previous;
    bool had_error;
} Lexer;

// Initialize lexer with source code
void init_lexer(Lexer* lexer, const char* source);

// Get the next token
Token scan_token(Lexer* lexer);

// Utility functions
bool is_at_end(Lexer* lexer);
const char* token_type_to_string(TokenType type);
void print_token(Token token);

#endif // LEXER_H
