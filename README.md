# KASD Programming Language

KASD is a fast, lightweight interpreted programming language with C-like syntax. This implementation focuses on speed and simplicity, using optimized data structures and avoiding conditional branches where possible.

## Features

- C-like syntax
- Static typing
- Fast interpreter
- Detailed error reporting
- REPL mode
- Configurable logging levels

## Supported Types

- `int`: 64-bit integer
- `float`: Double-precision floating point
- `bool`: Boolean (true/false)
- `string`: Text string
- `null`: Null value

## Syntax Example

```
// Variable declaration
let name: type = value;

// Examples
let x: int = 42;
let pi: float = 3.14159;
let is_valid: bool = true;
let message: string = "Hello, KASD!";
let nothing: null = null;
```

## Building

To build KASD, simply run:

```
make
```

This will create the `kasd` executable in the `bin` directory.

## Usage

### Running a File

```
bin/kasd path/to/file.kasd
```

### REPL Mode

```
bin/kasd
```

### Command-line Options

```
Usage: kasd [options] [file]
Options:
  -l, --log-level LEVEL  Set log level (0-4, default: 1)
  -h, --help             Show this help message

Log Levels:
  0: None
  1: Error (default)
  2: Warning
  3: Info
  4: Debug
```

## Error Reporting

KASD provides detailed error messages with line and column information:

```
Syntax Error at line 3, column 10: Expected ';' after variable declaration.
let x: int = 42
         ^
```

## License

KASD is licensed under the MIT License. See the LICENSE file for details.
