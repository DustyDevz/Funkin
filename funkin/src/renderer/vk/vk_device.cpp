// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include <renderer/renderer_common.hpp>
#include "vk_device.hpp"
#include <vector>
#include <set>

namespace Funkin::Renderer::VK {
    void VK_Device::init(VkInstance instance, VkSurfaceKHR surface) {
        pickPhysicalDevice(instance, surface);
        createLogicalDevice(surface);
    }

    void VK_Device::pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
        uint32_t count = 0;
        vkEnumeratePhysicalDevices(instance, &count, nullptr);
        if (count == 0)
            throw std::runtime_error("No Vulkan capable GPU found");

        std::vector<VkPhysicalDevice> devices(count);
        vkEnumeratePhysicalDevices(instance, &count, devices.data());

        for (auto& d : devices) {
            VkPhysicalDeviceProperties props;
            vkGetPhysicalDeviceProperties(d, &props);
            if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
                m_physical = d;
                break;
            }
        }
        if (m_physical == VK_NULL_HANDLE)
            m_physical = devices[0];

        uint32_t qcount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical, &qcount, nullptr);
        std::vector<VkQueueFamilyProperties> queues(qcount);
        vkGetPhysicalDeviceQueueFamilyProperties(m_physical, &qcount, queues.data());

        bool foundGraphics = false;
        bool foundPresent = false;

        for (uint32_t i = 0; i < qcount; ++i) {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                m_families.graphics = i;
                foundGraphics = true;
            }

            VkBool32 present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physical, i, surface, &present);
            if (present) {
                m_families.present = i;
                foundPresent = true;
            }

            if (foundGraphics && foundPresent) break;
        }
    }

    void VK_Device::createLogicalDevice(VkSurfaceKHR surface) {
        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = { m_families.graphics, m_families.present };

        float priority = 1.0f;
        for (uint32_t family : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo qi{};
            qi.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            qi.queueFamilyIndex = family;
            qi.queueCount = 1;
            qi.pQueuePriorities = &priority;
            queueCreateInfos.push_back(qi);
        }

        const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo ci{};
        ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.queueCreateInfoCount = (uint32_t)queueCreateInfos.size();
        ci.pQueueCreateInfos = queueCreateInfos.data();
        ci.enabledExtensionCount = 1;
        ci.ppEnabledExtensionNames = extensions;

        vkCheck(vkCreateDevice(m_physical, &ci, nullptr, &m_device),
            "Failed to create logical device");

        vkGetDeviceQueue(m_device, m_families.graphics, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_families.present, 0, &m_presentQueue);
    }

    void VK_Device::shutdown() {
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;
        }
    }
}