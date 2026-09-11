#include "JsonValue.hpp"
#include <stdexcept>
   
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

std::string JsonValue::getString() const {
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

JsonObject& JsonValue::getObject() {

    if (!isObject()) {
        throw std::invalid_argument("Value is not an object");
    }

    return std::get<JsonObject>(m_value);
}

template <typename Visitor>
auto JsonValue::getValue(Visitor&& visitor) const {
    return std::visit(std::forward<Visitor>(visitor), m_value);
}



