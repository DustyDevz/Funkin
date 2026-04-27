#pragma once
#include <vulkan/vulkan.h>

namespace Funkin::Renderer::VK {
    class VK_Surface {
    public:
        void init(VkInstance instance);
        void shutdown(VkInstance instance);

        VkSurfaceKHR get() const { return m_surface; }

    private:
        VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    };
}