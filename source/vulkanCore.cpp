//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <cassert> //(void)0 if NDEBUG defined
#include <algorithm>
#include <chrono>
#include <map>

#include "vulkanCore.hpp"
#include "utils.hpp"

void App::run() {
    initWindow();
    initVulkan();
    initProgram();
    mainLoop();
    cleanup();
 }

App::App() {
    BASE_WIDTH = 1080;
    BASE_HEIGHT = 720;
    WIDTH = (Config::parameters.getVec("resolution")).x;
    HEIGHT = (Config::parameters.getVec("resolution")).y;

    DEBUG_LEVEL = Config::parameters.getInt("debug-level");
    PRINT_DEBUG = Config::parameters.getBool("print-debug-output");

    FRAME = 0;
}

void App::initWindow() {
    glfwInit();
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());

    BASE_WIDTH = mode->width;
    BASE_HEIGHT = mode->height;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

std::vector<const char*> App::getRequiredExtensions() {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (enableValidationLayers) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

bool App::checkValidationLayerSupport() {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::unordered_set<std::string> availableLayersSet{};// (availableLayers.begin(), availableLayers.end());
    for (auto& layer : availableLayers) {
        availableLayersSet.insert(layer.layerName);
    }
    for (const auto& layerName : validationLayers) {
        if (!availableLayersSet.count(layerName)) {
            std::cerr << "Necessary validation layer " + std::string(layerName) + " not found in available layers!" << std::endl;
            return false;
        }
    }
    return true;
}

void App::createInstance() {
    if (enableValidationLayers && !checkValidationLayerSupport()) {
        throw std::runtime_error("Validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Real Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;


    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    const char* fpsLayer = "VK_LAYER_LUNARG_monitor";
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();

        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
    } else if (Config::parameters.getBool("force-show-fps")) {
        createInfo.enabledLayerCount = 1;
        createInfo.ppEnabledLayerNames = &fpsLayer;
    }
    else {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    uint32_t allAvailableExtensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &allAvailableExtensionCount, nullptr);
    std::vector<VkExtensionProperties> allAvailableExtensions(allAvailableExtensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &allAvailableExtensionCount, allAvailableExtensions.data());

    if (DEBUG && DEBUG_LEVEL >= SEVERE) {
        std::unordered_set<std::string> allAvailableExtensionsSet;
        if (PRINT_DEBUG) std::cout << "\nAvailable extensions\n" << std::endl;
     

        for (const auto& extension : allAvailableExtensions) {
            if (PRINT_DEBUG) std::cout << extension.extensionName << std::endl;
            allAvailableExtensionsSet.insert(extension.extensionName);
        }
        
        if (PRINT_DEBUG) std::cout << "\nChecking GLFW extensions : \n" << std::endl;
        for (uint32_t i = 0; i < extensions.size(); i++) {
            std::string extension = std::string(extensions[i]);
            if (!allAvailableExtensionsSet.count(extension)) {
                throw std::runtime_error("Necessray glfw extension " + extension + "not in list of available Vulkan extensions!");
            }
            if (PRINT_DEBUG) std::cout << extension << std::endl;

        }
        if (PRINT_DEBUG) std::cout << "\n" << std::endl;
    }

    if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create instance!");
    }
}

void App::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = static_cast<void *>(this);

}

void App::setupDebugMessenger() {
    if (!enableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo);

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger!");
    }
}

void App::createSurface() {
    if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

void App::printQueueFamilies(VkPhysicalDevice device) {
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;
    for (const auto& queueFamily : queueFamilies) {
        VkBool32 presentSupport = VK_FALSE;
        if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) != VK_SUCCESS) {
            throw std::runtime_error("Unable to get physical device surface support properties!");
        };
        std::cout << "\n Queue Family " << i << ":\n";
        std::cout << "---Graphics: " << std::boolalpha << static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) << "\n";
        std::cout << "---Transfer: " << std::boolalpha << static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) << "\n";
        std::cout << "---Compute: " << std::boolalpha << static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) << "\n";
        std::cout << "---Present: " << std::boolalpha << static_cast<bool>(presentSupport) << "\n";
        

        i++;
    }
    std::cout << std::endl;
}

