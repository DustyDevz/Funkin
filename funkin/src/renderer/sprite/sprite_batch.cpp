// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "sprite_batch.hpp"
#include "shaders/shader_program.hpp"
#include "shaders/sprites/sprite_shader.hpp"
#include <bx/math.h>

namespace Funkin::Renderer {
    SpriteBatch::SpriteBatch() {
        m_vertices.reserve(MAX_QUADS * 4);
        m_indices.reserve(MAX_QUADS * 6);
        m_uTexture = bgfx::createUniform("s_tex", bgfx::UniformType::Sampler);
    }

    SpriteBatch::~SpriteBatch() {
        if (bgfx::isValid(m_uTexture)) bgfx::destroy(m_uTexture);
    }

    SpriteBatch& SpriteBatch::get() {
        static SpriteBatch s;
        return s;
    }

    void SpriteBatch::begin(uint16_t viewId, uint32_t screenWidth, uint32_t screenHeight) {
        m_viewId       = viewId;
        m_screenWidth  = screenWidth;
        m_screenHeight = screenHeight;

        bx::mtxOrtho(m_proj,
            0.0f, (float)screenWidth,
            (float)screenHeight, 0.0f,
            0.0f, 1000.0f,
            0.0f, bgfx::getCaps()->homogeneousDepth);

        // bgfx::setViewTransform(m_viewId, nullptr, m_proj);
        bgfx::setViewRect(m_viewId, 0, 0, (uint16_t)screenWidth, (uint16_t)screenHeight);

        m_vertices.clear();
        m_indices.clear();
        m_currentTex = {};
    }

    void SpriteBatch::draw(const SpriteQuad& quad) {
        if (!quad.texture) return;

        if (m_currentTex && m_currentTex.get() != quad.texture.get()) {
            flush(m_currentTex);
        }
        if (m_vertices.size() >= MAX_QUADS * 4) {
            flush(m_currentTex);
        }

        m_currentTex = quad.texture;

        float hw = quad.width  * quad.scaleX * 0.5f;
        float hh = quad.height * quad.scaleY * 0.5f;

        float ox = quad.originX * quad.width  * quad.scaleX;
        float oy = quad.originY * quad.height * quad.scaleY;
        
        float lx0 = -ox,         ly0 = -oy;
        float lx1 = quad.width * quad.scaleX - ox;
        float ly1 = quad.height * quad.scaleY - oy;

        float corners[4][2] = {
            { lx0, ly0 },
            { lx1, ly0 },
            { lx1, ly1 },
            { lx0, ly1 }
        };

        float cr = bx::cos(quad.rotation);
        float sr = bx::sin(quad.rotation);

        float uvs[4][2] = {
            { quad.u0, quad.v0 },
            { quad.u1, quad.v0 },
            { quad.u1, quad.v1 },
            { quad.u0, quad.v1 }
        };

        uint16_t base = (uint16_t)m_vertices.size();

        for (int i = 0; i < 4; i++) {
            float rx = corners[i][0] * cr - corners[i][1] * sr;
            float ry = corners[i][0] * sr + corners[i][1] * cr;

            m_vertices.push_back({
                quad.x + rx,
                quad.y + ry,
                uvs[i][0],
                uvs[i][1],
                quad.color
            });
        }

        m_indices.push_back(base + 0);
        m_indices.push_back(base + 1);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 0);
        m_indices.push_back(base + 2);
        m_indices.push_back(base + 3);
    }

    void SpriteBatch::flush(const Assets::TextureHandle& tex) {
        if (m_vertices.empty() || !tex) return;

        auto prog = Funkin::Shader::Sprites::programHandle();
        if (!bgfx::isValid(prog)) {
            m_vertices.clear();
            m_indices.clear();
            return;
        }

        bgfx::TransientVertexBuffer tvb;
        bgfx::TransientIndexBuffer  tib;

        uint32_t numVerts = (uint32_t)m_vertices.size();
        uint32_t numIdx   = (uint32_t)m_indices.size();

        if (!bgfx::getAvailTransientVertexBuffer(numVerts, SpriteVertex::layout()) ||
            !bgfx::getAvailTransientIndexBuffer(numIdx)) {
            LOG_WARN("SpriteBatch: transient buffer full");
            m_vertices.clear();
            m_indices.clear();
            return;
        }

        bgfx::allocTransientVertexBuffer(&tvb, numVerts, SpriteVertex::layout());
        bgfx::allocTransientIndexBuffer(&tib, numIdx);

        memcpy(tvb.data, m_vertices.data(), numVerts * sizeof(SpriteVertex));
        memcpy(tib.data, m_indices.data(),  numIdx   * sizeof(uint16_t));

        bgfx::setTexture(0, m_uTexture, tex->handle, m_samplerFlags);
        bgfx::setVertexBuffer(0, &tvb);
        bgfx::setIndexBuffer(&tib);
        bgfx::setState(
            BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A |
            BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA)
        );
        bgfx::submit(m_viewId, prog);

        m_vertices.clear();
        m_indices.clear();
    }

    void SpriteBatch::end() {
        if (!m_vertices.empty()) {
            auto prog = Funkin::Shader::Sprites::programHandle();
            if (!bgfx::isValid(prog)) {
                m_vertices.clear();
                m_indices.clear();
                m_currentTex = nullptr;
                return;
            }
            flush(m_currentTex);
        }
        m_currentTex = nullptr;
    }
}