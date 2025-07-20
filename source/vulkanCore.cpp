//#define GLFW_INCLUDE_VULKAN
//#include <GLFW/glfw3.h>
#include <math.h>
#include <iostream>
#include <vector>
#include <unordered_set>
#include <cassert> //(void)0 if NDEBUG defined
#include <algorithm>
#include <chrono>
#include <array>
#include <map>

#include "vulkanCore.hpp"
#include "utils.hpp"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> //Get rid of this once we have a way of updating uniform information
#include "input.hpp"


App::App(ParamMap commandLineArguments) {
    if (commandLineArguments.p.empty()) return;

    WIDTH = commandLineArguments.getVec("resolution").x;
    HEIGHT = commandLineArguments.getVec("resolution").y;

    globalParameters.HEADLESS = commandLineArguments.getBool("headless");
    globalParameters.PHYSICAL_DEVICE = commandLineArguments.getString("physical-device");
    globalParameters.LIST_PHYSICAL_DEVICES = commandLineArguments.getBool("list-physical-devices");
    globalParameters.SEPERATE_QUEUE_FAMILIES = commandLineArguments.getBool("seperate-queue-families");
    globalParameters.DEBUG_LEVEL = commandLineArguments.getInt("debug-level");
    globalParameters.PRINT_DEBUG = commandLineArguments.getBool("print-debug-output");
    globalParameters.SWAPCHAIN_MODE = commandLineArguments.getString("swapchain-mode");
    globalParameters.FORCE_SHOW_FPS = commandLineArguments.getBool("force-show-fps");
    globalParameters.COMBINED_INDEX_VERTEX = commandLineArguments.getBool("combined-vertex-index");
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
    glfwSetKeyCallback(window, Input::keyCallback);
    glfwSetCursorPosCallback(window, Input::cursorPosCallback);
    glfwSetJoystickCallback((Input::joystickCallback));
    glfwSetCursorEnterCallback(window, Input::cursorEnterCallback);
    glfwSetMouseButtonCallback(window, Input::mouseButtonCallback);
    glfwSetScrollCallback(window, Input::scrollCallback);
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
    } else if (globalParameters.FORCE_SHOW_FPS) {
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

    if (DEBUG && globalParameters.DEBUG_LEVEL >= SEVERE) {
        std::unordered_set<std::string> allAvailableExtensionsSet;
        if (globalParameters.PRINT_DEBUG) std::cout << "\nAvailable extensions\n" << std::endl;
     

        for (const auto& extension : allAvailableExtensions) {
            if (globalParameters.PRINT_DEBUG) std::cout << extension.extensionName << std::endl;
            allAvailableExtensionsSet.insert(extension.extensionName);
        }
        
        if (globalParameters.PRINT_DEBUG) std::cout << "\nChecking GLFW extensions : \n" << std::endl;
        for (uint32_t i = 0; i < extensions.size(); i++) {
            std::string extension = std::string(extensions[i]);
            if (!allAvailableExtensionsSet.count(extension)) {
                throw std::runtime_error("Necessray glfw extension " + extension + "not in list of available Vulkan extensions!");
            }
            if (globalParameters.PRINT_DEBUG) std::cout << extension << std::endl;

        }
        if (globalParameters.PRINT_DEBUG) std::cout << "\n" << std::endl;
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
    if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= ALL) printQueueFamilies(device);
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

    int i = 0;
    for (const auto& queueFamily : queueFamilies) {
        if (!globalParameters.SEPERATE_QUEUE_FAMILIES) {
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

    uint32_t vulkanVersion;
    vkEnumerateInstanceVersion(&vulkanVersion);
    uint32_t versionMajor = VK_API_VERSION_MAJOR(vulkanVersion);
    uint32_t versionMinor = VK_API_VERSION_MINOR(vulkanVersion);
    uint32_t versionPatch = VK_API_VERSION_PATCH(vulkanVersion);


    if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= ALL) {
        std::cout << "\nUsing vulkan version " << versionMajor << "." << versionMinor << "." << versionPatch << std::endl;
    }

    if (useDynamicRendering && versionMinor < 2) {
        requiredExtensions.emplace(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        if (versionMinor < 1) {
            requiredExtensions.emplace(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
    }

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    if (useDynamicRendering && !requiredExtensions.empty()) {
        if (requiredExtensions.count(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME)) {
            requiredExtensions.erase(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
            useDynamicRendering = false;
        }
        if (requiredExtensions.count(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            requiredExtensions.erase(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
            useDynamicRendering = false;
        }
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

    if (deviceProperties.deviceName == globalParameters.PHYSICAL_DEVICE) return std::numeric_limits<uint32_t>::max();

    uint32_t score = 0;
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;

    score += deviceProperties.limits.maxImageDimension2D;

    if (DEBUG && (globalParameters.LIST_PHYSICAL_DEVICES || globalParameters.DEBUG_LEVEL >= SEVERE)) {
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

void App::determineOptionalExtensions(const VkPhysicalDevice& device) {
    if (optionalDeviceExtensions.size() <= 0) return;

    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    //add necessary extensions based on queried vulkan version
    uint32_t vulkanVersion;
    vkEnumerateInstanceVersion(&vulkanVersion);
    uint32_t versionMajor = VK_API_VERSION_MAJOR(vulkanVersion);
    uint32_t versionMinor = VK_API_VERSION_MINOR(vulkanVersion);
    uint32_t versionPatch = VK_API_VERSION_PATCH(vulkanVersion);


    if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= ALL) {
        std::cout << "\nUsing vulkan version " << versionMajor << "." << versionMinor << "." << versionPatch << std::endl;
    }

    //TODO why do I need to include these even though vulkan version should include these by default?
    if (useDynamicRendering /* && versionMinor < 2*/) {
        optionalDeviceExtensions.push_back(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
        //if (versionMinor < 1) {
        optionalDeviceExtensions.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        //}
    }

    //now check that all extensions are supported
    std::unordered_set<std::string> optionalExtensions(optionalDeviceExtensions.begin(), optionalDeviceExtensions.end());
    std::unordered_set<const char*> finalOptionalExtensions(optionalDeviceExtensions.begin(), optionalDeviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        optionalExtensions.erase(extension.extensionName);
    }

    if (useDynamicRendering) {
        if (optionalExtensions.count(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME) ||
            optionalExtensions.count(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME) ||
            optionalExtensions.count(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME)) {
            useDynamicRendering = false;
            //have to actually remove optional extensions from optionalExtensionsList
            finalOptionalExtensions.erase(VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME);
            finalOptionalExtensions.erase(VK_KHR_DEPTH_STENCIL_RESOLVE_EXTENSION_NAME);
            finalOptionalExtensions.erase(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
        }
    }

    //now insert the actual used optional extensions into end of full extension list
    deviceExtensions.insert(deviceExtensions.end(), finalOptionalExtensions.begin(), finalOptionalExtensions.end());

    return;
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

    determineOptionalExtensions(physicalDevice);

    if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= ALL) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        std::cout << "--Chosen device : " << deviceProperties.deviceName << std::endl;
    }

    queueFamilyIndices = findQueueFamilies(physicalDevice);
    if (DEBUG && globalParameters.DEBUG_LEVEL >= 2) assert(queueFamilyIndices.isComplete());
    if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= 3) {
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

    //VkPhysicalDeviceFeatures enabledFeatures{};
    vkGetPhysicalDeviceFeatures(physicalDevice, &deviceFeatures);
    vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties); //TODO do I need to be caching this ? probably

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    constexpr VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
            .dynamicRendering = VK_TRUE,
    };
    if (useDynamicRendering) {
        createInfo.pNext = &dynamicRenderingFeature;
    }


    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();

    //TODO do I need to be enabling all features ?
    createInfo.pEnabledFeatures = &deviceFeatures;

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
        if (DEBUG && globalParameters.DEBUG_LEVEL >= MODERATE) assert(entry.second != VK_NULL_HANDLE);
        if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= ALL_OUTPUT) {
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
    VkPresentModeKHR targetMode = modes[globalParameters.SWAPCHAIN_MODE];
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
    if (DEBUG && globalParameters.PRINT_DEBUG && globalParameters.DEBUG_LEVEL >= ALL_OUTPUT) {
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
    if (DEBUG && globalParameters.DEBUG_LEVEL >= ALL_OUTPUT) assert(indices.size() >= 1);
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

VkImageView App::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags = VK_IMAGE_ASPECT_COLOR_BIT) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
    createInfo.subresourceRange.aspectMask = aspectFlags;
    createInfo.subresourceRange.baseMipLevel = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.layerCount = 1;

    VkImageView imageView;
    if (vkCreateImageView(device, &createInfo, nullptr, &imageView) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swap chain image views!");
    }

    return imageView;
}

void App::createSwapChainImageViews() {
    swapChainImageViews.resize(swapChainImages.size());

    for (size_t i = 0; i < swapChainImages.size(); i++) {
        swapChainImageViews[i] = createImageView(swapChainImages[i], swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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

VkFormat App::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features, const std::vector<VkFormat>* fallbackCandidates = nullptr) {
    for (VkFormat format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);

        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    //none found, choose one of formats that MUST be supported according to Vulkan spec
    if (fallbackCandidates == nullptr) throw std::runtime_error("No supported format found and fallback candidate list empty!");
    for (VkFormat format : *fallbackCandidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        if (tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) {
            return format;
        }
        else if (tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }

    throw std::runtime_error("No supported format found : not even when using fallback candidates!");
}

VkFormat App::findDepthFormat() {
    const std::vector<VkFormat> fallbackCandidates = { VK_FORMAT_X8_D24_UNORM_PACK32 };
    return findSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT , VK_FORMAT_D24_UNORM_S8_UINT },
        VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT, &fallbackCandidates);
}

void App::createDepthResources() {
    
    depthFormat = findDepthFormat();


    createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, depthImage, depthImageMemory);

    depthImageView = createImageView(depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

    transitionImageLayout(depthImage, depthFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
}

//TODO add more attachments / subpasses / depnedencies for subpasses / deferred
//TODO add support for multisampling
void App::createRenderPass(const Mode& mode) {
    std::unordered_map<int, VkSampleCountFlagBits> sampleMap =
    { {1, VK_SAMPLE_COUNT_1_BIT}, {2, VK_SAMPLE_COUNT_2_BIT}, {4, VK_SAMPLE_COUNT_4_BIT},
     {8, VK_SAMPLE_COUNT_8_BIT}, {16, VK_SAMPLE_COUNT_16_BIT}, {32, VK_SAMPLE_COUNT_32_BIT},
    {64, VK_SAMPLE_COUNT_64_BIT} };
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = sampleMap[mode.modeParameters.MULTI_SAMPLES];
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; //TODO, maybe unnecessary if doing headless?
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference colorAttachmentRefs[] = { colorAttachmentRef };

    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = sampleMap[mode.modeParameters.MULTI_SAMPLES];
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;


    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1; //referenced by "layout(location = 0) out vec4 outColor" directive
    subpass.pColorAttachments = colorAttachmentRefs;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;
    //pInputAttatchments <- attatchments read from shader
    //pResolveAttatchments <- attatchments used for multisampling color attatchments
    //pPreserveAttatchments <- attatchments that are not used by preserved

    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    // VK_SUBPASS_EXTERNAL refers to b4 render pass if src, after renderpass if dst
    //dst subpass must alway be higher than src to prevent cycles
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

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


VkShaderModule App::createShaderModule(const Mode& mode, uint32_t i) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = mode.shaderSizes[i]; 
    createInfo.pCode = reinterpret_cast<const uint32_t*>(mode.shaders[i]);
    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create shader module!");
    }
    return shaderModule;
}

void App::createDescriptorSetLayouts(const Mode& mode) {
    //TODO maybe unnecessary since values are already mapped
    std::unordered_map<DescriptorTypeT, VkDescriptorType> descriptorNameMap = {
        {SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER},
        {COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
        {STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
        {UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
        {STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
        {UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
        {STORAGE_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC}
    };

    for (uint32_t i = 0; i < mode.descriptorBindings.size(); i++) {
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        VkDescriptorSetLayoutBinding binding{};
        
        for (uint32_t j = 0; j < mode.descriptorBindings[i].size(); j++) {
            DescriptorBinding myBinding = mode.descriptorBindings[i][j];
            binding.binding = j;
            binding.descriptorCount = myBinding.count;
            assert(descriptorNameMap.count(myBinding.type));
            binding.descriptorType = descriptorNameMap[myBinding.type];
            binding.stageFlags = myBinding.stages;
            binding.pImmutableSamplers = nullptr;

            bindings.emplace_back(binding);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        descriptorSetLayouts.emplace_back(VkDescriptorSetLayout{});
        if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &(descriptorSetLayouts[i])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create descriptor set layout!");
        }
    }

};

//TODO TODO use dynamic rendering instead of renderpass system
void App::createGraphicsPipeline(const Mode& mode) {
    std::vector<VkShaderModule> shaderModules;
    for (uint32_t i = 0; i < mode.shaders.size(); i++) {
        shaderModules.emplace_back(createShaderModule(mode, i));
    }

    std::unordered_map<ShaderStageT, VkShaderStageFlagBits> flags = 
        { {VERTEX_STAGE, VK_SHADER_STAGE_VERTEX_BIT}, {FRAGMENT_STAGE, VK_SHADER_STAGE_FRAGMENT_BIT},
          {TESSELLATION_CONTROL_STAGE, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT },
          {TESSELLATION_EVALUATION_STAGE, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT },
          {GEOMETRY_STAGE, VK_SHADER_STAGE_GEOMETRY_BIT}, {MESH_STAGE, VK_SHADER_STAGE_MESH_BIT_EXT},
          {COMPUTE_STAGE, VK_SHADER_STAGE_COMPUTE_BIT},
          {CLUSTER_CULLING_HUAWEI_STAGE, VK_SHADER_STAGE_CLUSTER_CULLING_BIT_HUAWEI } };

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
    [[maybe_unused]]
    static bool derivePipelines = mode.modeParameters.DERIVE_PIPELINES;
    std::vector<VkGraphicsPipelineCreateInfo> pipelineInfos{};
    pipelineInfos.reserve(mode.shaderStages.size());
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    VkVertexInputBindingDescription bindingDescription{};
    VkPipelineDepthStencilStateCreateInfo depthStencil{};
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

    uint32_t i = 0;
    std::vector<VkPipeline> pipelinesList;
    pipelinesList.resize(mode.shaderStages.size());
    for (auto& stage : mode.shaderStages) {
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
        //TODO change shaderStages variable in mode.hpp to specifiy which descriptor set layouts instead of using all
        pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size();// (stage.type == MAIN_RENDER) ? descriptorSetLayouts.size() : 0;
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();// (stage.type == MAIN_RENDER) ? descriptorSetLayouts.data() : nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = mode.pushConstantRanges.size();
        pipelineLayoutInfo.pPushConstantRanges = mode.pushConstantRanges.size() > 0  ? reinterpret_cast<const VkPushConstantRange*>(mode.pushConstantRanges.data()) : nullptr;

        if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create pipeline layout!");
        }

        //vertexInput
        bindingDescription = getVertexBindingDescription();
        attributeDescriptions = getVertexAttributeDescriptions();

        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.vertexBindingDescriptionCount = 1;
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

        //inputAssembly
        inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        static bool stripify = mode.modeParameters.STRIPIFY; 
        inputAssembly.primitiveRestartEnable = ((stripify && (stage.type == MAIN_RENDER)) || (stage.type == DEBUG_DRAW)) ? VK_TRUE : VK_FALSE;
        if (stripify && stage.type == MAIN_RENDER)  inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP; //restart index is 0xFFFFFFFF
        else if (stage.type == DEBUG_DRAW) inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP; //like tri strip, restart is 0xFFFFFFFF
        else inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

        //rasterizer
        rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterizer.depthClampEnable = (stage.type == SHADOWMAP && deviceFeatures.depthClamp) ? VK_TRUE :VK_FALSE;
        rasterizer.polygonMode = (stage.type == DEBUG_DRAW && deviceFeatures.fillModeNonSolid) ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(physicalDevice, &deviceProperties);
        static float lineMin = deviceProperties.limits.lineWidthRange[0];
        static float lineMax = deviceProperties.limits.lineWidthRange[1];
        float debugLineWidth = std::clamp(3.0f, lineMin, lineMax);
        rasterizer.lineWidth = (stage.type == DEBUG_DRAW && deviceFeatures.wideLines) ? debugLineWidth : 1.0f;
        rasterizer.cullMode =  (stage.type == SHADOWMAP || stage.type == DEBUG_DRAW) ? VK_CULL_MODE_NONE : VK_CULL_MODE_BACK_BIT;
        rasterizer.frontFace = (stage.type == SHADOWMAP || stage.type == DEBUG_DRAW) ? VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterizer.depthBiasEnable = (stage.type == SHADOWMAP) ? VK_TRUE : VK_FALSE;
        rasterizer.depthBiasConstantFactor = (stage.type == SHADOWMAP) ?  4.0f : 0.0f;
        if (stage.type != SHADOWMAP) rasterizer.depthBiasClamp = 0.0f;
        rasterizer.depthBiasSlopeFactor = (stage.type == SHADOWMAP) ? 1.5f : 0.0f;
       
        if (stage.type == SHADOWMAP || stage.type == MAIN_RENDER || stage.type == DEBUG_DRAW) {
            depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
            depthStencil.depthTestEnable = VK_TRUE;
            depthStencil.depthWriteEnable = VK_TRUE;
            depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
            depthStencil.depthBoundsTestEnable = VK_FALSE;
            depthStencil.minDepthBounds = 0.0f; //optional
            depthStencil.maxDepthBounds = 1.0; //optional
            depthStencil.stencilTestEnable = VK_FALSE;
            depthStencil.front = {};
            depthStencil.back = {};
        }

        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.stageCount = static_cast<uint32_t>(stage.shaderInfos.size());
        pipelineInfo.pStages = &stages[stageIdx];
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &inputAssembly;
        pipelineInfo.pViewportState = &viewportState;
        pipelineInfo.pRasterizationState = &rasterizer;
        pipelineInfo.pMultisampleState = &multisampling;
        pipelineInfo.pDepthStencilState = &depthStencil;// ((stage.type == SHADOWMAP) || (stage.type == MAIN_RENDER)) ? &depthStencil : nullptr;
        pipelineInfo.pColorBlendState = &colorBlending;
        pipelineInfo.pDynamicState = &dynamicState;
        pipelineInfo.layout = pipelineLayout;
        pipelineInfo.renderPass = useDynamicRendering ? nullptr : renderPass;

        pipelineInfo.subpass = 0;

        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipelinesList[i]) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create graphics pipelines!");
        }

        //if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, static_cast<uint32_t>(pipelineInfos.size()), pipelineInfos.data(), nullptr, pipelinesList.data()) != VK_SUCCESS) {
        //    throw std::runtime_error("Failed to create graphics pipelines!");
        //}

        //if (stage.type == MAIN_RENDER && derivePipelines) {
        //    pipelineInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
        //    pipelineInfos.insert(pipelineInfos.begin(), pipelineInfo);
        //}
        //else if(derivePipelines) {
        //    pipelineInfo.flags = VK_PIPELINE_CREATE_DERIVATIVE_BIT;
        //    pipelineInfo.basePipelineIndex = 0;
        //    pipelineInfos.emplace_back(pipelineInfo);
        //}
        //else {
        //    pipelineInfos.emplace_back(pipelineInfo);
        //    pipelineInfo.basePipelineIndex = -1;
        //    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        //}
        i++;
    }

    //if (DEBUG && globalParameters.DEBUG_LEVEL >= MODERATE) {
    //    assert(pipelineInfos.size() == mode.shaderStages.size());
    //    assert(pipelineInfos.size() == 0 || !derivePipelines || (pipelineInfos[0].flags & VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT));
    //}
    //pipelinesList.resize(mode.shaderStages.size());

    
    for (size_t i = 0; i < pipelinesList.size(); i++) {
        pipelines.insert(std::make_pair(mode.shaderStages[i].type, pipelinesList[i]));
    }

    for (auto& module : shaderModules) {
        vkDestroyShaderModule(device, module, nullptr);
    }
}

void App::createFramebuffers() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        std::array<VkImageView, 2> attachments = {
            swapChainImageViews[i], //TODO add attatchments for tonemapping / deferred rendering
            depthImageView
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
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

//TODO add more mempry types
void App::createUniformBuffers(Mode& mode) {
    //uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    //uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    //uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for (auto &descriptorSet : mode.descriptorBindings) {
            int j = 0;
            for (auto& binding : descriptorSet) {
                if (binding.type != UNIFORM_BUFFER) continue;

                VkDeviceSize uniformBufferSize = static_cast<VkDeviceSize>(binding.size);

                uniformBuffersMemory[i].emplace_back(VkDeviceMemory{});
                uniformBuffers[i].emplace_back(VkBuffer{});
                uniformBuffersMapped[i].emplace_back();

                createBuffer(uniformBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, uniformBuffers[i][j], uniformBuffersMemory[i][j]);
                mapMemory(device, uniformBuffersMemory[i][j], 0, uniformBufferSize, 0, &uniformBuffersMapped[i][j]);

                binding.index = j; //set where idx is location of buffer as App::uniformBuffers[frame][idx] and uniformBuffers[frame][idx]
                j++;
            }
        }
    }
}

void App::createTextureImageView() {
    textureImageView = createImageView(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
}

//TODO allow for different types of textures samplers
void App::createTextureSamplers(Mode& mode) {
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    int i = 0;
    for (auto& set : mode.descriptorBindings) {
        for (auto& binding : set) {
            if (binding.type != COMBINED_IMAGE_SAMPLER && binding.type != SAMPLER) continue;
            samplerInfo.magFilter = VK_FILTER_LINEAR;
            samplerInfo.minFilter = VK_FILTER_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            samplerInfo.anisotropyEnable = deviceFeatures.samplerAnisotropy;


            samplerInfo.maxAnisotropy = deviceFeatures.samplerAnisotropy ? deviceProperties.limits.maxSamplerAnisotropy : 1.0f;
            samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
            samplerInfo.unnormalizedCoordinates = VK_FALSE;
            samplerInfo.compareEnable = VK_FALSE;
            samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.mipLodBias = 0.0f;
            samplerInfo.minLod = 0.0f;
            samplerInfo.maxLod = 0.0f;

            textureImageSamplers.emplace_back(VkSampler{});
            if (vkCreateSampler(device, &samplerInfo, nullptr, &(textureImageSamplers[i])) != VK_SUCCESS) {
                throw std::runtime_error("Failed to create texture sampler!");
            }
            binding.index = i;
        }
    }
}

void App::createDescriptorPool(const Mode& mode) {
    //technically unnecessary since SamplerTypeT values match VkDescriptor type, but just in case...
    //TODO consider getting rid of it?
    std::unordered_map<DescriptorTypeT, VkDescriptorType> descriptorNameMap = {
        {SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER},
        {COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
        {STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
        {UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
        {STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
        {UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
        {STORAGE_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC}
    };
    
    std::vector<VkDescriptorPoolSize> poolSizes{};
    VkDescriptorPoolSize poolSize{};
    for (auto set : mode.descriptorBindings) {
        for (auto binding : set) {
            assert(descriptorNameMap.count(binding.type));
            poolSize.type = descriptorNameMap[binding.type];
            //TODO def need max frames in flight number for uniform and storage buffers, do I need for samplers?
            poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT); 

            poolSizes.emplace_back(poolSize);
        }
    }

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();
    //TODO if we modify / free descriptor pool, need VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT
    //TODO is this calculation of max sets correct? also cannot be going over maxBoundDescriptorSets physical device limit
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT * mode.descriptorBindings.size());

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create descriptor pool!");
    }
}

//TODO def an area to check whether we really need a MAX_FRAMES_IN_FLGHT allocation for each set
void App::createDescriptorSets(const Mode& mode) {
    std::unordered_map<DescriptorTypeT, VkDescriptorType> descriptorNameMap = {
        {SAMPLER, VK_DESCRIPTOR_TYPE_SAMPLER},
        {COMBINED_IMAGE_SAMPLER, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER},
        {SAMPLED_IMAGE, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE},
        {STORAGE_IMAGE, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE},
        {UNIFORM_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER},
        {STORAGE_TEXEL_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER},
        {UNIFORM_BUFFER, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER},
        {STORAGE_BUFFER, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER},
        {UNIFORM_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC},
        {STORAGE_BUFFER_DYNAMIC, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC}
    };

    std::array<std::vector<VkDescriptorSetLayout>, MAX_FRAMES_IN_FLIGHT> layouts;
    uint32_t totalNumSets = 0;
    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for (auto descriptorSetLayout : descriptorSetLayouts) {
            totalNumSets++;
            layouts[i].emplace_back(descriptorSetLayout);
        }
    }

    if (DEBUG && globalParameters.DEBUG_LEVEL >= ALL) std::cout << "Number of descriptor sets written: " << totalNumSets << std::endl;

    for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts[i].size());
        allocInfo.pSetLayouts = layouts[i].data();

        descriptorSets[i].resize(layouts[i].size());
        if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets[i].data()) != VK_SUCCESS) {
            throw std::runtime_error("Failed to allocate descriptor sets!");
        }
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for (size_t j = 0; j < mode.descriptorBindings.size(); j++) {
            std::vector<VkWriteDescriptorSet> descriptorWrites(mode.descriptorBindings[j].size());
            VkDescriptorBufferInfo bufferInfo{};
            VkDescriptorImageInfo imageInfo{};
            for (size_t bindingIdx = 0; bindingIdx < mode.descriptorBindings[j].size(); bindingIdx++) {
                DescriptorBinding binding = mode.descriptorBindings[j][bindingIdx];

                descriptorWrites[bindingIdx].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
                descriptorWrites[bindingIdx].dstSet = descriptorSets[i][j];
                descriptorWrites[bindingIdx].dstBinding = bindingIdx;
                descriptorWrites[bindingIdx].dstArrayElement = 0;
                descriptorWrites[bindingIdx].descriptorType = descriptorNameMap[binding.type];
                descriptorWrites[bindingIdx].descriptorCount = binding.count;
                if (binding.type == UNIFORM_BUFFER) {
                    bufferInfo.buffer = uniformBuffers[i][binding.index];
                    bufferInfo.offset = 0;
                    bufferInfo.range = binding.size;
                    descriptorWrites[bindingIdx].pBufferInfo = &bufferInfo;
                }
                else if (binding.type == COMBINED_IMAGE_SAMPLER) {
                    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
                    imageInfo.imageView = textureImageView; //TODO do not hardcode to be this one texture
                    imageInfo.sampler = textureImageSamplers[binding.index];
                    descriptorWrites[bindingIdx].pImageInfo = &imageInfo;
                }
            }
            //TODO do fill in desciptor copy count to copy MAX_FRAMES_IN_FLIGHT times so we don't need this outer loop
            vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
        }

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

void App::initProgram(Mode& mode) {
    if (!useDynamicRendering) {
        createRenderPass(mode);
    }
    createDescriptorSetLayouts(mode);
    createGraphicsPipeline(mode);
    createCommandPools(); //TODO mode to into initVulkan ?
#if defined(COMBINED_VERTEX_INDEX_BUFFER) && COMBINED_VERTEX_INDEX_BUFFER
    createVertexIndexBuffer();
#else
    createVertexBuffer();
    createIndexBuffer();
#endif
    createUniformBuffers(mode);
    createDepthResources();
    if (!useDynamicRendering) {
        createFramebuffers();
    }
    createTextureImage("statue.jpg");
    createTextureImageView();
    createTextureSamplers(mode);
    createDescriptorPool(mode);
    createDescriptorSets(mode);
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
    createDepthResources();
    if(!useDynamicRendering) createFramebuffers();
}

void App::updateUniformBuffer(uint32_t uniformIndex, const void* uniformData, uint32_t uniformSize) const{
    memcpy(uniformBuffersMapped[flightFrame][uniformIndex], uniformData, static_cast<size_t>(uniformSize));
}

void App::updatePushConstants(VkCommandBuffer commandBuffer, ShaderStageT shaderStages, uint32_t size, uint32_t offset, const void* pushConstant) const {
    vkCmdPushConstants(commandBuffer, pipelineLayout, shaderStages, offset, size, pushConstant);
}

/* At a high level
- Wait for the previous frame to finish
-Acquire an image from the swap chain
-Transition attachment to correct layout
*/
//TODO wrap dynamic rendering image layout transition, vertexIndex buffer binding, and scissor/viewport set to use Mode::draw ? 
std::pair<VkCommandBuffer, uint32_t> App::beginFrame() {
    vkWaitForFences(device, 1, &inFlightFences[flightFrame], VK_TRUE, UINT64_MAX); //host waits

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device, swapChain, UINT64_MAX, imageAvailableSemaphores[flightFrame], VK_NULL_HANDLE, &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        framebufferResized = false;
        recreateSwapChain();
        return {nullptr, -1U};
    }
    else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("Failed to acquire swapchain image!");
    }

    vkResetFences(device, 1, &inFlightFences[flightFrame]);

    VkCommandBuffer commandBuffer = commandBuffers[GRAPHICS_QUEUE][flightFrame];
    vkResetCommandBuffer(commandBuffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer!");
    }
    //TODO reconfigure for secondary command buffers (VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS)

    if (useDynamicRendering) { //will only be true if macro set to true
#if defined(DYNAMIC_RENDERING) and DYNAMIC_RENDERING
        transitionImageLayout(swapChainImages[imageIndex], swapChainImageFormat, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, commandBuffer);
        //TODO recheck whether I actually don't need to transition depth layout ? not validation layers are triggered at least...
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        const VkRenderingAttachmentInfoKHR colorAttachmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = swapChainImageViews[imageIndex],
            .imageLayout = VK_IMAGE_LAYOUT_ATTACHMENT_OPTIMAL_KHR,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValues[0]
        };

        const VkRenderingAttachmentInfoKHR depthAttchmentInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR,
            .imageView = depthImageView,
            .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .clearValue = clearValues[1]
        };


        const VkRenderingInfoKHR renderingInfo{
            .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
            .renderArea = {{ 0, 0 },  swapChainExtent},
            .layerCount = 1,
            .colorAttachmentCount = 1,
            .pColorAttachments = &colorAttachmentInfo,
            .pDepthAttachment = &depthAttchmentInfo
        };

        vkCmdBeginRendering(commandBuffer, &renderingInfo);
#endif 
    }
    else {
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = swapChainFramebuffers[imageIndex];
        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = swapChainExtent;

        //TODO change when adding subpasses / deferred rendering
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { {0.0f, 0.0f, 0.0f, 1.0f} };
        clearValues[1].depthStencil = { 1.0f, 0 };

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    /*static*/ VkPipeline pipeline = pipelines[MAIN_RENDER];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
#if defined(COMBINED_VERTEX_INDEX_BUFFER) && COMBINED_VERTEX_INDEX_BUFFER
    bindVertexIndexBuffer(commandBuffer);
#else 
    bindVertexBuffer(commandBuffer);

    bindIndexBuffer(commandBuffer);
#endif

    //TODO change if dynamic pipeline is different
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
    scissor.extent = { swapChainExtent.width, swapChainExtent.height };
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    //TODO desciptorSets sohuld actually be a 2D array not 3D probably
    if (uniformBuffers[flightFrame].size() > 0) {
        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[flightFrame][0], 0, nullptr);
    }

    return { commandBuffer, imageIndex };
}
/* At a high level
- end renderPass /(id using dynamic rendering) end rendering 
- (if using dynamic rendering) transition image layout to present  
- Submit the recorded command buffer
- Present the swap chain image
*/

void App::endFrame(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
    if (useDynamicRendering) {
#if defined(DYNAMIC_RENDERING) and DYNAMIC_RENDERING
        vkCmdEndRendering(commandBuffer);
#endif
    }
    else {
        vkCmdEndRenderPass(commandBuffer);
    }

    if (useDynamicRendering) {
#if defined(DYNAMIC_RENDERING) and DYNAMIC_RENDERING
        transitionImageLayout(swapChainImages[imageIndex], swapChainImageFormat, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, commandBuffer);
#endif
    }

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer!");
    }

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    //TODO change for multiple subpasses
    VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[flightFrame] };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1; //TODO change for using multiple command buffers
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[imageIndex] };
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

    static VkQueue presentQueue = queues[queueFamilyIndices.present()];
    vkQueuePresentKHR(presentQueue, &presentInfo);

    flightFrame = (flightFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void App::cleanupSwapChain() {
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);

    if (!useDynamicRendering) {
        for (size_t i = 0; i < swapChainFramebuffers.size(); i++) {
            vkDestroyFramebuffer(device, swapChainFramebuffers[i], nullptr);
        }
    }

    for (size_t i = 0; i < swapChainImageViews.size(); i++) {
        vkDestroyImageView(device, swapChainImageViews[i], nullptr);
    }

    vkDestroySwapchainKHR(device, swapChain, nullptr);
}

void App::cleanupProgram() {
    vkDeviceWaitIdle(device);
    cleanupSwapChain();

    for (auto sampler : textureImageSamplers) {
        vkDestroySampler(device, sampler, nullptr);
    }
    textureImageSamplers.clear();

    //TODO chnage to not hardcode this one texture
    vkDestroyImageView(device, textureImageView, nullptr);
    vkDestroyImage(device, textureImage, nullptr);
    vkFreeMemory(device, textureImageMemory, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        for (size_t j = 0; j < uniformBuffers[i].size(); j++) {
            freeBuffer(uniformBuffers[i][j], uniformBuffersMemory[i][j]);
        }
        uniformBuffers[i].clear();
        uniformBuffersMemory[i].clear();
    }

    vkDestroyDescriptorPool(device, descriptorPool, nullptr);

    for (auto descriptorSetLayout : descriptorSetLayouts) {
        vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    }
    descriptorSetLayouts.clear();

#if defined(COMBINED_VERTEX_INDEX_BUFFER) && COMBINED_VERTEX_INDEX_BUFFER
    freeBuffer(vertexIndexBuffer, vertexIndexBufferMemory);
#else
    freeBuffer(indexBuffer, indexBufferMemory);

    freeBuffer(vertexBuffer, indexBufferMemory);
#endif

    for (auto& semaphore : imageAvailableSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
    imageAvailableSemaphores.clear();

    for (auto& semaphore : renderFinishedSemaphores) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }
    renderFinishedSemaphores.clear();

    for (auto& fence : inFlightFences) {
        vkDestroyFence(device, fence, nullptr);
    }
    inFlightFences.clear();

    for (auto& pool : commandPools) {
        vkDestroyCommandPool(device, pool.second, nullptr);
    }
    commandPools.clear();

    for (auto& pipeline : pipelines) {
        vkDestroyPipeline(device, pipeline.second, nullptr);
    }
    pipelines.clear();
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr); //TODO need more than one push constant definition

    if (!useDynamicRendering) vkDestroyRenderPass(device, renderPass, nullptr);
}

void App::cleanupVulkan() {
    vkDeviceWaitIdle(device);

    vkDestroyDevice(device, nullptr);

    if (enableValidationLayers) {
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    }
    
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    
    glfwDestroyWindow(window);
    glfwTerminate();
}