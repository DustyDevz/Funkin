#pragma once
#ifdef _WIN32
    #define VK_USE_PLATFORM_WIN32_KHR
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif

#include <vulkan/vulkan.h>

namespace Funkin::Renderer::VK {
    class VK_Instance {
    public:
        void init();
        void shutdown();

        VkInstance get() const { return m_instance; }

    private:
        VkInstance m_instance = VK_NULL_HANDLE;
    };
}