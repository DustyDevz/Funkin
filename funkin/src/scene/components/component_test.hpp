// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <directx/d3d12.h>
#include <wrl/client.h>
#include <renderer/dx12/dx12_pipeline.hpp>

using Microsoft::WRL::ComPtr;
namespace Funkin::Scene::Components {
    struct Vertex {
        float position[3];
        float color[4];
    };

    class Test {
    public:
        void init(ID3D12Device* device);
        void draw(ID3D12GraphicsCommandList* cmd,
                  const Funkin::Renderer::DX12::DX12_Pipeline& pipeline,
                  D3D12_VIEWPORT viewport,
                  D3D12_RECT scissor);

        void shutdown();

    private:
        ComPtr<ID3D12Resource>   m_vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vbView;
    };
}