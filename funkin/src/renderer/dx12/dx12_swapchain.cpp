// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "dx12_swapchain.hpp"
#include <platform/window/window_win32.hpp>
#include <stdexcept>

namespace Funkin::Renderer::DX12 {
    void DX12Swapchain::init(ID3D12Device* device, IDXGIFactory6* factory,
        ID3D12CommandQueue* queue, int w, int h, bool vsync) {
        m_vsync = vsync;

        DXGI_SWAP_CHAIN_DESC1 desc{};
        desc.BufferCount = FRAME_COUNT;
        desc.Width = w;
        desc.Height = h;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        desc.SampleDesc = { 1, 0 };

        ComPtr<IDXGISwapChain1> sc1;
        if (FAILED(factory->CreateSwapChainForHwnd(
            queue, Funkin::Platform::Window_Win32::get().hwnd(),
            &desc, nullptr, nullptr, &sc1)))
            throw std::runtime_error("Failed to create swapchain");

        factory->MakeWindowAssociation(Funkin::Platform::Window_Win32::get().hwnd(), DXGI_MWA_NO_ALT_ENTER);
        sc1.As(&m_swapchain);
        m_index = m_swapchain->GetCurrentBackBufferIndex();

        createRTVHeap(device);
        createRenderTargets(device);
    }

    void DX12Swapchain::createRTVHeap(ID3D12Device* device) {
        D3D12_DESCRIPTOR_HEAP_DESC desc{};
        desc.NumDescriptors = FRAME_COUNT;
        desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_rtvHeap))))
            throw std::runtime_error("Failed to create RTV heap");

        m_rtvDescSize = device->GetDescriptorHandleIncrementSize(
            D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    }

    void DX12Swapchain::createRenderTargets(ID3D12Device* device) {
        D3D12_CPU_DESCRIPTOR_HANDLE handle =
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart();

        for (int i = 0; i < FRAME_COUNT; ++i) {
            if (FAILED(m_swapchain->GetBuffer(i, IID_PPV_ARGS(&m_rts[i]))))
                throw std::runtime_error("Failed to get swapchain buffer");

            device->CreateRenderTargetView(m_rts[i].Get(), nullptr, handle);
            handle.ptr += m_rtvDescSize;
        }
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12Swapchain::rtv(int i) const {
        D3D12_CPU_DESCRIPTOR_HANDLE h =
            m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
        h.ptr += (SIZE_T)i * m_rtvDescSize;
        return h;
    }

    void DX12Swapchain::present() {
        m_swapchain->Present(m_vsync ? 1 : 0, 0);
        m_index = m_swapchain->GetCurrentBackBufferIndex();
    }

    void DX12Swapchain::shutdown() {
        for (auto& rt : m_rts) rt.Reset();
        m_rtvHeap.Reset();
        m_swapchain.Reset();
    }
}