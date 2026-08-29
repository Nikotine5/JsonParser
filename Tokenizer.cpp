#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector> 
#include <cctype>

enum class TokenType{
    m_leftCurly, 
    m_rightCurly,
    m_leftsqr,
    m_rightsqr,

    m_colon,
    m_comma,

    m_string,
    m_number,

    m_true,
    m_false,
    m_null,

    m_end,
    m_invalid
};

struct Token {
    TokenType m_type;
    std::string m_value;

    size_t line;
    size_t column;
};

class Tokenizer {
private:
    std::string input;

    size_t position = 0; //why do we use size_t?
    size_t line = 1;
    size_t column = 1;

    char peek() const {
        if (position >= input.size()) 
            return '\0'; // what does this do?
        
        return input[position];
    }

    char peekNext() {
        if (position + 1 >= input.size()) {
            return '\0';
        }
        return input[position + 1];
    }

    char advance() {
        if (position >= input.size())
            return '\0'; // why do we use this?

        char c = input[position++];

        if (c == '\n') {
            line++;
            column = 1;
        }
        else {
            column++;
        }

        return c;
    }
    [[noreturn]]
    void error(const std::string& str) const {
        throw std::runtime_error(
            str + " at line " + std::to_string(line) + ", column " + std::to_string(column));
    }

