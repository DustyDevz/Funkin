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

    D3D12_RESOURCE_BARRIER DX12Renderer::transitionBarrier(
        ID3D12Resource*       resource,
        D3D12_RESOURCE_STATES before,
        D3D12_RESOURCE_STATES after)
    {
        D3D12_RESOURCE_BARRIER b{};
        b.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        b.Transition.pResource   = resource;
        b.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
        b.Transition.StateBefore = before;
        b.Transition.StateAfter  = after;
        return b;
    }

    void DX12Renderer::init(int w, int h, bool vsync) {
        m_width  = w;
        m_height = h;

        m_device.init();
        m_commands.init(m_device.device());
        m_swapchain.init(m_device.device(), m_device.factory(), m_commands.queue(), w, h, vsync);
    }

    void DX12Renderer::resize(int w, int h) {
        if (w == m_width && h == m_height) return;
        m_commands.waitIdle();
        m_swapchain.resize(m_device.device(), w, h);
        m_width  = w;
        m_height = h;

        int   frame = m_swapchain.currentIndex();
        auto* list  = m_commands.list();
        auto* rt    = m_swapchain.renderTarget(frame);
        auto  rtv   = m_swapchain.rtv(frame);

        m_commands.reset(frame);

        auto toRT = transitionBarrier(rt, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        list->ResourceBarrier(1, &toRT);

        const float clear[] = { 0.1f, 0.2f, 0.3f, 1.0f };
        list->ClearRenderTargetView(rtv, clear, 0, nullptr);

        auto toPresent = transitionBarrier(rt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        list->ResourceBarrier(1, &toPresent);

        m_commands.close();
        m_commands.execute();
        m_commands.waitIdle();
        m_swapchain.present();
    }

    void DX12Renderer::beginFrame() {
        int   frame = m_swapchain.currentIndex();
        auto* list  = m_commands.list();
        auto* rt    = m_swapchain.renderTarget(frame);
        auto  rtv   = m_swapchain.rtv(frame);
        auto  vp    = viewport();
        auto  sc    = scissor();

        m_commands.waitForFrame(frame);
        m_commands.reset(frame);

        list->RSSetViewports(1, &vp);
        list->RSSetScissorRects(1, &sc);

        auto b = transitionBarrier(rt, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
        list->ResourceBarrier(1, &b);

        const float clear[] = { 0.1f, 0.2f, 0.3f, 1.0f };
        list->ClearRenderTargetView(rtv, clear, 0, nullptr);
        list->OMSetRenderTargets(1, &rtv, FALSE, nullptr);
    }

    void DX12Renderer::endFrame() {
        int   frame = m_swapchain.currentIndex();
        auto* list  = m_commands.list();
        auto* rt    = m_swapchain.renderTarget(frame);

        auto b = transitionBarrier(rt, D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        list->ResourceBarrier(1, &b);

        m_commands.close();
        m_commands.execute();
        m_commands.signal(frame);
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