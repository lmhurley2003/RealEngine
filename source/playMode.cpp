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
#include "debugColorFrag.cpp"


PlayMode::PlayMode() : Mode(commandLineParameters.toModeParameters()) {
	descriptorBindings = { {{ UNIFORM_BUFFER, VERTEX_STAGE, 1, {sizeof(UniformBuffer)}, -1 }, 
							{ COMBINED_IMAGE_SAMPLER, FRAGMENT_STAGE, 1, {LINEAR_SAMPLER}, -1 }} };

	shaders = {triBufferTexturedMatrixVert, dotLightingFrag, debugColorFrag};
	shaderSizes = { triBufferTexturedMatrixVertSize, dotLightingFragSize, debugColorFragSize};
	shaderStages = { {MAIN_RENDER, {{VERTEX_STAGE, 0}, {FRAGMENT_STAGE, 1}}}, 
									{DEBUG_DRAW, {{VERTEX_STAGE, 0}, {FRAGMENT_STAGE, 2}}} };
	pushConstantRanges = { {static_cast<ShaderStageT>(VERTEX_STAGE | FRAGMENT_STAGE), 0, static_cast<uint32_t>(sizeof(PushConsants))} };


	if(sizeof(Vertex) != (sizeof(float) * 12 + sizeof(uint32_t))) {
		std::cerr << "Vertex not packed! expected" << (sizeof(float) * 12 + sizeof(uint32_t)) << "got" << sizeof(Vertex) << std::endl;
		throw std::runtime_error("");
	};
	scene = Scene(modeParameters.SCENE_NAME, modeParameters);
	
	sceneCamera = scene.renderCameraID;
	userCamera = scene.addOrbitCamera();
	return;
}

bool PlayMode::handleEvent(std::queue<Input::Event>& events, glm::uvec2 const& windowSize) {
	actionsDown.fill(0.0);
	actionsReleased.fill(0.0);
	cursorXOffset = 0.0;
	cursorYOffset = 0.0;

	while (!events.empty()) {
		Input::Event event = events.front();
		if (event.type == Input::Event::KEY_DOWN) {
			if (Input::actionMap.count(event.key)) {
				if (actionsHeld[Input::actionMap[event.key]] == 0.0) {
					actionsDown[Input::actionMap[event.key]] = 1;
				}
				actionsHeld[Input::actionMap[event.key]] += 1;
			}
			else {
				if (actionsHeld[Input::MISC] == 0) actionsDown[Input::MISC] = 1;
				actionsHeld[Input::MISC] += 1;
			}
		}
		else if (event.type == Input::Event::KEY_RELEASE) {
			if (Input::actionMap.count(event.key)) {
				if (actionsHeld[Input::actionMap[event.key]] == 1) {
					actionsReleased[Input::actionMap[event.key]] = 1;
				}
				actionsHeld[Input::actionMap[event.key]] -= 1;
			}
			else {
				if (actionsHeld[Input::MISC] == 1) actionsReleased[Input::MISC] = 1;
				actionsHeld[Input::actionMap[event.key]] -= 1;
			}
		}
		else if (event.type == Input::Event::SCROLL_EVENT) {
			actionsDown[Input::SCROLL_HORIZONAL] += event.xOffset;
			actionsDown[Input::SCROLL_VERTICAL] += event.yOffset;
		} //else, offset set to zero

		if (event.type == Input::Event::CURSOR_MOVE) {
			cursorXOffset += (event.xPos / static_cast<double>(windowSize.x) - cursorXPosition);
			cursorYOffset += (event.yPos / static_cast<double>(windowSize.y) - cursorYPosition);
			cursorXPosition = event.xPos / static_cast<double>(windowSize.x);
			cursorYPosition = event.yPos / static_cast<double>(windowSize.y);
		}
		events.pop();
	}
	return true;
}

