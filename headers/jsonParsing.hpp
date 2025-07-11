#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <variant>
#include <optional>
#include <vector>
#include <unordered_map>
#include <unordered_set>
// uncomment to disable assert()
// #define NDEBUG
#include <cassert>

namespace JSONParser {

    namespace {
        const std::unordered_set<char> WHITESPACE = { ' ', '\n', '\r', '\t' };
        const std::unordered_set<char> DIGITS = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0' };
        const std::unordered_set<char> NUM_CHARS = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', 'e', 'E', '.' };
        const std::unordered_set<char> ESCAPES = { '"', '\\', '/', 'b', 'f', 'n', 'r', 't' };
        const std::unordered_set<char> HEX_DIGITS = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'a', 'b', 'c', 'd', 'e', 'f' };

        const bool CHECK_VALIDITY = false;
        const bool DEBUG = false;
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

    class Parser {
        size_t i = 0;
        std::string file;
        size_t fileSize = 0;

        bool readJSONFile(std::string filename) {
            std::ifstream jsonFile(filename);

            if (!jsonFile.good()) {
                return false;
            }

            if (!jsonFile.is_open()) {
                throw std::runtime_error("failed to open json file");
            }

            std::stringstream stream;
            stream << jsonFile.rdbuf();

            jsonFile.close();

            file = stream.str();

            return true;
        }

        char at() {
            if (CHECK_VALIDITY) assert(i < fileSize);
            return file[i];
        }

        char at(int j) {
            if (CHECK_VALIDITY) assert((i + j) < fileSize);
            return file[i + j];
        }

        //return true if did skip any whitespace
        void skipWhiteSpace() {
            while (WHITESPACE.contains(at())) {
                i++;
            }
        }


        Object parseObject() {
            if (DEBUG) assert(at() == '{');
            i++;

            Object retObject{};
            while (at() != '}') {
                skipWhiteSpace();
                if (CHECK_VALIDITY) assert(at() == '"');
                std::string key = parseString();
                skipWhiteSpace();
                if (CHECK_VALIDITY) assert(at() == ':');
                i++;
                Value value = parseValue();

                retObject.insert({ key, value });

                if (at() == ',') {
                    i++;
                    if (CHECK_VALIDITY) assert(at() != '}');
                }
                else {
                    skipWhiteSpace();
                }
            }
            i++;
            return retObject;
        }

        Array parseArray() {
            if (DEBUG) assert(at() == '[');
            Array retArray{};
            i++;
            size_t lastI = i;
            skipWhiteSpace();
            if (CHECK_VALIDITY && lastI != i) assert(at() == ']');
            while (at() != ']') {
                retArray.emplace_back(parseValue());
                skipWhiteSpace();
                if (CHECK_VALIDITY) assert(at() == ',' || at() == ']');
                if (at() == ']') break;
                else i++;
            }
            i++;
            return retArray;
        }

        Number parseNumber() {
            if (DEBUG) {
                if (!DIGITS.contains(at()) && (at() != '-')) {
                    assert(false);
                }
            }

            Number retNumber{};
            retNumber.type = INT;
            std::string numString = "";
            retNumber.negative = false;
            if (!CHECK_VALIDITY) {
                if (at() == '-') retNumber.negative = true;
                while (NUM_CHARS.contains(at())) {
                    numString += at();
                    if (at() == '.') retNumber.type = FLOAT;
                    else if (at() == 'e' || at() == 'E') retNumber.type = EXPONENTIAL;
                    i++;
                }
            }
            else {
                bool zeroPrefix = false;
                size_t lastI = i;
                while (NUM_CHARS.contains(at())) {
                    numString += at();
                    if (at() == '-') {
                        assert(!retNumber.negative);
                        retNumber.negative = true;
                    }
                    else if (at() == 0 && lastI == i) {
                        zeroPrefix = true;
                    }
                    else if (at() == '.') {
                        assert(retNumber.type == INT && i != lastI);
                        retNumber.type = FLOAT;
                    }
                    else if (at() == 'e' || at() == 'E') {
                        assert(retNumber.type != EXPONENTIAL && i != lastI);
                        assert(at(1) == '+' || at(1) == '-');
                        retNumber.type = EXPONENTIAL;
                        i++;
                        numString += at();
                    }
                    else if (DEBUG) {
                        assert(DIGITS.contains(at()));
                    }

                    i++;
                }
                assert(DIGITS.contains(at(-1))); //does not end in ., e, E, -. +
            }

            if (retNumber.negative && retNumber.type == INT) retNumber.val = stoi(numString);
            else if (retNumber.type == INT) {
                std::stringstream stream(numString);
                size_t numS;
                stream >> numS;
                retNumber.val = numS;
            }
            else retNumber.val = stof(numString);

            return retNumber;
        }

        std::string parseString() {
            if (DEBUG) assert(at() == '"');
            std::string retString = "";
            i++;
            while (at() != '"') {
                retString += at();
                if (at() == '\\') {
                    i++;
                    retString += at();
                    if (!CHECK_VALIDITY) continue;
                    if (at() == 'u') {
                        for (size_t j = 0; j < 4; j++) {
                            i++;
                            assert(HEX_DIGITS.contains(at()));
                            retString += at();
                        }
                    }
                    else {
                        assert(ESCAPES.contains(at()));
                    }
                }
                i++;
            }
            if (DEBUG) assert(at() == '"'); //sanity check
            i++;
            return retString;
        }

        bool parseBool() {
            if (DEBUG) assert(at() == 't' || at() == 'f');
            if (at() == 't') {
                if (CHECK_VALIDITY) assert(at(1) == 'r' && at(2) == 'u' && at(3) == 'e');
                i += 4;
                return true;
            }
            else if (at() == 'f') {
                if (CHECK_VALIDITY) assert(at(1) == 'a' && at(2) == 'l' && at(3) == 's' && at(5) == 'e');
                i += 5;
                return false;
            }
            return false;
        }

        std::nullptr_t parseNull() {
            if (DEBUG) assert(at() == 'n');
            if (CHECK_VALIDITY) assert(at(1) == 'u' && at(2) == 'l' && at(3) == 'l');
            i += 4;
            return nullptr;
        }

        Value parseValue() {
            Value retVal{};
            skipWhiteSpace();
            if (at() == '{') {
                Object retObject = parseObject();
                retVal = retObject;
                retVal.type = OBJECT;
            }
            else if (at() == '[') {
                Array retArray = parseArray();
                retVal = retArray;
                retVal.type = ARRAY;
            }
            else if (at() == '"') {
                std::string retString = parseString();
                retVal = retString;
                retVal.type = STRING;
            }
            else if (at() == 't' || at() == 'f') {
                bool retBool = parseBool();
                retVal = retBool;
                retVal.type = BOOL;
            }
            else if (at() == 'n') {
                std::nullptr_t retNull = parseNull();
                retVal = (retNull);
                retVal.type = NULLPTR;
            }
            else if (DIGITS.contains(at()) || at() == '-') {
                Number retNumber = parseNumber();
                retVal = retNumber;
                retVal.type = NUMBER;
            }
            else {
                fprintf(stderr, "\nunaccounted for character : ---| %c |---\n", at());
                throw std::runtime_error("");
            }
            return retVal;
        }

    public:
        Parser(std::string _filename, bool* fileGood) {
            if (_filename.empty()) throw std::runtime_error("JSON file string is null!");
            *fileGood = readJSONFile(_filename);

            fileSize = file.size();
        }

        Parser() {
            //throw std::runtime_error("must include filename when constructing Parser");
        }

        Value parse() {
            return parseValue();
        };

    };

};