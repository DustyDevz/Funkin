// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <renderer/renderer_common.hpp>

using Microsoft::WRL::ComPtr;
namespace Funkin::Renderer::DX12 {
    class DX12Commands {
    public:
        void init(ID3D12Device* device);
        void shutdown();

        void reset(int frame);
        void close();
        void execute();
        void waitForFrame(int frame);
        void waitIdle();

        ID3D12CommandQueue* queue() const { return m_queue.Get(); }
        ID3D12GraphicsCommandList* list()  const { return m_list.Get(); }

    private:
        ComPtr<ID3D12CommandQueue>        m_queue;
        ComPtr<ID3D12CommandAllocator>    m_alloc[FRAME_COUNT];
        ComPtr<ID3D12GraphicsCommandList> m_list;
        ComPtr<ID3D12Fence>               m_fence;

        HANDLE m_fenceEvent = nullptr;
        UINT64 m_fenceValues[FRAME_COUNT] = {};
    };
}