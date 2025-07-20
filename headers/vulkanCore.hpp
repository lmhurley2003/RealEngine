#pragma once
//#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
//#define GLFW_EXPOSE_NATIVE_WIN32
//#include <GLFW/glfw3native.h>
#include <vulkan/vk_enum_string_helper.h>

#include <vector>
#include <unordered_map>
#include <set>
#include <iostream>
#include <optional>
#include <array>

#include "commandArgs.hpp" //TODO make this a macro to include or not
#include "vertexIndex.hpp"
#include "mode.hpp"

class App {
public:
    App() = default;
    App(ParamMap commandlineParameters);
#ifdef NDEBUG
    const bool DEBUG = false;
#else
    const bool DEBUG = true;
#endif  
    int WIDTH = 1080;
    int HEIGHT = 1920;
    int BASE_WIDTH = 1920;
    int BASE_HEIGHT = 1080;
    uint32_t FRAME = 0;
    // DEBUG_LEVEL
    enum : uint32_t {
        NONE = 0,
        SEVERE = 1, //critical issues
        MODERATE = 2, //critical issues + validation
        ALL = 3 //critical issues + validation + sanity checks
    };
    GlobalConstantParameters globalParameters{};
    

private:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
//can't be const because device may not support it
#if defined(DYNAMIC_RENDERING) && DYNAMIC_RENDERING
    bool useDynamicRendering = true;
#else
    bool useDynamicRendering = false;
#endif
    const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_LUNARG_monitor"
    };
    std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    std::vector<const char*> optionalDeviceExtensions = {
#if defined(DYNAMIC_RENDERING) && DYNAMIC_RENDERING
       VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME,
#endif
    };
public:
    GLFWwindow* window = nullptr;
private:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures deviceFeatures;
    VkPhysicalDeviceProperties deviceProperties;

    VkDevice device;

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> transferFamily;
        std::optional<uint32_t> presentFamily;
        std::optional<uint32_t> computeFamily;

        uint32_t graphics() { return graphicsFamily.value(); };
        uint32_t transfer() { return transferFamily.value(); };
        uint32_t present() { return presentFamily.value(); };
        uint32_t compute() { return computeFamily.value(); };

        std::vector<uint32_t> indices() {
            std::set<uint32_t> set = {graphics(), transfer(), present(), compute()};
            return std::vector<uint32_t>(set.begin(), set.end());
        }

        const bool isComplete() {
            return graphicsFamily.has_value() && transferFamily.has_value() && presentFamily.has_value() && computeFamily.has_value();
        };
    };
    QueueFamilyIndices queueFamilyIndices;
    std::unordered_map<uint32_t, VkQueue> queues;

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    std::vector<VkImageView> swapChainImageViews;

    VkFormat depthFormat;
    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    VkPipelineLayout pipelineLayout; //push shader uniform definitions
    VkRenderPass renderPass;
public:
    std::unordered_map<PipelineStageT, VkPipeline> pipelines;
private:
    std::vector<VkFramebuffer> swapChainFramebuffers;

    enum QueueType : uint32_t {
        GRAPHICS_QUEUE,
        PRESENT_QUEUE,
        TRANSFER_QUEUE,
        COMPUTE_QUEUE
    };
    std::unordered_map<QueueType, VkCommandPool> commandPools;

    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    std::unordered_map<QueueType, std::vector<VkCommandBuffer>> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores; //size equal to images in swapchain
    std::vector<VkFence> inFlightFences;
    uint32_t flightFrame = 0;
    bool framebufferResized = false;

#if defined(COMBINED_VERTEX_INDEX_BUFFER) && COMBINED_VERTEX_INDEX_BUFFER
    VkBuffer vertexIndexBuffer;
    VkDeviceMemory vertexIndexBufferMemory;
