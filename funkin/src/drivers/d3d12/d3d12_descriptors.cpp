// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "d3d12_descriptors.hpp"

namespace Funkin::Renderer::GAL {
    void DX12DescriptorHeap::init(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type,
                                uint32_t capacity, bool shaderVisible)
    {
        m_capacity      = capacity;
        m_shaderVisible = shaderVisible;
        m_count         = 0;

        D3D12_DESCRIPTOR_HEAP_DESC desc = {};
        desc.Type           = type;
        desc.NumDescriptors = capacity;
        desc.Flags          = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE
                                        : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        desc.NodeMask       = 0;

        if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_heap))))
            throw std::runtime_error("Failed to create descriptor heap");

        m_cpuStart = m_heap->GetCPUDescriptorHandleForHeapStart();
        if (shaderVisible)
            m_gpuStart = m_heap->GetGPUDescriptorHandleForHeapStart();

        m_stride = device->GetDescriptorHandleIncrementSize(type);
    }

    void DX12DescriptorHeap::shutdown()
    {
        m_heap.Reset();
        m_count = 0;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::allocateCPU()
    {
        if (m_count >= m_capacity)
            throw std::runtime_error("Descriptor heap CPU capacity exceeded");
        return cpuAt(m_count++);
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::allocateGPU()
    {
        if (!m_shaderVisible)
            throw std::runtime_error("Descriptor heap is not shader-visible");
        if (m_count >= m_capacity)
            throw std::runtime_error("Descriptor heap GPU capacity exceeded");
        return gpuAt(m_count++);
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::cpuAt(uint32_t index) const
    {
        D3D12_CPU_DESCRIPTOR_HANDLE h = m_cpuStart;
        h.ptr += static_cast<SIZE_T>(index) * static_cast<SIZE_T>(m_stride);
        return h;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::gpuAt(uint32_t index) const
    {
        D3D12_GPU_DESCRIPTOR_HANDLE h = m_gpuStart;
        h.ptr += static_cast<UINT64>(index) * static_cast<UINT64>(m_stride);
        return h;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DX12DescriptorHeap::gpuFromCPU(D3D12_CPU_DESCRIPTOR_HANDLE cpu) const
    {
        uint32_t index = static_cast<uint32_t>((cpu.ptr - m_cpuStart.ptr) / m_stride);
        return gpuAt(index);
    }
}
