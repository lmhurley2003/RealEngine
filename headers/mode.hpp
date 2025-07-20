//inspired from base code of Jim McCann.s 15-466 Computer Game Programming at CMU :
// https://github.com/15-466/15-466-f20-base5/blob/master/Mode.hpp
//Mode also has to declare its descriptors (uniform buffers)

#pragma once
#include <input.hpp>
#include <glm/vec2.hpp>

#include <string>
#include <memory>
#include "parameters.hpp"
#include "vulkan/vulkan.h"

// DEBUG_LEVEL
enum : uint32_t {
	NONE = 0,
	SEVERE = 1, //critical issues
	MODERATE = 2, //critical issues + validation
	ALL_OUTPUT = 3 //critical issues + validation + sanity checks
};


//values same as vulkan enums
//TODO create map from ShaderStageT to VkShaderStageFlagBits in createDescriptorSetLayouts?
enum ShaderStageT : uint32_t {
	VERTEX_STAGE = 0x00000001,
	TESSELLATION_CONTROL_STAGE = 0x00000002,
	TESSELLATION_EVALUATION_STAGE = 0x00000004,
	GEOMETRY_STAGE = 0x00000008,
	FRAGMENT_STAGE = 0x00000010,
	COMPUTE_STAGE = 0x00000020,
	ALL_GRAPHICS_STAGES = 0x0000001F,
	ALL_STAGES = 0x7FFFFFFF,
	TASK_STAGE = 0x00000040,
	MESH_STAGE = 0x00000080,
	CLUSTER_CULLING_HUAWEI_STAGE = 0x00080000,
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

//matches enum definitions for VK_TYPE_DESCRITOR_BIT
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

enum SamplerTypeT : uint32_t {
	LINEAR_SAMPLER = 0
};


struct DescriptorBinding {
	DescriptorTypeT type;
	ShaderStageT stages; //OR to combine stages together
	int count = 1;
	union {
		uint32_t size; //used for uniform and storage buffers
		SamplerTypeT samplerType;
	};
	int index = -1; //idx into sampler/uniform buffer vectors saved on the side of VulkanCore;
};


struct PipelineStage {
	PipelineStageT type;
	std::vector<std::pair<ShaderStageT, uint32_t>> shaderInfos;

	PipelineStage(PipelineStageT _t, std::vector<std::pair<ShaderStageT, uint32_t>> _sns) :
		type(_t), shaderInfos(_sns) {
	};
};

//adding "#include vulkanCore.hpp" causes linking errors
class App;
struct Mode : std::enable_shared_from_this<Mode> {
	virtual ~Mode() {}

	//handle_event is called when new mouse or keyboard events are received:
	// (note that this might be many times per frame or never)
	//The function should return 'true' if it handled the event.
	virtual bool handleEvent(std::queue<Input::Event>& eventQueue, glm::uvec2 const& window_size) { return false; }

	//update is called at the start of a new frame, after events are handled:
	// 'elapsed' is time in seconds since the last call to 'update'
	virtual void update(float deltaTime, float totalTime) {}

	//draw is called after update:
	// inputs : current frame in flight commmand buffer and swapchain image index
	// TODO make cur command buffer and image index a App member so we don't need to pass it in?
	virtual void draw(const App& core, VkCommandBuffer commandBuffer, uint32_t imageIndex) = 0;

	ModeConstantParameters modeParameters;
	
	std::vector<std::vector<DescriptorBinding>> descriptorBindings{};
	std::vector<const unsigned char*> shaders{};
	std::vector<uint32_t> shaderSizes{}; //used if using embedded shaders
	std::vector<PipelineStage> shaderStages{};
	//shader stages, size, offset
	struct PushConstantRange {
		ShaderStageT stages;
		uint32_t offset;
		uint32_t size;
	};
	std::vector<PushConstantRange> pushConstantRanges{};

	Mode(ModeConstantParameters _params) : modeParameters(_params) {};
	Mode() : Mode(ModeConstantParameters()) {};
	
	//Mode::current is the Mode to which events are dispatched.
	// use 'set_current' to change the current Mode (e.g., to switch to a menu)
	static std::shared_ptr<Mode> current;
	static void set_current(std::shared_ptr<Mode> const &);
};