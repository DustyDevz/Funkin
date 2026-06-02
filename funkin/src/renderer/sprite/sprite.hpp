// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "sprite_batch.hpp"
#include "assets/assets_types.hpp"
#include "assets/assets.hpp"

namespace Funkin::Renderer {
    class Sprite {
    public:
        float    x        = 0.0f;
        float    y        = 0.0f;
        float    scaleX   = 1.0f;
        float    scaleY   = 1.0f;
        float    rotation = 0.0f;
        float    originX  = 0.0f;
        float    originY  = 0.0f;
        uint32_t color    = 0xFFFFFFFF;
        bool     visible  = true;
        float    width    = 0.0f;
        float    height   = 0.0f;
        float    alpha    = 1.0f;

        void loadTexture(const std::string& id, const std::string& group = "") {
            m_texture = Assets::AssetManager::get().load<Assets::Texture>(id, group);
            if (m_texture) {
                width  = (float)m_texture->width;
                height = (float)m_texture->height;
            }
        }

        void loadTextureAsync(const std::string& id, const std::string& group = "") {
            Assets::AssetManager::get().loadAsync<Assets::Texture>(id, group,
                [this, id](Assets::AssetHandle<Assets::Texture> tex) {
                    m_texture = tex;
                    if (m_texture && width == 0.0f && height == 0.0f) {
                        width  = (float)m_texture->width;
                        height = (float)m_texture->height;
                    }
                });
        }

        void setTexture(Assets::TextureHandle tex) {
            m_texture = tex;
            if (m_texture) {
                width  = (float)m_texture->width;
                height = (float)m_texture->height;
            }
        }

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

        void draw() {
            if (!visible || !m_texture) return;
            uint32_t c = (color & 0xFFFFFF00) | (uint32_t)(alpha * 255.0f);
            SpriteBatch::get().draw({
                m_texture,
                x, y,
                width, height,
                originX, originY,
                scaleX, scaleY,
                rotation,
                c,
                0.0f, 0.0f, 1.0f, 1.0f
            });
        }

    private:
        Assets::TextureHandle m_texture;
    };
}