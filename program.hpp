#pragma once
#include "scene.hpp"
#include <vector>
#include "commandArgs.hpp"
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
	std::vector<std::pair<ShaderStageT, std::string>> shaders;

	PipelineStage(PipelineStageT _t, std::vector<std::pair<ShaderStageT, std::string>> _sns) :
		type(_t), shaders(_sns){};
};

struct Program {
	ParamMap parameters;
	std::vector<Scene> scenes;
	Scene currentScene;
	std::vector<PipelineStage> stages;

	Program(ParamMap _p, std::vector<Scene> _scs, Scene _s, std::vector<PipelineStage> _sts) : 
		parameters(_p), scenes(_scs), currentScene(_s), stages(_sts) {};

	Program() : parameters(ParamMap{}), scenes({}), currentScene({}), stages({}) {};
};

Program curProgram();
