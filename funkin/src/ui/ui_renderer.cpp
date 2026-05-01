// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "ui_renderer.hpp"
#include "theme.hpp"
#include <renderer/shader/shader_loader.hpp>
#include <renderer/shader/shader_types.hpp>
#include <cmath>
#include <stdexcept>
#include <cstring>

namespace Funkin::UI {
    UIRenderer& UIRenderer::get() {
        static UIRenderer s;
        return s;
    }

    void UIRenderer::init(Renderer::GAL::IDAL* gal, uint32_t width, uint32_t height) {
        m_gal    = gal;
        m_width  = width;
        m_height = height;

        initWhiteTexture();
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
            16 * sizeof(float),
            Renderer::GAL::BufferUsage::Uniform,
            Renderer::GAL::MemoryHint::CPUWrite
        });

        m_sampler = m_gal->createSampler({
            Renderer::GAL::FilterMode::Linear,
            Renderer::GAL::WrapMode::Clamp,
            Renderer::GAL::WrapMode::Clamp
        });
    }

    void UIRenderer::initWhiteTexture() {
        Renderer::GAL::TextureDesc td{};
        td.width  = 1;
        td.height = 1;
        td.format = Renderer::GAL::PixelFormat::RGBA8_Unorm;
        td.usage  = Renderer::GAL::TextureUsage::Sampled;

        m_whiteTexture = m_gal->createTexture(td);

        uint8_t white[4] = { 255, 255, 255, 255 };
        m_gal->uploadTexture(m_whiteTexture, white, 4);
    }

    void UIRenderer::initPipeline() {
        auto& sl = Renderer::Shader::ShaderLoader::get();

        auto* vs = sl.get("ui", Renderer::Shader::ShaderStage::Vertex);
        auto* ps = sl.get("ui", Renderer::Shader::ShaderStage::Pixel);

        if (!vs || !ps)
            throw std::runtime_error("UI shaders not found — make sure ui.hlsl is in shaders/");

        m_vs = m_gal->createShader({ vs->bytecode.data(), vs->bytecode.size(),
                                    Renderer::GAL::ShaderStage::Vertex });
        m_ps = m_gal->createShader({ ps->bytecode.data(), ps->bytecode.size(),
                                    Renderer::GAL::ShaderStage::Pixel });

        Renderer::GAL::VertexAttribute attribs[] = {
            { "POSITION", 0, Renderer::GAL::PixelFormat::RG32_Float,    0  },
            { "TEXCOORD", 0, Renderer::GAL::PixelFormat::RG32_Float,    8  },
            { "COLOR",    0, Renderer::GAL::PixelFormat::RGBA32_Float,  16 },
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
    }

    void UIRenderer::ensureBatch(Renderer::GAL::TextureHandle tex) {
        if (m_batches.empty() || m_batches.back().texture != tex)
            m_batches.push_back({ {}, {}, tex });
    }

    void UIRenderer::pushQuad(Rect r, Color color,
                            Renderer::GAL::TextureHandle tex,
                            float u0, float v0, float u1, float v1) {
        ensureBatch(tex);
        auto& batch = m_batches.back();

        uint16_t base = (uint16_t)batch.vertices.size();

        batch.vertices.push_back({ r.x,        r.y,        u0, v0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ r.x + r.w,  r.y,        u1, v0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ r.x + r.w,  r.y + r.h,  u1, v1, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ r.x,        r.y + r.h,  u0, v1, color.r, color.g, color.b, color.a });

        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 1);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 3);
    }

    void UIRenderer::pushLine(Vec2 a, Vec2 b, Color color, float thickness) {
        ensureBatch(m_whiteTexture);
        auto& batch = m_batches.back();

        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float len = std::sqrt(dx*dx + dy*dy);
        if (len < 0.001f) return;

        float nx = (-dy / len) * (thickness * 0.5f);
        float ny = ( dx / len) * (thickness * 0.5f);

        uint16_t base = (uint16_t)batch.vertices.size();

        batch.vertices.push_back({ a.x + nx, a.y + ny, 0, 0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ a.x - nx, a.y - ny, 0, 0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ b.x - nx, b.y - ny, 0, 0, color.r, color.g, color.b, color.a });
        batch.vertices.push_back({ b.x + nx, b.y + ny, 0, 0, color.r, color.g, color.b, color.a });

        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 1);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 0);
        batch.indices.push_back(base + 2);
        batch.indices.push_back(base + 3);
    }

    void UIRenderer::drawFilledRect(Rect r, Color color) {
        pushQuad(r, color, m_whiteTexture);
    }

    void UIRenderer::drawRect(Rect r, Color fill, Color border,
                            float radius, float borderWidth) {
        pushQuad(r, fill, m_whiteTexture);

        pushLine({ r.x,        r.y        }, { r.x + r.w,  r.y        }, border, borderWidth);
        pushLine({ r.x + r.w,  r.y        }, { r.x + r.w,  r.y + r.h  }, border, borderWidth);
        pushLine({ r.x + r.w,  r.y + r.h  }, { r.x,        r.y + r.h  }, border, borderWidth);
        pushLine({ r.x,        r.y + r.h  }, { r.x,        r.y        }, border, borderWidth);
    }

    void UIRenderer::drawLine(Vec2 a, Vec2 b, Color color, float thickness) {
        pushLine(a, b, color, thickness);
    }

    void UIRenderer::drawCircle(Vec2 center, float radius, Color color, int segments) {
        ensureBatch(m_whiteTexture);
        auto& batch = m_batches.back();

        uint16_t centerIdx = (uint16_t)batch.vertices.size();
        batch.vertices.push_back({ center.x, center.y, 0.5f, 0.5f,
                                    color.r, color.g, color.b, color.a });

        for (int i = 0; i <= segments; ++i) {
            float angle = (float)i / (float)segments * 3.14159265f * 2.0f;
            float x = center.x + std::cos(angle) * radius;
            float y = center.y + std::sin(angle) * radius;
            batch.vertices.push_back({ x, y, 0, 0, color.r, color.g, color.b, color.a });
        }

        for (int i = 0; i < segments; ++i) {
            batch.indices.push_back(centerIdx);
            batch.indices.push_back((uint16_t)(centerIdx + 1 + i));
            batch.indices.push_back((uint16_t)(centerIdx + 2 + i));
        }
    }

    void UIRenderer::drawSprite(Renderer::GAL::TextureHandle tex,
                                Rect dest, Color tint) {
        pushQuad(dest, tint, tex);
    }

    void UIRenderer::drawText(const std::string& text, Rect bounds, Color color,
                            float fontSize, TextAlign align) {
        pushQuad({ bounds.x, bounds.y + bounds.h * 0.5f - 1.0f,
                bounds.w * 0.6f, 2.0f }, color, m_whiteTexture);
    }

    void UIRenderer::flush() {
        if (m_batches.empty()) return;

        float w = (float)m_width;
        float h = (float)m_height;
        float ortho[16] = {
            2.0f/w,  0.0f,    0.0f, 0.0f,
            0.0f,   -2.0f/h,  0.0f, 0.0f,
            0.0f,    0.0f,    1.0f, 0.0f,
            -1.0f,    1.0f,    0.0f, 1.0f,
        };

        void* cbData = m_gal->mapBuffer(m_cbuffer);
        memcpy(cbData, ortho, sizeof(ortho));
        m_gal->unmapBuffer(m_cbuffer);

        Renderer::GAL::RenderPassDesc rp{};
        rp.useSwapchainTarget = true;
        m_gal->beginRenderPass(rp);

        Renderer::GAL::Viewport vp{};
        vp.width  = w;
        vp.height = h;
        m_gal->setViewport(vp);
        m_gal->setScissor({ 0.0f, 0.0f, w, h });

        m_gal->setPipeline(m_pipeline);
        m_gal->setUniformBuffer(m_cbuffer, 0);
        m_gal->setSampler(m_sampler, 0);

        for (auto& batch : m_batches)
            flushBatch(batch);

        m_gal->endRenderPass();
        m_batches.clear();
    }

    void UIRenderer::flushBatch(UIBatch& batch) {
        if (batch.vertices.empty()) return;

        void* vData = m_gal->mapBuffer(m_vertexBuffer);
        memcpy(vData, batch.vertices.data(), batch.vertices.size() * sizeof(UIVertex));
        m_gal->unmapBuffer(m_vertexBuffer);

        void* iData = m_gal->mapBuffer(m_indexBuffer);
        memcpy(iData, batch.indices.data(), batch.indices.size() * sizeof(uint16_t));
        m_gal->unmapBuffer(m_indexBuffer);

        m_gal->setVertexBuffer(m_vertexBuffer);
        m_gal->setIndexBuffer(m_indexBuffer, Renderer::GAL::IndexType::Uint16);

        if (batch.texture.valid())
            m_gal->setTexture(batch.texture, 0);
        else
            m_gal->setTexture(m_whiteTexture, 0);

        m_gal->drawIndexed({
            (uint32_t)batch.indices.size(),
            1, 0, 0, 0
        });
    }

    void UIRenderer::shutdown() {
        m_gal->destroyBuffer(m_vertexBuffer);
        m_gal->destroyBuffer(m_indexBuffer);
        m_gal->destroyBuffer(m_cbuffer);
        m_gal->destroyTexture(m_whiteTexture);
        m_gal->destroySampler(m_sampler);
        m_gal->destroyPipeline(m_pipeline);
        m_gal->destroyShader(m_vs);
        m_gal->destroyShader(m_ps);
    }
}