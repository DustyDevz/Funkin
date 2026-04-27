#include <renderer/renderer_common.hpp>
#include "vk_commands.hpp"

namespace Funkin::Renderer::VK {
    void VK_Commands::init(VkDevice device, uint32_t graphicsFamily, size_t frameCount) {
        VkCommandPoolCreateInfo pi{};
        pi.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pi.queueFamilyIndex = graphicsFamily;
        pi.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        vkCheck(vkCreateCommandPool(device, &pi, nullptr, &m_pool),
            "Failed to create command pool");

        m_buffers.resize(frameCount);

        VkCommandBufferAllocateInfo ai{};
        ai.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        ai.commandPool        = m_pool;
        ai.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        ai.commandBufferCount = (uint32_t)frameCount;

        vkCheck(vkAllocateCommandBuffers(device, &ai, m_buffers.data()),
            "Failed to allocate command buffers");
    }

    void VK_Commands::begin(uint32_t imageIndex) {
        vkResetCommandBuffer(m_buffers[imageIndex], 0);

        VkCommandBufferBeginInfo bi{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
        vkCheck(vkBeginCommandBuffer(m_buffers[imageIndex], &bi),
            "Failed to begin command buffer");
    }

    void VK_Commands::end(uint32_t imageIndex) {
        vkCheck(vkEndCommandBuffer(m_buffers[imageIndex]),
            "Failed to end command buffer");
    }

    void VK_Commands::shutdown(VkDevice device) {
        vkDestroyCommandPool(device, m_pool, nullptr);
    }
}