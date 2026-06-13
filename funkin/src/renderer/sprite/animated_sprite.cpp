// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "animated_sprite.hpp"
#include "shared/log.hpp"

namespace Funkin::Renderer {

    void AnimatedSprite::loadAtlas(const std::string& xmlId, const std::string& group) {
        m_atlas = Assets::AssetManager::get().load<Assets::SparrowAtlas>(xmlId, group);
        if (m_atlas && !m_atlas->frames.empty())
            applyFrame(0);
    }

    void AnimatedSprite::loadAtlasAsync(const std::string& xmlId, const std::string& group) {
        Assets::AssetManager::get().loadAsync<Assets::SparrowAtlas>(xmlId, group,
            [this](Assets::AssetHandle<Assets::SparrowAtlas> atlas) {
                m_atlas = atlas;
                if (m_atlas && !m_atlas->frames.empty())
                    applyFrame(0);
            });
    }

    void AnimatedSprite::addAnimation(const std::string& name,
                                      const std::string& prefix,
                                      float fps,
                                      bool loop,
                                      int loopFrom) {
        if (!m_atlas) { LOG_WARN("AnimatedSprite: no atlas loaded"); return; }
        Animation anim;
        anim.name     = name;
        anim.fps      = fps;
        anim.loop     = loop;
        anim.loopFrom = loopFrom;
        anim.frames   = m_atlas->framesWithPrefix(prefix);
        if (anim.frames.empty()) {
            LOG_WARN("AnimatedSprite: no frames for prefix '{}'", prefix);
            return;
        }
        m_animIndex[name] = m_anims.size();
        m_anims.push_back(std::move(anim));
    }

    void AnimatedSprite::addAnimationByIndices(const std::string& name,
                                               const std::string& prefix,
                                               const std::vector<int>& indices,
                                               float fps,
                                               bool loop) {
        if (!m_atlas) return;
        auto all = m_atlas->framesWithPrefix(prefix);
        Animation anim;
        anim.name = name;
        anim.fps  = fps;
        anim.loop = loop;
        for (int i : indices)
            if (i >= 0 && i < (int)all.size())
                anim.frames.push_back(all[i]);
        if (anim.frames.empty()) return;
        m_animIndex[name] = m_anims.size();
        m_anims.push_back(std::move(anim));
    }

    void AnimatedSprite::play(const std::string& name, bool restart) {
        auto it = m_animIndex.find(name);
        if (it == m_animIndex.end()) {
            LOG_WARN("AnimatedSprite: animation '{}' not found", name);
            return;
        }
        if (m_currentAnim == it->second && !restart && !m_finished) return;
        m_currentAnim  = it->second;
        m_currentFrame = 0;
        m_elapsed      = 0.0f;
        m_finished     = false;
        m_paused       = false;
        if (!m_anims[m_currentAnim].frames.empty())
            applyFrame(m_anims[m_currentAnim].frames[0]);
    }

    void AnimatedSprite::pause()  { m_paused = true; }
    void AnimatedSprite::resume() { m_paused = false; }
    void AnimatedSprite::stop()   { m_paused = true; m_currentFrame = 0; m_elapsed = 0; }

    void AnimatedSprite::setFrame(const std::string& frameName) {
        if (!m_atlas) return;
        auto it = m_atlas->frameIndex.find(frameName);
        if (it != m_atlas->frameIndex.end())
            applyFrame(it->second);
    }

    void AnimatedSprite::update(float dt) {
        if (m_currentAnim >= m_anims.size() || m_paused || m_finished) return;
        auto& anim = m_anims[m_currentAnim];
        if (anim.frames.empty()) return;

        float clampedDt = std::min(dt, .1f);
        m_elapsed += clampedDt * playbackRate;
        float frameDur = 1.0f / anim.fps;

        while (m_elapsed >= frameDur) {
            m_elapsed -= frameDur;
            m_currentFrame++;

            if (m_currentFrame >= anim.frames.size()) {
                if (anim.loop) {
                    m_currentFrame = (size_t)anim.loopFrom;
                } else {
                    m_currentFrame = anim.frames.size() - 1;
                    m_finished     = true;
                    if (onAnimComplete)
                        onAnimComplete(anim.name);
                    break;
                }
            }
        }

        applyFrame(anim.frames[m_currentFrame]);
    }

    void AnimatedSprite::draw() {
        if (!visible || !m_atlas || !m_atlas->texture || !m_atlas->texture.isValid()) return;

        uint8_t r = (color >> 24) & 0xFF;
        uint8_t g = (color >> 16) & 0xFF;
        uint8_t b = (color >> 8) & 0xFF;
        uint8_t a = (uint8_t)(alpha * 255.f);
        uint32_t packedColor = (a << 24) | (b << 16) | (g << 8) | r;

        uint32_t sampler = pixel
            ? (BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT)
            : 0;

        SpriteBatch::get().setSamplerFlags(sampler);
        SpriteBatch::get().draw({
            m_atlas->texture,
            x + m_frameOffsetX * scaleX,
            y + m_frameOffsetY * scaleY,
            m_srcW * scaleX,
            m_srcH * scaleY,
            originX, originY,
            1.0f, 1.0f,
            rotation,
            packedColor,
            m_uvX, m_uvY, m_uvX + m_uvW, m_uvY + m_uvH,
            viewId
        });
    }

    const std::string& AnimatedSprite::currentAnim() const {
        static std::string empty;
        if (m_currentAnim >= m_anims.size()) return empty;
        return m_anims[m_currentAnim].name;
    }

    void AnimatedSprite::applyFrame(size_t atlasIndex) {
        if (!m_atlas || atlasIndex >= m_atlas->frames.size()) return;
        auto& f    = m_atlas->frames[atlasIndex];

        float texW = 1.f / (float)m_atlas->texture->width;
        float texH = 1.f / (float)m_atlas->texture->height;
        m_uvX          = f.x * texW;
        m_uvY          = f.y * texH;
        m_uvW          = f.w * texW;
        m_uvH          = f.h * texH;
        m_srcW         = f.w;
        m_srcH         = f.h;
        m_frameOffsetX = -f.frameX;
        m_frameOffsetY = -f.frameY;
        width          = f.frameW;
        height         = f.frameH;
    }
}