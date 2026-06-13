// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "sprite_batch.hpp"
#include "assets/assets_types.hpp"
#include "assets/assets.hpp"

namespace Funkin::Renderer {
    class Sprite {
    public:
        float    x             = 0.f;
        float    y             = 0.f;
        float    scaleX        = 1.f;
        float    scaleY        = 1.f;
        float    rotation      = 0.f;
        float    originX       = 0.f;
        float    originY       = 0.f;
        float    width         = 0.f;
        float    height        = 0.f;
        float    alpha         = 1.f;
        uint32_t color         = 0xFFFFFFFF;
        uint16_t viewId        = 0;
        bool     visible       = true;
        bool     flipX         = false;
        bool     flipY         = false;

        void loadTexture(const std::string& id, const std::string& group = "");
        void loadTextureAsync(const std::string& id, const std::string& group = "");
        void setTexture(Assets::TextureHandle tex);
        void draw();

        void setSize(float w, float h)          { width = w; height = h; }
        void setPosition(float px, float py)    { x = px; y = py; }
        void setScale(float sx, float sy)       { scaleX = sx; scaleY = sy; }
        void setScale(float s)                  { scaleX = s; scaleY = s; }
        void setOrigin(float ox, float oy)      { originX = ox; originY = oy; }
        void setOriginCenter()                  { originX = 0.5f; originY = 0.5f; }
        void setColor(uint32_t c)               { color = c; }
        void setAlpha(float a)                  { alpha = a; }
        void setRotation(float r)               { rotation = r; }

        bool hasTexture() const { return m_texture.isValid(); }
        Assets::TextureHandle texture() const { return m_texture; }

    private:
        Assets::TextureHandle m_texture;
        uint64_t              m_currentTransactionId = 0;
    };
}