// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <d3d12.h>
#include <wrl/client.h>
#include <cstdint>
#include <cstddef>

using Microsoft::WRL::ComPtr;
namespace Funkin::Renderer::DX12 {
    struct ShaderBlob {
        const uint8_t* data = nullptr;
        size_t         size = 0;

        D3D12_SHADER_BYTECODE d3d12() const { return { data, size }; }
    };

    class DX12_Pipeline {
    public:
        void init(ID3D12Device* device, DXGI_FORMAT rtvFormat,
                       ShaderBlob vs, ShaderBlob ps);
        void shutdown();

        ID3D12PipelineState* pso()           const { return m_pso.Get(); }
        ID3D12RootSignature* rootSignature() const { return m_rootSig.Get(); }
        
    private:
        void createRootSignature(ID3D12Device* device);
        void createPSO(ID3D12Device* device, DXGI_FORMAT rtvFormat,
                       ShaderBlob vs, ShaderBlob ps);

        ComPtr<ID3D12RootSignature> m_rootSig;
        ComPtr<ID3D12PipelineState> m_pso;
    };
}