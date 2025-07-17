#include "../include/interpreter.h"

// Forward declarations
static Value evaluate_node(Interpreter* interpreter, AstNode* node);
static Value evaluate_variable_declaration(Interpreter* interpreter, AstNode* node);
static Value evaluate_literal(Interpreter* interpreter, AstNode* node);

// Environment operations
static void env_define(Environment* env, const char* name, Value value);
static void env_free(Environment* env);

// Initialize interpreter
void init_interpreter(Interpreter* interpreter, bool repl_mode) {
    interpreter->env.head = NULL;
    interpreter->had_error = false;
    interpreter->repl_mode = repl_mode;
}

// Execute AST
Value interpret(Interpreter* interpreter, AstNode* node) {
    log_message(LOG_DEBUG, "Starting interpretation");
    
    if (node == NULL) {
        return create_null_value();
    }
    
    return evaluate_node(interpreter, node);
}

// Evaluate a node based on its type
static Value evaluate_node(Interpreter* interpreter, AstNode* node) {
    switch (node->type) {
        case NODE_VARIABLE_DECLARATION:
            return evaluate_variable_declaration(interpreter, node);
        case NODE_LITERAL:
            return evaluate_literal(interpreter, node);
        default:
            log_message(LOG_ERROR, "Unknown node type in interpreter");
            interpreter->had_error = true;
            return create_null_value();
    }
}

// Evaluate a variable declaration
static Value evaluate_variable_declaration(Interpreter* interpreter, AstNode* node) {
    log_message(LOG_DEBUG, "Evaluating variable declaration: %s", node->as.var_decl.name);
    
    // Evaluate initializer
    Value value = evaluate_node(interpreter, node->as.var_decl.initializer);
    
    // Define variable in environment
    env_define(&interpreter->env, node->as.var_decl.name, value);
    
    // In REPL mode, print the variable
    if (interpreter->repl_mode) {
        char* value_str = value_to_string(value);
        printf("%s: %s = %s\n", 
               node->as.var_decl.name, 
               value_type_to_string(node->as.var_decl.var_type),
               value_str);
        free(value_str);
    }
    
    return value;
}

// Evaluate a literal
static Value evaluate_literal(Interpreter* interpreter __attribute__((unused)), AstNode* node) {
    log_message(LOG_DEBUG, "Evaluating literal");
    return node->as.literal;
}

// Define a variable in the environment
static void env_define(Environment* env, const char* name, Value value) {
    // Check if variable already exists
    for (EnvEntry* entry = env->head; entry != NULL; entry = entry->next) {
        if (strcmp(entry->name, name) == 0) {
            // Free old value if it's a string
            if (entry->value.type == VALUE_STRING && entry->value.data.as_string != NULL) {
                free(entry->value.data.as_string);
            }
            
            // Copy new value
            entry->value = value;
            
            // If it's a string, duplicate it
            if (value.type == VALUE_STRING && value.data.as_string != NULL) {
                entry->value.data.as_string = strdup(value.data.as_string);
            }
            
            return;
        }
    }
    
    // Create new entry
    EnvEntry* entry = malloc(sizeof(EnvEntry));
    entry->name = strdup(name);
    entry->value = value;
    
    // If it's a string, duplicate it
    if (value.type == VALUE_STRING && value.data.as_string != NULL) {
        entry->value.data.as_string = strdup(value.data.as_string);
    }
    
    // Add to environment
    entry->next = env->head;
    env->head = entry;
    
    log_message(LOG_DEBUG, "Defined variable: %s", name);
}

// Free the environment
static void env_free(Environment* env) {
    EnvEntry* current = env->head;
    while (current != NULL) {
        EnvEntry* next = current->next;
        
        free(current->name);
        
        // Free string values
        if (current->value.type == VALUE_STRING && current->value.data.as_string != NULL) {
            free(current->value.data.as_string);
        }
        
        free(current);
        current = next;
    }
    env->head = NULL;
}

// Clean up interpreter
void free_interpreter(Interpreter* interpreter) {
    env_free(&interpreter->env);
}

// Print a value
void print_value(Value value) {
    char* value_str = value_to_string(value);
    printf("%s\n", value_str);
    free(value_str);
}
