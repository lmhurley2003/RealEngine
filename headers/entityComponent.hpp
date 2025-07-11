#pragma once
#define NOMINMAX
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

#include <unordered_map>
#include <cassert>

//TODO make macro to choose uint64_t for entity size instead ?
typedef uint32_t enititySize_t;

//TODO consider saving as simple mat4 ?
struct Transform {
	glm::quat rotation = glm::quat(0.0f, 0.0f, 0.0f, 1.0f);
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	Transform(glm::quat _rotation, glm::vec3 _translation, glm::vec3 _scale) : rotation(_rotation), translation(_translation), scale(_scale) {};
	Transform() = default;

	glm::mat4 localToParent();
	glm::mat4 parentToLocal();

	static glm::mat4 localToParent(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);
	static glm::mat4 parentToLocal(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);
};


struct Entity {
	Transform transform{}; //TODO,should transform be a component, or maybe just (2D? 3D?) position is inherent

private:
	enititySize_t id;
	static uint32_t currentEntities; //current number of entities in scene, incremented on entity addition, decremented on entity deletetion
	static uint32_t totalEntities; //total number of entities instantiated in runtime of scene, incremented on entity addition, NOT decremented on entity deletion
	static const uint32_t ENTITY_IS_ENABLED = (0x1U << 0);
	static const uint32_t ENTITY_IS_STATIC_FLAG = (0x1U << 1); //if not static, is dynamc
	static const uint32_t ENTITY_IS_DRIVER_ANIMATED = (0x1U << 2); //TODO implement driver animations, also maybe don't need static flag, just determine from whether it has animation components
	static const uint32_t ENTITY_HAS_MESH = (0x1U << 3);
	static const uint32_t ENTITY_IS_BONE_ANIMATED = (0x1U << 4);
	static const uint32_t ENTITY_HAS_LIGHT = (0x1U << 5);

	enititySize_t flags = (0x0U | ENTITY_IS_ENABLED | ENTITY_IS_STATIC_FLAG); //TODO is flag paramter necessary ?

public:
	inline bool isStatic();
	inline bool isDriverAnimated();
	inline bool hasMesh();
	inline bool hasLight();
	inline bool isBoneAnimated();

	enititySize_t getID() { return id; };

	Entity() {
		if (!totalEntities) totalEntities = 0;
		if (!currentEntities) currentEntities = 0;
		id = totalEntities;
		currentEntities++;
		totalEntities++;
	};
};

//make actual data a vector, but have unordered_map of type {entity id, internal component vector idx}
template<typename T>
class EnitityComponents {
	std::unordered_map<enititySize_t, T> _data{};
public:
	
	T& get(enititySize_t id) {
		assert(_data.count(id));
		return _data[id];
	}

	T& get(Entity& entity) {
		assert(_data.count(entity.getID()));
		return _data[entity.getID()];
	}
};




