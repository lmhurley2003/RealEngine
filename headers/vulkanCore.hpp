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

#include "config.hpp"

class App {
public:
    App();
#ifdef NDEBUG
    const bool DEBUG = false;
#else
    const bool DEBUG = true;
#endif  
    int DEBUG_LEVEL;
    bool PRINT_DEBUG;
    int BASE_WIDTH;
    int BASE_HEIGHT;
    int WIDTH;
    int HEIGHT;

    void run();

private:
#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif
    const std::vector<const char*> validationLayers = {
      "VK_LAYER_KHRONOS_validation",
      "VK_LAYER_LUNARG_monitor"
    };
    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };
    GLFWwindow* window = nullptr;
    uint32_t FRAME = 0;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkPhysicalDeviceFeatures deviceFeatures;
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

        bool isComplete() {
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

    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout; //push shader uniform definitions
    VkRenderPass renderPass;
    std::unordered_map<PipelineStageT, VkPipeline> pipelines;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    enum QueueType : uint32_t {
        GRAPHICS_QUEUE,
        PRESENT_QUEUE,
        TRANSFER_QUEUE,
        COMPUTE_QUEUE
    };
    std::unordered_map<QueueType, VkCommandPool> commandPools;

    static const int MAX_FRAMES_IN_FLIGHT = 2;
    std::unordered_map<QueueType, std::vector<VkCommandBuffer>> commandBuffers;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores; //size equal to images in swapchain
    std::vector<VkFence> inFlightFences;
    uint32_t flightFrame = 0;
    bool framebufferResized = false;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;
 
    void initWindow();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) {
        
        App* app = static_cast<App*>(pUserData);

        if (app->DEBUG_LEVEL == 0) return VK_FALSE;
        if (app->DEBUG_LEVEL <= 1 && messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) return VK_FALSE;
        if (app->DEBUG_LEVEL == 2 && messageSeverity <= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) return VK_FALSE;
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
    }

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
    void pickPhysicalDevice();
    void createLogicalDevice();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    void createSwapChain();
    void createSwapChainImageViews();
    void initVulkan();

    void createRenderPass();
    VkShaderModule createShaderModule(uint32_t i);
    void createDescriptorSetLayout();
    void createGraphicsPipeline();
    void createFramebuffers();
    void createCommandPools();
    
    //memory functions defined in vulkanMemory.cpp
    void initializeMemorySystem();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void mapMemory(VkDevice device, VkDeviceMemory deviceMemory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags, void** ppData);
    void freeBuffer(VkBuffer buffer, VkDeviceMemory);
    void freeMemorySystem();

    void createVertexBuffer();
    void createIndexBuffer();
    void createUniformBuffers();
    void createDescriptorPool();
    void createDescriptorSets();
    void createCommandBuffers();
    void createSynchObjects();
    void initProgram();

    void recreateSwapChain();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void drawFrame();
    void mainLoop();

    void cleanupSwapChain();
    void cleanup();
};
