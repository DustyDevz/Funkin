// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "pch.hpp"
#include "dx12_gal.hpp"

namespace Funkin::Renderer::GAL {
    BufferHandle DX12Gal::createBuffer(const BufferDesc& desc) {
        D3D12_HEAP_TYPE       heapType  = D3D12_HEAP_TYPE_DEFAULT;
        D3D12_RESOURCE_STATES initState = D3D12_RESOURCE_STATE_COMMON;

        switch (desc.memory) {
            case MemoryHint::GPU:
                heapType  = D3D12_HEAP_TYPE_DEFAULT;
                initState = D3D12_RESOURCE_STATE_COMMON;
                break;
            case MemoryHint::CPUWrite:
                heapType  = D3D12_HEAP_TYPE_UPLOAD;
                initState = D3D12_RESOURCE_STATE_GENERIC_READ;
                break;
            case MemoryHint::CPURead:
                heapType  = D3D12_HEAP_TYPE_READBACK;
                initState = D3D12_RESOURCE_STATE_COPY_DEST;
                break;
        }

        size_t actualSize = desc.size;
        if (desc.usage & BufferUsage::Uniform)
            actualSize = (actualSize + 255) & ~static_cast<size_t>(255);

        auto heapProps = CD3DX12_HEAP_PROPERTIES(heapType);
        auto resDesc   = CD3DX12_RESOURCE_DESC::Buffer(actualSize);

        DX12GalBuffer buf = {};
        buf.size   = actualSize;
        buf.usage  = desc.usage;
        buf.memory = desc.memory;
        buf.state  = initState;

        if (FAILED(m_device->CreateCommittedResource(
                &heapProps, D3D12_HEAP_FLAG_NONE, &resDesc,
                initState, nullptr, IID_PPV_ARGS(&buf.resource))))
            throw std::runtime_error("Failed to create buffer resource");

        if (desc.memory == MemoryHint::CPUWrite) {
            D3D12_RANGE readRange = {};
            buf.resource->Map(0, &readRange, &buf.mapped);
        }

        BufferHandle h;
        h.id = m_buffers.insert(std::move(buf));
        return h;
    }

    void* DX12Gal::mapBuffer(BufferHandle handle) {
        auto* b = m_buffers.get(handle.id);
        if (!b) return nullptr;
        if (b->mapped) return b->mapped;

        void* ptr = nullptr;
        b->resource->Map(0, nullptr, &ptr);
        b->mapped = ptr;
        return ptr;
    }

    void DX12Gal::unmapBuffer(BufferHandle handle) {
        auto* b = m_buffers.get(handle.id);
        if (!b || !b->mapped) return;

        if (b->memory != MemoryHint::CPUWrite) {
            b->resource->Unmap(0, nullptr);
            b->mapped = nullptr;
        }
    }

    void DX12Gal::destroyBuffer(BufferHandle handle) {
        auto* b = m_buffers.get(handle.id);
        if (!b) return;

        if (b->mapped) {
            b->resource->Unmap(0, nullptr);
            b->mapped = nullptr;
        }

        m_buffers.remove(handle.id);
    }
}
