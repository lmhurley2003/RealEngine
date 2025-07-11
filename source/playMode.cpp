#include "playMode.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "triBufferTexturedMatrixVert.cpp"
#include "triBufferTexturedFrag.cpp"

bool PlayMode::handleEvent(Input::Event const&, glm::uvec2 const& window_size) {
	return true;
}

void PlayMode::update(float elapsed) {
}

PlayMode::PlayMode(ParamMap commandLineParameters) {

};

PlayMode::PlayMode() {
	//if(modeParameters.SCENE_NAME != "filename")
	//scene = Scene()
	descriptorBindings = { {{ UNIFORM_BUFFER, VERTEX, 1, {sizeof(UniformBuffer)}, -1 }, 
							{ COMBINED_IMAGE_SAMPLER, FRAGMENT, 1, {LINEAR_SAMPLER}, -1 }} };

	shaders = {triBufferTexturedMatrixVert, triBufferTexturedFrag };
	shaderSizes = { triBufferTexturedMatrixVertSize, triBufferTexturedFragSize };
	shaderStages = { {MAIN_RENDER, {{VERTEX, 0}, {FRAGMENT, 1}}} };
}

PlayMode::~PlayMode() {
}