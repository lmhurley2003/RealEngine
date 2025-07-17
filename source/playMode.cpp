#include "playMode.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include "vulkanCore.hpp"

#include "triBufferTexturedMatrixVert.cpp"
#include "dotLightingFrag.cpp"


PlayMode::PlayMode() : Mode(commandLineParameters.toModeParameters()) {
	descriptorBindings = { {{ UNIFORM_BUFFER, VERTEX_STAGE, 1, {sizeof(UniformBuffer)}, -1 }, 
							{ COMBINED_IMAGE_SAMPLER, FRAGMENT_STAGE, 1, {LINEAR_SAMPLER}, -1 }} };

	shaders = {triBufferTexturedMatrixVert, dotLightingFrag };
	shaderSizes = { triBufferTexturedMatrixVertSize, dotLightingFragSize };
	shaderStages = { {MAIN_RENDER, {{VERTEX_STAGE, 0}, {FRAGMENT_STAGE, 1}}} };
	pushConstantRanges = { {static_cast<ShaderStageT>(VERTEX_STAGE | FRAGMENT_STAGE), 0, static_cast<uint32_t>(sizeof(PushConsants))} };


	if(sizeof(Vertex) != (sizeof(float) * 12 + sizeof(uint32_t))) {
		std::cerr << "Vertex not packed! expected" << (sizeof(float) * 12 + sizeof(uint32_t)) << "got" << sizeof(Vertex) << std::endl;
		throw std::runtime_error("");
	};
	scene = Scene(modeParameters.SCENE_NAME, modeParameters);
	

	return;
}

bool PlayMode::handleEvent(Input::Event const& event, glm::uvec2 const& window_size) {
	return true;
}

void PlayMode::update(float deltaTime, float totalTime) {
	scene.updateDrivers(totalTime, modeParameters);
}

void PlayMode::draw(const App& core, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	uint32_t numIndices, indicesSize;
	indexBufferSize(&numIndices, &indicesSize);

	std::vector<Scene::DrawParameters> drawParams;
	glm::mat4 cameraTransform;
	scene.drawScene(drawParams, cameraTransform);
	glm::mat4 view = glm::inverse(cameraTransform);
	Camera cameraParams = scene.sceneHasCamera() ? scene.cameras.get(scene.cameraID) : Camera();
	glm::mat4 proj = glm::perspective(cameraParams.vfov, cameraParams.aspect, cameraParams.nearPlane, cameraParams.farPlane);
	glm::mat4 viewMatrix = glm::lookAt(glm::vec3(4.0f, 4.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), core.WIDTH / static_cast<float>(core.HEIGHT), 0.01f, 10.0f);
	//UniformBuffer ubo = { viewMatrix, projMatrix };
	UniformBuffer ubo = { viewMatrix, projMatrix };
	ubo.proj[1][1] *= -1; //difference between OpenGL and Vulkan

	core.updateUniformBuffer(descriptorBindings[0][0].index, &ubo, sizeof(UniformBuffer));
	//cameraTransform = 
	for (const Scene::DrawParameters& drawParam : drawParams) {
		core.updatePushConstants(commandBuffer, (ShaderStageT)(VERTEX_STAGE | FRAGMENT_STAGE), sizeof(PushConsants), 0, &drawParam.modelMat);
		vkCmdDrawIndexed(commandBuffer, drawParam.numIndices, 1, drawParam.indicesStart, 0, 0);
	}
}


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

PlayMode::~PlayMode() {
}