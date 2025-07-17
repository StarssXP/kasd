#include "../include/parser.h"

// Forward declarations
static AstNode* parse_declaration(Parser* parser);
static AstNode* parse_variable_declaration(Parser* parser);
static AstNode* parse_expression(Parser* parser);
static AstNode* parse_literal(Parser* parser);
static ValueType token_to_value_type(TokenType type);

// Advance to the next token
static void advance(Parser* parser) {
    parser->previous = parser->current;
    parser->current = scan_token(parser->lexer);
    
    log_message(LOG_DEBUG, "Advanced to token: %s", token_type_to_string(parser->current.type));
}

// Check if the current token is of the given type
static bool check(Parser* parser, TokenType type) {
    return parser->current.type == type;
}

// Consume the current token if it matches the expected type
static bool match(Parser* parser, TokenType type) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    return false;
}

// Expect the current token to be of the given type
static bool consume(Parser* parser, TokenType type, const char* message) {
    if (check(parser, type)) {
        advance(parser);
        return true;
    }
    
    set_error(ERROR_SYNTAX, parser->current.line, parser->current.column, 
              message, parser->lexer->source, 
              (int)(parser->current.start - parser->lexer->source), 
              parser->current.length);
    
    parser->had_error = true;
    return false;
}

// Create a new AST node
static AstNode* create_node(NodeType type, int line, int column) {
    AstNode* node = malloc(sizeof(AstNode));
    node->type = type;
    node->line = line;
    node->column = column;
    return node;
}

// Initialize parser with a lexer
void init_parser(Parser* parser, Lexer* lexer) {
    parser->lexer = lexer;
    parser->had_error = false;
    parser->panic_mode = false;
    advance(parser); // Prime the parser with the first token
}

// Parse source code into AST
AstNode* parse(Parser* parser) {
    log_message(LOG_DEBUG, "Starting parsing");
    
    AstNode* node = parse_declaration(parser);
    
    // Check for end of file
    if (!check(parser, TOKEN_EOF) && !parser->had_error) {
        set_error(ERROR_SYNTAX, parser->current.line, parser->current.column,
                 "Expected end of file.", parser->lexer->source,
                 (int)(parser->current.start - parser->lexer->source),
                 parser->current.length);
        parser->had_error = true;
    }
    
    return node;
}

// Parse a declaration
static AstNode* parse_declaration(Parser* parser) {
    log_message(LOG_DEBUG, "Parsing declaration");
    
    // Currently we only support variable declarations
    return parse_variable_declaration(parser);
}

// Parse a variable declaration: let name: type = value;
static AstNode* parse_variable_declaration(Parser* parser) {
    log_message(LOG_DEBUG, "Parsing variable declaration");
    
    // Check for 'let' keyword
    if (!consume(parser, TOKEN_LET, "Expected 'let' keyword.")) {
        return NULL;
    }
    
    // Get variable name
    if (!consume(parser, TOKEN_IDENTIFIER, "Expected variable name.")) {
        return NULL;
    }
    
    // Save the identifier token
    Token name_token = parser->previous;
    
    // Check for colon
    if (!consume(parser, TOKEN_COLON, "Expected ':' after variable name.")) {
        return NULL;
    }
    
    // Get variable type
    TokenType type_tokens[] = {
        TOKEN_TYPE_INT, TOKEN_TYPE_FLOAT, TOKEN_TYPE_BOOL, 
        TOKEN_TYPE_STRING, TOKEN_TYPE_NULL
    };
    
    bool found_type = false;
    TokenType type_token = TOKEN_EOF;
    
    for (int i = 0; i < 5; i++) {
        if (match(parser, type_tokens[i])) {
            found_type = true;
            type_token = type_tokens[i];
            break;
        }
    }
    
    if (!found_type) {
        set_error(ERROR_SYNTAX, parser->current.line, parser->current.column,
                 "Expected type (int, float, bool, string, or null).", 
                 parser->lexer->source,
                 (int)(parser->current.start - parser->lexer->source),
                 parser->current.length);
        parser->had_error = true;
        return NULL;
    }
    
    // Convert token type to value type
    ValueType var_type = token_to_value_type(type_token);
    
    // Check for equals sign
    if (!consume(parser, TOKEN_EQUAL, "Expected '=' after type.")) {
        return NULL;
    }
    
    // Parse initializer expression
    AstNode* initializer = parse_expression(parser);
    if (initializer == NULL) {
        return NULL;
    }
    
    // Check for semicolon
    if (!consume(parser, TOKEN_SEMICOLON, "Expected ';' after variable declaration.")) {
        free_ast(initializer);
        return NULL;
    }
    
    // Create variable declaration node
    AstNode* node = create_node(NODE_VARIABLE_DECLARATION, name_token.line, name_token.column);
    node->as.var_decl.name = strndup(name_token.start, name_token.length);
    node->as.var_decl.var_type = var_type;
    node->as.var_decl.initializer = initializer;
    
    return node;
}

