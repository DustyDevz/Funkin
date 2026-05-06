// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "ui_renderer.hpp"
#include "theme.hpp"
#include <renderer/shader/shader_loader.hpp>
#include <renderer/shader/shader_types.hpp>
#include <cmath>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <renderer/gal/ui/ui_dx12_text.hpp>

namespace Funkin::UI {
    UIRenderer& UIRenderer::get() {
        static UIRenderer s;
        return s;
    }

    void UIRenderer::init(Renderer::GAL::IDAL* gal, uint32_t width, uint32_t height) {
        m_gal    = gal;
        m_width  = width;
        m_height = height;

        initPipeline();

        m_vertexBuffer = m_gal->createBuffer({
            MAX_VERTICES * sizeof(UIVertex),
            Renderer::GAL::BufferUsage::Vertex,
            Renderer::GAL::MemoryHint::CPUWrite
        });

        m_indexBuffer = m_gal->createBuffer({
            MAX_INDICES * sizeof(uint16_t),
            Renderer::GAL::BufferUsage::Index,
            Renderer::GAL::MemoryHint::CPUWrite
        });

        m_cbuffer = m_gal->createBuffer({
            256,
            Renderer::GAL::BufferUsage::Uniform,
            Renderer::GAL::MemoryHint::CPUWrite
        });

        m_samplerNearest = m_gal->createSampler({
            Renderer::GAL::FilterMode::Nearest,
            Renderer::GAL::WrapMode::Clamp,
            Renderer::GAL::WrapMode::Clamp
        });

        m_samplerLinear = m_gal->createSampler({
            Renderer::GAL::FilterMode::Linear,
            Renderer::GAL::WrapMode::Clamp,
            Renderer::GAL::WrapMode::Clamp
        });

        uint32_t white = 0xFFFFFFFF;
        Renderer::GAL::TextureDesc desc{};
        desc.width  = 1;
        desc.height = 1;
        desc.mips   = 1;
        desc.format = Renderer::GAL::PixelFormat::RGBA8_Unorm;
        desc.usage  = Renderer::GAL::TextureUsage::Sampled;

        m_whiteTexture = m_gal->createTexture(desc);
        m_gal->uploadTexture(m_whiteTexture, &white, sizeof(white));
    }

    void UIRenderer::initPipeline() {
        auto& sl = Renderer::Shader::ShaderLoader::get();

        auto* vs = sl.get("ui", Renderer::Shader::ShaderStage::Vertex);
        auto* ps = sl.get("ui", Renderer::Shader::ShaderStage::Pixel);

        if (!vs || !ps)
            throw std::runtime_error("UI shaders not found");

        m_vs = m_gal->createShader({ vs->bytecode.data(), vs->bytecode.size(),
                                    Renderer::GAL::ShaderStage::Vertex });
        m_ps = m_gal->createShader({ ps->bytecode.data(), ps->bytecode.size(),
                                    Renderer::GAL::ShaderStage::Pixel });

        Renderer::GAL::VertexAttribute attribs[] = {
            { "POSITION", 0, Renderer::GAL::PixelFormat::RG32_Float,   0  },
            { "TEXCOORD", 0, Renderer::GAL::PixelFormat::RG32_Float,   8  },
            { "COLOR",    0, Renderer::GAL::PixelFormat::RGBA32_Float, 16 },
        };

        Renderer::GAL::PipelineDesc pd{};
        pd.vs                  = m_vs;
        pd.ps                  = m_ps;
        pd.vertexLayout        = { attribs, 3, sizeof(UIVertex) };
        pd.blend               = Renderer::GAL::BlendMode::Alpha;
        pd.topology            = Renderer::GAL::PrimitiveTopology::TriangleList;
        pd.renderTargetFmt     = Renderer::GAL::PixelFormat::BGRA8_Unorm;
        pd.renderTargetCount   = 1;
        pd.depth.testEnabled   = false;
        pd.depth.writeEnabled  = false;
        pd.raster.cullMode     = Renderer::GAL::CullMode::None;

        m_pipeline = m_gal->createPipeline(pd);
    }

    void UIRenderer::resize(uint32_t width, uint32_t height) {
        m_width  = width;
        m_height = height;
    }

    void UIRenderer::beginFrame() {
        m_batches.clear();
        m_textCmds.clear();
    }

    void UIRenderer::ensureBatch(bool isText) {
        if (m_batches.empty() || m_batches.back().isText != isText)
            m_batches.push_back({ {}, {}, isText });
    }

