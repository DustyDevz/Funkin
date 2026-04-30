// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "component_test.hpp"

namespace Funkin::Scene::Components {
    void Test::init(ID3D12Device* device) {
        Vertex vertices[] = {
            { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        };

        const UINT size = sizeof(vertices);
        CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_UPLOAD);
        CD3DX12_RESOURCE_DESC   buf = CD3DX12_RESOURCE_DESC::Buffer(size);

        if (FAILED(device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &buf,
            D3D12_RESOURCE_STATE_GENERIC_READ,
            nullptr, IID_PPV_ARGS(&m_vertexBuffer))))
        throw std::runtime_error("Failed to create vertex buffer");

        void* mapped = nullptr;
        m_vertexBuffer->Map(0, nullptr, &mapped);
        memcpy(mapped, vertices, size);
        m_vertexBuffer->Unmap(0, nullptr);

        m_vbView.BufferLocation = m_vertexBuffer->GetGPUVirtualAddress();
        m_vbView.StrideInBytes  = sizeof(Vertex);
        m_vbView.SizeInBytes    = size;
    }

    void Test::draw(ID3D12GraphicsCommandList* cmd,
                    const Funkin::Renderer::DX12::DX12Pipeline& pipeline,
                    D3D12_VIEWPORT viewport,
                    D3D12_RECT scissor) {
        cmd->SetGraphicsRootSignature(pipeline.rootSignature());
        cmd->SetPipelineState(pipeline.pso());
        cmd->RSSetViewports(1, &viewport);
        cmd->RSSetScissorRects(1, &scissor);
        cmd->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        cmd->IASetVertexBuffers(0, 1, &m_vbView);
        cmd->DrawInstanced(3, 1, 0, 0);
    }

    void Test::shutdown() {
        m_vertexBuffer.Reset();
    }
}