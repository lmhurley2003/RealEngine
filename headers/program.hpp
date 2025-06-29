#pragma once
#include "scene.hpp"
#include <vector>
#include "commandArgs.hpp"
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

struct Program {
	ParamMap parameters;
	std::vector<Scene> scenes;
	Scene currentScene;
	static const std::vector<const unsigned char*> shaders;
	static const std::vector<uint32_t> shaderSizes;
	std::vector<PipelineStage> shaderStages;

	Program(ParamMap _p, std::vector<Scene> _scs, Scene _s, std::vector<PipelineStage> _sts) : 
		parameters(_p), scenes(_scs), currentScene(_s), shaderStages(_sts) {};

	Program() : parameters(ParamMap{}), scenes({}), currentScene({}), shaderStages({}) {};

	struct Vertex;
	static void vertexBufferSize(uint32_t* numElements, uint32_t* elementSize);
	static VkVertexInputBindingDescription getVertexBindingDescription();
	static std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();
	static std::vector<Vertex> vertices;
	struct Index;
	static void indexBufferSize(uint32_t* numElements, uint32_t* elementSize);
	static std::vector<Index> indices;

	struct UniformBufferObject;
	static VkShaderStageFlags uniformBufferObjectStages();
	static uint32_t uniformBufferObjectSize();
	static void updateUniformBuffer(std::vector<void*>& uniformBuffersMapped, uint32_t image, float time, uint32_t width, uint32_t height);
};

Program curProgram();
