// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <renderer/gal/idal.hpp>
#include <string>
#include <unordered_map>

namespace Funkin::UI {
    struct GlyphInfo {
        float u0, v0, u1, v1;
        float x0, y0, x1, y1;
        float advance;
    };

    class Font {
    public:
        bool load(Renderer::GAL::IDAL* gal,
                  const std::string& path,
                  float pixelHeight,
                  uint32_t atlasSize = 512);

        const GlyphInfo* glyph(char c) const;

        Renderer::GAL::TextureHandle texture()      const { return m_texture; }
        Renderer::GAL::TextureHandle whiteTexture() const { return m_whiteTexture; }
        
        float ascent()    const { return m_ascent; }
        float descent()   const { return m_descent; }
        float lineGap()   const { return m_lineGap; }
        float bakeSize()  const { return m_bakeSize; }
        uint32_t atlasSize() const { return m_atlasSize; }

    private:
        std::unordered_map<char, GlyphInfo> m_glyphs;
        Renderer::GAL::TextureHandle        m_texture;
        Renderer::GAL::TextureHandle m_whiteTexture;

        float    m_ascent    = 0;
        float    m_descent   = 0;
        float    m_lineGap   = 0;
        float    m_bakeSize  = 32.0f;
        uint32_t m_atlasSize = 512;
    };
}