#ifndef JSONPARSER_H
#define JSONPARSER_H
#include "Tokenizer.cpp"
#include "jsonValue.cpp"
class JsonParser {
private:
    Tokenizer m_tokenizer;
    Token m_current;

    void advance() {};
    [[noreturn]]
    void error(const std::string& msg){};
    void consume(TokenType expected){};
    JsonValue parseValue(){};
    JsonValue ParseArray(){};
    JsonValue ParseObj(){};


public:
    explicit JsonParser(std::string input);
    JsonValue parse();
};


#endif