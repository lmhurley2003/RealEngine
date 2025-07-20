#include "mesh.hpp"
#include <fstream>
#include <algorithm>


void Bounds::enclose(float x, float y, float z) {
	minX = std::min(x, minX); minY = std::min(y, minY); minZ = std::min(z, minZ);
	maxX = std::max(x, maxX); maxY = std::max(y, maxY); maxZ = std::max(z, maxZ);
}

void Bounds::enclose(glm::vec3 pt) {
	minX = std::min(pt.x, minX); minY = std::min(pt.y, minY); minZ = std::min(pt.z, minZ);
	maxX = std::max(pt.x, maxX); maxY = std::max(pt.y, maxY); maxZ = std::max(pt.z, maxZ);
}
void Bounds::enclose(Bounds bounds) {
	enclose(bounds.minX, bounds.minY, bounds.minZ);
	enclose(bounds.maxX, bounds.maxY, bounds.maxZ);
}
void Bounds::fixZeroVolume(){
	if (maxX - minX < MIN_AXIS_SIZE) {
		float centerX = (maxX + minX) / 2.0f;
		minX = centerX - (MIN_AXIS_SIZE / 2.0f);
		maxX = centerX + (MIN_AXIS_SIZE / 2.0f);
	}
	if (maxY - minY < MIN_AXIS_SIZE) {
		float centerY = (maxY + minY) / 2.0f;
		minY = centerY - (MIN_AXIS_SIZE / 2.0f);
		maxY = centerY + (MIN_AXIS_SIZE / 2.0f);
	}
	if (maxZ - minZ < MIN_AXIS_SIZE) {
		float centerZ = (maxZ + minZ) / 2.0f;
		minZ = centerZ - (MIN_AXIS_SIZE / 2.0f);
		maxZ = centerZ + (MIN_AXIS_SIZE / 2.0f);
	}
}


//will populate vertex and index buffers manually
void Mesh::toIndexed(const std::vector<Vertex>& srcBuffer) {
	indices.reserve(indices.size() + srcBuffer.size());
	std::unordered_map<Vertex, Index> duplicateCheck{};
	for (const Vertex& vertex : srcBuffer) {
		if (!duplicateCheck.count(vertex)) {
			duplicateCheck.insert({ vertex, vertices.size() });
			indices.emplace_back(vertices.size());
			vertices.emplace_back(vertex);
		}
		else {
			indices.emplace_back(duplicateCheck[vertex]);
		}
	}
}