App::QueueFamilyIndices App::findQueueFamilies(VkPhysicalDevice device) {
    if (DEBUG && PRINT_DEBUG && DEBUG_LEVEL >= ALL) printQueueFamilies(device);
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    enum : uint32_t {
        GRAPHICS = 0,
        PRESENT = 1,
        TRANSFER = 2,
        COMPUTE = 3,
    };
    //.first : idx of min indexed by above enums, .second : min score of number of queueFamily access
    //std::vector<std::pair<uint32_t, uint32_t>> scores = {{0, std::numeric_limits<uint32>},  {0, std::numeric_limits<uint32>},  {0, std::numeric_limits<uint32>},  {0, std::numeric_limits<uint32>} };
    std::vector<uint32_t> scores(4, std::numeric_limits<uint32_t>::max());

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    static const bool seperateQueues = Config::parameters.getBool("separate-queue-families");
    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (!seperateQueues) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) indices.graphicsFamily = i;
            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) indices.transferFamily = i;
            if (queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) indices.computeFamily = i;
            VkBool32 presentSupport = VK_FALSE;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface support properties!");
            };
            if (presentSupport) indices.presentFamily = i;
            if (indices.isComplete()) break;
        }
        else {
            VkBool32 presentSupport = VK_FALSE;
            if (vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport) != VK_SUCCESS) {
                throw std::runtime_error("Unable to get physical device surface support properties!");
            };
            uint32_t curScore = 0;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)) curScore++;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)) curScore++;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT)) curScore++;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)) curScore++;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_VIDEO_DECODE_BIT_KHR)) curScore++;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_VIDEO_ENCODE_BIT_KHR)) curScore++;
            if (static_cast<bool>(queueFamily.queueFlags & VK_QUEUE_OPTICAL_FLOW_BIT_NV)) curScore++;
            if (static_cast<bool>(presentSupport)) curScore++;

            if ((queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) && (scores[GRAPHICS] > curScore)) {scores[GRAPHICS] = curScore; indices.graphicsFamily = i;}
            if ((queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT) && (scores[TRANSFER] > curScore)) {scores[TRANSFER] = curScore; indices.transferFamily = i;}
            if ((queueFamily.queueFlags & VK_QUEUE_COMPUTE_BIT) && (scores[COMPUTE] > curScore)) {scores[COMPUTE] = curScore; indices.computeFamily = i;}
            if (presentSupport && (scores[PRESENT] > curScore)) {scores[PRESENT] = curScore; indices.presentFamily = i;}
        }

        i++;
    }

    return indices;
};

bool App::checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::unordered_set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}

App::SwapChainSupportDetails App::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;

    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

//Higher the score the more suitable, 0 score means device is invalid
uint32_t App::rateDeviceSuitability(const VkPhysicalDevice& device) {
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    if (deviceProperties.deviceName == Config::parameters.getString("physical-device")) return std::numeric_limits<uint32_t>::max();

    uint32_t score = 0;
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    static const bool printDevice = Config::parameters.getBool("list-physical-devices");
    if (DEBUG && (printDevice || DEBUG_LEVEL >= SEVERE)) {
        std::cout << "Available device: " << deviceProperties.deviceName << std::endl;
    }

    QueueFamilyIndices indices = findQueueFamilies(device);
    if (!indices.isComplete()) return 0;
    if (!checkDeviceExtensionSupport(device)) return 0;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
    if (swapChainSupport.formats.empty()) return 0;
    if (swapChainSupport.presentModes.empty()) return 0;
    
    return score;
}

