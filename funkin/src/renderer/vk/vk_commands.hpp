// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Funkin::Renderer::VK {
    class VK_Commands {
    public:
        void init(VkDevice device, uint32_t graphicsFamily, size_t frameCount);
        void shutdown(VkDevice device);

        void begin(uint32_t imageIndex);
        void end(uint32_t imageIndex);

        VkCommandBuffer buffer(uint32_t i) const { return m_buffers[i]; }

    private:
        VkCommandPool                m_pool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> m_buffers;
    };
}