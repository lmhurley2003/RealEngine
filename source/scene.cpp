#include "scene.hpp"
#include <algorithm>
#include <stack>
#include "utils.cpp"
#include "jsonParsing.cpp" //TODO why does linking fail if I don't include this
#include <cmath>
#include <algorithm>
#include <random>

glm::mat4 Transform::localToParent() const {
	return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation)* glm::scale(glm::mat4(1.0), scale);
	//return glm::translate((glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale)), translation);
}
glm::mat4 Transform::parentToLocal() const {
	glm::vec3 scaleCorrect = glm::vec3(scale.x == 0.0f ? 1.0f : scale.x, scale.y == 0.0f ? 1.0f : scale.y, scale.z == 0.0f ? 1.0f : scale.z);
	//return glm::scale((glm::mat4_cast(glm::inverse(rotation)) * glm::translate(glm::mat4(1.0f), -translation)), 1.0f / scaleCorrect);
	return glm::scale(glm::mat4(1.0f), 1.0f / scaleCorrect) * glm::mat4_cast(inverse(rotation)) * glm::translate(glm::mat4(1.0f), -translation);
}

//forward is -z axis with +y being upward and +x being rightward
glm::mat4 Transform::cameraParentToLocal() const {
	glm::mat3 rotationMatrix(
		rotation * glm::vec3(1.0f, 0.0f, 0.0f), //right vector
		rotation * glm::vec3(0.0f, 1.0f, 0.0f), // up vector
		rotation * glm::vec3(0.0f, 0.0f, 1.0f) //forward vector
	);
	return glm::mat4_cast(inverse(rotation)) * glm::translate(glm::mat4(1.0f), -translation);
}
glm::mat4 Transform::cameraLocalToParent() const {
	glm::mat3 rotationMatrix(
		rotation * glm::vec3(1.0f, 0.0f, 0.0f), //right vector
		rotation * glm::vec3(0.0f, 1.0f, 0.0f), // up vector
		rotation * glm::vec3(0.0f, 0.0f, 1.0f) //forward vector
	);
	return glm::translate(glm::mat4(1.0f), translation) * glm::mat4_cast(rotation);
}

glm::mat4 Transform::localToParent(glm::vec3 translation, glm::quat rotation, glm::vec3 scale) {
	return glm::translate((glm::mat4_cast(rotation) * glm::scale(glm::mat4(1.0f), scale)), translation);
}
glm::mat4 Transform::parentToLocal(glm::vec3 translation, glm::quat rotation, glm::vec3 scale) {
	glm::vec3 scaleCorrect = glm::vec3(scale.x == 0.0f ? 1.0f : scale.x, scale.y == 0.0f ? 1.0f : scale.y, scale.z == 0.0f ? 1.0f : scale.z);
	return glm::scale((glm::mat4_cast(glm::inverse(rotation)) * glm::translate(glm::mat4(1.0f), -translation)), 1.0f / scaleCorrect);
}

void Transform::matchOrbitControl(const OrbitControl& orbit) {
	std::pair<glm::quat, glm::vec3> orientationPosition = orbit.toOrientationPosition();
	rotation = orientationPosition.first;
	translation = orientationPosition.second;
}

bool isSimpleMaterial(Object obj) {
	return !(obj.count("pbr") || obj.count("lambertian") || obj.count("environment") || obj.count("mirror"));
}


