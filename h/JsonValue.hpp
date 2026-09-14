#ifndef JSONVALUE_HPP
#define JSONVALUE_HPP
#include <variant>
#include <vector>
#include <string>
#include <map>
struct JsonValue;
using JsonArray = std::vector<JsonValue>;
//im doing it like this for a reaon not as an oversight
//O(log n) #Needthat
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
    explicit JsonValue(JsonObject obj);

    bool isNull() const;
    bool isBool() const;
    bool isDouble() const;
    bool isString() const;
    bool isArray() const;
    bool isObject() const;

    bool getBool() const;
    double getDouble() const;
    std::string getString();
    const std::string& getString() const;
    JsonArray& getArray();
    const JsonArray& getArray() const;
    JsonObject& getObject();
    const JsonObject& getObject() const;

    std::string toString() const;

    //template because i want it for smtg but we do that later
    template <typename Visitor>
    auto getValue(Visitor&& visitor) const {
    return std::visit(std::forward<Visitor>(visitor), m_value);
}
};

#endif // JSONVALUE_HPP
