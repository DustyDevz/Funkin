#pragma once
#include <renderer/renderer.hpp>
#include "dx12_device.hpp"
#include "dx12_swapchain.hpp"
#include "dx12_commands.hpp"

namespace Funkin::Renderer::DX12 {
    class DX12Renderer : public IRenderer {
    public:
        static DX12Renderer& get();

        void init(int width, int height, bool vsync) override;
        void beginFrame() override;
        void endFrame()   override;
        void waitIdle()   override;
        void shutdown()   override;

    private:
        DX12Renderer() = default;

        DX12Device    m_device;
        DX12Swapchain m_swapchain;
        DX12Commands  m_commands;
    };
}