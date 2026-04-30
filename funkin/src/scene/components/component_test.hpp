// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <renderer/gal/idal.hpp>

namespace Funkin::Scene::Components {

    struct Vertex {
        float position[3];
        float color[4];
    };

    class Test {
    public:
        void init(Renderer::GAL::IDAL*       gal,
                  Renderer::GAL::ShaderHandle vs,
                  Renderer::GAL::ShaderHandle ps);
        void draw(Renderer::GAL::IDAL* gal);
        void shutdown(Renderer::GAL::IDAL* gal);

    private:
        Renderer::GAL::BufferHandle   m_vertexBuffer;
        Renderer::GAL::PipelineHandle m_pipeline;
    };

}
