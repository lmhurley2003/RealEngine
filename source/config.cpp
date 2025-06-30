#include "config.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//type definitions
struct Config::Vertex {
    glm::vec2 pos;
    glm::vec3 color;
};

struct Config::Index {
    uint16_t idx;
};

struct Config::UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};

//functions likely constant between program instantiations
void Config::vertexBufferSize(uint32_t* numElements, uint32_t* elementSize) {
    *numElements = vertices.size();
    *elementSize = sizeof(Vertex);
}

void Config::indexBufferSize(uint32_t* numElements, uint32_t* elementSize) {
    *numElements = indices.size();
    *elementSize = sizeof(Index);
}

//functions likely to differ between programs
VkVertexInputBindingDescription Config::getVertexBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}
std::vector<VkVertexInputAttributeDescription> Config::getVertexAttributeDescriptions() {
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


std::vector<Config::Vertex> Config::vertices = {
    { {-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f} },
    { {0.5f, -0.5f}, {0.0f, 1.0f, 0.0f} },
    { {0.5f, 0.5f}, {0.0f, 0.0f, 1.0f} },
    { {-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f} }
};

std::vector<Config::Index> Config::indices = {
    {0}, {1}, {2}, {0}, {2}, {3}
};

VkShaderStageFlags Config::uniformBufferObjectStages() {
    return VK_SHADER_STAGE_VERTEX_BIT;
}

uint32_t Config::uniformBufferObjectSize() {
    return sizeof(UniformBufferObject);
}

void Config::updateUniformBuffer(std::vector<void*>& uniformBuffersMapped, uint32_t image, float time, uint32_t width, uint32_t height) {
    glm::mat4 modelMatrix = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 viewMatrix = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 projMatrix = glm::perspective(glm::radians(45.0f), width / static_cast<float>(height), 0.01f, 10.0f);
    projMatrix[1][1] *= -1;

    UniformBufferObject ubo = { modelMatrix, viewMatrix, projMatrix };
    memcpy(uniformBuffersMapped[image], &ubo, sizeof(ubo));
}

#include "triBufferMatrixVert.cpp"
#include "triBufferSimpleFrag.cpp"
const std::vector<const unsigned char*> Config::shaders = {triBufferMatrixVert, triBufferSimpleFrag };
const std::vector<uint32_t> Config::shaderSizes = {triBufferMatrixVertSize, triBufferSimpleFragSize};

std::vector<PipelineStage> Config::shaderStages = { {MAIN_RENDER, {{VERTEX, 0}, {FRAGMENT, 1}}} };


