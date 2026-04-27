#pragma once

#include <renderer/renderer.hpp>
#include "vk_instance.hpp"
#include "vk_surface.hpp"
#include "vk_device.hpp"
#include "vk_swapchain.hpp"
#include "vk_commands.hpp"
#include "vk_sync.hpp"

namespace Funkin::Renderer::VK {
    class VK_Renderer : public IRenderer {
    public:
        static VK_Renderer& get();

        void init(int width, int height, bool vsync) override;
        void beginFrame() override;
        void endFrame()   override;
        void waitIdle()   override;
        void shutdown()   override;

    private:
        VK_Renderer() = default;

        VK_Instance  m_instance;
        VK_Surface   m_surface;
        VK_Device    m_device;
        VK_Swapchain m_swapchain;
        VK_Commands  m_commands;
        VK_Sync      m_sync;

        uint32_t m_imageIndex = 0;
        uint32_t m_currentFrame = 0;
        const int MAX_FRAMES_IN_FLIGHT = 2;
    };
}