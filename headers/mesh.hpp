#pragma once
#include "parameters.hpp"
#include "vertexIndex.hpp"
#include "jsonParsing.hpp"
#include <array>

struct Bounds {
	//glm::vec3 min = glm::vec3(0.0f);
	//glm::vec3 max = glm::vec3(0.0f);88
	float minX = std::numeric_limits<float>().max();
	float minY = std::numeric_limits<float>().max();
	float minZ = std::numeric_limits<float>().max();
	float maxX = std::numeric_limits<float>().min();
	float maxY = std::numeric_limits<float>().min();
	float maxZ = std::numeric_limits<float>().min();

	static inline const float MIN_AXIS_SIZE = 0.001f;

	void enclose(float x, float y, float z);
	void enclose(glm::vec3 pt);
	void enclose(Bounds bounds);

	inline bool isIn(glm::vec3 point) {
		return !(point.x < minX || point.x > maxX || point.y < minY || point.y > maxX || point.z < minZ || point.z > maxZ);
	}
	inline bool isIn(float pX, float pY, float pZ) {
		return !(pX < minX || pX > maxX || pY < minY || pY > maxX || pZ < minZ || pZ > maxZ);
	}

	//set zero-length axes to MIN_AXIS_SIZE
	void fixZeroVolume();
};

//TODO put in own mesh.hpp files
struct Mesh {
	Bounds bounds = Bounds();
	uint32_t indexOffset = 0; //offset into index buffer
	uint32_t numIndices = 0; //num indices
	uint32_t material = 0; //idx into material array
	uint32_t debugVertexOffset = 0; //offset from the START of TEMP_DEBUG_VERTICES

	// since the indices for each debug bounds will be the same minus a fixed offset
	// will keep track of the index to the start of the tempDebugVertices part of the vertex buffer
	// so indexing into debug bounds vertices will be indices start : sharedDebugIndexOffset, num indices: DEBUG_BOUNDS_INDICES_SIZE, vertexOffset : 
	inline static uint32_t sharedDebugIndexOffset = 0; //offset into indices to access start of debug bounds cube rendering (dont need num indices since size is fixed)
	inline static uint32_t sharedDebugVertexOffset = 0;
	inline static constexpr float BOUNDS_INFLATE_FACTOR = 0.00f; //inflate bounds by a little bit so it doesn't clip/zfight with mesh
	/*
	*    7------6
	*   / |    /|
	*  4------5 |
	*  |  |   | |
	*  |  3---| 2
	*  | /    |/
	*  0------1
	*/
	static inline constexpr uint32_t DEBUG_BOUNDS_INDICES_SIZE = 19; //see indices in initMesh
	static inline const std::array<Index, DEBUG_BOUNDS_INDICES_SIZE> debugIndices = { 0, 1, 2, 3, 0, 4, 5, 6, 7, 4,
																					 PRIMITIVE_RESTART_IDX, 3, 7,
																					 PRIMITIVE_RESTART_IDX, 2, 6,
															                         PRIMITIVE_RESTART_IDX, 1, 5 };

	//loads mesh data in place and copies vertex data into vertex buffer
	void loadMeshData(const std::string filename, const Object& JSONObj, const ModeConstantParameters& parameters);
	void toIndexed(const std::vector<Vertex>& srcBuffer);
};