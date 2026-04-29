// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "dx12_pipeline.hpp"
#include <d3d12.h>
#include <d3dx12.h>
#include <stdexcept>

namespace Funkin::Renderer::DX12 {
    void DX12_Pipeline::init(ID3D12Device* device, DXGI_FORMAT rtvFormat,
                            ShaderBlob vs, ShaderBlob ps) {
        createRootSignature(device);
        createPSO(device, rtvFormat, vs, ps);
    }

    void DX12_Pipeline::createRootSignature(ID3D12Device* device) {
        D3D12_ROOT_SIGNATURE_DESC desc{};
        desc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

        ComPtr<ID3DBlob> blob, error;
        if (FAILED(D3D12SerializeRootSignature(
                &desc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error)))
            throw std::runtime_error("Failed to serialize root signature");

        if (FAILED(device->CreateRootSignature(
                0, blob->GetBufferPointer(), blob->GetBufferSize(),
                IID_PPV_ARGS(&m_rootSig))))
            throw std::runtime_error("Failed to create root signature");
    }

    void DX12_Pipeline::createPSO(ID3D12Device* device, DXGI_FORMAT rtvFormat,
                                  ShaderBlob vs, ShaderBlob ps) {
        D3D12_INPUT_ELEMENT_DESC inputLayout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0,  0,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12,
            D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        };

        D3D12_GRAPHICS_PIPELINE_STATE_DESC desc{};
        desc.pRootSignature        = m_rootSig.Get();
        desc.VS                    = vs.d3d12();
        desc.PS                    = ps.d3d12();
        desc.InputLayout           = { inputLayout, 2 };
        desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        desc.NumRenderTargets      = 1;
        desc.RTVFormats[0]         = rtvFormat;
        desc.SampleDesc            = { 1, 0 };
        desc.SampleMask            = UINT_MAX;
        desc.RasterizerState       = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
        desc.BlendState            = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
        desc.DepthStencilState.DepthEnable   = FALSE;
        desc.DepthStencilState.StencilEnable = FALSE;

        if (FAILED(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_pso))))
            throw std::runtime_error("Failed to create PSO");
    }

    void DX12_Pipeline::shutdown() {
        m_pso.Reset();
        m_rootSig.Reset();
    }
}