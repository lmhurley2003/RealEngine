#pragma once
#include "entityComponent.hpp"
#include "material.hpp"
#include "light.hpp"
#include <string>


struct Camera {
	enum cameraType_t: uint8_t {
		PERSPECTIVE = 0,
		ORTHOGRAPHIC = 1
	};
	float aspect = 1.77777f;
	float vfov = 1.04719f;
	float nearPlane = 0.1f;
	float farPlane = 100.0f;
	cameraType_t type = PERSPECTIVE;
};

struct Mesh {
	uint32_t indexOffset; //offset into index buffer
	uint32_t meshSize; //num indices
	uint32_t material = 0; //idx into material array
};

struct Scene {
	//traverse down tree by looping through next of root, foeach descend into child
	Entity graph; //one of roots of scene grpah
	EnitityComponents<Entity> next{}; 
	EnitityComponents<Entity> child{}; 

	EnitityComponents<Light> lights{};
	std::vector<Material> materials{};

	Scene() = default;
	Scene(std::string filename);
};