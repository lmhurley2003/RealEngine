#pragma once
#include "mode.hpp"
#include "scene.hpp"

#include "commandArgs.hpp"
#include <vector>
#include <array>
#include <deque>


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

	//actions that just triggered this frame
	std::array<double, Input::ActionType_t::MISC + 1> actionsHeld = std::array<double, Input::ActionType_t::MISC + 1>{0.0};
	//actions that are held down, +1 for each key pertaining to that action is held (so may be more than 1)
	std::array<double, Input::ActionType_t::MISC+1> actionsDown;
	//actions that are just released this frame
	std::array<double, Input::ActionType_t::MISC+1> actionsReleased;
	//saved position is in 0-1 range
	double cursorXPosition = 0.0f;
	double cursorYPosition = 0.0f;
	double cursorXOffset = 0.0f;
	double cursorYOffset = 0.0f;

	//scene local to this program instance
	Scene scene{};
	entitySize_t sceneCamera = std::numeric_limits<entitySize_t>().max();
	entitySize_t userCamera = std::numeric_limits<entitySize_t>().max();
	bool debugViewMode = false;

	//functions called by main loop:
	virtual bool handleEvent(std::queue<Input::Event>& eventQueue, glm::uvec2 const& window_size) override;

	virtual void update(float deltaTime, float totalTime) override;

	virtual void draw(const App& core, VkCommandBuffer commandBuffer, uint32_t imageIndex) override; //TODO make draw a part of program?
};