Mesh Scene::initMesh(const Object& JSONObj, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	Mesh retMesh = Mesh();

	Object attributes = JSONUtils::getVal(JSONObj, "attributes", OBJECT).toObject();
	Object position = JSONUtils::getVal(attributes, "POSITION", OBJECT).toObject();
	std::string sourceFile = JSONUtils::getVal(position, "src", STRING).toString();
	
	retMesh.loadMeshData(sourceFile, JSONObj, parameters);

	//add vertices and indices for wireframe cube representing the bounds of the mesh
	if (parameters.ENABLE_DEBUG_VIEW) {

		std::string name = JSONUtils::getVal(JSONObj, "name", STRING).toString();
		uint32_t nameHash = std::hash<std::string>{}(name);
		std::mt19937 randomGen(nameHash); // Seed the Mersenne Twister engine
		std::uniform_int_distribution<uint32_t> distribution(0, std::numeric_limits<uint32_t>::max()); 
		uint32_t randomColor = distribution(randomGen); // Generate a random uint32_t
		randomColor |= 0xFF; //make sure alpha is 1

		Bounds b = retMesh.bounds;
		b.minX -= Mesh::BOUNDS_INFLATE_FACTOR; b.minY -= Mesh::BOUNDS_INFLATE_FACTOR; b.minZ -= Mesh::BOUNDS_INFLATE_FACTOR;
		b.maxX += Mesh::BOUNDS_INFLATE_FACTOR; b.maxY += Mesh::BOUNDS_INFLATE_FACTOR; b.maxZ += Mesh::BOUNDS_INFLATE_FACTOR;
		//sanity check, make sure Bounds b was copied locally and is not a reference
		if (CHECK_VALIDITY) assert(Mesh::BOUNDS_INFLATE_FACTOR == 0.0f || (retMesh.bounds.minX > b.minX));

		/*
		*    7------6
		*   / |    /|
		*  4------5 |
		*  |  |   | |
		*  |  3---| 2
		*  | /    |/
		*  0------1
		*/

		std::vector<glm::vec3> boundsPositions = { /*0*/{b.minX, b.minY, b.minZ}, /*1*/ {b.maxX, b.minY, b.minZ},
												   /*2*/{b.maxX, b.maxY, b.minZ}, /*3*/ {b.minX, b.maxY, b.minZ},
			                                       /*4*/{b.minX, b.minY, b.maxZ}, /*5*/ {b.maxX, b.minY, b.maxZ},
												   /*6*/{b.maxX, b.maxY, b.maxZ}, /*7*/ {b.minX, b.maxY, b.maxZ} };
		Vertex vert{};
		vert.color = randomColor;// randomColor;
		retMesh.debugVertexOffset = tempDebugVertices.size();
		for (glm::vec3 pos : boundsPositions) {
			vert.position = pos;
			tempDebugVertices.emplace_back(vert);
		}
	}
	
	return retMesh;
};

Material Scene::initMaterial(const Object& JSONobj, const ModeConstantParameters& parameters) {
	Material retMaterial = Material();

	return retMaterial;
}

Camera Scene::initCamera(const Object& JSONObj, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	if (CHECK_VALIDITY) assert(JSONUtils::getVal(JSONObj, "type", STRING).toString() == "CAMERA");
	Camera retCamera = Camera();

	if (JSONObj.count("perspective")) {
		const Object& perspectiveObj = JSONUtils::getVal(JSONObj, "perspective", OBJECT).toObject();
		if (perspectiveObj.count("aspect")) retCamera.aspect =  JSONUtils::getVal(perspectiveObj, "aspect", NUMBER).toNumber().toFloatDestructive();
		if (perspectiveObj.count("vfov")) retCamera.vfov = JSONUtils::getVal(perspectiveObj, "vfov", NUMBER).toNumber().toFloatDestructive();
		if (perspectiveObj.count("near")) retCamera.nearPlane = JSONUtils::getVal(perspectiveObj, "near", NUMBER).toNumber().toFloatDestructive();
		if (perspectiveObj.count("far")) retCamera.farPlane = JSONUtils::getVal(perspectiveObj, "far", NUMBER).toNumber().toFloatDestructive();
	}

	return retCamera;
}

Environment Scene::initEnvironment(const Object& JSONObj, const ModeConstantParameters& parameters) {
	Environment retEnvironment = Environment();

	return retEnvironment;
}

Light Scene::initLight(const Object &JSONObj, const ModeConstantParameters& parameters) {
	Light retLight = Light();

	return retLight;
}

Driver Scene::initDriver(const Object &JSONObj, entitySize_t entityID, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	Driver retDriver;

	retDriver.entityID = entityID;
	retDriver.values = JSONUtils::getFloats(JSONObj, "values");
	retDriver.times = JSONUtils::getFloats(JSONObj, "times");
	//set as not static
	graph.get(entityID).entity.setIsStatic(false);
	
	std::string channel = JSONUtils::getVal(JSONObj, "channel", STRING).toString();
	if (channel == "rotation") {
		retDriver.setChannelRotation(true);
		if (CHECK_VALIDITY) assert((retDriver.values.size() % 4) == 0);
	}
	else if (channel == "translation") {
		retDriver.setChannelTranslation(true);
		if (CHECK_VALIDITY) assert((retDriver.values.size() % 3) == 0);
	}
	else {
		if (CHECK_VALIDITY) assert((channel == "scale") && ((retDriver.values.size() % 3) == 0));
		retDriver.setChannelScale(true);
	}
	if (CHECK_VALIDITY) assert(retDriver.values.size() % retDriver.times.size() == 0);

	std::string interpolation = JSONUtils::getVal(JSONObj, "interpolation", STRING).toString();
	if (interpolation == "LINEAR") {
		retDriver.setInterpolationLinear(true);
	}
	else if (interpolation == "SLERP") {
		retDriver.setInterpolationSlerp(true);
	}
	else {
		if (CHECK_VALIDITY) assert(interpolation == "STEP");
		retDriver.setInterpolationStep(true);
	}

	return retDriver;
}