void Mesh::loadMeshData(const std::string filename, const Object& JSONObj, const ModeConstantParameters& parameters) {
	const bool CHECK_VALIDITY = parameters.DEBUG && parameters.DEBUG_LEVEL >= 3;
	//TODO store meshes in folder other than scenes ?
	std::ifstream file;
	for (std::string tryPath : std::vector<std::string>{ "scenes\\" + filename, filename, "meshes\\" + filename, "scenes\\meshes\\" + filename }) {
		file = std::ifstream(tryPath, std::ios::ate | std::ios::binary);
		if (file.good()) break;
	}
	if (!file.good() || !file.is_open())  throw std::runtime_error("Failed to find or open file!");
	file.seekg(0);


	struct VertexAttribute {
		std::string name;
		std::string format;
		uint32_t formatSize;
		uint32_t stride;
		uint32_t offset;
	};
	std::vector<VertexAttribute> vertexAttributes;
	Object attributesObj = JSONUtils::getVal(JSONObj, "attributes", OBJECT).toObject();
	vertexAttributes.reserve(attributesObj.size());
	//TODO actually deal with different format types, mayeb by outputing a VertexBingindAttributes array ?
	for (const auto& pair : attributesObj) {
		VertexAttribute curAttrib{};
		curAttrib.name = pair.first;
		Object attribInfo = JSONUtils::getVal(attributesObj, curAttrib.name, OBJECT).toObject();
		if (CHECK_VALIDITY) {
			curAttrib.format = JSONUtils::getVal(attribInfo, "format", STRING).toString();
			if (curAttrib.format == "R32G32B32_SFLOAT") curAttrib.formatSize = 12;
			else if (curAttrib.format == "R32G32B32A32_SFLOAT") curAttrib.formatSize = 16;
			else if (curAttrib.format == "R32G32_SFLOAT") curAttrib.formatSize = 8;
			else if (curAttrib.format == "R32_SFLOAT") curAttrib.formatSize = 4;
			else if (curAttrib.format == "R8G8B8A8_UNORM") curAttrib.formatSize = 4;
			else {
				throw std::runtime_error("\n\nUnseen attribute format : " + curAttrib.format + "!");
			}
		}
		curAttrib.stride = JSONUtils::getVal(attribInfo, "stride", NUMBER).toNumber().toSizeT();
		curAttrib.offset = JSONUtils::getVal(attribInfo, "offset", NUMBER).toNumber().toSizeT();

		vertexAttributes.emplace_back(curAttrib);
	}

	bool dataPacked = true;
	if (CHECK_VALIDITY) {
		//sort by stride
		std::sort(vertexAttributes.begin(), vertexAttributes.end(), [](VertexAttribute a, VertexAttribute b)
			{
				return a.offset < b.offset;
			});
		uint32_t vertexSize = vertexAttributes.size() > 0 ? vertexAttributes[0].offset : 0;
		for (int i = 1; i < vertexAttributes.size(); i++) {
			if (vertexAttributes[i].stride != vertexAttributes[i - 1].stride) dataPacked = false;
			vertexSize += vertexAttributes[i].offset;
		}
		if ((vertexAttributes.size() == 0) || (vertexAttributes[0].stride != vertexSize)) dataPacked = false;
		//TODO what to do if data is not packed ?
	}
	uint32_t count = JSONUtils::getVal(JSONObj, "count", NUMBER).toNumber().toSizeT();
	uint32_t stride = vertexAttributes[0].stride;

	size_t fileSize = count * stride;
	std::vector<Vertex> buffer(count);

	if (dataPacked && vertexAttributes.size() == 5 && vertexAttributes[0].name == "POSITION" && vertexAttributes[1].name == "NORMAL" &&
		vertexAttributes[2].name == "TANGENT" && vertexAttributes[3].name == "TEXCOORD" && vertexAttributes[4].name == "COLOR") {
#if defined(SIMPLE_VERTEX) && SIMPLE_VERTEX
		int positionOffset = -1, colorOffset = -1;
		for (VertexAttribute attr : vertexAttributes) {
			if (attr.name == "POSITION") positionOffset = attr.offset;
			else if (attr.name == "COLOR") colorOffset = attr.offset;
		}
		std::vector<char>charBuffer(count * stride);
		file.read(charBuffer.data(), fileSize);
		for (uint32_t i = 0; i < count; i++) {
			//will just trust that alignment issues won't mess things up...
			//if it is, will need to use .seekg manually and fill single position/color buffers
			if (positionOffset != -1) buffer[i].position = *(reinterpret_cast<glm::vec3*>(charBuffer.data() + i * stride + positionOffset));

			if (colorOffset != -1) buffer[i].color = *(reinterpret_cast<uint32_t*>(charBuffer.data() + i * stride + colorOffset));

		}
		charBuffer.clear();
#else
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
#endif
	}
#if defined(SIMPLE_VERTEX) && SIMPLE_VERTEX
	else if (dataPacked && vertexAttributes.size() == 2 && vertexAttributes[0].name == "POSITION" && vertexAttributes[1].name == COLOR) {
		file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
	}
#endif
	else {
		int positionOffset = -1, normalOffset = -1, tangentOffset = -1, texCoordOffset = -1, colorOffset = -1;
		for (VertexAttribute attr : vertexAttributes) {
			if (attr.name == "POSITION") positionOffset = attr.offset;
			else if (attr.name == "COLOR") colorOffset = attr.offset;
#if defined(SIMPLE_VERTEX) && SIMPLE_VERTEX
#else
			else if (attr.name == "NORMAL") normalOffset = attr.offset;
			else if (attr.name == "TANGENT") tangentOffset = attr.offset;
			else if (attr.name == "TEXCOORD") texCoordOffset = attr.offset;
#endif
		}
		uint32_t stride = vertexAttributes[0].stride;
		std::vector<char>charBuffer(count * stride);
		file.read(charBuffer.data(), fileSize);
		for (uint32_t i = 0; i < count; i++) {
			//will just trust that alignment issues won't mess things up...
			//if it is, will need to use .seekg manually and fill single position/color buffers
			if (positionOffset != -1) buffer[i].position = *(reinterpret_cast<glm::vec3*>(charBuffer.data() + i * stride + positionOffset));
			if (colorOffset != -1) buffer[i].color = *(reinterpret_cast<uint32_t*>(charBuffer.data() + i * stride + colorOffset));
			
#if defined(SIMPLE_VERTEX) && SIMPLE_VERTEX
#else
			if (normalOffset != -1) buffer[i].normal = *(reinterpret_cast<glm::vec3*>(charBuffer.data() + i * stride + normalOffset));
			if (tangentOffset != -1) buffer[i].tangent = *(reinterpret_cast<glm::vec4*>(charBuffer.data() + i * stride + tangentOffset));
			if (texCoordOffset != -1) buffer[i].texCoord = *(reinterpret_cast<glm::vec4*>(charBuffer.data() + i * stride + texCoordOffset));

#endif
		}
	}

	indexOffset = indices.size();
	uint32_t uniqueVerticesStart = vertices.size();
	//now stream indices if available
	if (JSONObj.count("indicies")) {
		Object indicesAttr = JSONUtils::getVal(JSONObj, "indices", OBJECT).toObject();

		std::string indicesFilename = JSONUtils::getVal(indicesAttr, "src", STRING).toString();
		uint32_t offset = JSONUtils::getVal(indicesAttr, "offset", NUMBER).toNumber().toSizeT();

		if (indicesFilename != filename) {
			file.close();
			for (std::string tryPath : std::vector<std::string>{ "scenes\\" + indicesFilename, indicesFilename, "meshes\\" + indicesFilename, "scenes\\meshes\\" + indicesFilename }) {
				file = std::ifstream(tryPath, std::ios::ate | std::ios::binary);
				if (file.good()) break;
			}
			if (!file.good() || !file.is_open())  throw std::runtime_error("Failed to find or open file!");
		}

		size_t indexCharBufferSize = static_cast<size_t>(file.tellg()) - offset;
		file.seekg(offset); 
		std::string indexFormat = JSONUtils::getVal(JSONObj, "format", STRING).toString();
		uint32_t indexSize;
		if (indexFormat == "UINT16") indexSize = 2;
		else if (indexFormat == "UINT32") indexSize = 4;
		else if (indexFormat == "UINT8") indexSize = 1;
		else throw std::runtime_error("Index format unrecognized!");
		numIndices = indexCharBufferSize / indexSize;

		//TODO allow for specificiation of which index buffer we're reading into?
		uint32_t localIndexBufferOffset = indices.size();
		uint32_t numPrevVertices = vertices.size();
		if (indexSize == sizeof(Index)) {
			indices.resize(localIndexBufferOffset + numIndices);
			file.read(reinterpret_cast<char*>(indices.data() + localIndexBufferOffset), indexCharBufferSize);
			//need to account for vertex buffer might already have prev data so indices will point to wrong vertices
			for (uint32_t i = localIndexBufferOffset; i < indices.size(); i++) indices[i] += numPrevVertices;
		}
		else {
			indices.reserve(localIndexBufferOffset + numIndices);
			if (indexSize == 1) {
				std::vector<uint8_t> cachedIndices(numIndices);
				file.read(reinterpret_cast<char*>(cachedIndices.data()), indexCharBufferSize);
				for (uint8_t newIdx : cachedIndices) { indices.emplace_back(static_cast<Index>(newIdx) + static_cast<Index>(numPrevVertices)); }
			}
			else if (indexSize == 2) {
				std::vector<uint16_t> cachedIndices(numIndices);
				file.read(reinterpret_cast<char*>(cachedIndices.data()), indexCharBufferSize);
				for (uint8_t newIdx : cachedIndices) { indices.emplace_back(static_cast<Index>(newIdx) + static_cast<Index>(numPrevVertices)); }
			}
			else if (indexSize == 4) {
				std::vector<uint32_t> cachedIndices(numIndices);
				file.read(reinterpret_cast<char*>(cachedIndices.data()), indexCharBufferSize);
				for (uint8_t newIdx : cachedIndices) { indices.emplace_back(static_cast<Index>(newIdx) + static_cast<Index>(numPrevVertices)); }
			}
		}
		file.close();
		vertices.insert(vertices.end(), buffer.begin(), buffer.end());
	}
	else {
		file.close();
		numIndices = buffer.size();
		toIndexed(buffer);
	}
	uint32_t uniqueVerticesEnd = vertices.size();
	//fill in bounds structure
	for (uint32_t i = uniqueVerticesStart; i < uniqueVerticesEnd; i++) {
		bounds.enclose(vertices[i].position);
	}
	bounds.fixZeroVolume();

}