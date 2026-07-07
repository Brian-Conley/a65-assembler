# 6502 Multi-Pass Assembler
A high-performance compiler pipeline designed to translate MOS 6502 Assembly into executable machine code.

## Assembly Pipeline
The assembly process operates as a deterministic, three-stage pipeine.

### Lexing
The Lexer breaks the raw file buffer into atomic token units optimized for minimal heap overhead.
- Zero-Copy Tokenization: To eliminate expensive dynamic string allocations, tokens store their values as string views mapped directly to the original file buffer.
- Token Vector Pre-allocation: The token vector reserves memory based on the input source size, minimizing vector reallocations during runtime.

### Parsing
The Parser groups the tokens into structured statements.
- Index-Based Statement Grouping: Statements do not copy or store token objects. Instead, statements are formed by grouping token indices into logical structures.
- Statement Vector Pre-allocation: The statement vector reserves memory based on the number of tokens provided, minimizing vector reallocations during runtime.

### Assembly
The Assembler executes the final transformation from statements into machine code.
- 2D Opcode Matrix: Maps combinations of instruction_type, address_mode to specific hex values, providing instant O(1) opcode validation and resolution.
- Multi-Pass Symbol Resolution: The assembly process uses an N-pass appraoch.
  - Pass 1: Evaluate instruction sizes, populate the symbol table.
  - Pass 2+: Iterate through statements using calculated instruction sizes until the symbol table stabilizes.
  - Final pass: Generate the machine code.
- Forward Reference Handling: In the first pass, if the assembler encounters an undefined symbol, the symbol is assumed to be a two-byte address. In subsequent passes the symbol will be defined and referenced accordingly.

## Diagnostics
All warning and error reports are handled through a Diagnostics singleton object.
- Tracks total errors and warnings during runtime.
- Implements error typing (```ParserUnexpectedToken```, ```AssemblerSymbolRedefinition```).
- Supports three severity levels: ```Warning```, ```Error```, and ```Fatal```.

## Getting Started
### Prerequisites
- A compiler that supports the C++20 standard
- Cmake

### Writing Assembly
You can find the assembly specification for this implementation in [assembly-specification.md](docs/assembly-specification.md). The specification lists supported features and syntax requirements, and assumes you already have an understanding of 6502 instructions.

### Building and Running
```
./build.sh
build/a65 <file.a65>
```