Scene::SceneNode Scene::initNode(const Object& JSONObj, const ModeConstantParameters& parameters, const entitySize_t parent) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	if (CHECK_VALIDITY) assert(JSONObj.count("type") && JSONUtils::getVal(JSONObj, "type", STRING).toString() == "NODE");

	SceneNode retNode = SceneNode();
	retNode.parent = parent;
	//all .s72 transform defaults set in Transform constructor, so if the values don't exist in file we don't need the else clause
	if (JSONObj.count("translation")) {
		retNode.transform.translation = JSONUtils::getVec3(JSONObj, "translation", CHECK_VALIDITY);
	}
	if (JSONObj.count("rotation")) {
		retNode.transform.rotation = JSONUtils::getQuat(JSONObj, "rotation", CHECK_VALIDITY);
	}
	if (JSONObj.count("scale")) {
		retNode.transform.scale = JSONUtils::getVec3(JSONObj, "scale", CHECK_VALIDITY);
	}
	//MESH CASE
	if (JSONObj.count("mesh")) {
		std::string meshName = JSONUtils::getVal(JSONObj, "mesh", STRING).toString();
		retNode.entity.setHasMesh(true);
		if (CHECK_VALIDITY) assert(retNode.entity.hasMesh());
		if (tempComponents.count({MESH, meshName })) {
			uint32_t idx = tempComponents[{MESH, meshName}];
			meshes.insertExisting(retNode.entity, idx);
		}
		else {
			if (CHECK_VALIDITY) assert(tempGraph.count({MESH, meshName }));
			tempGraph[{MESH, meshName}].references++;
			tmpNodeData tempMeshData = tempGraph[{MESH, meshName}];
			const Object& meshObj = tempMeshData.object;
			//load the new mesh
			Mesh newMesh = initMesh(meshObj, parameters);
			int idx = (meshes.insert(retNode.entity, newMesh));
			tempComponents[{MESH, meshName}] = idx;
		}
		//MATERIAL CASE
		const Object& meshObj = tempGraph[{MESH, meshName}].object;
		if (JSONObj.count("material")) {
			std::string materialName = JSONUtils::getVal(meshObj, "material", STRING).toString();
			//no need to set hasMaterial flag cause it's assumed all meshes have a material
			if (tempComponents.count({MATERIAL, materialName })) {
				uint32_t idx = tempComponents[{MATERIAL, materialName}];
				materials.insertExisting(retNode.entity, idx);
			}
			else {
				if (CHECK_VALIDITY) assert(tempGraph.count({ MATERIAL, materialName }));
				tempGraph[{MATERIAL, materialName}].references++;
				tmpNodeData tempMeshData = tempGraph[{MATERIAL, materialName}];
				const Object& materialObj = tempMeshData.object;
				int idx = (materials.insert(retNode.entity, initMaterial(materialObj, parameters)));
				tempComponents[{MATERIAL, materialName}] = idx;
			}
		}

	}
	//CAMERA CASE
	if (JSONObj.count("camera")) {
		std::string cameraName = JSONUtils::getVal(JSONObj, "camera", STRING).toString();
		retNode.entity.setHasCamera(true);
		if(CHECK_VALIDITY) assert(retNode.entity.hasCamera());
		if (tempComponents.count({ CAMERA, cameraName })) {
			uint32_t idx = tempComponents[{CAMERA, cameraName}];
			cameras.insertExisting(retNode.entity, idx);
		}
		else {
			if(CHECK_VALIDITY)assert(tempGraph.count({CAMERA, cameraName}));
			tempGraph[{CAMERA, cameraName}].references++;
			tmpNodeData tempMeshData = tempGraph[{CAMERA, cameraName}];
			const Object& cameraObj = tempMeshData.object;
			//load the new camera
			int idx = (cameras.insert(retNode.entity, initCamera(cameraObj, parameters)));
			tempComponents[{CAMERA, cameraName}] = idx;
			//setting camera from command line
			if (cameraName == parameters.START_CAMERA_NAME) {
				renderCameraID = retNode.entity.getID();
				cullingCameraID = retNode.entity.getID();
			}
			else if (parameters.START_CAMERA_NAME == "default" && !sceneHasCamera()) {
				renderCameraID = retNode.entity.getID();
				cullingCameraID = retNode.entity.getID();
			}
		}
	}
	//ENVIRONMENT CASE
	if (JSONObj.count("environment")) {
		std::string environmentName = JSONUtils::getVal(JSONObj, "environment", STRING).toString();
		retNode.entity.setHasCamera(true);
		if (tempComponents.count({ ENVIRONMENT, environmentName })) {
			uint32_t idx = tempComponents[{ENVIRONMENT, environmentName}];
			cameras.insertExisting(retNode.entity, idx);
		}
		else {
			if (CHECK_VALIDITY) assert(tempGraph.count({ ENVIRONMENT, environmentName }));
			tempGraph[{ENVIRONMENT, environmentName}].references++;
			tmpNodeData tempMeshData = tempGraph[{ENVIRONMENT, environmentName}];
			const Object& environmentObj = tempMeshData.object;
			//load the new camera
			int idx = (environments.insert(retNode.entity, initEnvironment(environmentObj, parameters)));
			tempComponents[{ENVIRONMENT, environmentName}] = idx;
		}
	}
	//LIGHT CASE
	if (JSONObj.count("light")) {
		std::string lightName = JSONUtils::getVal(JSONObj, "light", STRING).toString();
		retNode.entity.setHasCamera(true);
		if (tempComponents.count({ LIGHT, lightName })) {
			uint32_t idx = tempComponents[{LIGHT, lightName}];
			lights.insertExisting(retNode.entity, idx);
		}
		else {
			if (CHECK_VALIDITY) assert(tempGraph.count({LIGHT, lightName}));
			tempGraph[{LIGHT, lightName}].references++;
			tmpNodeData tempMeshData = tempGraph[{LIGHT, lightName}];
			const Object& environmentObj = tempMeshData.object;
			//load the new camera
			int idx = (environments.insert(retNode.entity, initEnvironment(environmentObj, parameters)));
			tempComponents[{LIGHT, lightName}] = idx;
		}
	}

	//guaranteed that node has name
	std::string nodeName = JSONUtils::getVal(JSONObj, "name", STRING).toString();
	//add current scene node to temp components map
	graph.insert(retNode.entity, retNode);
	//idx in temp component is entity ID NOT idx in _data array
	tempComponents[{NODE, nodeName}] = static_cast<uint32_t>(retNode.entity.getID());
	//TODO don't update values of retNode directly from here on since it won;t get updated in scene graph


	//NOW INIT CHILDREN
	//TODO change retNode to refernce to stop repeated calls to graph.get() 
	if (JSONObj.count("children")) {
		std::vector<std::string> childNames = JSONUtils::getIndicesNames(JSONObj, "children", CHECK_VALIDITY);
		
		if (childNames.size() > 0 && tempComponents.count({ NODE, childNames[0] })) {
			entitySize_t id = static_cast<entitySize_t>(tempComponents[{NODE, childNames[0]}]);
			graph.get(retNode.entity).child = id;
			if (CHECK_VALIDITY) assert(graph.contains(id));
		}
		else if (childNames.size() > 0) {
			if(CHECK_VALIDITY) assert(tempGraph.count({NODE, childNames[0]}));
			graph.get(retNode.entity).child = initNode(tempGraph[{NODE, childNames[0]}].object, parameters, retNode.entity.getID()).entity.getID();
		}
		else {
			return graph.get(retNode.entity);
		}

		if(CHECK_VALIDITY) assert(graph.contains(graph.get(retNode.entity).child));
		entitySize_t curSceneNodeID = graph.get(retNode.entity).child;
		entitySize_t siblingID;
		for (uint32_t i = 1; i < childNames.size(); i++) {
			const std::string& childName = childNames[i];
			if (tempComponents.count({ NODE, childName })) {
				siblingID = static_cast<entitySize_t>(tempComponents[{NODE, childName}]);
				graph.get(curSceneNodeID).sibling = siblingID;
				if (CHECK_VALIDITY) assert(graph.contains(siblingID));
			}
			else {
				if(CHECK_VALIDITY) assert(tempGraph.count({NODE, childName}));
				siblingID = initNode(tempGraph[{NODE, childName}].object, parameters, retNode.parent).entity.getID();
				graph.get(curSceneNodeID).sibling = siblingID;
			}
			curSceneNodeID = siblingID;
		}
	}

	return graph.get(retNode.entity);
}