// Parse an expression
static AstNode* parse_expression(Parser* parser) {
    log_message(LOG_DEBUG, "Parsing expression");
    
    // Currently we only support literals
    return parse_literal(parser);
}

// Parse a literal value
static AstNode* parse_literal(Parser* parser) {
    log_message(LOG_DEBUG, "Parsing literal");
    
    AstNode* node = NULL;
    
    // Handle different literal types
    switch (parser->current.type) {
        case TOKEN_INT: {
            node = create_node(NODE_LITERAL, parser->current.line, parser->current.column);
            node->as.literal = create_int_value(parser->current.value.as_int);
            advance(parser);
            break;
        }
        case TOKEN_FLOAT: {
            node = create_node(NODE_LITERAL, parser->current.line, parser->current.column);
            node->as.literal = create_float_value(parser->current.value.as_float);
            advance(parser);
            break;
        }
        case TOKEN_STRING: {
            node = create_node(NODE_LITERAL, parser->current.line, parser->current.column);
            node->as.literal = create_string_value(parser->current.value.as_string);
            advance(parser);
            break;
        }
        case TOKEN_TRUE: {
            node = create_node(NODE_LITERAL, parser->current.line, parser->current.column);
            node->as.literal = create_bool_value(true);
            advance(parser);
            break;
        }
        case TOKEN_FALSE: {
            node = create_node(NODE_LITERAL, parser->current.line, parser->current.column);
            node->as.literal = create_bool_value(false);
            advance(parser);
            break;
        }
        case TOKEN_NULL: {
            node = create_node(NODE_LITERAL, parser->current.line, parser->current.column);
            node->as.literal = create_null_value();
            advance(parser);
            break;
        }
        default: {
            set_error(ERROR_SYNTAX, parser->current.line, parser->current.column,
                     "Expected literal value.", parser->lexer->source,
                     (int)(parser->current.start - parser->lexer->source),
                     parser->current.length);
            parser->had_error = true;
            return NULL;
        }
    }
    
    return node;
}

// Convert token type to value type
static ValueType token_to_value_type(TokenType type) {
    static const ValueType type_map[] = {
        [TOKEN_TYPE_INT] = VALUE_INT,
        [TOKEN_TYPE_FLOAT] = VALUE_FLOAT,
        [TOKEN_TYPE_BOOL] = VALUE_BOOL,
        [TOKEN_TYPE_STRING] = VALUE_STRING,
        [TOKEN_TYPE_NULL] = VALUE_NULL
    };
    
    return type_map[type];
}

// Free AST nodes
void free_ast(AstNode* node) {
    if (node == NULL) {
        return;
    }
    
    switch (node->type) {
        case NODE_VARIABLE_DECLARATION:
            free(node->as.var_decl.name);
            free_ast(node->as.var_decl.initializer);
            break;
        case NODE_LITERAL:
            free_value(node->as.literal);
            break;
    }
    
    free(node);
}

// Debug print AST
void print_ast(AstNode* node, int indent) {
    if (node == NULL) {
        return;
    }
    
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    switch (node->type) {
        case NODE_VARIABLE_DECLARATION: {
            printf("VariableDeclaration: %s (type: %s)\n", 
                   node->as.var_decl.name, 
                   value_type_to_string(node->as.var_decl.var_type));
            
            print_ast(node->as.var_decl.initializer, indent + 1);
            break;
        }
        case NODE_LITERAL: {
            char* value_str = value_to_string(node->as.literal);
            printf("Literal: %s (type: %s)\n", 
                   value_str, 
                   value_type_to_string(node->as.literal.type));
            free(value_str);
            break;
        }
    }
}
