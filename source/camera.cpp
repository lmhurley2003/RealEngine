//#include <numbers> // For std::numbers::pi
#define _USE_MATH_DEFINES
#include <cmath> // For fmod()
#include <algorithm>
#include "camera.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>


void OrbitControl::update(double scroll, double turnCursorHorizontal, double turnCursorVertical,
										 double moveCursorHorizontal, double moveCursorVertical) {
	if (scroll != 0.0) {
		distance -= scroll * distance * SCROLL_FACTOR;
		distance = std::max(distance, MIN_DISTANCE);
	}

	if (turnCursorHorizontal != 0.0) {
		azimuth -= turnCursorHorizontal * TURN_FACTOR;
		azimuth = fmod(azimuth, 2.0 * M_PI);
	}

	if (turnCursorVertical != 0.0) {
		altitude += turnCursorVertical * TURN_FACTOR;
		altitude = std::clamp(altitude, -M_PI / 2.0, M_PI / 2.0);
	}

	if (moveCursorVertical != 0.0) {
		orbitPoint.z += moveCursorVertical * distance * MOVE_FACTOR;
	}

	if (moveCursorHorizontal != 0.0) {
		glm::vec3 xyOrientation = distance * glm::vec<3, double>(std::cos(azimuth), std::sin(azimuth), 0.0f);
		xyOrientation = glm::normalize(xyOrientation);
		glm::vec3 localHoriztonalAxis = glm::cross(-xyOrientation, glm::vec3(0.0f, 0.0f, 1.0f));
		orbitPoint -= static_cast<float>(moveCursorHorizontal * distance * MOVE_FACTOR) * localHoriztonalAxis;
	}

	return;
}


std::pair<glm::quat, glm::vec3> OrbitControl::toOrientationPosition() const {
	glm::vec3 pos = distance * glm::vec<3, double>(std::cos(altitude) * std::cos(azimuth), std::cos(altitude) * std::sin(azimuth), std::sin(altitude));
	pos += orbitPoint;

	glm::quat pitch = glm::angleAxis(static_cast<float>(M_PI/2.0 - altitude), glm::vec3(1.0f, 0.0f, 0.0f)); 
	glm::quat yaw = glm::angleAxis(static_cast<float>(M_PI/2 + azimuth), glm::vec3(0.0f, 0.0f, 1.0f)); 
	glm::quat rot = yaw * pitch;

	return {rot, pos};
}