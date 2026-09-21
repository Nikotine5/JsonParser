#include "JsonValue.hpp"
#include <stdexcept>

static std::string escapeString(std::string str) {
    std::string out;
    out.reserve(str.size() + 2); // Reserve space for the string and quotes
    out.push_back('"');
    for (unsigned char c : str) {
        switch (c) {
            case '"': out += "\\\""; break;
            case '\\': out += "\\\\"; break;
            case '\b': out += "\\b"; break;
            case '\f': out += "\\f"; break;
            case '\n': out += "\\n"; break;
            case '\r': out += "\\r"; break;
            case '\t': out += "\\t"; break;
            default:
                if (c < 0x20 || c > 0x7E) {
                    char buffer[7];
                    snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    out += buffer;
                } else {
                    out.push_back(c);
                }
        }
    }
    out.push_back('"');
    return out;
}
   
struct overloaded {
    std::string operator()(std::nullptr_t) const { return std::string("null"); }
    std::string operator()(bool b) const { return b ? "true" : "false"; }
    std::string operator()(double d) const { return std::to_string(d); }
    std::string operator()(const std::string& str) const {  return escapeString(str);}
    std::string operator()(const JsonArray& arr) const {
        std::string result = "[";
        for (size_t i = 0; i < arr.size(); ++i) {
            result += arr[i].getValue(*this);
            if (i + 1 < arr.size()) {
                result += ", ";
            }
        }
        result += "]";
        return result;
    }
    std::string operator()(const JsonObject& obj) const {
        std::string result = "{";
        size_t count = 0;
        for (const auto& [key, value] : obj) {
            result += "\"" + key + "\": " + value.getValue(*this);
            if (count + 1 < obj.size()) {
                result += ", ";
            }
            ++count;
        }
        result += "}";
        return result;
    }
};

JsonValue::JsonValue() : m_value(nullptr) {}
JsonValue::JsonValue(bool b) : m_value(b) {}
JsonValue::JsonValue(double d) : m_value(d) {}
JsonValue::JsonValue(std::string str) : m_value(std::move(str)) {}
JsonValue::JsonValue(JsonArray arr) : m_value(std::move(arr)) {}
JsonValue::JsonValue(JsonObject obj) : m_value(std::move(obj)) {}

bool JsonValue::isNull() const {
    return std::holds_alternative<std::nullptr_t>(m_value);
}


bool JsonValue::isBool() const {
    return std::holds_alternative<bool>(m_value);
}


bool JsonValue::isDouble() const {
    return std::holds_alternative<double>(m_value);
}


bool JsonValue::isString() const {
    return std::holds_alternative<std::string>(m_value);
}


bool JsonValue::isArray() const {
    return std::holds_alternative<JsonArray>(m_value);
}


bool JsonValue::isObject() const {
    return std::holds_alternative<JsonObject>(m_value);
}


bool JsonValue::getBool() const {
    if (!isBool()) {
        throw std::invalid_argument("Value is not a boolean");
    }
    return std::get<bool>(m_value);
}
    
double JsonValue::getDouble() const {
    if (!isDouble()) {
        throw std::invalid_argument("Value is not a double");
    }
    return std::get<double>(m_value);
}

const std::string& JsonValue::getString() const {
    if (!isString()) {
        throw std::invalid_argument("Value is not a string");
    }
    return std::get<std::string>(m_value);
}

std::string JsonValue::getString() {
    
    if (!isString()) {
        throw std::invalid_argument("Value is not a string");
    }
    return std::get<std::string>(m_value);
}

JsonArray& JsonValue::getArray() {
    if (!isArray()) {
        throw std::invalid_argument("Value is not an array");
    }
    
    return std::get<JsonArray>(m_value);
}

const JsonArray& JsonValue::getArray() const {
    if (!isArray()) {
        throw std::invalid_argument("Value is not an array");
    }
    
    return std::get<JsonArray>(m_value);
}

JsonObject& JsonValue::getObject() {
    if (!isObject()) {
        throw std::invalid_argument("Value is not an object");
    }

    return std::get<JsonObject>(m_value);
}

const JsonObject& JsonValue::getObject() const {
    if (!isObject()) {
        throw std::invalid_argument("Value is not an object");
    }
    return std::get<JsonObject>(m_value);
}

std::string JsonValue::toString() const {
    return std::visit(overloaded{}, m_value);
}






