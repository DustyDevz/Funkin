// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "sprite.hpp"
#include "renderer/camera/camera.hpp"

namespace Funkin::Renderer {
    void Sprite::loadTexture(const std::string& id, const std::string& group) {
        m_texture = Funkin::Assets::AssetManager::get().load<Funkin::Assets::Texture>(id, group);
        if (m_texture) {
            width  = (float)m_texture->width;
            height = (float)m_texture->height;
        }
    }

    void Sprite::loadTextureAsync(const std::string& id, const std::string& group) {
        const uint64_t transactionId = ++m_currentTransactionId;

        Assets::AssetManager::get().loadAsync<Assets::Texture>(id, group,
        [this, transactionId](Assets::AssetHandle<Assets::Texture> tex) {
            if (transactionId != m_currentTransactionId) return;

            m_texture = tex;
            if (m_texture && width == 0.f, height == 0.f) {
                width  = (float)m_texture->width;
                height = (float)m_texture->height;
            }
        });
    }

    void Sprite::setTexture(Assets::TextureHandle tex) {
        m_texture = tex;
        if (m_texture) {
            width  = (float)m_texture->width;
            height = (float)m_texture->height;
        }
    }

    void Sprite::draw() {
        if (!visible || !m_texture || alpha <= 0.f) return;

        uint8_t r = (color >> 24) & 0xFF;
        uint8_t g = (color >> 16) & 0xFF;
        uint8_t b = (color >> 8) & 0xFF;
        uint8_t a = (uint8_t)(alpha * 255.f);
        uint32_t packedColor = (a << 24) | (b << 16) | (g << 8) | r;

        float u0 = 0.f; float v0 = 0.f;
        float u1 = 1.f; float v1 = 1.f;

        if (flipX) std::swap(u0, u1);
        if (flipY) std::swap(v0, v1);

        SpriteBatch::get().draw({
            m_texture,
            x, y,
            width, height,
            originX, originY,
            scaleX, scaleY,
            rotation,
            packedColor,
            u0, v0, u1, v1,
            viewId
        });
    }
}
