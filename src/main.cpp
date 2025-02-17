/******************************************************************************
 * Fire Compiler
 * Author: aman99dex (Aman)
 * Started: February 2025
 * 
 * A modern C++ compiler for the Fe (Fire) language that transforms source code
 * into x86_64 assembly. The name "Fire" symbolizes the transformation process,
 * much like how fire transforms raw materials.
 * 
 * Core Features:
 * - Efficient lexical analysis with string_view
 * - Small string optimization (SSO) for token values
 * - Zero-copy string handling where possible
 * - Escape sequence support in strings (\n, \t, \", \\)
 * - Precise error reporting with line/column tracking
 * - Direct assembly generation for x86_64
 * 
 * Design Principles:
 * - Performance: Use of modern C++ features for efficiency
 * - Memory: Minimal allocations and copying
 * - Robustness: Strong error handling and reporting
 * - Simplicity: Clear, well-documented code structure
 * 
 * Language Features:
 * - Integer literals
 * - String literals with escape sequences
 * - Return statements
 * - Comments (// style)
 * 
 * Assembly Output:
 * - NASM syntax for x86_64
 * - Linux syscall interface
 * - Position-independent code
 * 
 * Future Enhancements:
 * - Function declarations
 * - Variables and expressions
 * - Control flow (if/else, loops)
 * - More data types
 * - Optimization passes
 ******************************************************************************/

// Standard library includes for I/O, containers, and string operations
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <cctype>
#include <memory_resource>
#include <algorithm>  // for std::copy_n
#include <cstring>   // for std::memcpy

// Use string_view for better performance with string literals
// string_view provides a lightweight non-owning reference to a string
using sv = std::string_view;

// Define all possible token types that our lexer can recognize
enum class TokenType {
    // Language keywords
    _return,    // 'return' keyword
    
    // Different types of literals
    int_lit,    // Integer literals (e.g., 42)
    string_lit, // String literals (e.g., "Hello")
    identifier, // Variable/function names
    
    // Punctuation symbols
    semi,       // Semicolon (;)
    quote,      // Double quote (")
    
    // Special tokens
    eof,        // End of file marker
    invalid     // Invalid/error token
};

// Add these new assembly operations to AsmOp
enum class AsmOp {
    mov,    // Move data
    ret,    // Return from function
    push,   // Push to stack
    pop,    // Pop from stack
    syscall,// System call
    add,    // Add
    sub,    // Subtract
    lea,    // Load effective address
    invalid // Invalid operation
};

// Replace KEYWORDS map with perfect hash
constexpr uint32_t hash_keyword(std::string_view str) {
    uint32_t hash = 2166136261u;
    for (char c : str) {
        hash ^= static_cast<uint32_t>(c);
        hash *= 16777619u;
    }
    return hash;
}

// In Tokenizer class
TokenType get_keyword_type(std::string_view word) {
    switch (hash_keyword(word)) {
        case hash_keyword("return"): return TokenType::_return;
        default: return TokenType::identifier;
    }
}

// Tracks the position of tokens in source code for error reporting
struct Position {
    size_t line;    // Line number (1-based)
    size_t column;  // Column number (1-based)
    
    Position(size_t l = 1, size_t c = 1) : line(l), column(c) {}
};

// Optimize string storage
constexpr size_t SMALL_STRING_SIZE = 24;
using small_string = std::string_view;  // For strings <= 24 bytes
using large_string = std::string;       // For strings > 24 bytes

// Represents a single token in the source code
class Token {
private:
    TokenType type;
    union {
        char small[SMALL_STRING_SIZE];
        large_string* large;
    } value;
    bool is_small;
    Position pos;

public:
    Token(TokenType t, std::string_view v, Position p) 
        : type(t), pos(p) {
        if (v.length() < SMALL_STRING_SIZE) {
            is_small = true;
            std::copy_n(v.data(), v.length(), value.small);
            value.small[v.length()] = '\0';
        } else {
            is_small = false;
            value.large = new large_string(v);
        }
    }

    // Add destructor
    ~Token() {
        if (!is_small) {
            delete value.large;
        }
    }