//TODO use integrated GPU for UI stuff ?
void App::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("Failed to find GPUs with Vulkan Support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    std::multimap<uint32_t, VkPhysicalDevice> candidates;

    for (const auto& device : devices) {
        candidates.insert(std::make_pair(rateDeviceSuitability(device), device));
    }

    if (candidates.size() <= 0 || candidates.rbegin()->first <= 0) {
        throw std::runtime_error("Failed to find suitable GPU");
    }
    else {
        physicalDevice = candidates.rbegin()->second;
    }

    if (DEBUG && PRINT_DEBUG && DEBUG_LEVEL >= ALL) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        std::cout << "--Chosen device : " << deviceProperties.deviceName << std::endl;
    }

    queueFamilyIndices = findQueueFamilies(physicalDevice);
    if (DEBUG && DEBUG_LEVEL >= 2) assert(queueFamilyIndices.isComplete());
    if (DEBUG && PRINT_DEBUG && DEBUG_LEVEL >= 3) {
        std::cout << "\n";
        std::cout << "Graphics queue family index: " << queueFamilyIndices.graphics() << "\n";
        std::cout << "Transfer queue family index: " << queueFamilyIndices.transfer() << "\n";
        std::cout << "Present queue family index: " << queueFamilyIndices.present() << "\n";
        std::cout << "Compute queue family index: " << queueFamilyIndices.compute() << std::endl;
    }
}


void App::createLogicalDevice() {

    queues.insert(std::make_pair<uint32_t, VkQueue>(queueFamilyIndices.graphics(), VK_NULL_HANDLE));
    if (!queues.count(queueFamilyIndices.transfer())) queues.insert(std::make_pair<uint32_t, VkQueue>(queueFamilyIndices.transfer(), VK_NULL_HANDLE));
    if (!queues.count(queueFamilyIndices.present())) queues.insert(std::make_pair<uint32_t, VkQueue>(queueFamilyIndices.present(), VK_NULL_HANDLE));
    if (!queues.count(queueFamilyIndices.compute())) queues.insert(std::make_pair<uint32_t, VkQueue>(queueFamilyIndices.compute(), VK_NULL_HANDLE));


    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    for (auto& entry : queues) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = entry.first;
        queueCreateInfo.queueCount = 1;

        float queuePriority = 1.0f;
        queueCreateInfo.pQueuePriorities = &queuePriority;

        queueCreateInfos.emplace_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures enabledFeatures{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    createInfo.pEnabledFeatures = &enabledFeatures;

    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    if (enableValidationLayers) {
        createInfo.enabledLayerCount = static_cast<int32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        createInfo.enabledLayerCount = 0;
    }

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device)) {
        throw std::runtime_error("failed to create logical device!");
    }

    for (auto& entry : queues) {
        vkGetDeviceQueue(device, entry.first, 0, &(entry.second));
        if (DEBUG && DEBUG_LEVEL >= MODERATE) assert(entry.second != VK_NULL_HANDLE);
        if (DEBUG && PRINT_DEBUG && DEBUG_LEVEL >= ALL) {
            std::cout << "Queue with index " << entry.first << " has location " << entry.second << std::endl;
        }
    }
}

VkSurfaceFormatKHR App::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }

    return availableFormats[0];
}

VkPresentModeKHR App::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    std::unordered_map<std::string, VkPresentModeKHR> modes = {
        {"immediate", VK_PRESENT_MODE_IMMEDIATE_KHR}, {"fifo", VK_PRESENT_MODE_FIFO_KHR},
        {"fifo-relaxed", VK_PRESENT_MODE_FIFO_RELAXED_KHR}, {"mailbox", VK_PRESENT_MODE_MAILBOX_KHR} };
    std::string targetModeString = Config::parameters.getString("swapchain-mode");
    VkPresentModeKHR targetMode = modes[targetModeString];
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == targetMode) {
            return targetMode;
        }
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D App::chooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent{
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}

void App::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
    
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapChainExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }
    if (DEBUG && PRINT_DEBUG && DEBUG_LEVEL >= ALL) {
        std::cout << "Image count of swapchain : " << imageCount << std::endl;
    }
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1; //not one if steroscopic, ie using VR
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; //TODO change to do post-processing like tonemapping

    std::vector<uint32_t> indices = queueFamilyIndices.indices();
    if (DEBUG && DEBUG_LEVEL >= ALL) assert(indices.size() >= 1);
    if (indices.size() == 1) {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    else {
        //TODO make sharing mode exclusive and handle ownership manually
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT; 
        createInfo.queueFamilyIndexCount = static_cast<uint32_t>(indices.size());
        createInfo.pQueueFamilyIndices = indices.data();
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    //TODO experiment with funky transparent windows
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR; 
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain!");
    }

    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;

    uint32_t finalImageCount = 0;
    vkGetSwapchainImagesKHR(device, swapChain, &finalImageCount, nullptr);
    swapChainImages.resize(finalImageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &finalImageCount, swapChainImages.data());
}

