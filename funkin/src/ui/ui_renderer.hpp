// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <math/rect.hpp>
#include <math/vec2.hpp>
#include <math/color.hpp>
#include <renderer/gal/idal.hpp>
#include <renderer/gal/descriptors.hpp>
#include <string>
#include <vector>

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
        Rect         bounds;
        Color        color;
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

        void drawFilledRect(Rect r, Color color);
        void drawRect(Rect r, Color fill, Color border, float radius = 0.0f, float borderWidth = 1.0f);
        void drawText(const std::string& text, Rect bounds, Color color,
                      float fontSize, TextAlign align);
        void drawSprite(Renderer::GAL::TextureHandle tex, Rect dest, Color tint = { 1,1,1,1 });
        void drawLine(Vec2 a, Vec2 b, Color color, float thickness = 1.0f);
        void drawCircle(Vec2 center, float radius, Color color, int segments = 32);
        void drawRoundedRect(Rect r, Color fill, float radius);

        uint32_t width()  const { return m_width; }
        uint32_t height() const { return m_height; }

        Renderer::GAL::IDAL* gal() const { return m_gal; }

    private:
        UIRenderer() = default;

        void ensureBatch(bool isText);
        void pushQuad(Rect r, Color color,
                      float u0, float v0, float u1, float v1,
                      bool isText);
        void pushLine(Vec2 a, Vec2 b, Color color, float thickness);
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