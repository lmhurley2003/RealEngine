#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <vector>

#if defined(SIMPLE_VERTEX) && SIMPLE_VERTEX
//type definitions
struct Vertex {
    glm::vec3 position;
    uint32_t color;

    bool operator==(const Vertex& other) const {
        return position == other.position && color == other.color;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec4>()(glm::vec4(vertex.position, static_cast<float>(vertex.color)))));

        }
    };
}
#else
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec4 tangent;
    glm::vec2 texCoord;
    uint32_t color;

    bool operator==(const Vertex& other) const {
        return position == other.position && normal == other.normal && tangent == other.tangent &&
            texCoord == other.texCoord && color == other.color;
    }
};

//TODO include other elements into the hash ?
namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position) ^
                (hash<uint32_t>()(vertex.color) << 1)) >> 1) ^
                (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}
#endif

#if defined(INDEX_32BIT) && INDEX_32BIT
typedef uint32_t Index;
#else
typedef uint16_t Index;
#endif

extern std::vector<Vertex> vertices;
extern std::vector<Index> indices;

//functions likely to differ between programs
VkVertexInputBindingDescription getVertexBindingDescription();

std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

void vertexBufferSize(uint32_t* numElements, uint32_t* elementSize);

void indexBufferSize(uint32_t* numElements, uint32_t* elementSize);