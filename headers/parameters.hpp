#pragma once
#include <string>

struct ModeConstantParameters {
	std::string SCENE_NAME = "";
	std::string START_CAMERA_NAME = "";
	bool DERIVE_PIPELINES = false;
	int MULTI_SAMPLES = 1;
	bool FRUSTUM_CULLING = false;
	bool OCCLUSION_CULLING = false;
	bool STRIPIFY = false;
	bool CLUSTER = false;
	bool CLUSTER_SIZE = 64;
	bool DEBUG = false;
	int DEBUG_LEVEL = 0;
	bool PRINT_DEBUG_OUTPUT = false;
	ModeConstantParameters() = default;
};

//TODO when eventually get rid of command line arguments
struct GlobalConstantParameters { 
    bool HEADLESS = false;
    std::string PHYSICAL_DEVICE = "";
    bool LIST_PHYSICAL_DEVICES = false;
    bool SEPERATE_QUEUE_FAMILIES = true;
    uint32_t DEBUG_LEVEL = 0;
	bool DEBUG = false;
    bool PRINT_DEBUG = false;
    std::string SWAPCHAIN_MODE = "fifo";
    bool FORCE_SHOW_FPS = false;
    bool COMBINED_INDEX_VERTEX = true;
    bool STRIPIFY = false;
    GlobalConstantParameters() = default;
};