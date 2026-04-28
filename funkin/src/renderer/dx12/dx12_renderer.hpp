// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

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

        ID3D12Device*              device()   const { return m_device.device(); }
        ID3D12GraphicsCommandList* cmdList()  const { return m_commands.list(); }
        D3D12_VIEWPORT             viewport() const { return { 0.0f, 0.0f, (float)m_width, (float)m_height, 0.0f, 1.0f }; }
        D3D12_RECT                 scissor()  const { return { 0, 0, m_width, m_height }; }

    private:
        DX12Renderer() = default;

        DX12Device    m_device;
        DX12Swapchain m_swapchain;
        DX12Commands  m_commands;

        int m_width = 0;
        int m_height = 0;
    };
}