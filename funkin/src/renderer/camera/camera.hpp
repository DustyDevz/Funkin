// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <bgfx/bgfx.h>
#include <cstdint>
#include <string>
#include <vector>
#include <cmath>
#include "math/math_lerp.hpp"
#include "math/math_ease.hpp"
#include "math/math_clamp.hpp"

namespace Funkin::Renderer::Camera {
    struct CameraState {
        float x    = 0.f;
        float y    = 0.f;
        float zoom = 1.f;
    };

    struct CameraBounds {
        bool  enabled = false;
        float minX    = -10000.f;
        float minY    = -10000.f;
        float maxX    =  10000.f;
        float maxY    =  10000.f;
    };

    struct WorldPoint  { float x, y; };
    struct ScreenPoint { float x, y; };

    enum class EaseType {
        Linear,
        QuadIn,
        QuadOut,
        QuadInOut,
        CubicOut,
        SineOut,
    };

    struct CameraTween {
        float    fromX = 0.f, fromY = 0.f, fromZoom = 1.f;
        float    toX   = 0.f, toY   = 0.f, toZoom   = 1.f;
        float    duration = 1.f, elapsed = 0.f;
        EaseType ease     = EaseType::CubicOut;
        bool     active   = false;
        bool     done     = false;

        void start(float fx, float fy, float fz,
                   float tx, float ty, float tz,
                   float dur, EaseType e = EaseType::CubicOut);
        void update(float dt, float& outX, float& outY, float& outZoom);
    };

    class Camera {
    public:
        virtual ~Camera() = default;

        virtual void             update(float dt)         = 0;
        virtual const CameraState& state() const          = 0;

        void apply(uint16_t viewId, uint32_t screenW, uint32_t screenH) const;

        WorldPoint  screenToWorld(ScreenPoint sp, uint32_t screenW, uint32_t screenH) const;
        ScreenPoint worldToScreen(WorldPoint  wp, uint32_t screenW, uint32_t screenH) const;

        std::string name;
        bool        active = true;
    };

    class EditorCamera final : public Camera {
    public:
        EditorCamera();

        void update(float dt) override;
        const CameraState& state() const override { return m_state; }

        void onScroll(float delta, float screenX, float screenY, float screenW, float screenH);
        void onMiddleMousePress(float screenX, float screenY);
        void onMiddleMouseRelease();
        void onMouseMove(float screenX, float screenY);
        void onKeyPan(float dx, float dy, float dt);

        void reset();
        void zoomToFit(float worldW, float worldH, uint32_t screenW, uint32_t screenH);
        void centerOn(float worldX, float worldY);
        bool isWorldVisible(float worldX, float worldY, uint32_t screenW, uint32_t screenH) const;

        float        zoomSpeed   = 0.12f;
        float        minZoom     = 0.05f;
        float        maxZoom     = 32.f;
        float        keyPanSpeed = 400.f;
        CameraBounds bounds;

    private:
        void clampToBounds();

        CameraState m_state;
        bool        m_panning   = false;
        float       m_panStartX = 0.f, m_panStartY = 0.f;
        float       m_camStartX = 0.f, m_camStartY = 0.f;
    };

    class GameCamera final : public Camera {
    public:
        GameCamera();

        void update(float dt) override;
        const CameraState& state() const override { return m_state; }

        void setTarget(float x, float y);
        void snapToTarget();

        void tweenTo(float x, float y, float zoom, float duration, EaseType ease = EaseType::CubicOut);
        void tweenToTarget(float duration, EaseType ease = EaseType::CubicOut);

        void onBeat(float intensity = 0.015f);
        void bumpZoom(float amount);
        bool isTweening() const { return m_tween.active && !m_tween.done; }

        float        followLerp = 6.f;
        float        zoomLerp   = 5.f;
        float        baseZoom   = 1.f;
        CameraBounds bounds;

    private:
        void clampToBounds();

        CameraState  m_state;
        float        m_targetX    = 0.f;
        float        m_targetY    = 0.f;
        float        m_targetZoom = 1.f;
        CameraTween  m_tween;
    };

    struct CameraLayer {
        std::string name;
        uint16_t    viewId  = 0;
        Camera*     camera  = nullptr;
        float       scrollX = 1.f;
        float       scrollY = 1.f;
        bool        fixed   = false;
        bool        visible = true;

        void apply(uint32_t screenW, uint32_t screenH) const;
    };

    class CameraManager {
    public:
        static CameraManager& get();

        EditorCamera& editor() { return m_editor; }
        GameCamera&   game()   { return m_game; }

        void    setEditorMode(bool enabled) { m_editorMode = enabled; }
        bool    isEditorMode() const        { return m_editorMode; }
        Camera& active()                    { return m_editorMode ? (Camera&)m_editor : (Camera&)m_game; }

        void update(float dt);
        void applyAll(uint32_t screenW, uint32_t screenH) const;

        CameraLayer* addLayer(const std::string& name, uint16_t viewId, Camera* cam, bool fixed = false);
        CameraLayer* getLayer(const std::string& name);
        void         removeLayer(const std::string& name);
        void         clearLayers();

        const std::vector<CameraLayer>& layers() const { return m_layers; }

    private:
        CameraManager() = default;

        EditorCamera             m_editor;
        GameCamera               m_game;
        bool                     m_editorMode = true;
        std::vector<CameraLayer> m_layers;
    };

}