//if no parent is supplied, adds as sibling to root node
entitySize_t Scene::addSceneNode(entitySize_t parent, SceneNode node) {
	//constructor of SceneNode automatically creates a new entity
	entitySize_t entityID = node.entity.getID();

	if (parent != std::numeric_limits<entitySize_t>().max()) {
		SceneNode& parentNode = graph.get(parent);
		if (parentNode.hasChild()) {
			node.sibling = parentNode.child;
		}
		parentNode.child = entityID;
		node.parent = parent;
	}
	else {
		SceneNode& siblingNode = graph.get(rootID);
		if (siblingNode.hasSibling()) {
			node.sibling = siblingNode.sibling;
		}
		siblingNode.sibling = entityID;
	}

	graph.insert(entityID, node);
	return entityID;
}

entitySize_t Scene::addCamera(entitySize_t parent, const Camera& camera) {
	entitySize_t entityID = addSceneNode(parent);
	Entity& entity = graph.get(entityID).entity;
	entity.setHasCamera(true);
	cameras.insert(entityID, camera);

	return entityID;
}


entitySize_t Scene::addOrbitCamera(entitySize_t parent, const OrbitControl& orbit, const Camera& camera) {
	entitySize_t entityID = addCamera(parent, camera);
	Entity& entity = graph.get(entityID).entity;
	entity.setHasOrbitControl(true);

	orbitControls.insert(entityID, orbit);

	//update scene transform to match the implied transform from the orbit
	graph.get(entityID).transform.matchOrbitControl(orbit);

	return entityID;
}

