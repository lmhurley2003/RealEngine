#include "playMode.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "triBufferTexturedMatrixVert.cpp"
#include "triBufferTexturedFrag.cpp"

[[maybe_unused]]
void debugHandleEvent(Input::Event const& event) {
	std::cout << "\nEvent of type : ";
	switch (event.type) {
	case Input::Event::KEY_DOWN:
		std::cout << "KEY_DOWN";
		break;
	case Input::Event::KEY_RELEASE:
		std::cout << "KEY_RELEASE";
		break;
	case Input::Event::KEY_REPEAT:
		std::cout << "KEY_REPEAT";
		break;
	case Input::Event::CURSOR_MOVE:
		std::cout << "CURSOR_MOVE";
		break;
	case Input::Event::CURSUR_LEAVE_OR_ARRIVE:
		std::cout << "CURSUR_LEAVE_OR_ARRIVE";
		break;
	case Input::Event::JOYSTICK_CONNECT:
		std::cout << "JOYSTICK_CONNECT";
		break;
	case Input::Event::SCROLL:
		std::cout << "SCROLL";
		break;
	default:
		std::cout << "SOME UNKOWN EVENT ?";
		break;
	}
	std::cout << std::endl;
}

bool PlayMode::handleEvent(Input::Event const& event, glm::uvec2 const& window_size) {
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