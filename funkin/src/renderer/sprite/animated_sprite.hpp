// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "sprite_batch.hpp"
#include "assets/assets_types.hpp"
#include "assets/assets.hpp"
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
        float    x            = 0.0f;
        float    y            = 0.0f;
        float    scaleX       = 1.0f;
        float    scaleY       = 1.0f;
        float    rotation     = 0.0f;
        float    originX      = 0.0f;
        float    originY      = 0.0f;
        uint32_t color        = 0xFFFFFFFF;
        uint16_t viewId       = 0;
        bool     visible      = true;
        float    width        = 0.0f;
        float    height       = 0.0f;
        float    alpha        = 1.0f;
        float    playbackRate = 1.0f;
        bool     pixel        = false;

        std::function<void(const std::string&)> onAnimComplete;

        void loadAtlas(const std::string& xmlId, const std::string& group = "");
        void loadAtlasAsync(const std::string& xmlId, const std::string& group = "");

        void addAnimation(const std::string& name,
                          const std::string& prefix,
                          float fps      = 24.0f,
                          bool  loop     = true,
                          int   loopFrom = 0);

        void addAnimationByIndices(const std::string& name,
                                   const std::string& prefix,
                                   const std::vector<int>& indices,
                                   float fps  = 24.0f,
                                   bool  loop = true);

        void play(const std::string& name, bool restart = false);
        void pause();
        void resume();
        void stop();

        void setFrame(const std::string& frameName);
        void update(float dt);
        void draw();

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

        const std::string& currentAnim() const;
        size_t currentFrameIndex() const { return m_currentFrame; }
        size_t frameCount()        const { return m_atlas ? m_atlas->frames.size() : 0; }

        Assets::SparrowHandle atlas() const { return m_atlas; }

    private:
        void applyFrame(size_t atlasIndex);

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