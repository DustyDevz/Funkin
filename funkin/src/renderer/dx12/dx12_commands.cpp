// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "dx12_commands.hpp"

namespace Funkin::Renderer::DX12 {
    void DX12Commands::init(ID3D12Device* device) {
        D3D12_COMMAND_QUEUE_DESC qd{};
        qd.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
        qd.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;

        if (FAILED(device->CreateCommandQueue(&qd, IID_PPV_ARGS(&m_queue))))
            throw std::runtime_error("Failed to create command queue");

        for (int i = 0; i < FRAME_COUNT; ++i) {
            if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_alloc[i]))))
                throw std::runtime_error("Failed to create command allocator");
        }

        if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_alloc[0].Get(), nullptr, IID_PPV_ARGS(&m_list))))
            throw std::runtime_error("Failed to create command list");
        m_list->Close();

        if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence))))
            throw std::runtime_error("Failed to create fence");

        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (!m_fenceEvent)
            throw std::runtime_error("Failed to create fence event");
    }

    void DX12Commands::reset(int frame) {
        m_alloc[frame]->Reset();
        m_list->Reset(m_alloc[frame].Get(), nullptr);
    }

    void DX12Commands::close() {
        m_list->Close();
    }

    void DX12Commands::execute() {
        ID3D12CommandList* lists[] = { m_list.Get() };
        m_queue->ExecuteCommandLists(1, lists);
    }

    void DX12Commands::signal(int frame) {
        m_frameFenceValues[frame] = ++m_fenceValue;
        m_queue->Signal(m_fence.Get(), m_fenceValue);
    }

    void DX12Commands::waitForFrame(int frame) {
        if (m_fence->GetCompletedValue() < m_frameFenceValues[frame]) {
            m_fence->SetEventOnCompletion(m_frameFenceValues[frame], m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    void DX12Commands::waitIdle() {
        const UINT64 val = ++m_fenceValue;
        m_queue->Signal(m_fence.Get(), val);
        if (m_fence->GetCompletedValue() < val) {
            m_fence->SetEventOnCompletion(val, m_fenceEvent);
            WaitForSingleObject(m_fenceEvent, INFINITE);
        }
    }

    void DX12Commands::shutdown() {
        if (m_fenceEvent) CloseHandle(m_fenceEvent);
        m_fence.Reset();
        m_list.Reset();
        for (auto& a : m_alloc) a.Reset();
        m_queue.Reset();
    }
}