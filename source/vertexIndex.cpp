#include <glm/glm.hpp>
#include "vertexIndex.hpp"

#ifdef SIMPLE_VERTEX
//type definitions
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
};

std::vector<Vertex> vertices = {
    { {-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f} },
    { {0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} },
    { {0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} },
    { {-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f} }
};

std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    return attributeDescriptions;
}
#else
//type definitions
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
    {{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
};

std::vector<VkVertexInputAttributeDescription> getVertexAttributeDescriptions() {
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);

    attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].location = 2;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

    return attributeDescriptions;
}

#endif 

//functions likely to differ between programs
VkVertexInputBindingDescription getVertexBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};

    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

//functions likely constant between program instantiations
void vertexBufferSize(uint32_t* numElements, uint32_t* elementSize) {
    *numElements = static_cast<uint32_t>(vertices.size());
    *elementSize = sizeof(Vertex);
}

struct Index {
#ifdef INDEX_32BIT
    uint32_t idx;
#else
    uint16_t idx;
#endif
};

std::vector<Index> indices = {
    {0}, {1}, {2}, {0}, {2}, {3},
    {4}, {5}, {6}, {6}, {7}, {4}
};

void indexBufferSize(uint32_t* numElements, uint32_t* elementSize) {
    *numElements = indices.size();
    *elementSize = sizeof(Index);
}

#ifdef COMBINED_VERTEX_INDEX_BUFFER
void App::createVertexIndexBuffer() {
    uint32_t numVertices, vertexSizeSize = 0;
    vertexBufferSize(&numVertices, &vertexSizeSize);
    uint32_t numIndices, indexSize = 0;
    indexBufferSize(&numIndices, &indexSize);
    VkDeviceSize vertexBufferSize = static_cast<VkDeviceSize>(numVertices * vertexSizeSize);
    VkDeviceSize indexBufferSize = static_cast<VkDeviceSize>(numIndices * indexSize);
    VkDeviceSize bufferSize = vertexBufferSize + indexBufferSize;
    
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    mapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(vertexBufferSize));
    memcpy(reinterpret_cast<char*>(data) + vertexBufferSize, indices.data(), static_cast<size_t>(indexBufferSize));
    unmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexIndexBuffer, vertexIndexBufferMemory);

    copyVertexIndexBuffer(stagingBuffer, vertexIndexBuffer, vertexBufferSize, indexBufferSize);
    //copyBuffer(stagingBuffer, vertexIndexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

 }

void App::bindVertexIndexBuffer(VkCommandBuffer commandBuffer) {
    VkBuffer vertexBuffers[] = { vertexIndexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets); //TODO bind both vertex+index in one call

    static uint32_t numIndices, indexSize = 0;
    indexBufferSize(&numIndices, &indexSize);
    if (numIndices > 0) {
        static uint32_t numVertices, vertexSize = 0;
        vertexBufferSize(&numVertices, &vertexSize);
        static uint32_t vertexBufferSize = static_cast<uint32_t>(numVertices * vertexSize);
        //ony two sizes of indices allowed
        static VkIndexType indexType = indexSize <= 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(commandBuffer, vertexIndexBuffer, vertexBufferSize, indexType);
    }
}
#else
void App::createVertexBuffer() {
    uint32_t numElements, elementSize = 0;
    vertexBufferSize(&numElements, &elementSize);
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(numElements * elementSize);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    //TODO compare to explicitly flushing memory (instead of using HOST_COHERENT bit?)
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void App::createIndexBuffer() {
    uint32_t numElements, elementSize = 0;
    indexBufferSize(&numElements, &elementSize);
    VkDeviceSize bufferSize = numElements * elementSize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void App::bindVertexBuffer(VkCommandBuffer commandBuffer) {
    VkBuffer vertexBuffers[] = { vertexBuffer };
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets); //TODO bind both vertex+index in one call
}

void App::bindIndexBuffer(VkCommandBuffer commandBuffer) {
    //TODO remember this is static
    static uint32_t numIndices, indexSize = 0;
    indexBufferSize(&numIndices, &indexSize);
    if (numIndices > 0) {
        //ony two sizes of indices allowed
        static VkIndexType indexType = indexSize <= 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, indexType);
    }
}
#endif