    // Add move constructor
    Token(Token&& other) noexcept 
        : type(other.type), is_small(other.is_small), pos(other.pos) {
        if (is_small) {
            std::memcpy(value.small, other.value.small, SMALL_STRING_SIZE);
        } else {
            value.large = other.value.large;
            other.value.large = nullptr;
        }
    }

    // Delete copy constructor and assignment
    Token(const Token&) = delete;
    Token& operator=(const Token&) = delete;
    Token& operator=(Token&&) = delete;

    // Getter methods for token properties
    TokenType get_type() const { return type; }
    std::string get_value() const { 
        return is_small ? std::string(value.small) : *value.large; 
    }
    const Position& get_position() const { return pos; }
};

// Add this struct for string literals
struct StringLiteral {
    std::string label;
    std::string value;
    size_t length;
};

// Add near StringLiteral struct
class StringPool {
private:
    std::unordered_map<std::string_view, size_t> string_map;
    std::vector<StringLiteral> strings;

public:
    // Get or create string literal
    size_t get_or_add(std::string_view value) {
        auto it = string_map.find(value);
        if (it != string_map.end()) {
            return it->second;
        }
        size_t index = strings.size();
        std::string label = "str_" + std::to_string(index);
        strings.push_back({label, std::string(value), value.length()});
        string_map.emplace(strings.back().value, index);
        return index;
    }

    const std::vector<StringLiteral>& get_strings() const {
        return strings;
    }
};

// Modify AsmInstruction to include string data
struct AsmInstruction {
    AsmOp op;
    std::string operand1;
    std::string operand2;
    std::vector<StringLiteral> strings;
    
    std::string to_string() const {
        switch (op) {
            case AsmOp::mov:
                return "    mov " + operand1 + ", " + operand2;
            case AsmOp::ret:
                return "    ret";
            case AsmOp::push:
                return "    push " + operand1;
            case AsmOp::pop:
                return "    pop " + operand1;
            case AsmOp::syscall:
                return "    syscall";
            case AsmOp::add:
                return "    add " + operand1 + ", " + operand2;
            case AsmOp::sub:
                return "    sub " + operand1 + ", " + operand2;
            case AsmOp::lea:
                return "    lea " + operand1 + ", " + operand2;
            default:
                return "; invalid instruction";
        }
    }
};

// Main lexical analyzer class that converts source code into tokens
class Tokenizer {
private:
    std::string_view input;
    std::vector<Token> tokens;
    size_t length;       // Move these to private
    size_t pos = 0;
    size_t line = 1;
    size_t column = 1;

public:
    // Modified constructor
    explicit Tokenizer(std::string_view source) 
        : input(source), length(source.length()) {  // Initialize length
        // Estimate token count (roughly 1 token per 5 characters)
        reserve_tokens(source.length() / 5);
    }

    // Pre-allocate token vector to avoid reallocations
    void reserve_tokens(size_t estimated_count) {
        tokens.reserve(estimated_count);
    }

    // Look at current character without advancing
    char peek() const {
        return pos < length ? input[pos] : '\0';
    }

    // Move to next character and return current one
    char advance() {
        column++;
        return pos < length ? input[pos++] : '\0';
    }

    // Check if character is a digit (0-9)
    bool is_digit(char c) const {
        return c >= '0' && c <= '9';
    }

    // Skip until end of line
    void skip_comment() {
        while (pos < length && input[pos] != '\n') {
            advance();
        }
    }

    // Skip whitespace and comments, update line/column counters
    void skip_whitespace_and_comments() {
        while (pos < length) {
            // Skip spaces, tabs, newlines
            if (std::isspace(input[pos])) {
                if (input[pos] == '\n') {
                    line++;
                    column = 1;
                } else {
                    column++;
                }
                pos++;
                continue;
            }
            
            // Check for comments
            if (pos + 1 < length && input[pos] == '/' && input[pos + 1] == '/') {
                skip_comment();
                continue;
            }
            
            break;
        }
    }

    // Process numeric literals
    void handle_number() {
        size_t start = pos;
        size_t start_col = column;
        
        // Collect all consecutive digits
        while (pos < length && is_digit(input[pos])) {
            advance();
        }
        
        // Create token from collected digits
        std::string number(input.data() + start, pos - start);
        tokens.emplace_back(TokenType::int_lit, std::move(number), 
                          Position(line, start_col));
    }

