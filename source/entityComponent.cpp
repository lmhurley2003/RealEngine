#include "entityComponent.hpp"

uint32_t Entity::totalEntities = 0;
uint32_t Entity::currentEntities = 0;

glm::mat4 Transform::localToParent() {
	return glm::translate((glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale)), translation);
}
glm::mat4 Transform::parentToLocal() {
	glm::vec3 scaleCorrect = glm::vec3(scale.x == 0.0f ? 1.0f : scale.x, scale.y == 0.0f ? 1.0f : scale.y, scale.z == 0.0f ? 1.0f : scale.z);
	return glm::scale((glm::mat4_cast(glm::inverse(rotation)) * glm::translate(glm::mat4(1.0f), -translation)), 1.0f / scaleCorrect);
    //return glm::scale(mat4(1.0f), 1.0f / scale) * toMat4(inverse(rotation)) * translate(mat4(1.0f), -translation);
}

glm::mat4 Transform::localToParent(glm::vec3 translation, glm::quat rotation, glm::vec3 scale) {
	return glm::translate((glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale)), translation);
}
glm::mat4 Transform::parentToLocal(glm::vec3 translation, glm::quat rotation, glm::vec3 scale) {
	glm::vec3 scaleCorrect = glm::vec3(scale.x == 0.0f ? 1.0f : scale.x, scale.y == 0.0f ? 1.0f : scale.y, scale.z == 0.0f ? 1.0f : scale.z);
	return glm::scale((glm::mat4_cast(glm::inverse(rotation)) * glm::translate(glm::mat4(1.0f), -translation)), 1.0f / scaleCorrect);
}

inline bool Entity::isStatic() {
	return (flags & ENTITY_IS_STATIC_FLAG);
}

inline bool Entity::isDriverAnimated() {
	return (flags & ENTITY_IS_DRIVER_ANIMATED);
}

inline bool Entity::hasMesh() {
	return (flags & ENTITY_HAS_MESH);
}

inline bool Entity::isBoneAnimated() {
	return (flags & ENTITY_IS_BONE_ANIMATED);
}

inline bool Entity::hasLight() {
	return (flags & ENTITY_HAS_LIGHT);
}
