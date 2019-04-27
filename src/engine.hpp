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
#include <cstring>
#include <string>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

/*************/
class Engine
{
  public:
    Engine() = default;
    ~Engine() = default;

    bool init()
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

        return true;
    }

    bool step()
    {
        if (glfwWindowShouldClose(_window))
            return false;
        glfwPollEvents();
        return true;
    }

    void cleanup()
    {
#ifndef NDEBUG
        DestroyDebugUtilsMessengerEXT(_vkInstance, _debugMessenger, nullptr);
#endif

        vkDestroyInstance(_vkInstance, nullptr);
        glfwDestroyWindow(_window);
        glfwTerminate();
    }

  private:
    GLFWwindow* _window{nullptr};
    VkInstance _vkInstance;
#ifndef NDEBUG
    const std::vector<const char*> _validationLayerNames{"VK_LAYER_LUNARG_standard_validation"};
    VkDebugUtilsMessengerEXT _debugMessenger;
#endif

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        else
            return VK_ERROR_EXTENSION_NOT_PRESENT;
    }

    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator)
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    bool setupDebugMessenger()
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

    bool createInstance()
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
            .apiVersion = VK_API_VERSION_1_0};

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

    bool checkValidationLayerSupport(const std::vector<const char*>& layerNames)
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

    std::vector<const char*> getRequiredExtensions()
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

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* /*userData*/)
    {
        printf("Validation layer: %s\n", callbackData->pMessage);
        return VK_FALSE;
    }
};
