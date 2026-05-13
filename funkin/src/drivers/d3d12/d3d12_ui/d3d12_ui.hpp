// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <math/math_rect.hpp>
#include <math/math_vec2.hpp>
#include <math/math_color.hpp>
#include <drivers/d3d12/d3d12.hpp>

namespace Funkin::UI {
    enum class TextAlign;

    struct UIVertex {
        float x, y;
        float u, v;
        float r, g, b, a;
    };

    struct UIBatch {
        std::vector<UIVertex>  vertices;
        std::vector<uint16_t>  indices;
        bool                   isText = false;
    };

    struct UITextCmd {
        std::wstring text;
        Math::Rect         bounds;
        Math::Color        color;
        float        fontSize;
    };

    class UIRenderer {
    public:
        static UIRenderer& get();

        void init(Renderer::GAL::IDAL* gal, uint32_t width, uint32_t height);
        void shutdown();
        void resize(uint32_t width, uint32_t height);

        void beginFrame();
        void flush();
        void flushText();

        void drawFilledRect(Math::Rect r, Math::Color color);
        void drawRect(Math::Rect r, Math::Color fill, Math::Color border, float radius = 0.0f, float borderWidth = 1.0f);
        void drawText(const std::string& text, Math::Rect bounds, Math::Color color,
                      float fontSize, TextAlign align);
        void drawSprite(Renderer::GAL::TextureHandle tex, Math::Rect dest, Math::Color tint = { 1,1,1,1 });
        void drawLine(Math::Vec2 a, Math::Vec2 b, Math::Color color, float thickness = 1.0f);
        void drawCircle(Math::Vec2 center, float radius, Math::Color color, int segments = 32);
        void drawRoundedRect(Math::Rect r, Math::Color fill, float radius);

        uint32_t width()  const { return m_width; }
        uint32_t height() const { return m_height; }

        Renderer::GAL::IDAL* gal() const { return m_gal; }

    private:
        UIRenderer() = default;

        void ensureBatch(bool isText);
        void pushQuad(Math::Rect r, Math::Color color,
                      float u0, float v0, float u1, float v1,
                      bool isText);
        void pushLine(Math::Vec2 a, Math::Vec2 b, Math::Color color, float thickness);
        void initPipeline();

        Renderer::GAL::IDAL*          m_gal    = nullptr;
        uint32_t                      m_width  = 0;
        uint32_t                      m_height = 0;

        Renderer::GAL::ShaderHandle   m_vs;
        Renderer::GAL::ShaderHandle   m_ps;
        Renderer::GAL::PipelineHandle m_pipeline;
        Renderer::GAL::SamplerHandle  m_samplerNearest;
        Renderer::GAL::SamplerHandle  m_samplerLinear;
        Renderer::GAL::BufferHandle   m_vertexBuffer;
        Renderer::GAL::BufferHandle   m_indexBuffer;
        Renderer::GAL::BufferHandle   m_cbuffer;
        Renderer::GAL::TextureHandle  m_whiteTexture{};

        std::vector<UIBatch>   m_batches;
        std::vector<UITextCmd> m_textCmds;

        static constexpr uint32_t MAX_VERTICES = 65536;
        static constexpr uint32_t MAX_INDICES  = 131072;
    };
}