Scene::Scene(std::string filename, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	if (filename == "") throw std::runtime_error("No scene name given to scene constructor!");

	bool fileExists = false;
	JSONParser sceneParser;
	for (std::string tryPath : std::vector<std::string>{filename, filename + ".s72", "scenes\\" + filename, "scenes\\"+filename+".s72"}) {
		sceneParser = JSONParser(tryPath, &fileExists);
		if (fileExists) break;
	}
	if (!fileExists) throw std::runtime_error("Failed to find {scene}, {scene}.s72, scenes/{scene}, or scenes/{scene}.s72!");


	Value retSceneJSON = sceneParser.parse();

	if (CHECK_VALIDITY) assert(retSceneJSON.type == ARRAY);

	std::vector<Value> sceneArr = retSceneJSON.toArray();
	size_t arraySize = sceneArr.size();

	if (CHECK_VALIDITY) assert(sceneArr[0].type == STRING);
	fileVersion = sceneArr[0].toString();

	//only used for scene construction, cleared at end
	std::string sceneName = "";
	for (size_t i = 1; i < arraySize; i++) {
		Value curVal = sceneArr[i];
		if (CHECK_VALIDITY) assert(curVal.type == OBJECT);
		Object curObj = curVal.toObject();

		Value typeV = curObj.at("type");
		if (CHECK_VALIDITY) assert(typeV.type == STRING);
		const std::string type = typeV.toString();
		std::string name = JSONUtils::getVal(curObj, "name", STRING).toString();
		if (type == "SCENE") {
			sceneName = name;
			tempGraph.insert({ {SCENE, name}, {curObj, 1} });
		}
		else if (type == "MESH") {
			tempGraph.insert({ {MESH, name}, {curObj, 0} });
		}
		else if (type == "LIGHT") {
			tempGraph.insert({ {LIGHT, name}, {curObj, 0} });
		}
		else if (type == "NODE") {
			tempGraph.insert({ {NODE, name}, {curObj, 0} });
		}
		else if (type == "CAMERA") {
			tempGraph.insert({ {CAMERA, name}, {curObj, 0} });
		}
		else if (type == "DRIVER") {
			tempDrivers.emplace_back(std::make_pair(name, curObj));
		}
		else if (type == "DATA") {
			tempGraph.insert({ {DATA, name}, { curObj, 0} });
		}
		else if (type == "MATERIAL") {
			tempGraph.insert({ {MATERIAL, name}, {curObj, 0} });
		}
		else if (type == "ENVIRONMENT") {
			tempGraph.insert({ {ENVIRONMENT, name}, {curObj, 0} });
		}
		else {
			std::cerr << "unrecognized object type in element " << i << "of scene file array!";
			assert(false);
		}
	}
	
	//now decend starting from root node, which is SCENE node
	tmpNodeData sceneNode = tempGraph[{SCENE, sceneName}];
	if (!sceneNode.object.count("roots")) {
		tempGraph.clear();
		tempComponents.clear();
		return;
	}

	std::vector<std::string> rootNames = JSONUtils::getIndicesNames(sceneNode.object, "roots");
	if (rootNames.size() > 0) {
		if (CHECK_VALIDITY) assert(tempGraph.count({NODE, rootNames[0] }));
		rootID = initNode(tempGraph[{NODE, rootNames[0]}].object, parameters, std::numeric_limits<entitySize_t>().max()).entity.getID();
		if (CHECK_VALIDITY) assert(graph.contains(rootID));

		entitySize_t curSceneNodeID = rootID;
		for (uint32_t i = 1; i < rootNames.size(); i++) {
			const std::string& rootName = rootNames[i];
			entitySize_t siblingID;
			if (tempComponents.count({ NODE, rootName })) {
				siblingID = static_cast<entitySize_t>(tempComponents[{NODE, rootName}]);
				graph.get(curSceneNodeID).sibling = siblingID;
				if (CHECK_VALIDITY) assert(graph.contains(siblingID));
				//check that assignment actually appears in the graph
				if (CHECK_VALIDITY) assert(graph.get(curSceneNodeID).sibling == siblingID);
			}
			else {
				if (CHECK_VALIDITY) assert(tempGraph.count({NODE, rootName }));
				siblingID = initNode(tempGraph[{NODE, rootName}].object, parameters, std::numeric_limits<entitySize_t>().max()).entity.getID();
				if (CHECK_VALIDITY) assert(graph.contains(siblingID));
				graph.get(curSceneNodeID).sibling = siblingID;
				//check that assignment actually appears in the graph
				if (CHECK_VALIDITY) assert(graph.get(curSceneNodeID).sibling == siblingID);
			}
			curSceneNodeID = siblingID;
		}
	}

	//now insert drivers
	for (const auto& [driverName, driverObject] : tempDrivers) {
		if (CHECK_VALIDITY) assert(driverObject.count("node"));
		std::string nodeName = JSONUtils::getVal(driverObject, "node", STRING).toString();
		if (CHECK_VALIDITY) assert(tempComponents.count({ NODE, nodeName}));
		entitySize_t entityID = tempComponents[{NODE, nodeName}];

		graph.get(entityID).entity.setIsDriverAnimated(true);
		drivers.insert(entityID, initDriver(driverObject, entityID, parameters));
	}

	//now insert debug bounds vertices and indices into true vertex/index buffer
	if (parameters.ENABLE_DEBUG_VIEW) {
		if (CHECK_VALIDITY) assert(tempDebugVertices.size() % 8 == 0);

		Mesh::sharedDebugIndexOffset = indices.size();
		Mesh::sharedDebugVertexOffset = vertices.size();
		vertices.reserve(vertices.size() + tempDebugVertices.size());
		vertices.insert(vertices.end(), tempDebugVertices.begin(), tempDebugVertices.end());
		indices.reserve(indices.size() + Mesh::DEBUG_BOUNDS_INDICES_SIZE);
		indices.insert(indices.end(), Mesh::debugIndices.begin(), Mesh::debugIndices.end());
	}

	tempGraph.clear();
	tempComponents.clear();
	tempDrivers.clear();
	tempDebugVertices.clear();
	return;
}

