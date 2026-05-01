// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "component_test.hpp"

namespace Funkin::Scene::Components {

    void Test::init(Renderer::GAL::IDAL*       gal,
                    Renderer::GAL::ShaderHandle vs,
                    Renderer::GAL::ShaderHandle ps)
    {
        Vertex vertices[] = {
            { {  0.0f,  0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { {  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.5f, -0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } },
        };

        Renderer::GAL::BufferDesc vbDesc{};
        vbDesc.size   = sizeof(vertices);
        vbDesc.usage  = Renderer::GAL::BufferUsage::Vertex;
        vbDesc.memory = Renderer::GAL::MemoryHint::CPUWrite;

        m_vertexBuffer = gal->createBuffer(vbDesc);

        void* mapped = gal->mapBuffer(m_vertexBuffer);
        std::memcpy(mapped, vertices, sizeof(vertices));
        gal->unmapBuffer(m_vertexBuffer);

        Renderer::GAL::VertexAttribute attrs[] = {
            { "POSITION", 0, Renderer::GAL::PixelFormat::RGB32_Float,  0  },
            { "COLOR",    0, Renderer::GAL::PixelFormat::RGBA32_Float, 12  },
        };

        Renderer::GAL::VertexLayout layout{};
        layout.attributes = attrs;
        layout.count      = 2;
        layout.stride     = sizeof(Vertex);

        Renderer::GAL::PipelineDesc pd{};
        pd.vs              = vs;
        pd.ps              = ps;
        pd.vertexLayout    = layout;
        pd.blend           = Renderer::GAL::BlendMode::None;
        pd.topology        = Renderer::GAL::PrimitiveTopology::TriangleList;
        pd.renderTargetFmt = Renderer::GAL::PixelFormat::BGRA8_Unorm;
        pd.raster.cullMode = Renderer::GAL::CullMode::None;

        m_pipeline = gal->createPipeline(pd);
    }

    void Test::draw(Renderer::GAL::IDAL* gal)
    {
        gal->setPipeline(m_pipeline);
        gal->setVertexBuffer(m_vertexBuffer);
        gal->draw({ 3, 1, 0, 0 });
    }

    void Test::shutdown(Renderer::GAL::IDAL* gal)
    {
        gal->destroyPipeline(m_pipeline);
        gal->destroyBuffer(m_vertexBuffer);
        m_pipeline      = {};
        m_vertexBuffer  = {};
    }

}
