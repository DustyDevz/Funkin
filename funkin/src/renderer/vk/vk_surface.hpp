// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

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