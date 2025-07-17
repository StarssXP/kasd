#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

// Node types for AST
typedef enum {
    NODE_VARIABLE_DECLARATION,
    NODE_LITERAL
} NodeType;

// AST node structure
typedef struct AstNode {
    NodeType type;
    int line;
    int column;
    
    union {
        // Variable declaration
        struct {
            char* name;
            ValueType var_type;
            struct AstNode* initializer;
        } var_decl;
        
        // Literal value
        Value literal;
    } as;
} AstNode;

// Parser structure
typedef struct {
    Lexer* lexer;
    Token current;
    Token previous;
    bool had_error;
    bool panic_mode;
} Parser;

// Initialize parser with a lexer
void init_parser(Parser* parser, Lexer* lexer);

// Parse source code into AST
AstNode* parse(Parser* parser);

// Free AST nodes
void free_ast(AstNode* node);

// Debug print AST
void print_ast(AstNode* node, int indent);

#endif // PARSER_H
