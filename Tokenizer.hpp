#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector> 
#include <cctype>

enum class TokenType {
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

    size_t position = 0;
    size_t line = 1;
    size_t column = 1;

    char peek() const;
    char peekNext();
    char advance();
    [[noreturn]]
    void error(const std::string& str) const;
    void skipWhitespace();
    bool isHexDigit(char c) const;
    int hexValue(char c) const;
    uint16_t readHex4();
    void appendUtf8(std::string& output, uint32_t codepoint);
    void readUnicodeEsc(std::string& value);
    Token readString(size_t startline, size_t startColumn);
    Token readNumber(size_t startline, size_t startColumn);
    void expectLit(const std::string& lit);

public:
    explicit Tokenizer(const std::string& text);
    Token nextToken();
    std::string normalizeTT(TokenType tt);
};

#endif // TOKENIZER_HPP