void App::createSwapChainImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create swap chain image views!");
        }
    }
}

void App::initVulkan() {
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicalDevice();
    createSwapChain();
    createSwapChainImageViews();
}

//TODO look at dynamic rendering, add more attachments / subpasses / depnedencies for subpasses / deferred
void App::createRenderPass() {
    std::unordered_map<int, VkSampleCountFlagBits> sampleMap = 
        {{1, VK_SAMPLE_COUNT_1_BIT}, {2, VK_SAMPLE_COUNT_2_BIT}, {4, VK_SAMPLE_COUNT_4_BIT}, 
         {8, VK_SAMPLE_COUNT_8_BIT}, {16, VK_SAMPLE_COUNT_16_BIT}, {32, VK_SAMPLE_COUNT_32_BIT},
        {64, VK_SAMPLE_COUNT_64_BIT} };
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = sampleMap[Config::parameters.getInt("multisamples")];
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //TODO, maybe unnecessary if doing headless?
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachments[] = { colorAttachmentRef };

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1; //referenced by "layout(location = 0) out vec4 outColor" directive
    subpass.pColorAttachments = &colorAttachmentRef;
    //pInputAttatchments <- attatchments read from shader
    //pResolveAttatchments <- attatchments used for multisampling color attatchments
    //pDepthStencilAttatchment <- attatchmentt for depth + stencil data
    //pPreserveAttatchments <- attatchments that are not used by preserved

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // VK_SUBPASS_EXTERNAL refers to b4 render pass if src, after renderpass if dst
    //dst subpass must alway be higher than src to prevent cycles
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    if (vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

/*VkShaderModule App::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
    return shaderModule;
} */

VkShaderModule App::createShaderModule(uint32_t i) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = Config::shaderSizes[i]; 
    createInfo.pCode = reinterpret_cast<const uint32_t*>(Config::shaders[i]);
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
    return shaderModule;
}

void App::createDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = Config::uniformBufferObjectStages();
    uboLayoutBinding.pImmutableSamplers = nullptr;
    
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor set layout!");
    }

};

