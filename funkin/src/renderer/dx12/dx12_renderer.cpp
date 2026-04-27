// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "dx12_renderer.hpp"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

namespace Funkin::Renderer::DX12 {
    DX12Renderer& DX12Renderer::get() {
        static DX12Renderer s;
        return s;
    }

    void DX12Renderer::init(int w, int h, bool vsync) {
        m_device.init();
        m_commands.init(m_device.device());
        m_swapchain.init(
            m_device.device(), m_device.factory(),
            m_commands.queue(), w, h, vsync
        );
    }

    void DX12Renderer::beginFrame() {
        int frame = m_swapchain.currentIndex();
        m_commands.waitForFrame(frame);
        m_commands.reset(frame);

        auto* list = m_commands.list();
        auto* rt = m_swapchain.renderTarget(frame);

        D3D12_RESOURCE_BARRIER b{};
        b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        b.Transition.pResource = rt;
        b.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        b.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        list->ResourceBarrier(1, &b);

        auto rtv = m_swapchain.rtv(frame);
        const float clear[] = { 0.0f, 0.0f, 0.0f, 1.0f };
        list->ClearRenderTargetView(rtv, clear, 0, nullptr);
        list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    }

    void DX12Renderer::endFrame() {
        int frame = m_swapchain.currentIndex();
        auto* list = m_commands.list();
        auto* rt = m_swapchain.renderTarget(frame);

        D3D12_RESOURCE_BARRIER b{};
        b.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        b.Transition.pResource = rt;
        b.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        b.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        list->ResourceBarrier(1, &b);

        m_commands.close();
        m_commands.execute();
        m_swapchain.present();
    }

    void DX12Renderer::waitIdle() {
        m_commands.waitIdle();
    }

    void DX12Renderer::shutdown() {
        m_commands.waitIdle();
        m_swapchain.shutdown();
        m_commands.shutdown();
        m_device.shutdown();
    }
}