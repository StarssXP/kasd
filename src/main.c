#include "../include/common.h"
#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/semantic.h"
#include "../include/interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LENGTH 1024

// Forward declarations
static void usage(const char* program_name);
static void repl(int log_level);
static bool run_file(const char* filename, int log_level);
static bool run_source(const char* source, int log_level, bool repl_mode);
static char* read_file(const char* filename);

int main(int argc, char* argv[]) {
    int log_level = LOG_ERROR;
    char* filename = NULL;
    
    // Parse command line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--log-level") == 0 || strcmp(argv[i], "-l") == 0) {
            if (i + 1 < argc) {
                log_level = atoi(argv[++i]);
                if (log_level < LOG_NONE || log_level > LOG_DEBUG) {
                    fprintf(stderr, "Invalid log level: %d\n", log_level);
                    usage(argv[0]);
                    return 1;
                }
            } else {
                fprintf(stderr, "Missing log level value\n");
                usage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "Unknown option: %s\n", argv[i]);
            usage(argv[0]);
            return 1;
        } else {
            if (filename != NULL) {
                fprintf(stderr, "Only one file can be specified\n");
                usage(argv[0]);
                return 1;
            }
            filename = argv[i];
        }
    }
    
    // Initialize KASD state
    init_kasd_state(log_level);
    
    // Run file or REPL
    if (filename != NULL) {
        if (!run_file(filename, log_level)) {
            return 1;
        }
    } else {
        repl(log_level);
    }
    
    return 0;
}

// Print usage information
static void usage(const char* program_name) {
    printf("Usage: %s [options] [file]\n", program_name);
    printf("Options:\n");
    printf("  -l, --log-level LEVEL  Set log level (0-4, default: 1)\n");
    printf("  -h, --help             Show this help message\n");
    printf("\n");
    printf("Log Levels:\n");
    printf("  0: None\n");
    printf("  1: Error (default)\n");
    printf("  2: Warning\n");
    printf("  3: Info\n");
    printf("  4: Debug\n");
}

// Run the REPL
static void repl(int log_level) {
    char line[MAX_LINE_LENGTH];
    
    printf("KASD Language Interpreter v0.1\n");
    printf("Type 'exit' to quit\n");
    
    while (1) {
        printf("> ");
        if (!fgets(line, sizeof(line), stdin)) {
            break;
        }
        
        // Check for exit command
        if (strcmp(line, "exit\n") == 0) {
            break;
        }
        
        // Run the line
        run_source(line, log_level, true);
        
        // Clear any errors
        clear_error();
    }
}

// Run a file
static bool run_file(const char* filename, int log_level) {
    char* source = read_file(filename);
    if (source == NULL) {
        fprintf(stderr, "Could not read file: %s\n", filename);
        return false;
    }
    
    bool result = run_source(source, log_level, false);
    
    free(source);
    return result;
}

// Run source code
static bool run_source(const char* source, int log_level, bool repl_mode) {
    // Initialize components
    Lexer lexer;
    Parser parser;
    SemanticAnalyzer analyzer;
    Interpreter interpreter;
    
    init_lexer(&lexer, source);
    init_parser(&parser, &lexer);
    init_semantic_analyzer(&analyzer);
    init_interpreter(&interpreter, repl_mode);
    
    // Parse
    AstNode* ast = parse(&parser);
    
    // Check for parsing errors
    if (parser.had_error || ast == NULL) {
        print_error();
        free_ast(ast);
        return false;
    }
    
    // Analyze
    if (!analyze(&analyzer, ast)) {
        print_error();
        free_ast(ast);
        return false;
    }
    
    // Debug print AST if log level is high enough
    if (log_level >= LOG_DEBUG) {
        printf("AST:\n");
        print_ast(ast, 0);
    }
    
    // Interpret
    interpret(&interpreter, ast);
    
    // Clean up
    free_ast(ast);
    free_semantic_analyzer(&analyzer);
    free_interpreter(&interpreter);
    
    return true;
}

// Read a file into memory
static char* read_file(const char* filename) {
    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        return NULL;
    }
    
    // Get file size
    fseek(file, 0, SEEK_END);
    size_t size = ftell(file);
    rewind(file);
    
    // Allocate buffer
    char* buffer = malloc(size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    
    // Read file
    size_t bytes_read = fread(buffer, 1, size, file);
    if (bytes_read < size) {
        free(buffer);
        fclose(file);
        return NULL;
    }
    
    // Null terminate
    buffer[bytes_read] = '\0';
    
    fclose(file);
    return buffer;
}
