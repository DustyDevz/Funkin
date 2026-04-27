#pragma once
#include <vulkan/vulkan.h>
#include <vector>

namespace Funkin::Renderer::VK {
    class VK_Swapchain {
    public:
        void init(VkPhysicalDevice physical, VkDevice device,
            VkSurfaceKHR surface, int w, int h, bool vsync);
        void shutdown(VkDevice device);

        uint32_t                   imageCount()  const { return (uint32_t)m_images.size(); }
        VkSwapchainKHR             swapchain()   const { return m_swapchain; }
        VkRenderPass               renderPass()  const { return m_renderPass; }
        VkExtent2D                 extent()      const { return m_extent; }
        VkFramebuffer              framebuffer(uint32_t i) const { return m_framebuffers[i]; }

    private:
        void createSwapchain(VkPhysicalDevice physical, VkDevice device,
            VkSurfaceKHR surface, int w, int h, bool vsync);
        void createImageViews(VkDevice device);
        void createRenderPass(VkDevice device);
        void createFramebuffers(VkDevice device);

        VkSwapchainKHR           m_swapchain = VK_NULL_HANDLE;
        VkRenderPass             m_renderPass = VK_NULL_HANDLE;
        VkFormat                 m_format{};
        VkExtent2D               m_extent{};
        std::vector<VkImage>         m_images;
        std::vector<VkImageView>     m_imageViews;
        std::vector<VkFramebuffer>   m_framebuffers;
    };
}