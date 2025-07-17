#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "parser.h"

// Symbol table entry
typedef struct SymbolEntry {
    char* name;
    ValueType type;
    struct SymbolEntry* next;
} SymbolEntry;

// Symbol table
typedef struct {
    SymbolEntry* head;
} SymbolTable;

// Semantic analyzer
typedef struct {
    SymbolTable symbol_table;
    bool had_error;
} SemanticAnalyzer;

// Initialize semantic analyzer
void init_semantic_analyzer(SemanticAnalyzer* analyzer);

// Analyze AST for semantic errors
bool analyze(SemanticAnalyzer* analyzer, AstNode* node);

// Clean up semantic analyzer
void free_semantic_analyzer(SemanticAnalyzer* analyzer);

#endif // SEMANTIC_H