void PlayMode::update(float deltaTime, float totalTime) {
	if (actionsDown[Input::DEBUG_VIEW] >= 0.9) {
		debugViewMode = !debugViewMode;
		scene.cullingCameraID = sceneCamera;
	}

	if (actionsDown[Input::SCENE_CAMERA] >= 0.9) {
		scene.renderCameraID = sceneCamera;
		scene.cullingCameraID = sceneCamera;
	}
	if (actionsDown[Input::USER_CAMERA] >= 0.9) {
		scene.renderCameraID = userCamera;
		if (debugViewMode) scene.cullingCameraID = sceneCamera;
		else scene.cullingCameraID = scene.renderCameraID;
	}

	//only can switch to next/prev camera if we are rendering from a scene camera
	if (actionsDown[Input::NEXT_CAMERA] >= 0.9 && sceneCamera == scene.renderCameraID) {
		std::unordered_map<entitySize_t, uint32_t>::const_iterator sceneCameraIterator = scene.cameras.mapIterator(sceneCamera);
		sceneCameraIterator = std::next(sceneCameraIterator);
		if (sceneCameraIterator == scene.cameras.mapEnd()) sceneCameraIterator = scene.cameras.mapBegin();
		sceneCamera = sceneCameraIterator->first;
		scene.renderCameraID = sceneCamera;
		scene.cullingCameraID = sceneCamera;
	}

	if (actionsDown[Input::PREV_CAMERA] >= 0.9 && sceneCamera == scene.renderCameraID) {
		std::unordered_map<entitySize_t, uint32_t>::const_iterator sceneCameraIterator = scene.cameras.mapIterator(sceneCamera);
		if (sceneCameraIterator == scene.cameras.mapBegin()) sceneCameraIterator = scene.cameras.mapEnd();
		sceneCameraIterator = std::prev(sceneCameraIterator);
		sceneCamera = sceneCameraIterator->first;
		scene.renderCameraID = sceneCamera;
		scene.cullingCameraID = sceneCamera;
	}

	if (scene.renderCameraID == userCamera && scene.orbitControls.contains(userCamera)) {
		double scrollVertOffset = actionsDown[Input::SCROLL_VERTICAL];
		double turnCursorHorizontal = 0.0f;
		double turnCursorVertical = 0.0f;
		double moveCursorHorizontal = 0.0f;
		double moveCursorVertical = 0.0f;
		if (actionsHeld[Input::SHIFT] >= 0.9 && ((actionsHeld[Input::LEFT_CLICK] >= 0.9 && actionsHeld[Input::ALT] >= 0.9) || actionsHeld[Input::MIDDLE_CLICK] >= 0.9)) {
			moveCursorHorizontal = cursorXOffset;
			moveCursorVertical = cursorYOffset;
		}
		else if ((actionsHeld[Input::LEFT_CLICK] >= 0.9 && actionsHeld[Input::ALT] >= 0.9) || actionsHeld[Input::MIDDLE_CLICK] >= 0.9) {
			turnCursorHorizontal = cursorXOffset;
			turnCursorVertical = cursorYOffset;
		}

		if (scrollVertOffset != 0.0 || turnCursorHorizontal != 0.0f || turnCursorVertical != 0.0 || moveCursorHorizontal != 0.0 || moveCursorVertical != 0.0) {
			OrbitControl& orbit = scene.orbitControls.get(userCamera);
			orbit.update(scrollVertOffset, turnCursorHorizontal, turnCursorVertical, moveCursorHorizontal, moveCursorVertical);
			scene.graph.get(userCamera).transform.matchOrbitControl(orbit);
		}
	}

	scene.updateDrivers(totalTime, modeParameters);
}

void PlayMode::draw(const App& core, VkCommandBuffer commandBuffer, uint32_t imageIndex) {
	uint32_t numIndices, indicesSize;
	indexBufferSize(&numIndices, &indicesSize);

	std::vector<Scene::DrawParameters> drawParams;
	glm::mat4 cameraTransform;
	scene.drawScene(drawParams, cameraTransform);
	glm::mat4 view = glm::inverse(cameraTransform);
	Camera cameraParams = scene.sceneHasCamera() ? scene.cameras.get(scene.renderCameraID) : Camera();
	glm::mat4 proj = glm::perspective(cameraParams.vfov, cameraParams.aspect, cameraParams.nearPlane, cameraParams.farPlane);
	
	UniformBuffer ubo = { view, proj };
	ubo.proj[1][1] *= -1; //difference between OpenGL and Vulkan

	core.updateUniformBuffer(descriptorBindings[0][0].index, &ubo, sizeof(UniformBuffer));

	for (const Scene::DrawParameters& drawParam : drawParams) {
		core.updatePushConstants(commandBuffer, (ShaderStageT)(VERTEX_STAGE | FRAGMENT_STAGE), sizeof(PushConsants), 0, &drawParam.modelMat);
		vkCmdDrawIndexed(commandBuffer, drawParam.numIndices, 1, drawParam.indicesStart, 0, 0);
	}

	//draw bounds of each mesh for debugging purposes
	if (modeParameters.ENABLE_DEBUG_VIEW && debugViewMode) {
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, core.pipelines.at(DEBUG_DRAW));
		for (const Scene::DrawParameters& drawParam : drawParams) {
			core.updatePushConstants(commandBuffer, (ShaderStageT)(VERTEX_STAGE | FRAGMENT_STAGE), sizeof(PushConsants), 0, &drawParam.modelMat);
			vkCmdDrawIndexed(commandBuffer, Mesh::DEBUG_BOUNDS_INDICES_SIZE, 1, Mesh::sharedDebugIndexOffset, Mesh::sharedDebugVertexOffset + drawParam.debugIndicesStart, 0);
		}
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
	case Input::Event::SCROLL_EVENT:
		std::cout << "SCROLL_EVENT";
		break;
	default:
		std::cout << "SOME UNKOWN EVENT ?";
		break;
	}
	std::cout << std::endl;
}

PlayMode::~PlayMode() {
}