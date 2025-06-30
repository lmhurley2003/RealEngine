#pragma once
#include "scene.hpp"
#include <vector>
#include <unordered_map>
#include <variant>
#include <glm/vec2.hpp>
#include <string>
#include <vulkan/vulkan.h>
//A basis for info to hook into the existing vulkan skeleton to define what is this application actually doind
//Hopefully should make a good pipeline for swapping in various demos (ie shadow map demonstration, cascading shado maps, 
// procedural terrain, eg)

//NOTE TO SELF : try to not have to include any Vulkan headers
//TODO make input parameters in CommandArgs a part of this pipeline ?

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

struct PipelineStage {
	PipelineStageT type;
	std::vector<std::pair<ShaderStageT, uint32_t>> shaderInfos;

	PipelineStage(PipelineStageT _t, std::vector<std::pair<ShaderStageT, uint32_t>> _sns) :
		type(_t), shaderInfos(_sns){};
};

// DEBUG_LEVEL
enum : uint32_t {
	NONE = 0,
	SEVERE = 1, //critical issues
	MODERATE = 2, //critical issues + validation
	ALL = 3 //critical issues + validation + sanity checks
};

namespace Config {
	struct ParamMap {
		std::unordered_map<std::string, std::variant< bool, std::string, int, glm::uvec2>> p;

		bool getBool(std::string);
		int getInt(std::string);
		std::string getString(std::string);
		glm::uvec2 getVec(std::string);
	};
	extern ParamMap parameters;

	extern std::vector<Scene> scenes;
	extern Scene currentScene;

	extern const std::vector<const unsigned char*> shaders;
	extern const std::vector<uint32_t> shaderSizes;
	extern std::vector<PipelineStage> shaderStages;

	//--defined in vertex_index.cpp
	struct Vertex;
	extern std::vector<Vertex> vertices;
	extern std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();
	void vertexBufferSize(uint32_t* numElements, uint32_t* elementSize);
	VkVertexInputBindingDescription getVertexBindingDescription();

	struct Index;
	void indexBufferSize(uint32_t* numElements, uint32_t* elementSize);
	extern std::vector<Index> indices;

	//void createVertexIndexBuffer();
	//--

	struct UniformBufferObject;
	VkShaderStageFlags uniformBufferObjectStages();
	uint32_t uniformBufferObjectSize();
	void updateUniformBuffer(std::vector<void*>& uniformBuffersMapped, uint32_t image, float time, uint32_t width, uint32_t height);
};