template<typename T>
T interpolateLinearly(T A, T B, float t) {
	return (1.0f - t) * A + t * B;
}

template<typename T>
T interpolateSlerp(T A, T B, float t) {
	float cosTheta = std::clamp(glm::dot(A, B), 0.0f, 1.0f);
	float angle = std::acos(cosTheta);
	if (angle == 0.0f) return A;
	float co1 = std::sin((1.0f - t) * angle) / std::sin(angle);
	float co2 = std::sin(t * angle) / std::sin(angle);
	return co1 * A + co2 * B;
}

void Scene::updateDrivers(float elapsed, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	for (auto it = drivers.dataBegin(); it != drivers.dataEnd(); ++it) {
		const Driver& driver = *it;
		entitySize_t entityID = driver.entityID;
		if (!graph.get(driver.entityID).entity.isEnabled()) continue;
		if (CHECK_VALIDITY) assert(graph.get(driver.entityID).entity.isDriverAnimated());

		Transform& transform = graph.get(entityID).transform;

		float tMod = fmod(elapsed, driver.times.back());
		std::vector<float>::const_iterator tUpperIt = std::upper_bound(driver.times.begin(), driver.times.end(), tMod);
		if (CHECK_VALIDITY) assert(tUpperIt != driver.times.end());
		//TODO is this ternary necessary ? is it even possible to get the last element of the array?
		std::vector<float>::const_iterator tLowerIt = tUpperIt == driver.times.begin() ? tUpperIt : (tUpperIt - 1);
		float t = tUpperIt == tLowerIt ? 0.0f : (tMod - *tLowerIt) / (*tUpperIt - *tLowerIt);

		uint32_t tLowerIdx = tLowerIt - driver.times.begin();
		uint32_t tUpperIdx = tUpperIt - driver.times.begin();
		if (driver.isChannelRotation()) {
			glm::quat valA = glm::quat(driver.values[4 * tLowerIdx + 3], driver.values[4 * tLowerIdx], driver.values[4 * tLowerIdx + 1], driver.values[4 * tLowerIdx + 2]);
			glm::quat valB = glm::quat(driver.values[4 * tUpperIdx + 3], driver.values[4 * tUpperIdx], driver.values[4 * tUpperIdx + 1], driver.values[4 * tUpperIdx + 2]);
			if (driver.isInterpolationLinear()) transform.rotation = interpolateLinearly<glm::quat>(valA, valB, t);
			else if (driver.isInterpolationSlerp()) transform.rotation = interpolateSlerp<glm::quat>(valA, valB, t);
			else transform.rotation = valA;
		}
		else {
			if (CHECK_VALIDITY) assert(driver.isChannelTranslation() || driver.isChannelScale());
			glm::vec3 valA = glm::vec3(driver.values[3 * tLowerIdx], driver.values[3 * tLowerIdx + 1], driver.values[3 * tLowerIdx + 2]);
			glm::vec3 valB = glm::vec3(driver.values[3 * tUpperIdx], driver.values[3 * tUpperIdx + 1], driver.values[3 * tUpperIdx + 2]);
			glm::vec3 ret;
			if (driver.isInterpolationLinear()) ret = interpolateLinearly<glm::vec3>(valA, valB, t);
			else if (driver.isInterpolationSlerp()) ret = interpolateSlerp<glm::vec3>(valA, valB, t);
			else ret = valA;

			if (driver.isChannelTranslation()) transform.translation = ret;
			else transform.scale = ret;
		}
	}
}