//TODO TODO use dynamic rendering instead of renderpass system
void App::createGraphicsPipeline() {
    std::vector<VkShaderModule> shaderModules;
    for (uint32_t i = 0; i < Config::shaders.size(); i++) {
        shaderModules.emplace_back(createShaderModule(i));
    }

    std::unordered_map<ShaderStageT, VkShaderStageFlagBits> flags = 
        { {VERTEX, VK_SHADER_STAGE_VERTEX_BIT}, {FRAGMENT, VK_SHADER_STAGE_FRAGMENT_BIT},
          {TESSELATION_CONTROL, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT }, 
          {TESSELATION_EVALUATION, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT },
          {GEOMETRY, VK_SHADER_STAGE_GEOMETRY_BIT}, {MESH, VK_SHADER_STAGE_MESH_BIT_EXT},
          {COMPUTE, VK_SHADER_STAGE_COMPUTE_BIT},
          {CLUSTER_CULLING, VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI } };

    std::vector<VkDynamicState> dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR
    };

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    VkPipelineColorBlendAttachmentState  colorBlendAttatchment{};
    colorBlendAttatchment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttatchment.blendEnable = VK_TRUE;
    colorBlendAttatchment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttatchment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttatchment.colorBlendOp = VK_BLEND_OP_ADD; //TODO, create switch for emissive materials?
    colorBlendAttatchment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttatchment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttatchment.alphaBlendOp = VK_BLEND_OP_ADD; //TODO, make max ?

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttatchment;

    //main loop
    static bool derivePipelines = Config::parameters.getBool("derive-pipelines");
    std::vector<VkGraphicsPipelineCreateInfo> pipelineInfos{};
    pipelineInfos.reserve(Config::shaderStages.size());
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkVertexInputBindingDescription bindingDescription{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    uint32_t i = 0;
    for (auto& stage : Config::shaderStages) {
        uint32_t stageIdx = i;
        for (const auto& shader : stage.shaderInfos) {
            VkPipelineShaderStageCreateInfo shaderStageInfo{};
            shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStageInfo.stage = flags[shader.first];
            shaderStageInfo.module = shaderModules[shader.second];
            shaderStageInfo.pName = "main";

            stages.emplace_back(shaderStageInfo);
        }
        //pipeline layout info
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = (stage.type == MAIN_RENDER) ? 1 : 0;
        pipelineLayoutInfo.pSetLayouts = (stage.type == MAIN_RENDER) ? &descriptorSetLayout : nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        //vertexInput
        bindingDescription = Config::getVertexBindingDescription();
        attributeDescriptions = Config::getVertexAttributeDescriptions();

        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        //inputAssembly
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        static bool stripify = Config::parameters.getBool("stripify");
        inputAssembly.primitiveRestartEnable = ((stripify && (stage.type == MAIN_RENDER)) || (stage.type == DEBUG_DRAW)) ? VK_TRUE : VK_FALSE;
        if (stripify && stage.type == MAIN_RENDER)  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; //restart index is 0xFFFFFFFF
        else if (stage.type == DEBUG_DRAW) inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; //like tri strip, restart is 0xFFFFFFFF
        else inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        //rasterizer
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = (stage.type == SHADOWMAP && deviceFeatures.depthClamp) ? VK_TRUE :VK_FALSE;
        rasterizer.polygonMode = (stage.type == DEBUG_DRAW && deviceFeatures.fillModeNonSolid) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        static float lineMin = deviceProperties.limits.lineWidthRange[0];
        static float lineMax = deviceProperties.limits.lineWidthRange[1];
        float debugLineWidth = std::clamp(10.0f, lineMin, lineMax);
        rasterizer.lineWidth = (stage.type == DEBUG_DRAW && deviceFeatures.wideLines) ? debugLineWidth : 1.0f;
        rasterizer.cullMode = (stage.type == SHADOWMAP || stage.type == DEBUG_DRAW) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = (stage.type == SHADOWMAP || stage.type == DEBUG_DRAW) ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = (stage.type == SHADOWMAP) ? VK_TRUE : VK_FALSE;
        rasterizer.depthBiasConstantFactor = (stage.type == SHADOWMAP) ?  4.0f : 0.0f;
        if (stage.type != SHADOWMAP) rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = (stage.type == SHADOWMAP) ? 1.5f : 0.0f;

       
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(stage.shaderInfos.size());
        pipelineInfo.pStages = &stages[stageIdx];
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = renderPass;
        pipelineInfo.subpass = 0;

        if (stage.type == MAIN_RENDER && derivePipelines) {
            pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
            pipelineInfos.insert(pipelineInfos.begin(), pipelineInfo);
        }
        else if(derivePipelines) {
            pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
            pipelineInfo.basePipelineIndex = 0;
            pipelineInfos.emplace_back(pipelineInfo);
        }
        else {
            pipelineInfos.emplace_back(pipelineInfo);
            pipelineInfo.basePipelineIndex = -1;
            pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        }
        i++;
    }

    if (DEBUG && DEBUG_LEVEL >= MODERATE) {
        assert(pipelineInfos.size() == Config::shaderStages.size());
        assert(pipelineInfos.size() == 0 || !derivePipelines || (pipelineInfos[0].flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT));
    }

    std::vector<VkPipeline> pipelinesList;
    pipelinesList.resize(Config::shaderStages.size());
    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, static_cast<uint32_t>(pipelineInfos.size()), pipelineInfos.data(), nullptr, pipelinesList.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipelines!");
    }

    for (size_t i = 0; i < pipelinesList.size(); i++) {
        pipelines.insert(std::make_pair(Config::shaderStages[i].type, pipelinesList[i]));
    }

    for (auto& module : shaderModules) {
        vkDestroyShaderModule(device, module, nullptr);
    }
}

