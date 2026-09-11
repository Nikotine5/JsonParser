#ifndef JSONVALUE_HPP
#define JSONVALUE_HPP
#include <variant>
#include <vector>
#include <string>
#include <map>
struct JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;



using Value = std::variant<
    std::nullptr_t,
    bool,
    double,
    std::string,
    JsonArray,
    JsonObject
>;

class JsonValue {
private:
    Value m_value;
public:
    JsonValue();
    explicit JsonValue(bool b);
    explicit JsonValue(double d);
    explicit JsonValue(std::string str);
    explicit JsonValue(JsonArray arr);
    JsonValue(JsonObject obj);
    bool isNull() const;
    bool isBool() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;
    bool getBool() const;
    double getDouble() const;
    std::string getString() const;
    JsonArray& getArray();
    JsonObject& getObject();
    JsonValue getType() const;
    //template because i want it for smtg but we do that later
    template<typename Visitor>
    auto getValue(Visitor&& visitor) const;
};
#endif // JSONVALUE_HPP
