#pragma once
#include "mode.hpp"
#include "scene.hpp"

#include "commandArgs.hpp"
#include <vector>
#include <array>
#include <deque>

//Definitions to remember
/*
// DEBUG_LEVEL
enum : uint32_t {
	NONE = 0,
	SEVERE = 1, //critical issues
	MODERATE = 2, //critical issues + validation
	ALL = 3 //critical issues + validation + sanity checks
};

enum ShaderStageT : uint32_t {
	VERTEX,
	TESSELATION_CONTROL,
	TESSELATION_EVALUATION,
	GEOMETRY,
	MESH,
	CLUSTER_CULLING,
	COMPUTE,
	FRAGMENT
};

enum PipelineStageT : uint32_t {
	SHADOWMAP,
	PREPROCESS,
	GBUFFER,
	MAIN_RENDER,
	DEBUG_DRAW,
	POSTPROCESS,
	UI
};

//matches enum definitions for VK_TYOE_DESCRITOR_BIT
enum DescriptorTypeT : uint32_t {
	SAMPLER = 0,
	COMBINED_IMAGE_SAMPLER = 1,
	SAMPLED_IMAGE = 2,
	STORAGE_IMAGE = 3,
	UNIFORM_TEXEL_BUFFER = 4,
	STORAGE_TEXEL_BUFFER = 5,
	UNIFORM_BUFFER = 6,
	STORAGE_BUFFER = 7,
	UNIFORM_BUFFER_DYNAMIC = 8,
	STORAGE_BUFFER_DYNAMIC = 9,
	INPUT_ATTACHMENT = 10,
};


struct DescriptorBinding {
	DescriptorTypeT type;
	int count = 1;
	int size = 0; //used for uniform and storage buffers
	int bufferIndex = -1; //idx into buffer saved side of App;
};

*/

struct PlayMode : Mode {
	PlayMode();
	virtual ~PlayMode();

	//descriptor / stage dependencies / shaders
	struct UniformBuffer {
		alignas(16) glm::mat4 view = glm::mat4(1.0f);
		alignas(16) glm::mat4 proj = glm::mat4(1.0f);
	};

	struct PushConsants {
		alignas(16) glm::mat4 model;
	};

	//----- game state -----

	//input tracking:       //debug console is last elements
	//for digital inputs, 0 if released, 1 if pressed
	//cursor move ranges can vary
	//std::array<float, Input::ActionType_t::DEBUG_CONSOLE> actions;

	//scene local to this program instance
	Scene scene{};

	//functions called by main loop:
	virtual bool handleEvent(Input::Event const&, glm::uvec2 const& window_size) override;

	virtual void update(float deltaTime, float totalTime) override;

	virtual void draw(const App& core, VkCommandBuffer commandBuffer, uint32_t imageIndex) override; //TODO make draw a part of program?
};