# CS4031 - Assignment 02: LL(1) Parser

**Team Members:** RollNumber1, RollNumber2
**Language:** C++

## How to Compile & Run

**Using CMake (Recommended):**

1. Navigate to the root folder `22i1234-22i5678-A/`
2. Run `cmake .`
3. Run `make`
4. Execute `./compiler_test`

**Using Makefile:**

1. Navigate to the `docs/` folder.
2. Run `make`
3. The executable `compiler_test` will be placed in the root folder. Execute with `../compiler_test`

## Features Implemented

- Left Factoring & Indirect/Direct Left Recursion Removal
- FIRST & FOLLOW Set generation
- LL(1) Parsing Table Construction
- Stack-based Parser with Panic Mode Error Recovery
- Abstract Syntax Tree (AST) Generation
