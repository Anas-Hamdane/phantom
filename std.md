# Phantom language standard

## Lexical structure

### Character Set
- Supported character encoding: ASCII

### Comments
```
// Single-line comment
/* Multi-line comment */
```

### Identifiers
- Must contains only ASCII characters
- Must not start with a number
- Must not be a Reserved [keyword](#keywords)

### Keywords
- `return`: return syscall
- `write`: write syscall
- `while`: for while loop statements
- `true`, `false`: for boolean values
- `let`: for variable declarations
- `fn`: for function definitions/declarations
- `if`: for conditional statements

### Literals
- Integer literals: 42 (only supports decimals)
- Float literals: 3.14 (only supports decimals)
- Boolean literals: true, false
- Char Literals: 'a'
- String Literals: "Hello"

## Syntax and Grammar

### Basic rules
C-like rules:
- Statements are semicolon terminated
- Block structure: braces
- Whitespaces are completly ignored

## Variables and Data Types

### Data Types
- `bool`: 1-bit integer
- `char`: 8-bit integer
- `short`: 16-bit integer
- `int`: 32-bit integer
- `long`: 64-bit integer
- `float`: 32-bit integer
- `double`: 64-bit integer
- `quad`: 128-bit integer
- `ptr`: pointer type

### Variable Declaration
```
// declaration without initialization
let x: type; // type is required

// declaration with initialization
let x = 1; // automatically set to `int`

// referencing
let y: ptr;
let y = &x;

// dereferencing
*y = 2;
```
