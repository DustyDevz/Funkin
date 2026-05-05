// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "ui_font.hpp"
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_truetype.h>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cstring>

namespace Funkin::UI {
    bool Font::load(Renderer::GAL::IDAL* gal,
                    const std::string& path,
                    float pixelHeight,
                    uint32_t atlasSize)
    {
        std::ifstream f(path, std::ios::binary);
        if (!f) return false;
        std::vector<uint8_t> ttf(std::istreambuf_iterator<char>(f), {});

        m_bakeSize  = pixelHeight;
        m_atlasSize = atlasSize;

        stbtt_fontinfo info;
        stbtt_InitFont(&info, ttf.data(), 0);
        float scale = stbtt_ScaleForPixelHeight(&info, pixelHeight);

        std::vector<uint8_t> sdfBitmap(atlasSize * atlasSize, 0);
        stbtt_bakedchar baked[96] = {};

        int x = 1, y = 1, maxRowH = 0;
        const int padding = 6;

        for (int ci = 1; ci < 96; ++ci) {
            int glyph = stbtt_FindGlyphIndex(&info, 32 + ci);
            if (!glyph) continue;

            int w, h, xoff, yoff;
            uint8_t* sdf = stbtt_GetGlyphSDF(&info, scale, glyph,
                                              padding, 128, 64.0f,
                                              &w, &h, &xoff, &yoff);
            if (!sdf) continue;

            if (x + w + 1 >= (int)atlasSize) { x = 1; y += maxRowH + 1; maxRowH = 0; }
            if (y + h + 1 >= (int)atlasSize) { stbtt_FreeSDF(sdf, nullptr); continue; }

            for (int row = 0; row < h; ++row)
                memcpy(&sdfBitmap[(y + row) * atlasSize + x], &sdf[row * w], w);

            baked[ci].x0       = (stbtt_int16)x;
            baked[ci].y0       = (stbtt_int16)y;
            baked[ci].x1       = (stbtt_int16)(x + w);
            baked[ci].y1       = (stbtt_int16)(y + h);
            baked[ci].xoff     = (float)xoff;
            baked[ci].yoff     = (float)yoff;

            int adv, lsb;
            stbtt_GetGlyphHMetrics(&info, glyph, &adv, &lsb);
            baked[ci].xadvance = adv * scale;

            maxRowH = std::max(maxRowH, h);
            x += w + 1;
            stbtt_FreeSDF(sdf, nullptr);
        }

        {
            int spaceGlyph = stbtt_FindGlyphIndex(&info, 32);
            int adv, lsb;
            stbtt_GetGlyphHMetrics(&info, spaceGlyph, &adv, &lsb);
            baked[0].xadvance = adv * scale;
        }

        std::vector<uint8_t> rgba(atlasSize * atlasSize * 4);
        for (uint32_t i = 0; i < atlasSize * atlasSize; ++i) {
            rgba[i*4+0] = 255;
            rgba[i*4+1] = 255;
            rgba[i*4+2] = 255;
            rgba[i*4+3] = sdfBitmap[i];
        }

        Renderer::GAL::TextureDesc td{};
        td.width  = atlasSize;
        td.height = atlasSize;
        td.format = Renderer::GAL::PixelFormat::RGBA8_Unorm;
        td.usage  = Renderer::GAL::TextureUsage::Sampled;
        m_texture = gal->createTexture(td);
        gal->uploadTexture(m_texture, rgba.data(), rgba.size());

        uint8_t white[4] = { 255, 255, 255, 255 };
        Renderer::GAL::TextureDesc wd{};
        wd.width  = 1;
        wd.height = 1;
        wd.format = Renderer::GAL::PixelFormat::RGBA8_Unorm;
        wd.usage  = Renderer::GAL::TextureUsage::Sampled;
        m_whiteTexture = gal->createTexture(wd);
        gal->uploadTexture(m_whiteTexture, white, 4);
        printf("white tex id: %u  font tex id: %u\n", m_whiteTexture.id, m_texture.id);

        float uvScale = 1.0f / atlasSize;
        for (int i = 0; i < 96; ++i) {
            const auto& b = baked[i];
            GlyphInfo g;
            g.u0      = b.x0 * uvScale;
            g.v0      = b.y0 * uvScale;
            g.u1      = b.x1 * uvScale;
            g.v1      = b.y1 * uvScale;
            g.x0      = b.xoff;
            g.y0      = b.yoff;
            g.x1      = b.xoff + (float)(b.x1 - b.x0);
            g.y1      = b.yoff + (float)(b.y1 - b.y0);
            g.advance = b.xadvance;
            m_glyphs[(char)(32 + i)] = g;
        }

        int a, d, lg;
        stbtt_GetFontVMetrics(&info, &a, &d, &lg);
        float s = stbtt_ScaleForPixelHeight(&info, pixelHeight);
        m_ascent  = a  * s;
        m_descent = d  * s;
        m_lineGap = lg * s;

        return true;
    }

    const GlyphInfo* Font::glyph(char c) const {
        auto it = m_glyphs.find(c);
        return it != m_glyphs.end() ? &it->second : nullptr;
    }
}