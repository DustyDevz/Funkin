// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "sprite_batch.hpp"
#include "assets/assets_types.hpp"
#include "assets/assets.hpp"
#include "shared/log.hpp"
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace Funkin::Renderer {
    struct Animation {
        std::string         name;
        std::vector<size_t> frames;
        float               fps      = 24.0f;
        bool                loop     = true;
        int                 loopFrom = 0;
    };

    class AnimatedSprite {
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
        float    playbackRate = 1.0f;

        std::function<void(const std::string&)> onAnimComplete;

        void loadAtlas(const std::string& xmlId, const std::string& group = "") {
            m_atlas = Assets::AssetManager::get().load<Assets::SparrowAtlas>(xmlId, group);
            if (m_atlas && !m_atlas->frames.empty())
                applyFrame(0);
        }

        void loadAtlasAsync(const std::string& xmlId, const std::string& group = "") {
            Assets::AssetManager::get().loadAsync<Assets::SparrowAtlas>(xmlId, group,
                [this](Assets::AssetHandle<Assets::SparrowAtlas> atlas) {
                    m_atlas = atlas;
                    if (m_atlas && !m_atlas->frames.empty())
                        applyFrame(0);
                });
        }

        void addAnimation(const std::string& name,
                          const std::string& prefix,
                          float fps      = 24.0f,
                          bool  loop     = true,
                          int   loopFrom = 0) {
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

        void addAnimationByIndices(const std::string& name,
                                   const std::string& prefix,
                                   const std::vector<int>& indices,
                                   float fps  = 24.0f,
                                   bool  loop = true) {
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

        void play(const std::string& name, bool restart = false) {
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

        void pause()  { m_paused = true;  }
        void resume() { m_paused = false; }
        void stop()   { m_paused = true; m_currentFrame = 0; m_elapsed = 0; }

        void setFrame(const std::string& frameName) {
            if (!m_atlas) return;
            auto it = m_atlas->frameIndex.find(frameName);
            if (it != m_atlas->frameIndex.end())
                applyFrame(it->second);
        }

        void update(float dt) {
            if (m_currentAnim >= m_anims.size() || m_paused || m_finished) return;
            auto& anim = m_anims[m_currentAnim];
            if (anim.frames.empty()) return;

            m_elapsed += dt * playbackRate;
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

        void draw() {
            if (!visible || !m_atlas || !m_atlas->texture) return;
            uint32_t c = (color & 0xFFFFFF00) | (uint32_t)(alpha * 255.0f);
            SpriteBatch::get().draw({
                m_atlas->texture,
                x + m_frameOffsetX * scaleX,
                y + m_frameOffsetY * scaleY,
                m_srcW * scaleX,
                m_srcH * scaleY,
                originX, originY,
                1.0f, 1.0f,
                rotation,
                c,
                m_uvX, m_uvY, m_uvX + m_uvW, m_uvY + m_uvH
            });
        }

        void setPosition(float px, float py)  { x = px; y = py; }
        void setScale(float sx, float sy)      { scaleX = sx; scaleY = sy; }
        void setScale(float s)                 { scaleX = s; scaleY = s; }
        void setOrigin(float ox, float oy)     { originX = ox; originY = oy; }
        void setOriginCenter()                 { originX = 0.5f; originY = 0.5f; }
        void setColor(uint32_t c)              { color = c; }
        void setAlpha(float a)                 { alpha = a; }
        void setRotation(float r)              { rotation = r; }
        void setPlaybackRate(float r)          { playbackRate = r; }

        bool isFinished()  const { return m_finished; }
        bool isPaused()    const { return m_paused; }
        bool hasAtlas()    const { return m_atlas.isValid(); }
        bool hasAnim(const std::string& name) const { return m_animIndex.count(name) > 0; }

        const std::string& currentAnim() const {
            static std::string empty;
            if (m_currentAnim >= m_anims.size()) return empty;
            return m_anims[m_currentAnim].name;
        }

        size_t currentFrameIndex() const { return m_currentFrame; }
        size_t frameCount()        const { return m_atlas ? m_atlas->frames.size() : 0; }

        Assets::SparrowHandle atlas() const { return m_atlas; }

    private:
        void applyFrame(size_t atlasIndex) {
            if (!m_atlas || atlasIndex >= m_atlas->frames.size()) return;
            auto& f    = m_atlas->frames[atlasIndex];
            float texW = (float)m_atlas->texture->width;
            float texH = (float)m_atlas->texture->height;
            m_uvX          = f.x / texW;
            m_uvY          = f.y / texH;
            m_uvW          = f.w / texW;
            m_uvH          = f.h / texH;
            m_srcW         = f.w;
            m_srcH         = f.h;
            m_frameOffsetX = -f.frameX;
            m_frameOffsetY = -f.frameY;
            width          = f.frameW;
            height         = f.frameH;
        }

        Assets::SparrowHandle                   m_atlas;
        std::vector<Animation>                  m_anims;
        std::unordered_map<std::string, size_t> m_animIndex;
        size_t  m_currentAnim  = SIZE_MAX;
        size_t  m_currentFrame = 0;
        float   m_elapsed      = 0.0f;
        bool    m_finished     = false;
        bool    m_paused       = false;
        float   m_uvX = 0, m_uvY = 0, m_uvW = 1, m_uvH = 1;
        float   m_srcW = 0, m_srcH = 0;
        float   m_frameOffsetX = 0, m_frameOffsetY = 0;
    };
}