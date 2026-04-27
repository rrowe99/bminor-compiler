# B-Minor Compiler

A modular compiler backend written in C that generates x86-64 assembly from an abstract syntax tree (AST). This project implements a multi-pass compilation pipeline, translating high-level language constructs into low-level machine instructions.

---

## Features
- Multi-pass compiler architecture  
- AST traversal for code generation  
- x86-64 assembly output  
- Register management system  
- Stack-based variable storage  
- Expression evaluation  
- Symbol resolution and scope handling  
- Label-based control flow (loops and conditionals)  

---

## How It Works
Source Code → Parser → AST → Code Generator → Assembly Output

---

## How to Run
```bash
make
./compiler input.bminor > output.s
gcc output.s -o program
./program
```
## Project Structure
- parser.y, scanner.l → parsing and lexical analysis
- expr_codegen.c, stmt_codegen.c → code generation logic
- symbol.c, scope.c → symbol table and scope handling
- scratch.c, label.c → register and label management
## What I Learned
- Translating high-level constructs into assembly instructions
- Register allocation and stack-based memory management
- Designing and organizing multi-stage compiler pipelines
- Debugging complex, low-level systems code

## Example

**Input (tests/bad1.bminor):**
```c
/* Cannot assign a character to a global integer. */

x: integer = 'c';
```

**Output (error handling):**
```bash
type error: initializer for x does not match declared type
```

---

**Input (tests/good1.bminor):**
```c
/* Assign integer expression to local variable/ typechecking with function call. */

x: integer = 5;
y: integer = 10;

main: function void ()=
{
        x: integer = y * 10 + 15 - 4;
}
```

**Output (assembly excerpt):**
```bash
.globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    movq $0, %rax
    leave
    ret
```
