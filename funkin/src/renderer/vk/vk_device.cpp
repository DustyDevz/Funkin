#include <renderer/renderer_common.hpp>
#include "vk_device.hpp"
#include <vector>

namespace Funkin::Renderer::VK {
    void VK_Device::init(VkInstance instance, VkSurfaceKHR surface) {
        pickPhysicalDevice(instance, surface);
        createLogicalDevice();
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

        for (uint32_t i = 0; i < qcount; ++i) {
            if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                m_families.graphics = i;

            VkBool32 present = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_physical, i, surface, &present);
            if (present) m_families.present = i;
        }
    }

    void VK_Device::createLogicalDevice() {
        float priority = 1.0f;

        VkDeviceQueueCreateInfo qi{};
        qi.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qi.queueFamilyIndex = m_families.graphics;
        qi.queueCount       = 1;
        qi.pQueuePriorities = &priority;

        const char* extensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

        VkDeviceCreateInfo ci{};
        ci.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        ci.queueCreateInfoCount    = 1;
        ci.pQueueCreateInfos       = &qi;
        ci.enabledExtensionCount   = 1;
        ci.ppEnabledExtensionNames = extensions;

        vkCheck(vkCreateDevice(m_physical, &ci, nullptr, &m_device),
            "Failed to create logical device");

        vkGetDeviceQueue(m_device, m_families.graphics, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, m_families.present,  0, &m_presentQueue);
    }

    void VK_Device::shutdown() {
        if (m_device != VK_NULL_HANDLE) {
            vkDestroyDevice(m_device, nullptr);
            m_device = VK_NULL_HANDLE;
        }
    }

}