    // Process string literals with escape sequences
    void handle_string() {
        Position start{line, column};
        advance(); // Skip opening quote
        
        std::string str;
        while (peek() != '"' && peek() != '\0') {
            // Check for unterminated strings
            if (peek() == '\n') {
                std::cerr << "Error: Unterminated string at " << line << ":" << column << std::endl;
                tokens.emplace_back(TokenType::invalid, str, start);
                return;
            }
            // Handle escape sequences
            if (peek() == '\\') {
                advance(); // Skip backslash
                switch (peek()) {
                    case 'n': str += '\n'; break;  // Newline
                    case 't': str += '\t'; break;  // Tab
                    case '"': str += '"'; break;   // Quote
                    case '\\': str += '\\'; break; // Backslash
                    default: str += peek();        // Invalid escape
                }
            } else {
                str += peek();
            }
            advance();
        }
        
        // Check for proper string termination
        if (peek() == '"') {
            advance(); // Skip closing quote
            tokens.emplace_back(TokenType::string_lit, str, start);
            return;
        }
        
        // Handle unterminated strings
        std::cerr << "Error: Unterminated string at " << line << ":" << column << std::endl;
        tokens.emplace_back(TokenType::invalid, str, start);
    }

    // Main tokenization method that processes the entire input
    std::vector<Token> tokenize() {
        while (pos < length) {
            skip_whitespace_and_comments();  // Replace skip_whitespace() with this
            
            if (pos >= length) break;
            
            char c = peek();
            
            // Handle different token types based on first character
            if (c == '"') {
                handle_string();
            }
            else if (is_digit(c)) {
                handle_number();
            }
            else if (c == ';') {
                tokens.emplace_back(TokenType::semi, ";", Position(line, column));
                advance();
            }
            // Handle identifiers and keywords
            else if (std::isalpha(c)) {
                size_t start = pos;
                size_t start_col = column;
                
                // Collect identifier characters (alphanumeric + underscore)
                while (pos < length && (std::isalnum(input[pos]) || input[pos] == '_')) {
                    advance();
                }
                
                // Check if it's a keyword or identifier
                sv word(input.data() + start, pos - start);
                auto type = get_keyword_type(word);
                
                tokens.emplace_back(type, std::string(word), 
                                     Position(line, start_col));
            }
            else {
                // Handle unexpected characters
                std::cerr << "Unexpected character at " << line << ":" << column << ": '" 
                         << c << "'" << std::endl;
                advance();
            }
        }

        // Add EOF token at the end
        tokens.emplace_back(TokenType::eof, "EOF", Position(line, column));
        return std::move(tokens);
    }

