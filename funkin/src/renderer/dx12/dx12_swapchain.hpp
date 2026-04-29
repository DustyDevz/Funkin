// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <directx/d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <renderer/renderer_common.hpp>

using Microsoft::WRL::ComPtr;
namespace Funkin::Renderer::DX12 {
	class DX12Swapchain {
	public:
        void init(ID3D12Device* device, IDXGIFactory6* factory,
            ID3D12CommandQueue* queue, int w, int h, bool vsync);
        void shutdown();
        void present();

        int             currentIndex()         const { return m_index; }
        ID3D12Resource* renderTarget(int i)    const { return m_rts[i].Get(); }
        D3D12_CPU_DESCRIPTOR_HANDLE rtv(int i) const;

    private:
        void createRTVHeap(ID3D12Device* device);
        void createRenderTargets(ID3D12Device* device);

        ComPtr<IDXGISwapChain3>      m_swapchain;
        ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
        ComPtr<ID3D12Resource>       m_rts[FRAME_COUNT];

        UINT m_rtvDescSize = 0;
        int  m_index = 0;
        bool m_vsync = false;
	};
}