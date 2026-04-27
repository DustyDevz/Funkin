#include <renderer/renderer_common.hpp>
#include "vk_sync.hpp"

namespace Funkin::Renderer::VK {
    void VK_Sync::init(VkDevice device, int maxFramesInFlight) {
        m_imageAvailable.resize(maxFramesInFlight);
        m_renderFinished.resize(maxFramesInFlight);
        m_inFlight.resize(maxFramesInFlight);

        VkSemaphoreCreateInfo si{};
        si.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fi{};
        fi.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fi.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (int i = 0; i < maxFramesInFlight; i++) {
            vkCheck(vkCreateSemaphore(device, &si, nullptr, &m_imageAvailable[i]),
                "Failed to create imageAvailable semaphore");
            vkCheck(vkCreateSemaphore(device, &si, nullptr, &m_renderFinished[i]),
                "Failed to create renderFinished semaphore");
            vkCheck(vkCreateFence(device, &fi, nullptr, &m_inFlight[i]),
                "Failed to create inFlight fence");
        }
    }

    void VK_Sync::shutdown(VkDevice device) {
        for (auto s : m_imageAvailable) vkDestroySemaphore(device, s, nullptr);
        for (auto s : m_renderFinished) vkDestroySemaphore(device, s, nullptr);
        for (auto f : m_inFlight)       vkDestroyFence(device, f, nullptr);
    }
}