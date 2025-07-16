#include "jsonParsing.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <cassert>

namespace {
    const bool CHECK_VALIDITY = true;
    const bool DEBUG = false;
}

bool JSONParser::readJSONFile(std::string filename) {
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

char JSONParser::at() {
    if (CHECK_VALIDITY) assert(i < fileSize);
    return file[i];
}

char JSONParser::at(int j) {
    if (CHECK_VALIDITY) assert((i + j) < fileSize);
    return file[i + j];
}

void JSONParser::skipWhiteSpace() {
    while (WHITESPACE.contains(at())) {
        i++;
    }
}


Object JSONParser::parseObject() {
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

Array JSONParser::parseArray() {
    if (DEBUG) assert(at() == '[');
    Array retArray{};
    i++;
    [[maybe_unused]]
    size_t lastI = i;
    skipWhiteSpace();
    //TODO why was this here originally? makes no sense to me
    //if (CHECK_VALIDITY && lastI != i) assert(at() == ']');
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

Number JSONParser::parseNumber() {
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
        [[maybe_unused]]
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

std::string JSONParser::parseString() {
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

bool JSONParser::parseBool() {
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

std::nullptr_t JSONParser::parseNull() {
    if (DEBUG) assert(at() == 'n');
    if (CHECK_VALIDITY) assert(at(1) == 'u' && at(2) == 'l' && at(3) == 'l');
    i += 4;
    return nullptr;
}


Value JSONParser::parseValue() {
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

JSONParser::JSONParser(std::string _filename, bool* fileGood) {
    if (_filename.empty()) throw std::runtime_error("JSON file string is null!");
    *fileGood = readJSONFile(_filename);

    fileSize = file.size();
}

JSONParser::JSONParser() {
    //throw std::runtime_error("must include filename when constructing Parser");
}

Value JSONParser::parse() {
    return parseValue();
};

Value JSONUtils::getVal(const Object& obj, std::string keyName, JSONDataType type, bool CHECK_VALIDITY) {
    if (CHECK_VALIDITY && !obj.count(keyName)) {
        std::cerr << "json object does not have key : " << keyName << std::endl;
        throw std::runtime_error("");
    }
    Value retVal = obj.at(keyName);
    if (CHECK_VALIDITY && !(retVal.type == type)) {
        std::cerr << "json object value does not have type" << JSONDataTypeStrings.at(type) << std::endl;
        throw std::runtime_error("");
    }

    return retVal;
};

std::string JSONUtils::getName(const Object& JSONObj, bool CHECK_VALIDITY) {
    Value nameVal = JSONObj.at("name");
    if (CHECK_VALIDITY) assert(nameVal.type == STRING);
    return nameVal.toString();
};

void JSONUtils::getFloat3(Array arrVal, float* A, float* B, float* C) {
    assert(arrVal.size() == 3);
    assert(arrVal[0].type == NUMBER);
    *A = arrVal[0].toNumber().toFloatDestructive();
    assert(arrVal[1].type == NUMBER);
    *B = arrVal[1].toNumber().toFloatDestructive();
    assert(arrVal[2].type == NUMBER);
    *C = arrVal[2].toNumber().toFloatDestructive();
};

std::vector<size_t> JSONUtils::getIndices(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY) {
    std::vector<size_t> retIndices;
    Array indicesArr = getVal(JSONObj, attrName, ARRAY).toArray();
    size_t indicesSize = indicesArr.size();
    retIndices.reserve(indicesSize);
    for (size_t i = 0; i < indicesSize; i++) {
        Value curVal = indicesArr[i];
        if (CHECK_VALIDITY) assert(curVal.type == NUMBER);
        Number num = curVal.toNumber();
        if (CHECK_VALIDITY) assert(!num.negative && num.type == INT && std::holds_alternative<size_t>(num.val));
        retIndices.emplace_back(num.toSizeT());
    }
    return retIndices;
};

std::vector<std::string> JSONUtils::getIndicesNames(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY) {
    std::vector<std::string> retIndicesNames;
    Array indicesArr = getVal(JSONObj, attrName, ARRAY).toArray();
    size_t indicesSize = indicesArr.size();
    retIndicesNames.reserve(indicesSize);
    for (size_t i = 0; i < indicesSize; i++) {
        Value curVal = indicesArr[i];
        if (CHECK_VALIDITY) assert(curVal.type == STRING);
        std::string name = curVal.toString();
        retIndicesNames.emplace_back(name);
    }
    return retIndicesNames;
};

std::vector<float> JSONUtils::getFloats(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY) {
    std::vector<float> retFloats;
    Array floatsArr = getVal(JSONObj, attrName, ARRAY).toArray();
    size_t floatsSize = floatsArr.size();
    retFloats.reserve(floatsSize);
    for (size_t i = 0; i < floatsSize; i++) {
        Value curVal = floatsArr[i];
        if (CHECK_VALIDITY) assert(curVal.type == NUMBER);
        retFloats.emplace_back(curVal.toNumber().toFloatDestructive());
    }
    return retFloats;
};

glm::vec3 JSONUtils::getVec3(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY) {
    Value nameVal = JSONObj.at(attrName);
    if (CHECK_VALIDITY) assert(nameVal.type == ARRAY);
    Array arrVal = nameVal.toArray();
    if (CHECK_VALIDITY) assert(arrVal.size() == 3);
    if (CHECK_VALIDITY) assert(arrVal[0].type == NUMBER);
    float x = arrVal[0].toNumber().toFloatDestructive();
    if (CHECK_VALIDITY) assert(arrVal[1].type == NUMBER);
    float y = arrVal[1].toNumber().toFloatDestructive();
    if (CHECK_VALIDITY) assert(arrVal[2].type == NUMBER);
    float z = arrVal[2].toNumber().toFloatDestructive();
    return glm::vec3(x, y, z);
};

glm::quat JSONUtils::getQuat(const Object& JSONObj, std::string attrName, bool CHECK_VALIDITY) {
    if (CHECK_VALIDITY) assert(JSONObj.count(attrName) > 0);
    Value nameVal = JSONObj.at(attrName);
    if (CHECK_VALIDITY) assert(nameVal.type == ARRAY);
    Array arrVal = nameVal.toArray();
    if (CHECK_VALIDITY) assert(arrVal.size() == 4);
    if (CHECK_VALIDITY) assert(arrVal[0].type == NUMBER);
    float x = arrVal[0].toNumber().toFloatDestructive();
    float y = arrVal[1].toNumber().toFloatDestructive();
    float z = arrVal[2].toNumber().toFloatDestructive();
    float w = arrVal[3].toNumber().toFloatDestructive();
    return glm::quat(w, x, y, z); //TODO Change input order to reflect own implemenetation
};