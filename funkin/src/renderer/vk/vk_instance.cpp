// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include <renderer/renderer_common.hpp>
#include "vk_instance.hpp"

#ifdef _WIN32
    #include <vulkan/vulkan_win32.h>
#elif __linux__
    #include <vulkan/vulkan_xlib.h>
#endif

namespace Funkin::Renderer::VK {
    void VK_Instance::init() {
        VkApplicationInfo app{};
        app.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app.pApplicationName   = "Funkin";
        app.applicationVersion = VK_MAKE_VERSION(0, 1, 0);
        app.pEngineName        = "Funkin Engine";
        app.engineVersion      = VK_MAKE_VERSION(0, 1, 0);
        app.apiVersion         = VK_API_VERSION_1_2;

        const char* extensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            #ifdef _WIN32
                    VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
            #elif __linux__
                    VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
            #endif
        };

        VkInstanceCreateInfo ci{};
        ci.sType                   = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        ci.pApplicationInfo        = &app;
        ci.enabledExtensionCount   = 2;
        ci.ppEnabledExtensionNames = extensions;

        vkCheck(vkCreateInstance(&ci, nullptr, &m_instance),
            "Failed to create Vulkan instance");
    }

    void VK_Instance::shutdown() {
        if (m_instance != VK_NULL_HANDLE) {
            vkDestroyInstance(m_instance, nullptr);
            m_instance = VK_NULL_HANDLE;
        }
    }
}