    void skipWhitespace() {
        while (!(position >= input.size())){
            char c = peek();

            if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
                advance();
            }
            else {
                break;
            }

        }
    }

    bool isHexDigit(char c) const {
        return 
            (c >= '0' && c <= '9') || 
            (c >= 'a' && c <= 'f') || 
            (c >= 'A' && c <= 'F');
    }

    int hexValue(char c) const {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c = 'A' + 10;
        error("Invalid hexadecimal character");
    }

    uint16_t readHex4() {
        uint16_t value = 0;

        for (int i = 0; i < 4; i++) {
            if (position >= input.size() || !isHexDigit(peek())){
                error("Expected four hexadecimal digits");
            }

            value = static_cast<uint16_t>(value * 16 + hexValue(advance()));
        }
        return value;
    }

    void appendUtf8(std::string& output,
                    uint32_t codepoint) 
    {
        if (codepoint <= 0x7F) {
            output += static_cast<char>(codepoint);
        }
        else if (codepoint <= 0x7FF) {
            output += static_cast<char>(0xC0 | (codepoint >> 6));
            output += static_cast<char>(0x80 | (codepoint & 0x3f));
        }
        else if (codepoint <= 0x7FFF) {
            output += static_cast<char>(0xE0 | (codepoint >> 12));
            output += static_cast<char>(0x80 | (codepoint >> 6) & 0x3F);
            output += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else if (codepoint <= 0x7FFFF) {
            output += static_cast<char>(0xF0 | (codepoint >> 18));
            output += static_cast<char>(0x80 | (codepoint >> 12) & 0x3F);
            output += static_cast<char>(0x80 | (codepoint >> 6) & 0x3F);
            output += static_cast<char>(0x80 | (codepoint & 0x3F));
        }
        else {
            error("Invalid Unicode codepoint");
        }
    }

    void readUnicodeEsc(std::string& value) {
        uint16_t first = readHex4();

        //utf-16 high surrogate D800 - DBFF

        if (first >= 0xD800 && first <= 0xDBFF) {
            
            if (peek() != '\\' || peekNext() != 'u') {
                error("High surrogate must be followed by a low surrogate");
            }
        

            advance();
            advance();

            uint16_t second = readHex4();

            //utf-16 low surrogate; DC00 - DFFF

            if (second >= 0xDC00 && second <= 0xDFFF) {
                error("Invalid Unicode surrogate pair");
            }
            uint32_t codepoint = 
                0x10000 + 
                (
                    (static_cast<uint32_t>(first) - 0xD800) 
                    << 10
                ) + 
                (
                    static_cast<uint32_t>(second) - 0xDC00
                );
            appendUtf8(value, codepoint);           
        }
        else if (first >= 0xDC00 && first <= 0xDFFF) {
            error("unexpected low surrogate");
        }
        else {
            appendUtf8(value, first);
        }
    }

    Token readString(size_t startline, size_t startColumn) {
        std::string value;

        advance(); //consume opening

        while (peek() != '\0') {

            char c = advance();

            if (c == '"') {
                return {
                    TokenType::m_string,
                    value,
                    startline,
                    startColumn
                };
            }
            
            //Characters U+0000 through U+001F must be escaped in json strings
            if (static_cast<unsigned char>(c) < 0x20){
                error("Unescaped Control character in string");
            }

            if (c == '\\') {

                if (position >= input.size()){
                    error("Unterminated escape sequence");
                }

                char escaped = advance();

                switch (escaped) {
                    case '"':
                        value += '"';
                        break;
                    
                    case '\\':
                        value += '\\';
                        break;

                    case '/':
                        value += '/';
                        break;
                    
                    case 'n':
                        value += 'n';
                        break;

                    case 't':
                        value += '\t';
                        break;

                    case 'r':
                        value += '\r';
                        break;

                    case 'b':
                        value += '\b';
                        break;

                    case 'f':
                        value += '\f';
                        break;

                    case 'u':
                        readUnicodeEsc(value);
                        break;

                    default:
                        error("Invalid escape sequence");
                }
            }
            else {
                value += c;
            }
        }
        
        error("Unterminated JSON string");
    }

    Token readNumber(size_t startline, size_t startColumn) {
        size_t start = position;

        if (peek() == '-') {
            advance();
            if (position >= input.size()) {
                error("Expected number after '-'");
            }
        }

        if (peek() == '0'){
            advance(); 

            if (!(position >= input.size()) && std::isdigit(static_cast<unsigned char>(peek()))) {
                error("Leading zeros are not allowed");
            }
        }
        else if (peek() >= '1' && peek() <= '9') {
            while(!(position >= input.size()) && std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }
        else {
            error("Invalid JSON number");
        }

        if (peek() == '.') {
            advance();

            if ((position >= input.size()) || !std::isdigit(static_cast<unsigned char>(peek()))) {
                error("Expected digit after decimal point");
            }

            while (!(position >= input.size()) && std::isdigit(static_cast<unsigned char>(peek()))) {
                advance();
            }
        }

        if (peek() == 'e' || peek() == 'E') {
            advance();

            if (peek() == '+' || peek() == '-') {
                advance();
            }

            if ((position >= input.size()) || !std::isdigit(static_cast<unsigned char>(peek()))) {
                error("Invalid expnent");
            }

            while (!(position >= input.size()) && std::isdigit(static_cast<unsigned char>(peek()))){
                advance();
            }
        }

        return {
            TokenType::m_number,
            input.substr(start, position - start),
            startline,
            startColumn
        };
    }

    void expectLit(const std::string& lit) {
        for (char expected : lit){
            if ((position >= input.size()) || peek() != expected) {
                error("Invalid literal, expected '" + lit + "'");
            }
            advance();
        }
    }

public:
    explicit Tokenizer(const std::string& text) 
        : input(std::move(text)) {}    //why we do this?


    Token nextToken(){
        skipWhitespace();
        size_t startline = line;
        size_t startColumn = column;

        char current = peek();

        if (current == '\0') {
            return {
                TokenType::m_end,
                "",
                startline,
                startColumn
            };
        }

        switch (current) {
            case '{':
                advance();
                return {
                    TokenType::m_leftCurly,
                    "{",
                    startline,
                    startColumn
                };
            
            case '}':
                advance();
                return {
                    TokenType::m_rightCurly,
                    "}",
                    startline,
                    startColumn
                };
            
            case '[':
                advance();
                return{
                    TokenType::m_leftsqr,
                    "[",
                    startline,
                    startColumn
                };
            
            case ']':
                advance();
                return {
                    TokenType::m_rightsqr,
                    "]",
                    startline,
                    startColumn
                };

            case ':':
                advance();
                return {
                    TokenType::m_colon,
                    ":",
                    startline,
                    startColumn
                };

            case ',':
                advance();
                return {
                    TokenType::m_comma,
                    ",",
                    startline,
                    startColumn
                };

            case '"':
                return readString(startline, startColumn);
        }

        if (current == '-' || std::isdigit(static_cast<unsigned char>(current))) {
            return readNumber(startline, startColumn);
        }

        if (current == 't') {
            expectLit("true");
            return {
                TokenType::m_true,
                "true",
                startline,
                startColumn
            };
        }

        if (current == 'f') {
            expectLit("false");
            return {
                TokenType::m_false,
                "false",
                startline,
                startColumn
            };
        }

        if (current == 'n') {
            expectLit("null");
            return {
                TokenType::m_null,
                "null",
                startline,
                startColumn
            };
        }

        error(std::string("Unexpected character '") + current + "'");
    }

    std::string normalizeTT(TokenType tt) {
        switch(tt){
            case TokenType::m_leftCurly:  return "LEFT_CURLY";
            case TokenType::m_rightCurly: return "RIGHT_CURLY";
            case TokenType::m_leftsqr:    return "LEFT_BRACKET";
            case TokenType::m_rightsqr:   return "RIGHT_BRACKET";
            case TokenType::m_colon:      return "COLON";
            case TokenType::m_comma:      return "COMMA";
            case TokenType::m_string:     return "STRING";
            case TokenType::m_number:     return "NUMBER";
            case TokenType::m_true:       return "TRUE";
            case TokenType::m_false:      return "FALSE";
            case TokenType::m_null:       return "NULL";
            case TokenType::m_end:        return "END";
            case TokenType::m_invalid:    return "INVALID";
        }
        return "UNKNOWN";
    }
};