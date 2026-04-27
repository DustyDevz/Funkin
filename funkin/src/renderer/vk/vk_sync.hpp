// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Funkin::Renderer::VK {
    class VK_Sync {
    public:
        void init(VkDevice device, int maxFramesInFlight);
        void shutdown(VkDevice device);

        VkSemaphore imageAvailable(uint32_t i) const { return m_imageAvailable[i]; }
        VkSemaphore renderFinished(uint32_t i) const { return m_renderFinished[i]; }
        VkFence     inFlight(uint32_t i)       const { return m_inFlight[i]; }

    private:
        std::vector<VkSemaphore> m_imageAvailable;
        std::vector<VkSemaphore> m_renderFinished;
        std::vector<VkFence>     m_inFlight;
    };
}