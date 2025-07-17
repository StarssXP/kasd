#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "semantic.h"

// Environment entry
typedef struct EnvEntry {
    char* name;
    Value value;
    struct EnvEntry* next;
} EnvEntry;

// Environment
typedef struct {
    EnvEntry* head;
} Environment;

// Interpreter
typedef struct {
    Environment env;
    bool had_error;
    bool repl_mode;
} Interpreter;

// Initialize interpreter
void init_interpreter(Interpreter* interpreter, bool repl_mode);

// Execute AST
Value interpret(Interpreter* interpreter, AstNode* node);

// Clean up interpreter
void free_interpreter(Interpreter* interpreter);

// REPL utilities
void print_value(Value value);

#endif // INTERPRETER_H