#else
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
#endif 

    std::array<std::vector<VkBuffer>, MAX_FRAMES_IN_FLIGHT>  uniformBuffers{};
    std::array<std::vector<VkDeviceMemory>, MAX_FRAMES_IN_FLIGHT> uniformBuffersMemory{};
    std::array<std::vector<void*>, MAX_FRAMES_IN_FLIGHT> uniformBuffersMapped{};

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;

    std::vector<VkSampler> textureImageSamplers;

    VkDescriptorPool descriptorPool;
    std::array<std::vector<VkDescriptorSet>, MAX_FRAMES_IN_FLIGHT> descriptorSets{}; //idx with [frame][set#]

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        
        App* app = static_cast<App*>(pUserData);

        if (app->globalParameters.DEBUG_LEVEL == 0) return VK_FALSE;
        if (app->globalParameters.DEBUG_LEVEL <= 1 && messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
        if (app->globalParameters.DEBUG_LEVEL == 2 && messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) return VK_FALSE;
        std::cerr << "\n(-) Validation layer error of severity--" << string_VkDebugUtilsMessageSeverityFlagBitsEXT(messageSeverity) <<
            "--and type--" << string_VkDebugUtilsMessageTypeFlagsEXT(messageType) << "-- : \n" << pCallbackData->pMessage << "\n";

        return VK_FALSE;
    }
    static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
            func(instance, debugMessenger, pAllocator);
        }
    }

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<App*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
        //As of now this is only necessary so that handle_events has access to correct window size
        app->WIDTH = width;
        app->HEIGHT = height;
        // app->drawFrame(); TODO reconfigure to still update frame while window is being resize, as of now program stalls.
        //previous attempt resulted in vector subscript out of range error
    }


//FUNCTIONS USED TO INIT CORE RESOURCES (AND NOT MODE-DEPENDENT RESOURCES)
public:
    void initWindow();
private:
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    void createInstance();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void setupDebugMessenger();
    void createSurface();

    void printQueueFamilies(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    uint32_t rateDeviceSuitability(const VkPhysicalDevice& device);
    void determineOptionalExtensions(const VkPhysicalDevice& device);
    void pickPhysicalDevice();
    void createLogicalDevice();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    void createSwapChainImageViews();
public:
    void initVulkan();

private:
//MEMORY FUNCTIONS DEFINED IN VULKANMEMORY.CPP
    void initializeMemorySystem();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void copyVertexIndexBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t vertexBufferSize, uint32_t indexBufferSize);
    void mapMemory(VkDevice device, VkDeviceMemory deviceMemory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
    void unmapMemory(VkDevice device, VkDeviceMemory deviceMemory);
    void freeBuffer(VkBuffer buffer, VkDeviceMemory);
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, VkCommandBuffer commandBuffer = nullptr);
    void createTextureImage(std::string filename); //no need to prefix with texture directory
    void freeMemorySystem();


//FUNCTIONS DEPENDENT ON MODE OF PROGRAM DECLARED IN MODE.HPP
    void recreateSwapChain();
    void createRenderPass(const Mode& mode);
    VkShaderModule createShaderModule(const Mode& mode, uint32_t i);
    void createDescriptorSetLayouts(const Mode& mode);
    void createGraphicsPipeline(const Mode& mode);
    void createFramebuffers();
    void createCommandPools();
    //defined in vertexIndex.cpp
#ifdef COMBINED_VERTEX_INDEX_BUFFER
    void createVertexIndexBuffer();
    void bindVertexIndexBuffer(VkCommandBuffer commandBuffer);
#else
    void createVertexBuffer();
    void createIndexBuffer();
    void bindVertexBuffer(VkCommandBuffer commandBuffer);
    void bindIndexBuffer(VkCommandBuffer commandBuffer);
#endif
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>* fallbackCandidates);
    VkFormat findDepthFormat();
    void createDepthResources();
    void createUniformBuffers(Mode& mode);
    //TODO absract out to create more texture / attachments
    void createTextureImageView();
    void createTextureSamplers(Mode& mode);
    void createDescriptorPool(const Mode& mode);
    void createDescriptorSets(const Mode& mode);
    void createCommandBuffers();
    void createSynchObjects();
public:
    void initProgram(Mode& mode);

// FUNCTIONS USED FOR DRAWING, SOME EXPOSED TO Mode.hpp
    void updateUniformBuffer(uint32_t uniformIndex, const void* uniformData, uint32_t uniformSize) const;
    void updatePushConstants(VkCommandBuffer commandBuffer, ShaderStageT shaderStages, uint32_t size, uint32_t offset, const void* pushConstant) const;
    //current frame of flight command buffer, curren swapchain image index
    std::pair<VkCommandBuffer, uint32_t> beginFrame();
    void endFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex);

private:
    void cleanupSwapChain();
public:
    void cleanupProgram();
    void cleanupVulkan();
};
