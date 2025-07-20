#include "entityComponent.hpp"

bool Entity::isEnabled() const {
	return (flags & ENTITY_IS_ENABLED);
}

bool Entity::isStatic() const {
	return (flags & ENTITY_IS_STATIC);
}

bool Entity::isDriverAnimated() const {
	return (flags & ENTITY_IS_DRIVER_ANIMATED);
}

bool Entity::hasMesh() const {
	return (flags & ENTITY_HAS_MESH);
}

bool Entity::isBoneAnimated() const {
	return (flags & ENTITY_IS_BONE_ANIMATED);
}

bool Entity::hasLight() const {
	return (flags & ENTITY_HAS_LIGHT);
}

bool Entity::hasCamera() const {
	return (flags & ENTITY_HAS_CAMERA);
}

bool Entity::hasOrbitControl() const {
	return (flags & ENTITY_HAS_ORBIT_CONTROL);
}

bool Entity::hasEnvironmentNode() const {
	return (flags & ENTITY_HAS_ENVIRONMENT_NODE);
}

void Entity::setIsEnabled(bool onOff) {
	if (onOff) (flags |= ENTITY_IS_ENABLED);
	else (flags &= ~ENTITY_IS_ENABLED);
}

void Entity::setIsStatic(bool onOff) {
	if (onOff) (flags |= ENTITY_IS_STATIC);
	else (flags &= ~ENTITY_IS_STATIC);
}
void Entity::setIsDriverAnimated(bool onOff) {
	if (onOff) (flags |= ENTITY_IS_DRIVER_ANIMATED);
	else (flags &= ~ENTITY_IS_DRIVER_ANIMATED);
}
void Entity::setHasMesh(bool onOff) {
	if (onOff) (flags |= ENTITY_HAS_MESH);
	else (flags &= ~ENTITY_HAS_MESH);
}
void Entity::setIsBoneAnimation(bool onOff) {
	if (onOff) (flags |= ENTITY_IS_BONE_ANIMATED);
	else (flags &= ~ENTITY_IS_BONE_ANIMATED);
}

void Entity::setHasLight(bool onOff) {
	if (onOff) (flags |= ENTITY_HAS_LIGHT);
	else (flags &= ~ENTITY_HAS_LIGHT);
}

void Entity::setHasCamera(bool onOff) {
	if (onOff) (flags |= ENTITY_HAS_CAMERA);
	else (flags &= ~ENTITY_HAS_CAMERA);
}

void Entity::setHasOrbitControl(bool onOff) {
	if (onOff) (flags |= ENTITY_HAS_ORBIT_CONTROL);
	else (flags &= ~ENTITY_HAS_ORBIT_CONTROL);
}
void Entity::setHasEnvironmentNode(bool onOff) {
	if (onOff) (flags |= ENTITY_HAS_ENVIRONMENT_NODE);
	else (flags &= ~ENTITY_HAS_ENVIRONMENT_NODE);
}
