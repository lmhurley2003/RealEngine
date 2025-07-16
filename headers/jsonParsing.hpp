#pragma once
#include <variant>
#include <optional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#define GLM_FORCE_ALIGNED
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/vec3.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>
#include <iostream>

// uncomment to disable assert()
// #define NDEBUG


namespace {
    const std::unordered_set<char> WHITESPACE = { ' ', '\n', '\r', '\t' };
    const std::unordered_set<char> DIGITS = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
    const std::unordered_set<char> NUM_CHARS = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', 'e', 'E', '.' };
    const std::unordered_set<char> ESCAPES = { '"', '\\', '/', 'b', 'f', 'n', 'r', 't' };
    const std::unordered_set<char> HEX_DIGITS = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f' };
}

enum JSONDataType {
    OBJECT = 0,
    ARRAY = 1,
    NUMBER = 2,
    BOOL = 3,
    STRING = 4,
    NULLPTR = 5,
    MONOSTATE = 6
};

enum NumberType {
    INT = 0,
    FLOAT = 1,
    EXPONENTIAL = 2
};


const std::unordered_map<JSONDataType, std::string> JSONDataTypeStrings = { {OBJECT, "OBJECT"}, {ARRAY, "ARRAY"}, {NUMBER, "NUMBER"}, {BOOL, "BOOL"}, {STRING, "STRING"}, {NULLPTR, "NULLPTR"}, {MONOSTATE, "MONOSTATE"} };

typedef struct JSONNumber_struct {
    NumberType type;
    bool negative;
    std::variant<int, size_t, float> val;

    float toFloat() {
        assert(std::holds_alternative<float>(val));
        return std::get<float>(val);
    }
    size_t toSizeT() {
        assert(std::holds_alternative<size_t>(val));
        return std::get<size_t>(val);
    }
    int toInt() {
        assert(std::holds_alternative<int>(val));
        return std::get<int>(val);
    }

    float toFloatDestructive() {
        if (std::holds_alternative<float>(val)) return std::get<float>(val);
        else if (std::holds_alternative<size_t>(val)) return static_cast<float>(std::get<size_t>(val));
        else if (std::holds_alternative<int>(val)) return static_cast<float>(std::get<int>(val));
        else throw std::runtime_error("no value held in variant during call to toFloatDestructive()");
    }

} Number;

struct JSONValue;

using Object = std::unordered_map<std::string, JSONValue>;

using Array = std::vector<JSONValue>;

using Value = struct JSONValue : std::variant<
    Object,
    Array,
    Number,
    bool,
    std::string,
    std::nullptr_t,
    std::monostate>
{
    using variant::variant;

    JSONDataType type = MONOSTATE;

    Object toObject() {
        return std::get<Object>(*this);
    }

    Array toArray() {
        return std::get<Array>(*this);
    }

    Number toNumber() {
        return std::get<Number>(*this);
    }

    const std::string toString() {
        return std::get<std::string>(*this);
    }

    void printType() {
        std::cout << JSONDataTypeStrings.at(type);
    }

    /*template<typename T>
    void set(T _t) {
        emplace(_t);
    }*/

};

class JSONParser {
    size_t i = 0;
    std::string file;
    size_t fileSize = 0;

    bool readJSONFile(std::string filename);

    char at();

    char at(int j);

    //return true if did skip any whitespace
    void skipWhiteSpace();

    Object parseObject();

    Array parseArray();

    Number parseNumber();

    std::string parseString();

    bool parseBool();

    std::nullptr_t parseNull();

    Value parseValue();

public:
    JSONParser(std::string _filename, bool* fileGood);

    JSONParser();

    Value parse();
};

struct JSONUtils {
    static Value getVal(const Object& obj, std::string keyName, JSONDataType type, bool CHECK_VALIDITY = false);

    static std::string getName(const Object& JSONObj, bool CHECK_VALIDITY = false);

    static void getFloat3(Array arrVal, float* A, float* B, float* C);

    static std::vector<size_t> getIndices(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY = false);

    static std::vector<std::string> getIndicesNames(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY = false);

    static std::vector<float> getFloats(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY = false);

    static glm::vec3 getVec3(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY = false);

    //static glm::vec4 getVec4(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY = false);

    static glm::quat getQuat(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY = false);
};