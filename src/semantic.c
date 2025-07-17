#include "../include/semantic.h"

// Forward declarations
static bool analyze_node(SemanticAnalyzer* analyzer, AstNode* node);
static bool analyze_variable_declaration(SemanticAnalyzer* analyzer, AstNode* node);
static bool check_types_compatible(ValueType expected, ValueType actual);
static ValueType get_node_type(AstNode* node);

// Symbol table operations
static void add_symbol(SymbolTable* table, const char* name, ValueType type);
static SymbolEntry* find_symbol(SymbolTable* table, const char* name);
static void free_symbol_table(SymbolTable* table);

// Initialize semantic analyzer
void init_semantic_analyzer(SemanticAnalyzer* analyzer) {
    analyzer->symbol_table.head = NULL;
    analyzer->had_error = false;
}

// Analyze AST for semantic errors
bool analyze(SemanticAnalyzer* analyzer, AstNode* node) {
    log_message(LOG_DEBUG, "Starting semantic analysis");
    
    bool result = analyze_node(analyzer, node);
    
    return result && !analyzer->had_error;
}

// Analyze a node based on its type
static bool analyze_node(SemanticAnalyzer* analyzer, AstNode* node) {
    if (node == NULL) {
        return true;
    }
    
    switch (node->type) {
        case NODE_VARIABLE_DECLARATION:
            return analyze_variable_declaration(analyzer, node);
        case NODE_LITERAL:
            return true; // Literals are always valid
        default:
            log_message(LOG_ERROR, "Unknown node type in semantic analysis");
            return false;
    }
}

// Analyze variable declaration
static bool analyze_variable_declaration(SemanticAnalyzer* analyzer, AstNode* node) {
    log_message(LOG_DEBUG, "Analyzing variable declaration: %s", node->as.var_decl.name);
    
    // Check if variable already exists
    if (find_symbol(&analyzer->symbol_table, node->as.var_decl.name) != NULL) {
        set_error(ERROR_NAME, node->line, node->column,
                 "Variable already declared", NULL, 0, 0);
        analyzer->had_error = true;
        return false;
    }
    
    // Add variable to symbol table
    add_symbol(&analyzer->symbol_table, node->as.var_decl.name, node->as.var_decl.var_type);
    
    // Check initializer
    AstNode* initializer = node->as.var_decl.initializer;
    if (initializer != NULL) {
        // Get initializer type
        ValueType init_type = get_node_type(initializer);
        
        // Check if types are compatible
        if (!check_types_compatible(node->as.var_decl.var_type, init_type)) {
            char message[100];
            snprintf(message, sizeof(message), 
                    "Type mismatch: cannot assign %s to variable of type %s",
                    value_type_to_string(init_type),
                    value_type_to_string(node->as.var_decl.var_type));
            
            set_error(ERROR_TYPE, initializer->line, initializer->column,
                     message, NULL, 0, 0);
            analyzer->had_error = true;
            return false;
        }
    }
    
    return true;
}

// Check if two types are compatible for assignment
static bool check_types_compatible(ValueType expected, ValueType actual) {
    // Same types are always compatible
    if (expected == actual) {
        return true;
    }
    
    // NULL can be assigned to any type
    if (actual == VALUE_NULL) {
        return true;
    }
    
    // Type conversion table
    static const bool compatibility_table[5][5] = {
        // NULL   INT    FLOAT  BOOL   STRING
        {  true,  false, false, false, false }, // NULL
        {  false, true,  true,  false, false }, // INT
        {  false, false, true,  false, false }, // FLOAT
        {  false, false, false, true,  false }, // BOOL
        {  false, false, false, false, true  }  // STRING
    };
    
    return compatibility_table[expected][actual];
}

// Get the type of a node
static ValueType get_node_type(AstNode* node) {
    switch (node->type) {
        case NODE_LITERAL:
            return node->as.literal.type;
        case NODE_VARIABLE_DECLARATION:
            return node->as.var_decl.var_type;
        default:
            return VALUE_NULL;
    }
}

// Add a symbol to the symbol table
static void add_symbol(SymbolTable* table, const char* name, ValueType type) {
    SymbolEntry* entry = malloc(sizeof(SymbolEntry));
    entry->name = strdup(name);
    entry->type = type;
    entry->next = table->head;
    table->head = entry;
    
    log_message(LOG_DEBUG, "Added symbol: %s (type: %s)", 
               name, value_type_to_string(type));
}

// Find a symbol in the symbol table
static SymbolEntry* find_symbol(SymbolTable* table, const char* name) {
    for (SymbolEntry* entry = table->head; entry != NULL; entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            return entry;
        }
    }
    return NULL;
}

// Free the symbol table
static void free_symbol_table(SymbolTable* table) {
    SymbolEntry* current = table->head;
    while (current != NULL) {
        SymbolEntry* next = current->next;
        free(current->name);
        free(current);
        current = next;
    }
    table->head = NULL;
}

// Clean up semantic analyzer
void free_semantic_analyzer(SemanticAnalyzer* analyzer) {
    free_symbol_table(&analyzer->symbol_table);
}