void App::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attatchments[] = {
            swapChainImageViews[i] //TODO add attatchments for tonemapping / deferred rendering
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attatchments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device, &framebufferInfo, nullptr, &swapChainFramebuffers[i])) {
            throw std::runtime_error("Failed to create framebuffer!");
        }
    }
}

//TODO TODO create transient command pools, 
// seconary buffers to do multi threading
void App::createCommandPools() {
    std::vector<std::pair<QueueType, uint32_t>> loop = 
    { {GRAPHICS_QUEUE, queueFamilyIndices.graphics()}, {PRESENT_QUEUE, queueFamilyIndices.present()},
      {TRANSFER_QUEUE, queueFamilyIndices.transfer()},  {COMPUTE_QUEUE, queueFamilyIndices.compute()} };
    for (auto& type : loop) {
        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = type.first == TRANSFER_QUEUE ? VK_COMMAND_POOL_CREATE_TRANSIENT_BIT : VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = type.second;

        commandPools.insert(std::make_pair(type.first, VK_NULL_HANDLE));
        if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPools[type.first]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create command pool!");
        }
    }
}


void App::createCommandBuffers() {
    //For now only making command buffer for graphics queue, 
    //TODO make cmmand buffers for other queues (except transfer> only transient 
    //command pool ? )

    std::vector<VkCommandBuffer> buffers(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPools[GRAPHICS_QUEUE];
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(buffers.size());
    commandBuffers.insert({GRAPHICS_QUEUE, buffers});

    if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers[GRAPHICS_QUEUE].data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }

}

void App::createVertexBuffer() {
    uint32_t numElements, elementSize = 0;
    Config::vertexBufferSize(&numElements, &elementSize);
    VkDeviceSize bufferSize = static_cast<VkDeviceSize>(numElements * elementSize);

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    //TODO compare to explicitly flushing memory (instead of using HOST_COHERENT bit?)
    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, Config::vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void App::createIndexBuffer() {
    uint32_t numElements, elementSize = 0;
    Config::indexBufferSize(&numElements, &elementSize);
    VkDeviceSize bufferSize = numElements * elementSize;

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, Config::indices.data(), (size_t)bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

    copyBuffer(stagingBuffer, indexBuffer, bufferSize);

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);
}

void App::createUniformBuffers() {
    VkDeviceSize uniformBufferSize = Config::uniformBufferObjectSize();

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        createBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i], uniformBuffersMemory[i]);
        vkMapMemory(device, uniformBuffersMemory[i], 0, uniformBufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void App::createDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    //TODO if we modify / free descriptor pool, need VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

void App::createDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate descriptor sets!");
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = Config::uniformBufferObjectSize();

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr;
        descriptorWrite.pTexelBufferView = nullptr;

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

/*
* Semaphores : used for queue-queue synchronization, start as unsignaled,
* Queue submit can signal an unsignaled semaphore and wait for a semaphore to be signaled
* Fences : used for CPU-GPU synchronization (?)
* TODO : add more semaphores for more subpasses / mulithreading / UI rendering ?
*/
void App::createSynchObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(swapChainImageViews.size());
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);


    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT; //so first instance of vkWaitFence works

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(device, &fenceInfo, nullptr, &inFlightFences[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create sempaphores!");
        }
    }
    for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create semaphores!");
        }
    }
}

void App::initProgram() {
    createRenderPass();
    createDescriptorSetLayout();
    createGraphicsPipeline();
    createFramebuffers();
    createCommandPools();
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorPool();
    createDescriptorSets();
    createCommandBuffers();
    createSynchObjects();
}

void App::recreateSwapChain() {
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device);

    cleanupSwapChain();

    createSwapChain();
    createSwapChainImageViews();
    createFramebuffers();
}

