// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <unordered_map>
#include "renderer/gal/idal.hpp"

namespace Funkin::Renderer::GAL {
    struct DX12GalTexture {
        ComPtr<ID3D12Resource>      resource;
        D3D12_RESOURCE_STATES       state;
        DXGI_FORMAT                 format;
        uint32_t                    width, height, mips;
        TextureUsage                usage;
        D3D12_CPU_DESCRIPTOR_HANDLE srv = {};
        D3D12_CPU_DESCRIPTOR_HANDLE rtv = {};
        D3D12_CPU_DESCRIPTOR_HANDLE dsv = {};
    };

    struct DX12GalBuffer {
        ComPtr<ID3D12Resource> resource;
        D3D12_RESOURCE_STATES  state;
        size_t                 size;
        BufferUsage            usage;
        MemoryHint             memory = MemoryHint::GPU;
        void*                  mapped = nullptr;
    };

    struct DX12GalShader {
        std::vector<uint8_t> bytecode;
        ShaderStage          stage;
    };

    struct DX12GalPipeline {
        ComPtr<ID3D12PipelineState> pso;
        ComPtr<ID3D12RootSignature> rootSig;
        uint32_t                   vertexStride = 0;
        D3D12_PRIMITIVE_TOPOLOGY   topology     = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    };

    struct DX12GalSampler {
        D3D12_CPU_DESCRIPTOR_HANDLE handle = {};
    };

    template<typename T>
    struct ResourcePool {
        std::unordered_map<uint32_t, T> slots;
        uint32_t                        nextId = 1;

        uint32_t insert(T&& item) {
            uint32_t id = nextId++;
            slots.emplace(id, std::move(item));
            return id;
        }
        T*       get(uint32_t id)       { auto it = slots.find(id); return it != slots.end() ? &it->second : nullptr; }
        const T* get(uint32_t id) const { auto it = slots.find(id); return it != slots.end() ? &it->second : nullptr; }
        void     remove(uint32_t id)    { slots.erase(id); }
    };
}
