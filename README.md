# Fire Compiler

A modern C++ compiler that transforms Fe (Fire) language source code into x86_64 assembly. Built with performance and simplicity in mind.

[![GitHub license](https://img.shields.io/github/license/aman99dex/Fire)](https://github.com/aman99dex/Fire/blob/main/LICENSE)
[![GitHub issues](https://img.shields.io/github/issues/aman99dex/Fire)](https://github.com/aman99dex/Fire/issues)

## 📖 Table of Contents
- [Overview](#overview)
- [Features](#features)
- [Getting Started](#getting-started)
- [Usage](#usage)
- [Development](#development)
- [Roadmap](#roadmap)
- [Contributing](#contributing)

## Overview

Fire is a compiler that transforms `.Fe` (Iron) files into executable code. Just as fire transforms raw iron into refined steel, this compiler melts down source code into optimized machine instructions.

### The Iron Analogy
- `.Fe` files (Fe = chemical symbol for Iron)
- Lexical Analysis = Initial melting phase
- Tokens = Molten components
- Compilation = Metallurgical transformation
- Output = Refined executable

## Features

- **Efficient Lexical Analysis**
  - Zero-copy string handling with `string_view`
  - Small String Optimization for tokens
  - Precise error reporting with line/column tracking

- **Language Support**
  - Integer literals
  - String literals with escape sequences (`\n`, `\t`, `\"`, `\\`)
  - Return statements
  - Single-line comments (`// style`)

- **Modern C++ Design**
  - C++17 features
  - RAII principles
  - Exception safety
  - Memory efficient

## 🚀 Getting Started

### Prerequisites
- C++ compiler (GCC 9.0+)
- CMake (3.10+)
- Make
- NASM assembler
- Linux x86_64 system

### Quick Start
```bash
# Clone the repository
git clone https://github.com/aman99dex/Fire.git
cd Fire

# Create build directory
mkdir build && cd build

# Configure and build
cmake ..
make
```

## 📝 Usage

```bash
# Basic usage
./Fire input.Fe

# Example with test file
./Fire ../test/test3.Fe
```

## Example Code

```plaintext
// Basic Fe program
return "Hello, World!";  // Returns and prints string
return 42;              // Returns integer value
return "Multi\nLine";   // String with newline
```

## 🛠️ Development

### Project Structure
```
Fire/
├── src/
│   ├── main.cpp      # Compiler implementation
│   └── test3.Fe      # Example source file
├── build/             # Build artifacts
├── CMakeLists.txt    # Build configuration
└── README.md         # This file
```

### Building from Source
```bash
cd Fire
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## Implementation Details

### Lexical Analysis
- Token types: keywords, integers, strings, identifiers
- Efficient string handling with SSO
- Comment skipping and whitespace handling
- Escape sequence processing

### Assembly Generation
- NASM syntax for x86_64
- Linux syscalls for I/O
- String table generation
- Efficient instruction encoding

## 🗺️ Roadmap

- [ ] Function declarations and calls
- [ ] Variables and expressions
- [ ] Control flow statements
- [ ] More data types
- [ ] Optimization passes
- [ ] Symbol table
- [ ] Type checking

## 🤝 Contributing

Feel free to open issues or submit pull requests. All contributions are welcome!

## 📜 License

This project is open source and available under the MIT License.

```plaintext
MIT License

Copyright (c) 2025 Aman Gupta (aman99dex)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Fire Compiler"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software...
```

For the complete license text, please see the [LICENSE](LICENSE) file in the repository.

## 👤 Author

aman99dex (Aman)
Started: February 2025

---

<p align="center">
  Made with ❤️ by <a href="https://github.com/aman99dex">aman99dex</a>
</p>