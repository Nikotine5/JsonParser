#ifndef TOKENIZER_HPP
#define TOKENIZER_HPP

#include <iostream>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector> 
#include <cctype>
#include "Buffer.hpp"

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
    Buffer buffer;
    size_t line = 1;
    size_t column = 1;

    int peek() const;
    int peekNext();
    int advance();
    [[noreturn]]
    void error(const std::string& str) const;
    void skipWhitespace();
    bool isHexDigit(int c) const;
    int hexValue(int c) const;
    uint16_t readHex4();
    void appendUtf8(std::string& output, uint32_t codepoint);
    void readUnicodeEsc(std::string& value);
    Token readString(size_t startline, size_t startColumn);
    Token readNumber(size_t startline, size_t startColumn);
    void expectLit(const std::string& lit);

public:
    explicit Tokenizer(int fd);
    Token nextToken();
    std::string normalizeTT(TokenType tt);
};

#endif // TOKENIZER_HPP
