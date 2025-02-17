# 🔥 Fire Compiler

> A modern compiler that melts `.Fe` (Iron) files into executable form.

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

## ✨ Features

### Metallurgical Processing
- 🔥 Keyword refinement (`return`)
- ⚒️ Integer forging (`42`)
- 🛠️ String tempering with escape sequences (`"Hello\nWorld"`)
- 🏷️ Identifier hallmarking (`my_variable`)
- 📍 Heat precision tracking (line/column reporting)

### Token Types
| Type | Example | Description |
|------|---------|-------------|
| Keywords | `return` | Language reserved words |
| Integers | `42` | Numeric literals |
| Strings | `"Hello"` | Text in quotes |
| Identifiers | `counter` | Variable names |
| Symbols | `;` | Punctuation marks |

## 🚀 Getting Started

### Prerequisites
- C++ compiler (GCC 9.0+)
- CMake (3.10+)
- Make

### Quick Start
```bash
# Clone the repository
git clone https://github.com/aman99dex/Fire.git
cd Fire

# Build the project
mkdir build && cd build
cmake ..
make
```

## 📝 Usage

Create a `.Fe` file:
```fire
return "Hello World";
identifier_with_underscore;
return 42;
```

Run the compiler:
```bash
./Fire path/to/source.Fe
```

## 🛠️ Development

### Project Structure
```
Fire/
├── src/
│   ├── main.cpp        # Lexical analyzer implementation
│   └── test.Fe        # Example source file
├── build/             # Build artifacts
├── CMakeLists.txt    # Build configuration
└── README.md
```

### Building from Source
```bash
cd Fire
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

## 🗺️ Roadmap

- [ ] Extended keyword support
- [ ] Floating-point number support
- [ ] Enhanced escape sequences
- [ ] Character literals
- [ ] Improved error messages
- [ ] Parser implementation

## 🤝 Contributing

Contributions are welcome! Feel free to:
1. Fork the repository
2. Create a feature branch
3. Submit a pull request

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Aman**
- GitHub: [@aman99dex](https://github.com/aman99dex)
- Started: February 2025

---

<p align="center">
  Made with ❤️ by <a href="https://github.com/aman99dex">aman99dex</a>
</p>