//TODO almost def need to change for secondar command buffers / subpasses / shadowmapping
//also TODO change command buffer stucture so we dont have to wait on VKFence to being drawing ?
// ie instantiate command buffers per swapchain image
void App::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = { 0, 0 };
    renderPassInfo.renderArea.extent = swapChainExtent;

    //TODO change when adding subpasses / deferred rendering
    VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    //TODO reconfigure for secondary command buffers (VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS) and/or dynamic rendering
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    /*static*/ VkPipeline pipeline = pipelines[MAIN_RENDER];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    VkBuffer vertexBuffers[] = { vertexBuffer }; 
    VkDeviceSize offsets[] = { 0 };
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets); //TODO bind both vertex+index in one call
    
    //TODO remember this is static
    static uint32_t numIndices, indexSize = 0;
    Config::indexBufferSize(&numIndices, &indexSize);
    if (numIndices > 0) {
        //ony two sizes of indices allowed
        static VkIndexType indexType = indexSize <= 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT16);
    }

    //TODO change if dyanmic pipeline is different
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(swapChainExtent.width);
    viewport.height = static_cast<float>(swapChainExtent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f; //TODO chnage maxDepth for more resolution?
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = { 0, 0 };
    scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

  
    if (Config::uniformBufferObjectSize() > 0) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[flightFrame], 0, nullptr);
    }
    if (numIndices > 0) {
        vkCmdDrawIndexed(commandBuffer, numIndices, 1, 0, 0, 0);
    }
    else{
        uint32_t numVertices, vertexSize = 0;
        Config::vertexBufferSize(&numVertices, &vertexSize);
        if (numVertices > 0) {
            vkCmdDraw(commandBuffer, numVertices, 1, 0, 0);
        }
        else {
            vkCmdDraw(commandBuffer, 3, 1, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }
}


/* At a high level (from https://vulkan-tutorial.com/en/Drawing_a_triangle/Drawing/Rendering_and_presentation)
- Wait for the previous frame to finish
-Acquire an image from the swap chain
-Record a command buffer which draws the scene onto that image
-Submit the recorded command buffer
-Present the swap chain image
*/ 
void App::drawFrame() {
    vkWaitForFences(device, 1, &inFlightFences[flightFrame], VK_TRUE, UINT64_MAX); //host waits

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[flightFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        framebufferResized = false;
        recreateSwapChain();
        return;
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    vkResetFences(device, 1, &inFlightFences[flightFrame]);

    /*static*/ VkCommandBuffer commandBuffer = commandBuffers[GRAPHICS_QUEUE][flightFrame];
    vkResetCommandBuffer(commandBuffer, 0);
    
    //update scene behavior
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();
    Config::updateUniformBuffer(uniformBuffersMapped, flightFrame, time, swapChainExtent.width, swapChainExtent.height);


    recordCommandBuffer(commandBuffer, imageIndex);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //TODO change for multiple subpasses
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[flightFrame]};
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1; //TODO change for using multiple command buffers
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    /*static*/ VkQueue graphicsQueue = queues[queueFamilyIndices.graphics()];
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[flightFrame]) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer!");
    }
    
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = { swapChain };
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;
    // presentInfo.pResults = nullptr;  // Optional

    /*static*/ VkQueue presentQueue = queues[queueFamilyIndices.present()];
    vkQueuePresentKHR(presentQueue, &presentInfo);

    flightFrame = (flightFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        FRAME++;
        glfwPollEvents();
        drawFrame();
    }

    vkDeviceWaitIdle(device);
}

void App::cleanupSwapChain() {
    for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
        vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void App::cleanup() {
    cleanupSwapChain();

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        freeBuffer(uniformBuffers[i], uniformBuffersMemory[i]);
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);

    freeBuffer(indexBuffer, indexBufferMemory);

    freeBuffer(vertexBuffer, indexBufferMemory);

    for (auto& semaphore : imageAvailableSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    for (auto& semaphore : renderFinishedSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    for (auto& fence : inFlightFences) {
        vkDestroyFence(device, fence, nullptr);
    }

    for (auto& pool : commandPools) {
        vkDestroyCommandPool(device, pool.second, nullptr);
    }
    
    for (auto& pipeline : pipelines) {
        vkDestroyPipeline(device, pipeline.second, nullptr);
    }
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    

    glfwDestroyWindow(window);
    glfwTerminate();
}