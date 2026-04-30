// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

namespace Funkin::Renderer::GAL {

class DX12DescriptorHeap {
public:
    void init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type,
              uint32_t capacity, bool shaderVisible);
    void shutdown();

    D3D12_CPU_DESCRIPTOR_HANDLE allocateCPU();
    D3D12_GPU_DESCRIPTOR_HANDLE allocateGPU();

    D3D12_CPU_DESCRIPTOR_HANDLE cpuAt(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuAt(uint32_t index) const;
    D3D12_GPU_DESCRIPTOR_HANDLE gpuFromCPU(D3D12_CPU_DESCRIPTOR_HANDLE cpu) const;

    ID3D12DescriptorHeap* heap()           const { return m_heap.Get(); }
    uint32_t              descriptorSize() const { return m_stride; }
    uint32_t              count()          const { return m_count; }

private:
    ComPtr<ID3D12DescriptorHeap> m_heap;
    D3D12_CPU_DESCRIPTOR_HANDLE  m_cpuStart      = {};
    D3D12_GPU_DESCRIPTOR_HANDLE  m_gpuStart      = {};
    uint32_t                     m_stride        = 0;
    uint32_t                     m_count         = 0;
    uint32_t                     m_capacity      = 0;
    bool                         m_shaderVisible = false;
};

}
