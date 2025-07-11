#include "input.hpp"
#include <iostream>

void Input::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (action == GLFW_PRESS) {
		inputEventsQueue.emplace(Event(Event::EventType_t::KEY_DOWN, key, mods));
	}
	else if (action == GLFW_RELEASE) {
		inputEventsQueue.emplace(Event(Event::EventType_t::KEY_RELEASE, key, mods));
	}
	else {
		assert(action == GLFW_REPEAT);
		inputEventsQueue.emplace(Event(Event::EventType_t::KEY_REPEAT, key, mods));
	}
}

void Input::joystickCallback(int jid, int event) {
	inputEventsQueue.emplace(Event(Event::EventType_t::JOYSTICK_CONNECT, jid, event));
}
void Input::cursorEnterCallback(GLFWwindow* window, int entered) {
	inputEventsQueue.emplace(Event(Event::EventType_t::CURSUR_LEAVE_OR_ARRIVE, entered));
}
void Input::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (action == GLFW_PRESS) {
		inputEventsQueue.emplace(Event(Event::EventType_t::KEY_DOWN, button, mods));
	}
	else if (action == GLFW_RELEASE) {
		inputEventsQueue.emplace(Event(Event::EventType_t::KEY_RELEASE, button, mods));
	}
	else {
		assert(action == GLFW_REPEAT);
		inputEventsQueue.emplace(Event(Event::EventType_t::KEY_REPEAT, button, mods));
	}
}

std::queue<Input::Event> Input::inputEventsQueue{};
std::unordered_map<int, Input::ActionType_t> Input::actionMap =
{
	   {GLFW_KEY_W, UP},
	   {GLFW_KEY_UP, UP},

	   {GLFW_KEY_A, LEFT},
	   {GLFW_KEY_LEFT, LEFT},

	   {GLFW_KEY_S, DOWN},
	   {GLFW_KEY_DOWN, DOWN},

	   {GLFW_KEY_D, RIGHT},
	   {GLFW_KEY_RIGHT, RIGHT},

	   {GLFW_MOUSE_BUTTON_LEFT, LEFT_CLICK},
	   {GLFW_MOUSE_BUTTON_RIGHT, RIGHT_CLICK},

	   {GLFW_KEY_SPACE, ACCEPT},
	   {GLFW_KEY_ENTER, ACCEPT},

	   {GLFW_KEY_BACKSPACE, BACK},
	   {GLFW_KEY_ESCAPE, BACK},   //TODO should escape be special input that quits \ brings up quit menu?

	   {GLFW_KEY_BACKSLASH, DEBUG_CONSOLE}
};