// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <bx/math.h>
#include <vector>
#include <memory>
#include "assets/assets_types.hpp"

namespace Funkin::Renderer {
    struct SpriteVertex {
        float x, y;
        float u, v;
        uint32_t color;

        static bgfx::VertexLayout& layout() {
            static bgfx::VertexLayout s;
            static bool init = false;
            if (!init) {
                s.begin()
                    .add(bgfx::Attrib::Position,  2, bgfx::AttribType::Float)
                    .add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
                    .add(bgfx::Attrib::Color0,    4, bgfx::AttribType::Uint8, true)
                .end();
                init = true;
            }
            return s;
        }
    };

    struct SpriteQuad {
        Assets::TextureHandle texture;
        float x, y;
        float width, height;
        float originX, originY;
        float scaleX, scaleY;
        float rotation;
        uint32_t color;
        float u0, v0, u1, v1;
    };

    class SpriteBatch {
    public:
        SpriteBatch();
        ~SpriteBatch();

        void begin(uint16_t viewId, uint32_t screenWidth, uint32_t screenHeight);
        void draw(const SpriteQuad& quad);
        void setSamplerFlags(uint32_t flags) { m_samplerFlags = flags; }
        void end();

        static SpriteBatch& get();

    private:
        void flush(const Assets::TextureHandle& tex);

        bgfx::UniformHandle         m_uTexture  { BGFX_INVALID_HANDLE };
        bgfx::ProgramHandle         m_program   { BGFX_INVALID_HANDLE };

        std::vector<SpriteVertex>   m_vertices;
        std::vector<uint16_t>       m_indices;

        Assets::TextureHandle m_currentTex { nullptr };

        uint16_t    m_viewId        { 0 };
        uint32_t    m_screenWidth   { 1280 };
        uint32_t    m_screenHeight  { 720 };
        uint32_t    m_samplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT;
        float       m_proj[16]      {};

        static constexpr uint32_t MAX_QUADS = 4096;
    };
}