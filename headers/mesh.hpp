#pragma once
#include "parameters.hpp"
#include "vertexIndex.hpp"
#include "jsonParsing.hpp"

struct Bounds {
	//glm::vec3 min = glm::vec3(0.0f);
	//glm::vec3 max = glm::vec3(0.0f);
	float minx = 0.0f, minY = 0.0f, minZ = 0.0f;
	float maxX = 0.0f, maxY = 0.0f, maxZ = 0.0f;
};

//TODO put in own mesh.hpp files
struct Mesh {
	Bounds bounds = Bounds();
	uint32_t indexOffset = 0; //offset into index buffer
	uint32_t numIndices = 0; //num indices
	uint32_t material = 0; //idx into material array

	//loads mesh data in place and copies vertex data into vertex buffer
	void loadMeshData(const std::string filename, const Object& JSONObj, const ModeConstantParameters& parameters);
	void toIndexed(const std::vector<Vertex>& srcBuffer);
};