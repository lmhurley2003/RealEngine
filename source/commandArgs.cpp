#include <string>
#include <iostream>

#include <commandArgs.hpp>

void throwError(std::string message) {
	std::cerr << message << std::endl;
	exit(1);
}

bool isParam(char* arg) {
	return (strnlen(arg, 1000) > 2) && (arg[0] == '-') && (arg[1] == '-');
}

//TODO : make some of the more expesnsive arguments compiler arguments

// paramater names and default values defined here
ParamMap commandLineParameters = { {
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
		{"swapchain-mode", "mailbox"},
		{"force-show-fps", false},
		{"combined-vertex-index", false},
		{"enable-debug-view", false}
	}
};

bool ParamMap::getBool(std::string s) {
	return std::get<0>(p[s]);
}

std::string ParamMap::getString(std::string s) {
	return std::get<1>(p[s]);
}

int ParamMap::getInt(std::string s) {
	return std::get<2>(p[s]);
}

glm::uvec2 ParamMap::getVec(std::string s) {
	return std::get<3>(p[s]);
}

GlobalConstantParameters ParamMap::toGlobalParameters() {
	GlobalConstantParameters globalParameters{};
	globalParameters.HEADLESS = getBool("headless");
	globalParameters.PHYSICAL_DEVICE = getString("physical-device");
	globalParameters.LIST_PHYSICAL_DEVICES = getBool("list-physical-devices");
	globalParameters.SEPERATE_QUEUE_FAMILIES = getBool("seperate-queue-families");
	globalParameters.DEBUG_LEVEL = getInt("debug-level");
	globalParameters.PRINT_DEBUG = getBool("print-debug-output");
	globalParameters.SWAPCHAIN_MODE = getString("swapchain-mode");
	globalParameters.FORCE_SHOW_FPS = getBool("force-show-fps");
	globalParameters.COMBINED_INDEX_VERTEX = getBool("combined-vertex-index");
	return globalParameters;
}

ModeConstantParameters ParamMap::toModeParameters() {
	ModeConstantParameters modeParameters{};
	modeParameters.SCENE_NAME = getString("scene");
	modeParameters.START_CAMERA_NAME = getString("camera");;
	modeParameters.DERIVE_PIPELINES = getBool("derive-pipelines");;
	modeParameters.MULTI_SAMPLES = getInt("multisamples");
	modeParameters.FRUSTUM_CULLING = getBool("frustum-culling");
	modeParameters.OCCLUSION_CULLING = getBool("occlusion-culling");
	modeParameters.STRIPIFY = getBool("stripify");
	modeParameters.CLUSTER = getBool("cluster");
	modeParameters.CLUSTER_SIZE = getInt("cluster-size");
#if defined(NDEBUG) && NDEBUG
	modeParameters.DEBUG = false;
#else
	modeParameters.DEBUG = true;
#endif
	modeParameters.PRINT_DEBUG_OUTPUT = getBool("print-debug-output");
	modeParameters.DEBUG_LEVEL = getInt("debug-level");
	modeParameters.ENABLE_DEBUG_VIEW = getBool("enable-debug-view");
	return modeParameters;
}

// ================================================================================================
//
// ================================================================================================


//returns true if parsing succeeded, false elsewise
void parseCommandLine(int argc, char* argv[]) {
	for (size_t i = 0; i < argc; i++) {
		char* arg = argv[i];
		if (isParam(arg)) {
			std::string savedArgName = std::string(arg).erase(0, 2);
			if (!commandLineParameters.p.count(savedArgName)) throwError("undefined parameter type : " + savedArgName);


			size_t skipLen = 1;
			while ((i + skipLen < argc) && !isParam(argv[i + skipLen])) {
				std::string modifer = std::string(argv[i + skipLen]);

				//setting paramters and converting from string if necessary
				if (std::holds_alternative<std::string>(commandLineParameters.p[savedArgName])) commandLineParameters.p[savedArgName] = modifer;
				else if (std::holds_alternative<int>(commandLineParameters.p[savedArgName])) commandLineParameters.p[savedArgName] = stoi(modifer);
				else if (std::holds_alternative<glm::uvec2>(commandLineParameters.p[savedArgName])) std::get<3>(commandLineParameters.p[savedArgName])[static_cast<glm::uvec2::length_type>(skipLen - 1)] = stoi(modifer);

				skipLen++;
			}

			if ((skipLen > 1 && std::holds_alternative<bool>(commandLineParameters.p[savedArgName])) ||
				(skipLen > 2 && std::holds_alternative<std::string>(commandLineParameters.p[savedArgName])) ||
				(skipLen > 2 && std::holds_alternative<int>(commandLineParameters.p[savedArgName])) ||
				(skipLen > 3 && std::holds_alternative<glm::uvec2>(commandLineParameters.p[savedArgName]))
				) {
				throwError("parameter given too many arguments" + savedArgName);
			}
			else if ((skipLen < 2 && std::holds_alternative<std::string>(commandLineParameters.p[savedArgName])) ||
				(skipLen < 2 && std::holds_alternative<int>(commandLineParameters.p[savedArgName])) ||
				(skipLen < 3 && std::holds_alternative<glm::uvec2>(commandLineParameters.p[savedArgName]))) {
				throwError("parameter not given enough arguments : " + savedArgName);
			}

			if (std::holds_alternative<bool>(commandLineParameters.p[savedArgName])) commandLineParameters.p[savedArgName] = true;

			i += skipLen - 1;
		}
	}
	return;
}

void printParameters() {
	for (auto it = commandLineParameters.p.cbegin(); it != commandLineParameters.p.cend(); ++it)
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