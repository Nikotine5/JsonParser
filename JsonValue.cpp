#include <variant>
#include <vector>
#include <string>
#include <map>
struct JsonValue;
using JsonArray = std::vector<JsonValue>;
using JsonObject = std::map<std::string, JsonValue>;


struct JsonValue {
    
    using Value = std::variant<
        std::nullptr_t,
        bool,
        double,
        std::string,
        JsonArray,
        JsonObject
    >;

    Value value;

    JsonValue() : value(nullptr) {}

    JsonValue(bool b) : value(b) {}

    JsonValue(double d) : value(d) {}

    JsonValue(std::string str) : value(std::move(str)) {}

    JsonValue(JsonArray arr) : value(std::move(arr)) {}

    JsonValue(JsonObject obj) : value(std::move(obj)) {}
};