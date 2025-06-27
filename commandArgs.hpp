#pragma once
#include <string>
#include <map>
#include <variant>
#include <glm/vec2.hpp>

// DEBUG_LEVEL
enum : uint32_t{
	NONE = 0,
	SEVERE = 1, //critical issues
	MODERATE = 2, //critical issues + validation
	ALL = 3 //critical issues + validation + sanity checks
};

typedef std::map<std::string, std::variant< bool, std::string, int, glm::uvec2>> ParamMap;

extern ParamMap parameters;

void parseCommandLine(int argc, char* argv[]);

void printParameters();

