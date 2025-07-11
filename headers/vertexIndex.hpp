#pragma once
#include "vulkanCore.hpp"

//functions likely to differ between programs
VkVertexInputBindingDescription getVertexBindingDescription();

std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions();

void vertexBufferSize(uint32_t* numElements, uint32_t* elementSize);

void indexBufferSize(uint32_t* numElements, uint32_t* elementSize);