    // Update the to_asm function in the Tokenizer class
    std::vector<AsmInstruction> to_asm(const std::vector<Token>& tokens) {
        std::vector<AsmInstruction> instructions;
        std::vector<StringLiteral> strings;
        size_t string_count = 0;
        
        for (size_t i = 0; i < tokens.size(); ++i) {
            const Token& token = tokens[i];
            
            switch (token.get_type()) {
                case TokenType::_return: {
                    if (i + 1 < tokens.size()) {
                        const Token& next = tokens[i + 1];
                        switch (next.get_type()) {
                            case TokenType::int_lit: {
                                // Return integer
                                instructions.push_back({AsmOp::mov, "rax", "60"});  // exit syscall
                                instructions.push_back({AsmOp::mov, "rdi", next.get_value()});
                                instructions.push_back({AsmOp::syscall, "", ""});
                                i++;
                                break;
                            }
                            case TokenType::string_lit: {
                                // Return string
                                std::string label = "str_" + std::to_string(string_count++);
                                strings.push_back({label, next.get_value(), next.get_value().length()});
                                
                                // Print string syscall
                                instructions.push_back({AsmOp::mov, "rax", "1"});     // write syscall
                                instructions.push_back({AsmOp::mov, "rdi", "1"});     // stdout
                                instructions.push_back({AsmOp::lea, "rsi", "[" + label + "]"});
                                instructions.push_back({AsmOp::mov, "rdx", std::to_string(next.get_value().length())});
                                instructions.push_back({AsmOp::syscall, "", ""});
                                
                                // Exit syscall
                                instructions.push_back({AsmOp::mov, "rax", "60"});
                                instructions.push_back({AsmOp::mov, "rdi", "0"});
                                instructions.push_back({AsmOp::syscall, "", ""});
                                i++;
                                break;
                            }
                            default:
                                std::cerr << "Error: Invalid return value at " 
                                        << next.get_position().line << ":" 
                                        << next.get_position().column << std::endl;
                                break;
                        }
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
        // Add string data to the first instruction
        if (!instructions.empty() && !strings.empty()) {
            instructions[0].strings = strings;
        }
        
        return instructions;
    }
};

// Improved token printing
void print_tokens(const std::vector<Token>& tokens) {
    static const std::unordered_map<TokenType, const char*> TYPE_NAMES = {
        {TokenType::_return, "KEYWORD"},
        {TokenType::int_lit, "INTEGER"},
        {TokenType::string_lit, "STRING"},
        {TokenType::identifier, "IDENTIFIER"},
        {TokenType::semi, "SEMICOLON"},
        {TokenType::quote, "QUOTE"},
        {TokenType::eof, "EOF"},
        {TokenType::invalid, "INVALID"}
    };

    std::cout << "\nTokens:\n";
    for (const auto& token : tokens) {
        std::cout << TYPE_NAMES.at(token.get_type()) << " at " 
                 << token.get_position().line << ":" 
                 << token.get_position().column << ": '" 
                 << token.get_value() << "'\n";
    }
}

class AsmBuilder {
private:
    std::stringstream code;
    StringPool string_pool;

public:
    void add_instruction(const AsmInstruction& inst) {
        code << inst.to_string() << '\n';
    }

    void add_string(std::string_view str) {
        size_t index = string_pool.get_or_add(str);
        const auto& literal = string_pool.get_strings()[index];
        code << literal.label << ": db ";
        for (size_t i = 0; i < literal.value.length(); ++i) {
            code << static_cast<int>(literal.value[i]);
            if (i < literal.value.length() - 1) code << ", ";
        }
        code << '\n';
    }

    std::string build() {
        return code.str();
    }

    // Add to AsmBuilder class
    void generate_assembly(const std::vector<AsmInstruction>& instructions) {
        // Data section
        if (!instructions.empty() && !instructions[0].strings.empty()) {
            code << "section .data\n";
            for (const auto& str : instructions[0].strings) {
                code << str.label << ": db ";
                for (size_t i = 0; i < str.value.length(); ++i) {
                    code << static_cast<int>(str.value[i]);
                    if (i < str.value.length() - 1) code << ", ";
                }
                code << "\n";
            }
        }

        // Text section
        code << "\nsection .text\n";
        code << "global _start\n";
        code << "_start:\n";

        for (const auto& inst : instructions) {
            code << inst.to_string() << '\n';
        }
    }
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: Fire <input.Fe>\n";
        return EXIT_FAILURE;
    }

    try {
        // Memory map the file for faster reading
        std::ifstream input(argv[1], std::ios::binary | std::ios::ate);
        if (!input) {
            throw std::runtime_error("Could not open " + std::string(argv[1]));
        }

        // Get file size and reserve space
        size_t size = input.tellg();
        input.seekg(0);
        std::string contents;
        contents.reserve(size);
        contents.assign(std::istreambuf_iterator<char>(input),
                      std::istreambuf_iterator<char>());

        // Show input
        std::cout << "File contents:\n" << contents << std::endl;

        // Process tokens
        Tokenizer tokenizer(contents);
        auto tokens = tokenizer.tokenize();
        print_tokens(tokens);

        // Generate assembly
        AsmBuilder builder;
        auto instructions = tokenizer.to_asm(tokens);
        builder.generate_assembly(instructions);
        
        // Output assembly
        std::cout << "\nAssembly Output:\n" << builder.build();

        // Update in main() where assembly is written:
        // Generate assembly file in build directory
        std::string asm_path = "test3.asm";  // Simple fixed name in build dir
        std::ofstream asm_file(asm_path);
        if (!asm_file) {
            throw std::runtime_error("Could not create " + asm_path);
        }
        asm_file << builder.build();
        asm_file.close();

        std::cout << "Assembly written to: " << asm_path << std::endl;

        return EXIT_SUCCESS;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