glm::mat4 Scene::getParentToLocalFullSingular(entitySize_t entityID) {
	glm::mat4 retTransform = glm::mat4(1.0f);
	SceneNode curNode = graph.get(entityID);
	do {
		retTransform = retTransform * curNode.transform.parentToLocal();
		if(curNode.hasParent()) curNode = graph.get(curNode.parent);
	} while (curNode.hasParent());

	return retTransform;
}

//TODO is there a frustum x bounding box check that doesn't require 8 matrix multiplications ?
bool Scene::frustumCull(const std::vector<glm::vec4>& frustumPlanes, const Bounds& meshBounds, const glm::mat4& modelMat) {
	//get new world space bounding box by transforming 
	const glm::vec3 corners[8] = {
		glm::vec3(modelMat * glm::vec4(meshBounds.minX, meshBounds.minY, meshBounds.minZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.minX, meshBounds.maxY, meshBounds.minZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.minX, meshBounds.minY, meshBounds.maxZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.minX, meshBounds.maxY, meshBounds.maxZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.maxX, meshBounds.minY, meshBounds.minZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.maxX, meshBounds.maxY, meshBounds.minZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.maxX, meshBounds.minY, meshBounds.maxZ, 1.0f)),
		glm::vec3(modelMat * glm::vec4(meshBounds.maxX, meshBounds.maxY, meshBounds.maxZ, 1.0f))
	};

	Bounds newBounds = Bounds();
	for (size_t i = 0; i < 8; ++i) {
		newBounds.enclose(corners[i]);
	}

	for (size_t i = 0; i < frustumPlanes.size(); ++i) {
		const glm::vec4& g = frustumPlanes[i];
		if ((glm::dot(g, glm::vec4(newBounds.minX, newBounds.minY, newBounds.minZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.maxX, newBounds.minY, newBounds.minZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.minX, newBounds.maxY, newBounds.minZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.maxX, newBounds.maxY, newBounds.minZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.minX, newBounds.minY, newBounds.maxZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.maxX, newBounds.minY, newBounds.maxZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.minX, newBounds.maxY, newBounds.maxZ, 1.0f)) < 0.0) &&
			(glm::dot(g, glm::vec4(newBounds.maxX, newBounds.maxY, newBounds.maxZ, 1.0f)) < 0.0))
		{
			// Not visible - all returned negative
			return false;
		}
	}

	return true;
}

