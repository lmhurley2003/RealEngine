#pragma once
#include <utility>
#include "glm/vec3.hpp"
#include <glm/gtc/quaternion.hpp>


struct Camera {
	enum cameraType_t : uint8_t {
		PERSPECTIVE = 0,
		ORTHOGRAPHIC = 1
	};
	float aspect = 1.77777f;
	float vfov = 1.04719f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	cameraType_t type = PERSPECTIVE;
};

struct OrbitControl {
	glm::vec3 orbitPoint = glm::vec3(0.0f);
	double azimuth = 0.0; // in radians
	double altitude = 0.0; // in radians
	double distance = 3.0; // in meters (hopefully lol)

	inline static double TURN_FACTOR = 4.0;
	inline static double MOVE_FACTOR = 0.75;
	inline static double SCROLL_FACTOR = 0.1;
	inline static double MIN_DISTANCE = 0.05;

	void update(double scroll, double turnCursorHorizontal, double turnCursorVertical, double moveCursorHorizontal, double moveCursorVertical);
	std::pair<glm::quat, glm::vec3> toOrientationPosition() const;
};