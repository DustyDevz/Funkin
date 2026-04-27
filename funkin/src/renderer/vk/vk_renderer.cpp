#include "vk_renderer.hpp"

namespace Funkin::Renderer::VK {
    VK_Renderer& VK_Renderer::get() {
        static VK_Renderer s;
        return s;
    }

    void VK_Renderer::init(int w, int h, bool vsync) {
        m_instance.init();
        m_surface.init(m_instance.get());
        m_device.init(m_instance.get(), m_surface.get());
        m_swapchain.init(m_device.physical(), m_device.logical(),
                        m_surface.get(), w, h, vsync);
        
        uint32_t imageCount = m_swapchain.imageCount();
        
        m_commands.init(m_device.logical(),
                        m_device.families().graphics,
                        imageCount);
        m_sync.init(m_device.logical(), MAX_FRAMES_IN_FLIGHT);
    }

    void VK_Renderer::beginFrame() {
        auto dev      = m_device.logical();
        auto fence    = m_sync.inFlight(m_currentFrame);
        auto imgAvail = m_sync.imageAvailable(m_currentFrame);

        vkWaitForFences(dev, 1, &fence, VK_TRUE, UINT64_MAX);
        vkResetFences(dev, 1, &fence);

        vkAcquireNextImageKHR(dev, m_swapchain.swapchain(), UINT64_MAX,
                            imgAvail, VK_NULL_HANDLE, &m_imageIndex);

        m_commands.begin(m_imageIndex);

        VkClearValue clear{ {{ 0.0f, 0.0f, 0.0f, 1.0f }} };

        VkRenderPassBeginInfo rpi{};
        rpi.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        rpi.renderPass        = m_swapchain.renderPass();
        rpi.framebuffer       = m_swapchain.framebuffer(m_imageIndex);
        rpi.renderArea.extent = m_swapchain.extent();
        rpi.clearValueCount   = 1;
        rpi.pClearValues      = &clear;

        vkCmdBeginRenderPass(m_commands.buffer(m_imageIndex),
                             &rpi, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VK_Renderer::endFrame() {
        auto dev      = m_device.logical();
        auto cmd      = m_commands.buffer(m_imageIndex);
        auto imgAvail = m_sync.imageAvailable(m_currentFrame);
        auto renDone  = m_sync.renderFinished(m_currentFrame);
        auto fence    = m_sync.inFlight(m_currentFrame);
        auto sc       = m_swapchain.swapchain();

        vkCmdEndRenderPass(cmd);
        m_commands.end(m_imageIndex);

        VkPipelineStageFlags stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkSubmitInfo si{};
        si.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        si.waitSemaphoreCount   = 1;
        si.pWaitSemaphores      = &imgAvail;
        si.pWaitDstStageMask    = &stage;
        si.commandBufferCount   = 1;
        si.pCommandBuffers      = &cmd;
        si.signalSemaphoreCount = 1;
        si.pSignalSemaphores    = &renDone;

        vkQueueSubmit(m_device.graphics(), 1, &si, fence);

        VkPresentInfoKHR pi{};
        pi.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        pi.waitSemaphoreCount = 1;
        pi.pWaitSemaphores    = &renDone;
        pi.swapchainCount     = 1;
        pi.pSwapchains        = &sc;
        pi.pImageIndices      = &m_imageIndex;

        vkQueuePresentKHR(m_device.present(), &pi);

        m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
    }

    void VK_Renderer::waitIdle() {
        vkDeviceWaitIdle(m_device.logical());
    }

    void VK_Renderer::shutdown() {
        vkDeviceWaitIdle(m_device.logical());
        m_sync.shutdown(m_device.logical());
        m_commands.shutdown(m_device.logical());
        m_swapchain.shutdown(m_device.logical());
        m_surface.shutdown(m_instance.get());
        m_device.shutdown();
        m_instance.shutdown();
    }
}