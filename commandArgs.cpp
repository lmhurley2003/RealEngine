#include <string>
#include <iostream>

#include "commandArgs.hpp"

//TODO : make some of the more expesnsive arguments compiler arguments

// paramater names and default values defined here
ParamMap parameters = {
	{"resolution", glm::uvec2{1920, 1080}},
	{"scene", "default"},
	{"headless", false},
	{"camera", "default"},
	{"physical-device", "default"},
	{"list-physical-devices", false},
	{"multisamples", static_cast<int>(1)},
	{"derive-pipelines", false},
	{"frustum-culling", false},
	{"occlusion-culling", false},
	{"headless", false},
	{"stripify", false},
	{"cluster", false},
	{"cluster-size", static_cast<int>(64)},
	{"debug-level", static_cast<int>(0)},
	{"separate-queue-families", false},
	{"print-debug-output", false},
	{"swapchain-mode", "mailbox"}
};


// ================================================================================================
//
// ================================================================================================

void throwError(std::string message) {
	std::cerr << message << std::endl;
	exit(1);
}

bool isParam(char* arg) {
	return (strnlen(arg, 1000) > 2) && (arg[0] == '-') && (arg[1] == '-');
}

//returns true if parsing succeeded, false elsewise
void parseCommandLine(int argc, char* argv[]) {
	for (size_t i = 0; i < argc; i++) {
		char* arg = argv[i];
		if (isParam(arg)) {
			std::string savedArgName = std::string(arg).erase(0, 2);
			if (!parameters.count(savedArgName)) throwError("undefined parameter type : " + savedArgName);
	

			size_t skipLen = 1;
			while ((i + skipLen < argc) && !isParam(argv[i + skipLen])) {
				std::string modifer = std::string(argv[i + skipLen]);

				//setting paramters and converting from string if necessary
				if (std::holds_alternative<std::string>(parameters[savedArgName])) parameters[savedArgName] = modifer;
				else if (std::holds_alternative<int>(parameters[savedArgName])) parameters[savedArgName] = stoi(modifer);
				else if (std::holds_alternative<glm::uvec2>(parameters[savedArgName])) std::get<3>(parameters[savedArgName])[static_cast<glm::uvec2::length_type>(skipLen-1)] = stoi(modifer);

				skipLen++;
			}

			if (skipLen > 1 && std::holds_alternative<bool>(parameters[savedArgName]) ||
				skipLen > 2 && std::holds_alternative<std::string>(parameters[savedArgName]) ||
				skipLen > 2 && std::holds_alternative<int>(parameters[savedArgName]) ||
				skipLen > 3 && std::holds_alternative<glm::uvec2>(parameters[savedArgName])
				) {
				throwError("parameter given too many arguments" + savedArgName);
			}
			else if (skipLen < 2 && std::holds_alternative<std::string>(parameters[savedArgName]) ||
					 skipLen < 2 && std::holds_alternative<int>(parameters[savedArgName]) ||
					 skipLen < 3 && std::holds_alternative<glm::uvec2>(parameters[savedArgName])) {
				throwError("parameter not given enough arguments : " + savedArgName);
			}

			if(std::holds_alternative<bool>(parameters[savedArgName])) parameters[savedArgName] = true;

			i += skipLen - 1;
		}
	}

	return;
}

void printParameters() {
	for (auto it = parameters.cbegin(); it != parameters.cend(); ++it)
	{
		std::cout << it->first << ": ";
		if (std::holds_alternative<bool>(it->second)) std::cout << (std::get<0>(it->second) ? "true" : "false");
		else if (std::holds_alternative<std::string>(it->second)) std::cout << std::get<1>(it->second);
		else if (std::holds_alternative<int>(it->second)) std::cout << std::get<2>(it->second);
		else if (std::holds_alternative<glm::uvec2>(it->second)) std::cout << (std::get<3>(it->second)).x << " " << (std::get<3>(it->second)).y;
		std::cout << "\n";
	}
	std::cout << std::endl;
}