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
#include <optional>
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

    bool init();
    bool step();
    void cleanup();

  private:
    struct QueueFamilyIndices
    {
        std::optional<uint32_t> graphicsFamily;
        bool isComplete() { return graphicsFamily.has_value(); }
    };
    
    GLFWwindow* _window{nullptr};
    VkInstance _vkInstance;
    VkPhysicalDevice _physicalDevice{VK_NULL_HANDLE};
    VkDevice _device;
    VkQueue _graphicsQueue;

#ifndef NDEBUG
    const std::vector<const char*> _validationLayerNames{"VK_LAYER_LUNARG_standard_validation"};
    VkDebugUtilsMessengerEXT _debugMessenger;
#endif

    VkResult CreateDebugUtilsMessengerEXT(
        VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
    void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

    bool setupDebugMessenger();
    bool createInstance();
    bool checkValidationLayerSupport(const std::vector<const char*>& layerNames);
    std::vector<const char*> getRequiredExtensions();
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT /*messageSeverity*/,
        VkDebugUtilsMessageTypeFlagsEXT /*messageType*/,
        const VkDebugUtilsMessengerCallbackDataEXT* callbackData,
        void* /*userData*/);
    bool pickPhysicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    bool createLogicalDevice();
};
