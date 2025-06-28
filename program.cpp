#include "program.hpp"
#include "commandArgs.hpp"
#include <glm/glm.hpp>

//type definitions
struct Program::Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct Program::Index {
    uint16_t idx;
};

//functions likely constant between program instantiations
void Program::vertexBufferSize(uint32_t* numElements, uint32_t* elementSize) {
    *numElements = vertices.size();
    *elementSize = sizeof(Vertex);
}

void Program::indexBufferSize(uint32_t* numElements, uint32_t* elementSize) {
    *numElements = indices.size();
    *elementSize = sizeof(Index);
}

//functions likely to differ between programs
VkVertexInputBindingDescription Program::getVertexBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Program::Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}
std::vector<VkVertexInputAttributeDescription> Program::getVertexAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}


std::vector<Program::Vertex> Program::vertices = {
    { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} },
    { {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} },
    { {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} },
    { {-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f} }
};

std::vector<Program::Index> Program::indices = {
    {0}, {2}, {1}, {2}, {0}, {3}
};

//Program curProgram() { return  Program(parameters, {}, {}, { {MAIN_RENDER, {{VERTEX, "triTestVert"}, {FRAGMENT, "triTestFrag"}}}}); };
Program curProgram() { return  Program(parameters, {}, {}, { {MAIN_RENDER, {{VERTEX, "triBufferSimpleVert"}, {FRAGMENT, "triBufferSimpleFrag"}}} }); };

