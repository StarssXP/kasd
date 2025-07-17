#include "../include/common.h"
#include <stdarg.h>

// Global state instance
KasdState kasd_state;

// Initialize the KASD state
void init_kasd_state(int log_level) {
    kasd_state.log_level = log_level;
    clear_error();
}

// Error handling functions
void set_error(ErrorType type, int line, int column, const char* message, const char* source_line, int source_pos, int source_len) {
    if (kasd_state.error.has_error) {
        return; // Already have an error, don't overwrite
    }
    
    kasd_state.error.type = type;
    kasd_state.error.line = line;
    kasd_state.error.column = column;
    kasd_state.error.message = strdup(message);
    kasd_state.error.source_line = source_line ? strdup(source_line) : NULL;
    kasd_state.error.source_pos = source_pos;
    kasd_state.error.source_len = source_len;
    kasd_state.error.has_error = true;
}

void print_error(void) {
    if (!kasd_state.error.has_error) {
        return;
    }
    
    const char* error_type_str = "";
    const char* color = ANSI_RED;
    
    switch (kasd_state.error.type) {
        case ERROR_SYNTAX:  error_type_str = "Syntax Error"; break;
        case ERROR_TYPE:    error_type_str = "Type Error"; break;
        case ERROR_NAME:    error_type_str = "Name Error"; break;
        case ERROR_RUNTIME: error_type_str = "Runtime Error"; break;
        case ERROR_INTERNAL: error_type_str = "Internal Error"; break;
        default: error_type_str = "Unknown Error"; break;
    }
    
    fprintf(stderr, "%s%s at line %d, column %d: %s%s\n", 
            color, error_type_str, kasd_state.error.line, kasd_state.error.column, 
            kasd_state.error.message, ANSI_RESET);
    
    if (kasd_state.error.source_line) {
        fprintf(stderr, "%s\n", kasd_state.error.source_line);
        
        // Print caret pointing to error position
        if (kasd_state.error.source_pos >= 0) {
            for (int i = 0; i < kasd_state.error.source_pos; i++) {
                fputc(' ', stderr);
            }
            
            fprintf(stderr, "%s", color);
            for (int i = 0; i < kasd_state.error.source_len; i++) {
                fputc('^', stderr);
            }
            fprintf(stderr, "%s\n", ANSI_RESET);
        }
    }
}

void clear_error(void) {
    if (kasd_state.error.has_error) {
        free(kasd_state.error.message);
        if (kasd_state.error.source_line) {
            free(kasd_state.error.source_line);
        }
    }
    
    kasd_state.error.type = ERROR_NONE;
    kasd_state.error.line = 0;
    kasd_state.error.column = 0;
    kasd_state.error.message = NULL;
    kasd_state.error.source_line = NULL;
    kasd_state.error.source_pos = -1;
    kasd_state.error.source_len = 0;
    kasd_state.error.has_error = false;
}

// Logging functions
void log_message(int level, const char* format, ...) {
    if (level > kasd_state.log_level) {
        return;
    }
    
    const char* prefix = "";
    const char* color = ANSI_RESET;
    
    switch (level) {
        case LOG_ERROR:
            prefix = "ERROR";
            color = ANSI_RED;
            break;
        case LOG_WARNING:
            prefix = "WARNING";
            color = ANSI_YELLOW;
            break;
        case LOG_INFO:
            prefix = "INFO";
            color = ANSI_GREEN;
            break;
        case LOG_DEBUG:
            prefix = "DEBUG";
            color = ANSI_BLUE;
            break;
    }
    
    fprintf(stderr, "%s[%s]%s ", color, prefix, ANSI_RESET);
    
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    
    fprintf(stderr, "\n");
}

// Value functions
Value create_null_value(void) {
    Value value;
    value.type = VALUE_NULL;
    return value;
}

Value create_int_value(int64_t val) {
    Value value;
    value.type = VALUE_INT;
    value.data.as_int = val;
    return value;
}

Value create_float_value(double val) {
    Value value;
    value.type = VALUE_FLOAT;
    value.data.as_float = val;
    return value;
}

Value create_bool_value(bool val) {
    Value value;
    value.type = VALUE_BOOL;
    value.data.as_bool = val;
    return value;
}

Value create_string_value(const char* val) {
    Value value;
    value.type = VALUE_STRING;
    value.data.as_string = strdup(val);
    return value;
}

void free_value(Value value) {
    if (value.type == VALUE_STRING && value.data.as_string != NULL) {
        free(value.data.as_string);
    }
}

char* value_to_string(Value value) {
    char buffer[64];
    char* result = NULL;
    
    switch (value.type) {
        case VALUE_NULL:
            return strdup("null");
        case VALUE_INT:
            snprintf(buffer, sizeof(buffer), "%lld", (long long)value.data.as_int);
            return strdup(buffer);
        case VALUE_FLOAT:
            snprintf(buffer, sizeof(buffer), "%g", value.data.as_float);
            return strdup(buffer);
        case VALUE_BOOL:
            return strdup(value.data.as_bool ? "true" : "false");
        case VALUE_STRING:
            result = malloc(strlen(value.data.as_string) + 3);
            sprintf(result, "\"%s\"", value.data.as_string);
            return result;
    }
    
    return strdup("unknown");
}

const char* value_type_to_string(ValueType type) {
    switch (type) {
        case VALUE_NULL: return "null";
        case VALUE_INT: return "int";
        case VALUE_FLOAT: return "float";
        case VALUE_BOOL: return "bool";
        case VALUE_STRING: return "string";
        default: return "unknown";
    }
}
