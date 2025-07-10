#include "config.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Config::UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
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

#include "triBufferTexturedMatrixVert.cpp"
#include "triBufferTexturedFrag.cpp"
const std::vector<const unsigned char*> Config::shaders = {triBufferTexturedMatrixVert, triBufferTexturedFrag };
const std::vector<uint32_t> Config::shaderSizes = { triBufferTexturedMatrixVertSize, triBufferTexturedFragSize};

std::vector<PipelineStage> Config::shaderStages = { {MAIN_RENDER, {{VERTEX, 0}, {FRAGMENT, 1}}} };


