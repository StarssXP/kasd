#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

// Log levels
#define LOG_NONE 0
#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 3
#define LOG_DEBUG 4

// ANSI color codes
#define ANSI_RED     "\x1b[31m"
#define ANSI_GREEN   "\x1b[32m"
#define ANSI_YELLOW  "\x1b[33m"
#define ANSI_BLUE    "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN    "\x1b[36m"
#define ANSI_RESET   "\x1b[0m"

// Error types
typedef enum {
    ERROR_NONE,
    ERROR_SYNTAX,
    ERROR_TYPE,
    ERROR_NAME,
    ERROR_RUNTIME,
    ERROR_INTERNAL
} ErrorType;

// Error structure
typedef struct {
    ErrorType type;
    int line;
    int column;
    char* message;
    char* source_line;
    int source_pos;
    int source_len;
    bool has_error;
} Error;

// Value types
typedef enum {
    VALUE_NULL,
    VALUE_INT,
    VALUE_FLOAT,
    VALUE_BOOL,
    VALUE_STRING
} ValueType;

// Value structure
typedef struct {
    ValueType type;
    union {
        int64_t as_int;
        double as_float;
        bool as_bool;
        char* as_string;
    } data;
} Value;

// Global state
typedef struct {
    int log_level;
    Error error;
} KasdState;

// Global state instance
extern KasdState kasd_state;

// Initialize the KASD state
void init_kasd_state(int log_level);

// Error handling functions
void set_error(ErrorType type, int line, int column, const char* message, const char* source_line, int source_pos, int source_len);
void print_error(void);
void clear_error(void);

// Logging functions
void log_message(int level, const char* format, ...);

// Value functions
Value create_null_value(void);
Value create_int_value(int64_t value);
Value create_float_value(double value);
Value create_bool_value(bool value);
Value create_string_value(const char* value);
void free_value(Value value);
char* value_to_string(Value value);
const char* value_type_to_string(ValueType type);

#endif // COMMON_H
