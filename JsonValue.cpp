#include "JsonValue.hpp"
#include <stdexcept>
   
JsonValue::JsonValue() : value(nullptr) {}
JsonValue::JsonValue(bool b) : value(b) {}
JsonValue::JsonValue(double d) : value(d) {}
JsonValue::JsonValue(std::string str) : value(std::move(str)) {}
JsonValue::JsonValue(JsonArray arr) : value(std::move(arr)) {}
JsonValue::JsonValue(JsonObject obj) : value(std::move(obj)) {}

bool JsonValue::isNull() const {
    return std::holds_alternative<std::nullptr_t>(value);
}


bool JsonValue::isBool() const {
    return std::holds_alternative<bool>(value);
}


bool JsonValue::isDouble() const {
    return std::holds_alternative<double>(value);
}


bool JsonValue::isString() const {
    return std::holds_alternative<std::string>(value);
}


bool JsonValue::isArray() const {
    return std::holds_alternative<JsonArray>(value);
}


bool JsonValue::isObject() const {
    return std::holds_alternative<JsonObject>(value);
}


bool JsonValue::getBool() const {

    if (!isBool()) {
        throw std::invalid_argument("Value is not a boolean");
    }
    return std::get<bool>(value);
}
    
double JsonValue::getDouble() const {
    if (!isDouble()) {
        throw std::invalid_argument("Value is not a double");
    }
    return std::get<double>(value);
}

std::string JsonValue::getString() const {
    if (!isString()) {
        throw std::invalid_argument("Value is not a string");
    }
    return std::get<std::string>(value);
}

JsonArray JsonValue::getArray() const {

    if (!isArray()) {
        throw std::invalid_argument("Value is not an array");
    }
    
    return std::get<JsonArray>(value);
}

JsonObject JsonValue::getObject() const {

    if (!isObject()) {
        throw std::invalid_argument("Value is not an object");
    }

    return std::get<JsonObject>(value);
}

template <typename T>
const T& JsonValue::getValue() const {
    if (!std::holds_alternative<T>(value)) {
        throw std::invalid_argument("Value is not of the requested type");
    }
    return std::get<T>(value);
}


