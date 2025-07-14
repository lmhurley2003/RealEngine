#pragma once
#include <string>
#include <map>
#include <variant>
#include <unordered_map>
#include <glm/vec2.hpp>
#include "parameters.hpp"

struct ParamMap {
	std::unordered_map<std::string, std::variant< bool, std::string, int, glm::uvec2>> p;

	bool getBool(std::string);
	int getInt(std::string);
	std::string getString(std::string);
	glm::uvec2 getVec(std::string);


	ModeConstantParameters toModeParameters();
	GlobalConstantParameters toGlobalParameters();
};
extern ParamMap commandLineParameters;

void parseCommandLine(int argc, char* argv[]);

void printParameters();

