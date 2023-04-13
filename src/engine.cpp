//
// Copyright (C) Emmanuel Durand
//
// This file is part of v4k.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Splash is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Splash.  If not, see <http://www.gnu.org/licenses/>.
//

#include <algorithm>
#include <set>

#include "./engine.hpp"

/*************/
bool Engine::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    _window = glfwCreateWindow(800, 600, "v4k", nullptr, nullptr);

    if (!createInstance())
        return false;
#ifndef NDEBUG
    if (!setupDebugMessenger())
        return false;
#endif
    if (!createSurface())
        return false;
    if (!pickPhysicalDevice())
        return false;
    if (!createLogicalDevice())
        return false;

    return true;
}

/*************/
bool Engine::createSwapChain()
{
    auto swapChainSupport = querySwapChainSupport(_physicalDevice);
    auto surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    auto presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    auto extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = _surface,
        .minImageCount = imageCount,
        .imageFormat = surfaceFormat.format,
        .imageColorSpace = surfaceFormat.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
    };

    auto indices = findQueueFamilies(_physicalDevice);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};
    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain) != VK_SUCCESS)
        return false;

    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, nullptr);
    _swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(_device, _swapChain, &imageCount, _swapchainImages.data());
    _swapchainImageFormat = surfaceFormat.format;
    _swapchainExtent = extent;

    return true;
}

/*************/
bool Engine::step()
{
    if (glfwWindowShouldClose(_window))
        return false;
    glfwPollEvents();
    return true;
}

/*************/
void Engine::cleanup()
{
    vkDestroySwapchainKHR(_device, _swapChain, nullptr);
    vkDestroyDevice(_device, nullptr);
#ifndef NDEBUG
    DestroyDebugUtilsMessengerEXT(_vkInstance, _debugMessenger, nullptr);
#endif
    vkDestroySurfaceKHR(_vkInstance, _surface, nullptr);
    vkDestroyInstance(_vkInstance, nullptr);
    glfwDestroyWindow(_window);
    glfwTerminate();
}

/*************/
VkResult Engine::CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    else
        return VK_ERROR_EXTENSION_NOT_PRESENT;
}

/*************/
void Engine::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

/*************/
bool Engine::setupDebugMessenger()
{
#ifndef NDEBUG
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debugCallback,
        .pUserData = nullptr};

    if (CreateDebugUtilsMessengerEXT(_vkInstance, &createInfo, nullptr, &_debugMessenger) != VK_SUCCESS)
        return false;
#endif
    return true;
}

/*************/
bool Engine::createInstance()
{
#ifndef NDEBUG
    if (!checkValidationLayerSupport(_validationLayerNames))
    {
        printf("Validation layer requested, but not available\n");
        return false;
    }
#endif

    VkApplicationInfo appInfo = {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "v4k",
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = "Engine",
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_1};

    VkInstanceCreateInfo createInfo;
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    auto extensions = getRequiredExtensions();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
#ifndef NDEBUG
    createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayerNames.size());
    createInfo.ppEnabledLayerNames = _validationLayerNames.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateInstance(&createInfo, nullptr, &_vkInstance) != VK_SUCCESS)
        return false;

    return true;
}

/*************/
bool Engine::checkValidationLayerSupport(const std::vector<const char*>& layerNames)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(static_cast<size_t>(layerCount));
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto& layerName : layerNames)
    {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
            return false;
    }

    return true;
}

/*************/
std::vector<const char*> Engine::getRequiredExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
#ifndef NDEBUG
    extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    return extensions;
}

/*************/
VKAPI_ATTR VkBool32 VKAPI_CALL Engine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
    VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
    const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
    void* /*userData*/)
{
    printf("Validation layer: %s\n", callbackData->pMessage);
    return VK_FALSE;
}

/*************/
bool Engine::pickPhysicalDevice()
{
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, nullptr);
    if (deviceCount == 0)
        return false;
    std::vector<VkPhysicalDevice> devices(static_cast<size_t>(deviceCount));
    vkEnumeratePhysicalDevices(_vkInstance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device))
        {
            _physicalDevice = device;
            break;
        }
    }

    if (_physicalDevice == VK_NULL_HANDLE)
        return false;

    return true;
}

/*************/
bool Engine::isDeviceSuitable(VkPhysicalDevice device)
{
    QueueFamilyIndices indices = findQueueFamilies(device);
    auto extensionsSupported = checkDeviceExtensionSupport(device);
    
    bool swapChainAdequate = false;
    if (extensionsSupported)
    {
        auto swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

/*************/
bool Engine::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
    uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(static_cast<size_t>(extensionCount));
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(_deviceExtensions.begin(), _deviceExtensions.end());
    for (const auto& extension : availableExtensions)
        requiredExtensions.erase(extension.extensionName);

    return requiredExtensions.empty();
}

/*************/
Engine::QueueFamilyIndices Engine::findQueueFamilies(VkPhysicalDevice device)
{
    Engine::QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(static_cast<size_t>(queueFamilyCount));
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    int i = 0;
    for (const auto& queueFamily : queueFamilies)
    {
        if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            indices.graphicsFamily = i;

        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _surface, &presentSupport);
        if (queueFamily.queueCount > 0 && presentSupport)
            indices.presentFamily = i;

        if (indices.isComplete())
            break;

        ++i;
    }

    return indices;
}

/*************/
bool Engine::createLogicalDevice()
{
    QueueFamilyIndices indices = findQueueFamilies(_physicalDevice);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.f;
    for (const auto& queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
            .queueFamilyIndex = queueFamily,
            .queueCount = 1,
            .pQueuePriorities = &queuePriority
        };
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures = {};
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(_deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = _deviceExtensions.data();
#ifndef NDEBUG
    createInfo.enabledLayerCount = static_cast<uint32_t>(_validationLayerNames.size());
    createInfo.ppEnabledLayerNames = _validationLayerNames.data();
#else
    createInfo.enabledLayerCount = 0;
#endif

    if (vkCreateDevice(_physicalDevice, &createInfo, nullptr, &_device) != VK_SUCCESS)
        return false;

    vkGetDeviceQueue(_device, indices.graphicsFamily.value(), 0, &_graphicsQueue);
    vkGetDeviceQueue(_device, indices.presentFamily.value(), 0, &_presentQueue);
    return true;
}

/*************/
bool Engine::createSurface()
{
    if (glfwCreateWindowSurface(_vkInstance, _window, nullptr, &_surface) != VK_SUCCESS)
        return false;
    return true;
}

/*************/
Engine::SwapChainSupportDetails Engine::querySwapChainSupport(VkPhysicalDevice device)
{
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);
    if (formatCount != 0)
    {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);
    if (presentModeCount)
    {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
    }

    return details;
}

/*************/
VkSurfaceFormatKHR Engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
    if (availableFormats.size() == 1 && availableFormats[0].format == VK_FORMAT_UNDEFINED)
        return {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};

    for (const auto& availableFormat : availableFormats)
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            return availableFormat;

    return availableFormats[0];
}

/*************/
VkPresentModeKHR Engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
    VkPresentModeKHR bestMode = VK_PRESENT_MODE_FIFO_KHR;

    for (const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            return availablePresentMode;
        else if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
            bestMode = availablePresentMode;

    return bestMode;
}

/*************/
VkExtent2D Engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        return capabilities.currentExtent;
    }
    else
    {
        VkExtent2D actuelExtent = {WIDTH, HEIGHT};
        actuelExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actuelExtent.width));
        actuelExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actuelExtent.height));
        return actuelExtent;
    }
}
