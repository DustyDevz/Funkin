// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <renderer/dx12/dx12_pipeline.hpp>

namespace Funkin::Scene::Components {
    struct Vertex {
        float position[3];
        float color[4];
    };

    class Test {
    public:
        void init(ID3D12Device* device);
        void draw(ID3D12GraphicsCommandList* cmd,
                  const Funkin::Renderer::DX12::DX12Pipeline& pipeline,
                  D3D12_VIEWPORT viewport,
                  D3D12_RECT scissor);
        void shutdown();

    private:
        ComPtr<ID3D12Resource>   m_vertexBuffer;
        D3D12_VERTEX_BUFFER_VIEW m_vbView;
    };
}