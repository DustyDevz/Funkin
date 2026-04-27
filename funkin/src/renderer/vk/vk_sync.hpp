#pragma once
#include <vulkan/vulkan.h>

namespace Funkin::Renderer::VK {
    class VK_Sync {
    public:
        void init(VkDevice device);
        void shutdown(VkDevice device);

        VkSemaphore imageAvailable() const { return m_imageAvailable; }
        VkSemaphore renderFinished() const { return m_renderFinished; }
        VkFence     inFlight()       const { return m_inFlight; }

    private:
        VkSemaphore m_imageAvailable = VK_NULL_HANDLE;
        VkSemaphore m_renderFinished = VK_NULL_HANDLE;
        VkFence     m_inFlight       = VK_NULL_HANDLE;
    };
}