/******************************************************************************
 * Fire Compiler
 * First Major Project by aman99dex(Aman...)
 * Started: February 2025
 * 
 * This is my first significant programming project - a compiler that processes
 * '.Fe' (Iron) files, melting them down into executable form. Just as fire
 * transforms iron, this compiler transforms source code into machine instructions.
 * 
 * Key Features:
 * - Melts down keywords (like 'return')
 * - Forges integer literals into tokens
 * - Shapes string literals with escape sequences
 * - Identifies variable names and function signatures
 * - Precise temperature control (line and column tracking)
 * - High-efficiency crucible (string_view, unordered_map)
 * 
 * Learning Goals:
 * - Mastering the art of compilation
 * - Forging robust data structures
 * - Temperature control (I/O handling)
 * - Error detection and reporting
 * - Blueprint organization
 * 
 * Usage:
 * ./Fire <input.Fe>  # Melting iron into executable form
 * 
 * Example:
 * ./Fire test.Fe    # Watch it melt!
 * 
 * Future Refinements:
 * - Additional metallurgical processes (keywords)
 * - Support for floating-point precision
 * - Enhanced heat treatment (escape sequences)
 * - Char tempering
 * - Improved error detection
 ******************************************************************************/

// Standard library includes for I/O, containers, and string operations
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <string_view>
#include <cctype>

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

// Hash map for quick keyword lookup, using string_view for efficiency
// Maps keyword strings to their corresponding TokenType
static const std::unordered_map<sv, TokenType> KEYWORDS = {
    {"return", TokenType::_return}
};

// Tracks the position of tokens in source code for error reporting
struct Position {
    size_t line;    // Line number (1-based)
    size_t column;  // Column number (1-based)
    
    Position(size_t l = 1, size_t c = 1) : line(l), column(c) {}
};

// Represents a single token in the source code
class Token {
private:
    TokenType type;      // Type of the token
    std::string value;   // Actual text of the token
    Position pos;        // Position in source code

public:
    // Constructor takes ownership of the value string using std::move
    Token(TokenType t, std::string v, Position p) 
        : type(t), value(std::move(v)), pos(p) {}
    
    // Getter methods for token properties
    TokenType get_type() const { return type; }
    const std::string& get_value() const { return value; }
    const Position& get_position() const { return pos; }
};

// Main lexical analyzer class that converts source code into tokens
class Tokenizer {
private:
    const char* input;   // Pointer to input string
    size_t length;       // Length of input
    size_t pos = 0;      // Current position in input
    size_t line = 1;     // Current line number
    size_t column = 1;   // Current column number
    std::vector<Token> tokens;  // Collection of processed tokens

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
        std::string number(input + start, pos - start);
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

public:
    // Constructor takes input string and prepares for tokenization
    Tokenizer(const std::string& source) 
        : input(source.c_str()), length(source.length()) {}

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
                sv word(input + start, pos - start);
                auto it = KEYWORDS.find(word);
                
                if (it != KEYWORDS.end()) {
                    tokens.emplace_back(it->second, std::string(word), 
                                     Position(line, start_col));
                } else {
                    tokens.emplace_back(TokenType::identifier, std::string(word), 
                                     Position(line, start_col));
                }
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

int main(int argc, char *argv[])
{
    // Check command line arguments
    if (argc != 2)
    {
        std::cerr << "Incorrect usage. Correct usage is..." << std::endl;
        std::cerr << "Fire <input.Fe>" << std::endl;
        return EXIT_FAILURE;
    }

    // Open file and check if successful
    std::ifstream input(argv[1]);
    if (!input)
    {
        std::cerr << "Error: Could not open file " << argv[1] << std::endl;
        return EXIT_FAILURE;
    }

    // Read entire file into string
    std::string contents((std::istreambuf_iterator<char>(input)),
                         std::istreambuf_iterator<char>());
    input.close();

    // Print contents for testing
    std::cout << "File contents:" << std::endl;
    std::cout << contents << std::endl;

    // Process the contents
    Tokenizer tokenizer(contents);
    auto tokens = tokenizer.tokenize();
    print_tokens(tokens);

    return EXIT_SUCCESS;
}
