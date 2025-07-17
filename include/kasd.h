#ifndef KASD_H
#define KASD_H

#include <stdbool.h>

// Log levels
#define KASD_LOG_NONE 0
#define KASD_LOG_ERROR 1
#define KASD_LOG_WARNING 2
#define KASD_LOG_INFO 3
#define KASD_LOG_DEBUG 4

// KASD value types
typedef enum {
    KASD_VALUE_NULL,
    KASD_VALUE_INT,
    KASD_VALUE_FLOAT,
    KASD_VALUE_BOOL,
    KASD_VALUE_STRING
} KasdValueType;

// KASD value
typedef struct {
    KasdValueType type;
    union {
        long long as_int;
        double as_float;
        bool as_bool;
        char* as_string;
    } data;
} KasdValue;

// KASD context
typedef struct KasdContext KasdContext;

// Create a new KASD context
KasdContext* kasd_create_context(int log_level);

// Free a KASD context
void kasd_free_context(KasdContext* context);

// Execute KASD code
bool kasd_execute(KasdContext* context, const char* source);

// Execute KASD code in REPL mode
bool kasd_execute_repl(KasdContext* context, const char* source);

// Get the last error message
const char* kasd_get_error(KasdContext* context);

// Create KASD values
KasdValue kasd_null();
KasdValue kasd_int(long long value);
KasdValue kasd_float(double value);
KasdValue kasd_bool(bool value);
KasdValue kasd_string(const char* value);

// Free a KASD value
void kasd_free_value(KasdValue value);

#endif // KASD_H
