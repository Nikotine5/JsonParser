#ifndef JSONPARSER_H
#define JSONPARSER_H
#include "Tokenizer.hpp"
#include "JsonValue.hpp"
class JsonParser { 
private:
    Tokenizer m_tokenizer;
    Token m_current;
    int m_depth = 0;
    void advance();
    [[noreturn]]
    void error(const std::string& msg) const;
    void consume(TokenType expected);
    JsonValue parseValue();
    JsonValue ParseArray();
    JsonValue ParseObj();
public:
    explicit JsonParser(std::string_view input);
    JsonValue parse();
};
#endif