    void UIRenderer::pushQuad(Rect r, Color color,
                              float u0, float v0, float u1, float v1,
                              bool isText) {
        ensureBatch(isText);
        auto& batch = m_batches.back();
        uint16_t base = (uint16_t)batch.vertices.size();

        batch.vertices.push_back({ r.x,       r.y,       u0, v0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ r.x + r.w, r.y,       u1, v0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ r.x + r.w, r.y + r.h, u1, v1, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ r.x,       r.y + r.h, u0, v1, color.r, color.g, color.b, color.a });

        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 1);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 3);
    }

    void UIRenderer::pushLine(Vec2 a, Vec2 b, Color color, float thickness) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float len = std::sqrt(dx*dx + dy*dy);
        if (len < 0.001f) return;

        float nx = (-dy / len) * (thickness * 0.5f);
        float ny = ( dx / len) * (thickness * 0.5f);

        ensureBatch(false);
        auto& batch = m_batches.back();
        uint16_t base = (uint16_t)batch.vertices.size();

        batch.vertices.push_back({ a.x + nx, a.y + ny, 0.5f, 0.5f, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ a.x - nx, a.y - ny, 0.5f, 0.5f, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ b.x - nx, b.y - ny, 0.5f, 0.5f, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ b.x + nx, b.y + ny, 0.5f, 0.5f, color.r, color.g, color.b, color.a });

        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 1);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 3);
    }

    void UIRenderer::drawFilledRect(Rect r, Color color) {
        pushQuad(r, color, 0.0f, 0.0f, 1.0f, 1.0f, false);
    }

    void UIRenderer::drawRect(Rect r, Color fill, Color border,
                              float radius, float borderWidth) {
        drawFilledRect(r, fill);
        pushLine({ r.x,       r.y       }, { r.x + r.w, r.y       }, border, borderWidth);
        pushLine({ r.x + r.w, r.y       }, { r.x + r.w, r.y + r.h }, border, borderWidth);
        pushLine({ r.x + r.w, r.y + r.h }, { r.x,       r.y + r.h }, border, borderWidth);
        pushLine({ r.x,       r.y + r.h }, { r.x,       r.y       }, border, borderWidth);
    }

    void UIRenderer::drawLine(Vec2 a, Vec2 b, Color color, float thickness) {
        pushLine(a, b, color, thickness);
    }

    void UIRenderer::drawCircle(Vec2 center, float radius, Color color, int segments) {
        ensureBatch(false);
        auto& batch = m_batches.back();
        uint16_t centerIdx = (uint16_t)batch.vertices.size();

        batch.vertices.push_back({ center.x, center.y, 0.5f, 0.5f,
                                   color.r, color.g, color.b, color.a });

        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / (float)segments * 3.14159265f * 2.0f;
            float x = center.x + std::cos(angle) * radius;
            float y = center.y + std::sin(angle) * radius;
            batch.vertices.push_back({ x, y, 0.5f, 0.5f,
                                       color.r, color.g, color.b, color.a });
        }

        for (int i = 0; i < segments; ++i) {
            batch.indices.push_back(centerIdx);
            batch.indices.push_back((uint16_t)(centerIdx + 1 + i));
            batch.indices.push_back((uint16_t)(centerIdx + 2 + i));
        }
    }

    void UIRenderer::drawSprite(Renderer::GAL::TextureHandle tex,
                                Rect dest, Color tint) {
        pushQuad(dest, tint, 0.0f, 0.0f, 1.0f, 1.0f, false);
    }

    void UIRenderer::drawText(const std::string& text, Rect bounds, Color color,
                            float fontSize, TextAlign align) {
        if (text.empty()) return;
        m_textCmds.push_back({
            std::wstring(text.begin(), text.end()),
            bounds, color, fontSize
        });
    }

    void UIRenderer::flushGeometry() {
        if (m_batches.empty()) return;

        float w = (float)m_width;
        float h = (float)m_height;
        float ortho[16] = {
            2.0f/w,  0.0f,    0.0f,  0.0f,
            0.0f,   -2.0f/h,  0.0f,  0.0f,
            0.0f,    0.0f,    1.0f,  0.0f,
        -1.0f,    1.0f,    0.0f,  1.0f
        };

        void* cbData = m_gal->mapBuffer(m_cbuffer);
        memcpy(cbData, ortho, sizeof(ortho));
        m_gal->unmapBuffer(m_cbuffer);

        m_gal->setPipeline(m_pipeline);
        m_gal->setUniformBuffer(m_cbuffer, 0);

        std::vector<UIVertex> allVerts;
        std::vector<uint16_t> allIdx;

        for (auto& batch : m_batches) {
            uint16_t vertBase = (uint16_t)allVerts.size();
            for (auto& v : batch.vertices) allVerts.push_back(v);
            for (auto  i : batch.indices)  allIdx.push_back(i + vertBase);
        }

        void* vData = m_gal->mapBuffer(m_vertexBuffer);
        memcpy(vData, allVerts.data(), allVerts.size() * sizeof(UIVertex));
        m_gal->unmapBuffer(m_vertexBuffer);

        void* iData = m_gal->mapBuffer(m_indexBuffer);
        memcpy(iData, allIdx.data(), allIdx.size() * sizeof(uint16_t));
        m_gal->unmapBuffer(m_indexBuffer);

        m_gal->setVertexBuffer(m_vertexBuffer);
        m_gal->setIndexBuffer(m_indexBuffer, Renderer::GAL::IndexType::Uint16);

        uint32_t indexOffset = 0;
        for (auto& batch : m_batches) {
            if (batch.indices.empty()) continue;
            m_gal->setSampler(m_samplerNearest, 0);
            m_gal->setTexture(m_whiteTexture, 0);
            m_gal->drawIndexed({
                (uint32_t)batch.indices.size(),
                1, indexOffset, 0, 0
            });
            indexOffset += (uint32_t)batch.indices.size();
        }

        m_batches.clear();
    }

    void UIRenderer::flushText() {
    #ifdef _WIN32
        auto& txt = Funkin::Renderer::GAL::UI::DX12TextRenderer::get();
        for (auto& cmd : m_textCmds)
            txt.drawText(cmd.text, cmd.bounds, cmd.color, cmd.fontSize);
    #endif
        m_textCmds.clear();
    }

    void UIRenderer::drawRoundedRect(Rect r, Color fill, float radius) {
        if (radius <= 0.0f) { drawFilledRect(r, fill); return; }
        float rad = std::min(radius, std::min(r.w, r.h) * 0.5f);
        drawFilledRect({ r.x + rad,       r.y,       r.w - rad*2, r.h       }, fill);
        drawFilledRect({ r.x,             r.y + rad, rad,         r.h - rad*2 }, fill);
        drawFilledRect({ r.x + r.w - rad, r.y + rad, rad,         r.h - rad*2 }, fill);
        drawCircle({ r.x + rad,          r.y + rad       }, rad, fill, 12);
        drawCircle({ r.x + r.w - rad,    r.y + rad       }, rad, fill, 12);
        drawCircle({ r.x + rad,          r.y + r.h - rad }, rad, fill, 12);
        drawCircle({ r.x + r.w - rad,    r.y + r.h - rad }, rad, fill, 12);
    }

    void UIRenderer::flush() {
        if (m_batches.empty()) return;

        float w = (float)m_width;
        float h = (float)m_height;
        float ortho[16] = {
            2.0f/w,  0.0f,    0.0f,  0.0f,
            0.0f,   -2.0f/h,  0.0f,  0.0f,
            0.0f,    0.0f,    1.0f,  0.0f,
            -1.0f,    1.0f,    0.0f,  1.0f
        };

        void* cbData = m_gal->mapBuffer(m_cbuffer);
        memcpy(cbData, ortho, sizeof(ortho));
        m_gal->unmapBuffer(m_cbuffer);

        m_gal->setPipeline(m_pipeline);
        m_gal->setUniformBuffer(m_cbuffer, 0);

        std::vector<UIVertex> allVerts;
        std::vector<uint16_t> allIdx;

        for (auto& batch : m_batches) {
            uint16_t vertBase = (uint16_t)allVerts.size();
            for (auto& v : batch.vertices) allVerts.push_back(v);
            for (auto  i : batch.indices)  allIdx.push_back(i + vertBase);
        }

        void* vData = m_gal->mapBuffer(m_vertexBuffer);
        memcpy(vData, allVerts.data(), allVerts.size() * sizeof(UIVertex));
        m_gal->unmapBuffer(m_vertexBuffer);

        void* iData = m_gal->mapBuffer(m_indexBuffer);
        memcpy(iData, allIdx.data(), allIdx.size() * sizeof(uint16_t));
        m_gal->unmapBuffer(m_indexBuffer);

        m_gal->setVertexBuffer(m_vertexBuffer);
        m_gal->setIndexBuffer(m_indexBuffer, Renderer::GAL::IndexType::Uint16);

        uint32_t indexOffset = 0;
        for (auto& batch : m_batches) {
            if (batch.indices.empty()) continue;
            m_gal->setSampler(m_samplerNearest, 0);
            m_gal->setTexture(m_whiteTexture, 0);
            m_gal->drawIndexed({
                (uint32_t)batch.indices.size(),
                1, indexOffset, 0, 0
            });
            indexOffset += (uint32_t)batch.indices.size();
        }

        m_batches.clear();
    }

    void UIRenderer::shutdown() {
        m_gal->destroyTexture(m_whiteTexture);
        m_gal->destroyBuffer(m_vertexBuffer);
        m_gal->destroyBuffer(m_indexBuffer);
        m_gal->destroyBuffer(m_cbuffer);
        m_gal->destroySampler(m_samplerNearest);
        m_gal->destroySampler(m_samplerLinear);
        m_gal->destroyPipeline(m_pipeline);
        m_gal->destroyShader(m_vs);
        m_gal->destroyShader(m_ps);
    }
}