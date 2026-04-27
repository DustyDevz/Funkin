#include <renderer/renderer_common.hpp>
#include "vk_sync.hpp"

namespace Funkin::Renderer::VK {
    void VK_Sync::init(VkDevice device) {
        VkSemaphoreCreateInfo si{ VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        VkFenceCreateInfo     fi{ VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
        fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        vkCheck(vkCreateSemaphore(device, &si, nullptr, &m_imageAvailable),
            "Failed to create semaphore");
        vkCheck(vkCreateSemaphore(device, &si, nullptr, &m_renderFinished),
            "Failed to create semaphore");
        vkCheck(vkCreateFence(device, &fi, nullptr, &m_inFlight),
            "Failed to create fence");
    }

    void VK_Sync::shutdown(VkDevice device) {
        vkDestroySemaphore(device, m_imageAvailable, nullptr);
        vkDestroySemaphore(device, m_renderFinished, nullptr);
        vkDestroyFence(device, m_inFlight, nullptr);
    }
}