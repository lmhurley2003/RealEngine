#include "scene.hpp"
#include <algorithm>
#include <stack>
#include "utils.cpp"
#include "jsonParsing.cpp" //TODO why does linking fail if I don't include this

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

bool isSimpleMaterial(Object obj) {
	return !(obj.count("pbr") || obj.count("lambertian") || obj.count("environment") || obj.count("mirror"));
}


Mesh Scene::initMesh(const Object& JSONObj, const ModeConstantParameters& parameters) {
	Mesh retMesh = Mesh();
	//if (JSONObj.count("name")) {
	//	retMesh.name = JSONUtils::getVal(JSONObj, "name", STRING).toString();
	//}

	Object attributes = JSONUtils::getVal(JSONObj, "attributes", OBJECT).toObject();
	Object position = JSONUtils::getVal(attributes, "POSITION", OBJECT).toObject();
	std::string sourceFile = JSONUtils::getVal(position, "src", STRING).toString();
	
	retMesh.loadMeshData(sourceFile, JSONObj, parameters);
	return retMesh;
};

Material Scene::initMaterial(const Object& JSONobj, const ModeConstantParameters& parameters) {
	Material retMaterial = Material();

	return retMaterial;
}

Camera Scene::initCamera(const Object& JSONObj, const ModeConstantParameters& parameters) {
	Camera retCamera = Camera();

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

Scene::SceneNode Scene::initNode(const Object& JSONObj, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	if (CHECK_VALIDITY) assert(JSONObj.count("type") && JSONUtils::getVal(JSONObj, "type", STRING).toString() == "NODE");

	SceneNode retNode = SceneNode();
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
			if (cameraName == parameters.START_CAMERA_NAME) cameraID = retNode.entity.getID();
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
	if (JSONObj.count("children")) {
		std::vector<std::string> childNames = JSONUtils::getIndicesNames(JSONObj, "children", CHECK_VALIDITY);
		
		if (childNames.size() > 0 && tempComponents.count({ NODE, childNames[0] })) {
			entitySize_t id = static_cast<entitySize_t>(tempComponents[{NODE, childNames[0]}]);
			graph.get(retNode.entity).child = id;
			if (CHECK_VALIDITY) assert(graph.contains(id));
		}
		else if (childNames.size() > 0) {
			assert(tempGraph.count({NODE, childNames[0]}));
			graph.get(retNode.entity).child = initNode(tempGraph[{NODE, childNames[0]}].object, parameters).entity.getID();
		}
		else {
			return graph.get(retNode.entity);
		}

		if(CHECK_VALIDITY) assert(graph.contains(retNode.child));
		SceneNode& curSceneNode = graph.get(retNode.child);
		for (uint32_t i = 1; i < childNames.size(); i++) {
			const std::string& childName = childNames[i];
			if (tempComponents.count({ NODE, childName })) {
				entitySize_t id = static_cast<entitySize_t>(tempComponents[{NODE, childName}]);
				curSceneNode.sibling = id;
				if (CHECK_VALIDITY) assert(graph.contains(id));
			}
			else {
				if(CHECK_VALIDITY) assert(tempGraph.count({NODE, childName}));
				curSceneNode.sibling = initNode(tempGraph[{NODE, childName}].object, parameters).entity.getID();
			}
			curSceneNode = graph.get(curSceneNode.sibling);
		}
	}

	return graph.get(retNode.entity);
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
			tempGraph.insert({ {DRIVER, name}, {curObj, 0} });
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
		rootID = initNode(tempGraph[{NODE, rootNames[0]}].object, parameters).entity.getID();
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
				siblingID = initNode(tempGraph[{NODE, rootName}].object, parameters).entity.getID();
				if (CHECK_VALIDITY) assert(graph.contains(siblingID));
				graph.get(curSceneNodeID).sibling = siblingID;
				//check that assignment actually appears in the graph
				if (CHECK_VALIDITY) assert(graph.get(curSceneNodeID).sibling == siblingID);
			}
			curSceneNodeID = siblingID;
		}
	}

	tempGraph.clear();
	tempComponents.clear();
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
		if (CHECK_VALIDITY) assert(curID == curNode.entity.getID());
		//std::cout << prefixString << ">NODE with ID " << curID << ", HAS ";
		if (meshes.contains(curID)) {
			const Mesh& curMesh = meshes.get(curID);
			std::cout << "size of Mesh : " << sizeof(Mesh) << std::endl;
			std::cout << " Mesh(numIndices:" << curMesh.numIndices << ", indexOffset:" << curMesh.indexOffset;
			////TODO insert dummy material for mesh if it does not exist for mesh ?
			////if (CHECK_VALIDITY) assert(materials.contains(curID));
			std::cout << ", material:0), " << std::endl;
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

		if (curNode.hasChild()) {
			printStack.push(std::make_pair(curNode.child, prefixString + "--"));
		}
		if (curNode.hasSibling()) {
			printStack.push(std::make_pair(curNode.sibling, prefixString));
		}

	}
} 