void Scene::drawScene(std::vector<DrawParameters>& drawParams, glm::mat4& viewTransform, glm::mat4& projTransform, const ModeConstantParameters& parameters) {
	//scene transform
	std::stack <std::pair<glm::mat4, entitySize_t>> drawStack{};
	drawStack.push({glm::mat4(1.0f), rootID}); //populates mat4's diagonal with 1.0f (ie identity matrix)
	//assumes no loops in scene tree
	bool cameraSet = false;

	//CULLING SETUP
	glm::mat4 frustumView;
	frustumView = sceneHasFrustumCamera() ? getParentToLocalFullSingular(cullingCameraID) : glm::mat4(1.0f);
	if (renderCameraID == cullingCameraID) {
		viewTransform = frustumView;
		cameraSet = true;
	}
	glm::mat4 frustumProj;
	if (sceneHasFrustumCamera()) {
		const Camera& cam = cameras.get(cullingCameraID);
		frustumProj = glm::perspective(cam.vfov, cam.aspect, cam.nearPlane, cam.farPlane);
	}
	else {
		Camera cam = Camera();
		frustumProj = glm::perspective(cam.vfov, cam.aspect, cam.nearPlane, cam.farPlane);
	}
	frustumProj[1][1] *= -1;
	if (renderCameraID == cullingCameraID) {
		projTransform = frustumProj;
	}

	glm::mat4 cullingMatrix = glm::transpose(frustumProj * frustumView);
	const std::vector<glm::vec4> frustumPlanes = {
		// left, right, bottom, top
		(cullingMatrix[3] + cullingMatrix[0]),
		(cullingMatrix[3] - cullingMatrix[0]),
		(cullingMatrix[3] + cullingMatrix[1]),
		(cullingMatrix[3] - cullingMatrix[1]),
		// near, far
		(cullingMatrix[3] + cullingMatrix[2]),
		(cullingMatrix[3] - cullingMatrix[2]),
	};

	while (!drawStack.empty()) {
		std::pair<glm::mat4, entitySize_t> nodeEntry = drawStack.top();
		drawStack.pop();
		glm::mat4 curTransform = nodeEntry.first;
		entitySize_t curEntityID = nodeEntry.second;
		const SceneNode& curSceneNode = graph.get(curEntityID);
		const Entity& curEntity = curSceneNode.entity;
		
		if (!curEntity.isEnabled()) continue;

		//TODO check if all these explicity copy constructors + 1 one std::move saves time than just all copy constructors
		if (curSceneNode.hasSibling()) {
			drawStack.push({ glm::mat4(curTransform), curSceneNode.sibling });
		}

		curTransform = curTransform * curSceneNode.transform.localToParent();
		//since we insert child before sibling in EntityComponents, more spatial localtiy if we add child to stack last
		if (curSceneNode.hasChild()) {
			drawStack.push({ glm::mat4(curTransform), curSceneNode.child});
		}

		if (!cameraSet && curEntity.hasCamera() && (sceneHasCamera() && curEntityID == renderCameraID)) {
			cameraSet = true;
			viewTransform = glm::inverse(glm::mat4(curTransform)); //since we're using reference, need to use co[y constructor ?
			const Camera& cam = cameras.get(renderCameraID);
			projTransform = glm::perspective(cam.vfov, cam.aspect, cam.nearPlane, cam.farPlane);
			projTransform[1][1] *= -1;
		}

		
		if (curEntity.hasMesh()) {
			auto meshIt = meshes.dataIterator(curEntityID);
			if (!parameters.FRUSTUM_CULLING || frustumCull(frustumPlanes, meshIt->bounds, curTransform)) {
				drawParams.emplace_back(DrawParameters(curTransform, meshIt));
			}
		}
	}

	if (!cameraSet) {
		viewTransform = glm::mat4(1.0f);
		Camera cam = Camera();
		projTransform = glm::perspective(cam.vfov, cam.aspect, cam.nearPlane, cam.farPlane);
		projTransform[1][1] *= -1;
	}

	return;
}

void Scene::printScene(const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	if (!sceneHasRoot()) return;

	std::stack<std::pair<entitySize_t, std::string>> printStack{};
	printStack.push(std::make_pair(rootID, ""));
	while (!printStack.empty()) {
		std::pair<entitySize_t, std::string> entry = printStack.top();
		printStack.pop();
		entitySize_t curID = entry.first;
		std::string prefixString = entry.second;

		const SceneNode& curNode = graph.get(curID);
		//This might not actually be true because entities can aliased together?
		//if (CHECK_VALIDITY) assert(curID == curNode.entity.getID());
		const Transform& t = curNode.transform;
		std::cout << prefixString << ">NODE with ID " << curID << "| pos(" << t.translation.x << "," << t.translation.y << "," << t.translation.z << ") ";
		std::cout << "rot(" << t.rotation.x << "," << t.rotation.y << "," << t.rotation.z << "," << t.rotation.w << ") ";
		std::cout << "scale(" << t.scale.x << "," << t.scale.y << "," << t.scale.z << ")";
		std::cout << "| " << std::endl << "HAS ";
		if (meshes.contains(curID)) {
			const Mesh& curMesh = meshes.get(curID);
			std::cout << " Mesh(numIndices:" << curMesh.numIndices << ", indexOffset:" << curMesh.indexOffset;
			////TODO insert dummy material for mesh if it does not exist for mesh ?
			////if (CHECK_VALIDITY) assert(materials.contains(curID));
			std::cout << ", material:0), ";
		}
		if (cameras.contains(curID)) {
			Camera curCamera = cameras.get(curID);
			std::cout << "Camera(type:";
			if(curCamera.type == Camera::ORTHOGRAPHIC) std::cout << "orthographic), ";
			else {
				if (CHECK_VALIDITY) assert(curCamera.type == Camera::PERSPECTIVE);
				std::cout << "perspective" << curCamera.aspect << ",vfov:" << curCamera.vfov;
				std::cout << ",near:" << curCamera.nearPlane << ",far:" <<curCamera.farPlane << "), ";
			}
		}
		//TODO print out light info
		if (lights.contains(curID)) {
			[[maybe_unused]]
			Light curLight = lights.get(curID);
			std::cout << "Camera(info not yet filled), ";
		}
		if (environments.contains(curID)) {
			[[maybe_unused]]
			Environment environment = environments.get(curID);
			std::cout << "Environment(info not yet filled), ";
		}
		std::cout << std::endl;

		if (curNode.hasSibling()) {
			printStack.push(std::make_pair(curNode.sibling, prefixString));
		}
		//want to print children first
		if (curNode.hasChild()) {
			printStack.push(std::make_pair(curNode.child, prefixString + "--"));
		}

	}
} 