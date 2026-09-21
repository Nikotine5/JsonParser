#include "JsonParser.hpp"
#include <charconv>
#include <system_error>

void JsonParser::advance()
{
    m_current = m_tokenizer.nextToken();
}



[[noreturn]]
void JsonParser::error(const std::string& msg) const 
{
    throw std::runtime_error(
        msg + " at line " + std::to_string(m_current.line) + ", column " + std::to_string(m_current.column));
}



void JsonParser::consume(TokenType expected)
{
    if (m_current.m_type != expected) {
        this->error("Unexpected token");
    }

    advance();
}


JsonValue JsonParser::parseValue()
{
    switch(m_current.m_type) 
    {
        case TokenType::m_string: {
            std::string value = std::move(m_current.m_value);
            advance();
            return JsonValue(std::move(value));
        }


        case TokenType::m_number: 
        {
            double value{};
            const char* first = m_current.m_value.data();
            const char* last = first + m_current.m_value.size();
            const auto [ptr, ec] = std::from_chars(first, last, value);

            if (ec != std::errc{} || ptr != last) {
                this->error("Invalid number");
            }

            advance();
            return JsonValue(value);
        }


        case TokenType::m_true: 
        {
            advance();
            return JsonValue(true);
        }


        case TokenType::m_false: 
        {
            advance();
            return JsonValue(false);
        }


        case TokenType::m_null: 
        {
            advance();
            return JsonValue();
        }
        
        //recursive calls to parse nested structures
        case TokenType::m_leftCurly: {
            if (m_depth >= 1000) {
                this->error("Maximum nesting depth exceeded");
            }
            m_depth++;
            JsonValue result = ParseObj();
            m_depth--;
            return result;
        }


        case TokenType::m_leftsqr: {
            if (m_depth >= 1000) {
                this->error("Maximum nesting depth exceeded");
            }
            m_depth++;
            JsonValue result = ParseArray();
            m_depth--;
            return result;
        }


        default:
            this->error("Unexpected token");

    }
}


JsonValue JsonParser::ParseArray()
{
    consume(TokenType::m_leftsqr);
    JsonArray array;

    if (m_current.m_type == TokenType::m_rightsqr) {
        consume(TokenType::m_rightsqr);
        return JsonValue(std::move(array));
    }

    array.push_back(parseValue());                      //parse the first element of the array


    while (m_current.m_type == TokenType::m_comma) {
        consume(TokenType::m_comma);
        array.push_back(parseValue());
    }

    consume(TokenType::m_rightsqr);

    return JsonValue(std::move(array));
}


JsonValue JsonParser::ParseObj() 
{
    
    consume(TokenType::m_leftCurly);
    JsonObject object;

    if (m_current.m_type == TokenType::m_rightCurly) {
        consume(TokenType::m_rightCurly);
        return JsonValue(std::move(object));
    }


    while (true) {
        if (m_current.m_type != TokenType::m_string) {
            this->error("Expected string as key in object");
        }

        std::string key = std::move(m_current.m_value);
        advance();
        consume(TokenType::m_colon);
        JsonValue value = parseValue();
        object.insert_or_assign(std::move(key), std::move(value));

        if (m_current.m_type != TokenType::m_comma) {
            break;
        }

        consume(TokenType::m_comma);
    }

    consume(TokenType::m_rightCurly);
    return JsonValue(std::move(object));
}


JsonParser::JsonParser(std::string_view input)
    : m_tokenizer(std::string(input)),
      m_current(m_tokenizer.nextToken()){

}

JsonValue JsonParser::parse() 
{
    JsonValue root = parseValue();

    if (m_current.m_type != TokenType::m_end) {
        error("Unexpected data after JSON value");
    }
    
    return root;
}

