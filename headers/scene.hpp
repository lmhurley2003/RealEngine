#pragma once
#define NOMINMAX
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <string>

#include "jsonParsing.hpp"
#include "parameters.hpp"
#include "entityComponent.hpp"
#include "material.hpp"
#include "light.hpp"
#include "mesh.hpp"
#include "animation.hpp"
#include "camera.hpp"

//TODO consider saving as simple mat4 ?
struct Transform {
	//quat contructor is w, x, y, z
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 translation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);

	Transform(glm::quat _rotation, glm::vec3 _translation, glm::vec3 _scale) : rotation(_rotation), translation(_translation), scale(_scale) {};
	Transform() = default;

	//TODO make a 4x3 matric since these transformations are affine anf thus don't need bottom row?s
	glm::mat4 localToParent() const;
	glm::mat4 parentToLocal() const;
	glm::mat4 cameraLocalToParent() const;
	glm::mat4 cameraParentToLocal() const;

	void matchOrbitControl(const OrbitControl& orbit);

	static glm::mat4 localToParent(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);
	static glm::mat4 parentToLocal(glm::vec3 translation, glm::quat rotation, glm::vec3 scale);
};


//TODO graph a vector for static geometry ? How then to handle dynamic geometry that is parented to static geo ? 
struct Scene {
	std::string name;
	std::string fileVersion;

	struct SceneNode {
		Transform transform{};
		Entity entity{};
		//arbitrary other child of parent of entity
		//loop through this->sibling->sibling->... to loop through all children of parent of this entity
		entitySize_t sibling = std::numeric_limits<entitySize_t>().max(); 
		entitySize_t child = std::numeric_limits<entitySize_t>().max();
		//since having parent does make much sense for scene trees that aren't disjoint 
		// (nodes can be referenced by more than one node), this should be used limitedly, 
		// as of now this is only be being used for getting the camera transform
		entitySize_t parent = std::numeric_limits<entitySize_t>().max();

		//for some reason using default causes error with clang, but this works...
		SceneNode() noexcept{}; //NOTE creates new entity
		SceneNode(Transform _transform, entitySize_t _sibling, entitySize_t _child) : transform(_transform), sibling(_sibling), child(_child) {};

		const bool hasSibling() const {
			return (sibling != std::numeric_limits<entitySize_t>().max());
		}
		const bool hasChild() const {
			return (child != std::numeric_limits<entitySize_t>().max());
		}
		const bool hasParent() const {
			return (parent != std::numeric_limits<entitySize_t>().max());
		}
	};

	//ID of arbitrary root scene node in scene graph, will inevitably be first name in roots of SCENE node
	entitySize_t rootID = std::numeric_limits<entitySize_t>().max();
	bool sceneHasRoot() {
		return rootID != std::numeric_limits<entitySize_t>().max();
	}

	EnitityComponents<SceneNode> graph{};
	EnitityComponents<Mesh> meshes{};
	EnitityComponents<Material> materials{};
	entitySize_t renderCameraID = std::numeric_limits<entitySize_t>().max();
	entitySize_t cullingCameraID = std::numeric_limits<entitySize_t>().max();
	bool sceneHasCamera() {
		return renderCameraID != std::numeric_limits<entitySize_t>().max();
	}
	bool sceneHasFrustumCamera() {
		return cullingCameraID != std::numeric_limits<entitySize_t>().max();
	}
	EnitityComponents<Camera> cameras{};
	EnitityComponents<OrbitControl> orbitControls{};
	EnitityComponents<Light> lights{};
	EnitityComponents<Environment> environments{};
	EnitityComponents<Driver> drivers{};

	Scene() = default;
	Scene(std::string filename, const ModeConstantParameters& parameters = ModeConstantParameters());
	void printScene(const ModeConstantParameters& parameters);
	struct DrawParameters {
		glm::mat4 modelMat = glm::mat4();
		std::vector<Mesh>::const_iterator mesh;
	};

	void updateDrivers(float totalElapsed, const ModeConstantParameters& parameters = ModeConstantParameters());
	glm::mat4 getParentToLocalFullSingular(entitySize_t entityID);
	bool frustumCull(const std::vector<glm::vec4>& frustumPlanes, const Bounds& meshBounds, const glm::mat4& modelMat);
	void drawScene(std::vector<DrawParameters>& drawParams, glm::mat4& viewTransform, glm::mat4& projTransform, const ModeConstantParameters& parameters = ModeConstantParameters());

	entitySize_t addSceneNode(entitySize_t parent = std::numeric_limits<entitySize_t>().max(), SceneNode node = SceneNode());
	entitySize_t addCamera(entitySize_t parent = std::numeric_limits<entitySize_t>().max(), const Camera& camera = Camera());
	entitySize_t addOrbitCamera(entitySize_t parent = std::numeric_limits<entitySize_t>().max(), const OrbitControl& orbit = OrbitControl(), const Camera& camera = Camera());

private:
	enum objType : uint8_t {
		SCENE,
		NODE,
		MESH,
		CAMERA,
		DRIVER,
		DATA, // TODO is this even ever used ?
		MATERIAL,
		ENVIRONMENT,
		LIGHT,
		NONE
	};
	struct tmpNodeData {
		//TODO reconfigure JSON parser to pass references, may save a lot in excessive copy calls?
		Object object;
		uint32_t references = 0; //how many nodes actually reference this node ? 
	};
	struct tmpNodeIdx {
		objType type;
		std::string name;

		bool operator==(const tmpNodeIdx& other) const {
			return type == other.type && name == other.name;
		}
	};
	struct tmpNodeIdxHasher {
		std::size_t operator()(const tmpNodeIdx& key) const {
			std::size_t h2 = std::hash<uint8_t>{}(static_cast<uint8_t>(key.type));
			std::size_t h1 = std::hash<std::string>{}(key.name);
			return h1 ^ (h2 << 1);
		}
	};

	std::unordered_map<tmpNodeIdx, tmpNodeData, tmpNodeIdxHasher> tempGraph{};
	//names may be aliased so we need a map per type of component
	//for SceneNode, val is entityID,
	//for all else (componenets) idxs into _data components of EntityComponent arrays
	std::unordered_map<tmpNodeIdx, uint32_t, tmpNodeIdxHasher> tempComponents{}; 
	std::vector<std::pair<std::string, Object>> tempDrivers{};
	std::vector<Vertex> tempDebugVertices{};

	SceneNode initNode(const Object& JSONObj, const ModeConstantParameters& parameters, const entitySize_t parent);
	Mesh initMesh(const Object& JSONObj, const ModeConstantParameters& parameters);
	Material initMaterial(const Object& JSONobj, const ModeConstantParameters& parameters);
	Camera initCamera(const Object& JSONObj, const ModeConstantParameters& parameters);
	Environment initEnvironment(const Object& JSONObj, const ModeConstantParameters& parameters);
	Light initLight(const Object& JSONObj, const ModeConstantParameters& parameters);
	Driver initDriver(const Object& JSONObj, entitySize_t entityID, const ModeConstantParameters& parameters);
};