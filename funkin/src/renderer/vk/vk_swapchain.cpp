#include <renderer/renderer_common.hpp>
#include "vk_swapchain.hpp"

namespace Funkin::Renderer::VK {
    void VK_Swapchain::init(VkPhysicalDevice physical, VkDevice device,
                            VkSurfaceKHR surface, int w, int h, bool vsync) {
        createSwapchain(physical, device, surface, w, h, vsync);
        createImageViews(device);
        createRenderPass(device);
        createFramebuffers(device);
    }

    void VK_Swapchain::createSwapchain(VkPhysicalDevice physical, VkDevice device,
                                        VkSurfaceKHR surface, int w, int h, bool vsync) {
        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical, surface, &caps);

        m_format = VK_FORMAT_B8G8R8A8_UNORM;
        m_extent = { (uint32_t)w, (uint32_t)h };

        VkSwapchainCreateInfoKHR ci{};
        ci.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        ci.surface          = surface;
        ci.minImageCount    = 2;
        ci.imageFormat      = m_format;
        ci.imageColorSpace  = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        ci.imageExtent      = m_extent;
        ci.imageArrayLayers = 1;
        ci.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        ci.preTransform     = caps.currentTransform;
        ci.compositeAlpha   = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        ci.presentMode      = vsync ? VK_PRESENT_MODE_FIFO_KHR
                                    : VK_PRESENT_MODE_MAILBOX_KHR;
        ci.clipped          = VK_TRUE;

        vkCheck(vkCreateSwapchainKHR(device, &ci, nullptr, &m_swapchain),
                "Failed to create swapchain");

        uint32_t count = 0;
        vkGetSwapchainImagesKHR(device, m_swapchain, &count, nullptr);
        m_images.resize(count);
        vkGetSwapchainImagesKHR(device, m_swapchain, &count, m_images.data());
    }

    void VK_Swapchain::createImageViews(VkDevice device) {
        m_imageViews.resize(m_images.size());
        for (size_t i = 0; i < m_images.size(); ++i) {
            VkImageViewCreateInfo ci{};
            ci.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            ci.image                           = m_images[i];
            ci.viewType                        = VK_IMAGE_VIEW_TYPE_2D;
            ci.format                          = m_format;
            ci.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            ci.subresourceRange.levelCount     = 1;
            ci.subresourceRange.layerCount     = 1;

            vkCheck(vkCreateImageView(device, &ci, nullptr, &m_imageViews[i]),
                "Failed to create image view");
        }
    }

    void VK_Swapchain::createRenderPass(VkDevice device) {
        VkAttachmentDescription att{};
        att.format        = m_format;
        att.samples       = VK_SAMPLE_COUNT_1_BIT;
        att.loadOp        = VK_ATTACHMENT_LOAD_OP_CLEAR;
        att.storeOp       = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        att.finalLayout   = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference ref{};
        ref.attachment = 0;
        ref.layout     = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription sub{};
        sub.pipelineBindPoint    = VK_PIPELINE_BIND_POINT_GRAPHICS;
        sub.colorAttachmentCount = 1;
        sub.pColorAttachments    = &ref;

        VkRenderPassCreateInfo ci{};
        ci.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        ci.attachmentCount = 1;
        ci.pAttachments    = &att;
        ci.subpassCount    = 1;
        ci.pSubpasses      = &sub;

        vkCheck(vkCreateRenderPass(device, &ci, nullptr, &m_renderPass),
            "Failed to create render pass");
    }

    void VK_Swapchain::createFramebuffers(VkDevice device) {
        m_framebuffers.resize(m_imageViews.size());
        for (size_t i = 0; i < m_imageViews.size(); ++i) {
            VkFramebufferCreateInfo ci{};
            ci.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            ci.renderPass      = m_renderPass;
            ci.attachmentCount = 1;
            ci.pAttachments    = &m_imageViews[i];
            ci.width           = m_extent.width;
            ci.height          = m_extent.height;
            ci.layers          = 1;

            vkCheck(vkCreateFramebuffer(device, &ci, nullptr, &m_framebuffers[i]),
                "Failed to create framebuffer");
        }
    }

    void VK_Swapchain::shutdown(VkDevice device) {
        for (auto fb : m_framebuffers)   vkDestroyFramebuffer(device, fb, nullptr);
        for (auto iv : m_imageViews)     vkDestroyImageView(device, iv, nullptr);
        vkDestroyRenderPass(device, m_renderPass, nullptr);
        vkDestroySwapchainKHR(device, m_swapchain, nullptr);
    }
}