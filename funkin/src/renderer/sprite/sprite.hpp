// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "sprite_batch.hpp"
#include "assets/assets.hpp"

namespace Funkin::Renderer {
    class Sprite {
    public:
        float x        = 0.0f;
        float y        = 0.0f;
        float scaleX   = 1.0f;
        float scaleY   = 1.0f;
        float rotation = 0.0f;
        float originX  = 0.0f; // 0 = left, 0.5 = center, 1 = right
        float originY  = 0.0f; // 0 = top,  0.5 = center, 1 = bottom
        uint32_t color = 0xFFFFFFFF;
        bool visible   = true;

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
                    LOG_PRINT("Sprite texture ready: {} ({}x{})", id, (int)width, (int)height);
                });
        }

        void setTexture(Assets::TextureHandle tex) {
            m_texture = tex;
            if (m_texture) {
                width  = (float)m_texture->width;
                height = (float)m_texture->height;
            }
        }

        void draw() {
            if (!visible || !m_texture) return;
            SpriteBatch::get().draw({
                m_texture,
                x, y,
                width, height,
                originX, originY,
                scaleX, scaleY,
                rotation,
                color,
                0.0f, 0.0f, 1.0f, 1.0f
            });
        }

        float width  = 0.0f;
        float height = 0.0f;
        Assets::TextureHandle m_texture { nullptr };
    };
}