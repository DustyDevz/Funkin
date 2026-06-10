// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "camera.hpp"
#include <bx/math.h>
#include <algorithm>

namespace Funkin::Renderer::Camera {
    static float applyEase(float t, EaseType e) {
        switch (e) {
            case EaseType::QuadIn:    return Funkin::Math::easeQuadIn(t);
            case EaseType::QuadOut:   return Funkin::Math::easeQuadOut(t);
            case EaseType::QuadInOut: return Funkin::Math::easeQuadInOut(t);
            case EaseType::CubicOut:  return Funkin::Math::easeCubicOut(t);
            case EaseType::SineOut:   return Funkin::Math::easeSineOut(t);
            default:                  return Funkin::Math::easeLinear(t);
        }
    }

    static void buildOrtho(float* proj, float l, float r, float b, float t) {
        bx::mtxOrtho(proj, l, r, b, t, 0.f, 1000.f, 0.f, bgfx::getCaps()->homogeneousDepth);
    }

    void CameraTween::start(float fx, float fy, float fz, float tx, float ty, float tz, float dur, EaseType e) {
        fromX = fx; fromY = fy; fromZoom = fz;
        toX   = tx; toY   = ty; toZoom   = tz;
        duration = dur; elapsed = 0.f;
        ease = e; active = true; done = false;
    }

    void CameraTween::update(float dt, float& outX, float& outY, float& outZoom) {
        if (!active || done) return;
        elapsed += dt;
        float t = Funkin::Math::clamp01(elapsed / duration);
        if (t >= 1.f) done = true;
        float et  = applyEase(t, ease);
        outX      = Funkin::Math::lerp(fromX,    toX,    et);
        outY      = Funkin::Math::lerp(fromY,    toY,    et);
        outZoom   = Funkin::Math::lerp(fromZoom, toZoom, et);
    }

    void Camera::apply(uint16_t viewId, uint32_t screenW, uint32_t screenH) const {
        const auto& s = state();
        float hw = (float)screenW * 0.5f;
        float hh = (float)screenH * 0.5f;
        float proj[16];
        buildOrtho(proj,
            s.x - hw / s.zoom, s.x + hw / s.zoom,
            s.y + hh / s.zoom, s.y - hh / s.zoom);
        bgfx::setViewTransform(viewId, nullptr, proj);
        bgfx::setViewRect(viewId, 0, 0, (uint16_t)screenW, (uint16_t)screenH);
    }

    WorldPoint Camera::screenToWorld(ScreenPoint sp, uint32_t screenW, uint32_t screenH) const {
        const auto& s = state();
        return {
            s.x + (sp.x - (float)screenW * 0.5f) / s.zoom,
            s.y + (sp.y - (float)screenH * 0.5f) / s.zoom
        };
    }

    ScreenPoint Camera::worldToScreen(WorldPoint wp, uint32_t screenW, uint32_t screenH) const {
        const auto& s = state();
        return {
            (wp.x - s.x) * s.zoom + (float)screenW * 0.5f,
            (wp.y - s.y) * s.zoom + (float)screenH * 0.5f
        };
    }

    EditorCamera::EditorCamera() { name = "editor"; }

    void EditorCamera::update(float dt) {
        (void)dt;
        clampToBounds();
    }

    void EditorCamera::onScroll(float delta, float screenX, float screenY, float screenW, float screenH) {
        float factor  = 1.f + zoomSpeed * (delta > 0.f ? 1.f : -1.f);
        float newZoom = Funkin::Math::clamp(m_state.zoom * factor, minZoom, maxZoom);
        WorldPoint wp = screenToWorld({ screenX, screenY }, (uint32_t)screenW, (uint32_t)screenH);
        m_state.zoom  = newZoom;
        m_state.x     = wp.x - (screenX - screenW * 0.5f) / m_state.zoom;
        m_state.y     = wp.y - (screenY - screenH * 0.5f) / m_state.zoom;
        clampToBounds();
    }

    void EditorCamera::onMiddleMousePress(float screenX, float screenY) {
        m_panning = true;
        m_panStartX = screenX; m_panStartY = screenY;
        m_camStartX = m_state.x; m_camStartY = m_state.y;
    }

    void EditorCamera::onMiddleMouseRelease() { m_panning = false; }

    void EditorCamera::onMouseMove(float screenX, float screenY) {
        if (!m_panning) return;
        m_state.x = m_camStartX - (screenX - m_panStartX) / m_state.zoom;
        m_state.y = m_camStartY - (screenY - m_panStartY) / m_state.zoom;
        clampToBounds();
    }

    void EditorCamera::onKeyPan(float dx, float dy, float dt) {
        m_state.x += dx * keyPanSpeed * dt / m_state.zoom;
        m_state.y += dy * keyPanSpeed * dt / m_state.zoom;
        clampToBounds();
    }

    void EditorCamera::reset() { m_state = {}; m_panning = false; }

    void EditorCamera::zoomToFit(float worldW, float worldH, uint32_t screenW, uint32_t screenH) {
        float zx     = (float)screenW / worldW;
        float zy     = (float)screenH / worldH;
        m_state.zoom = Funkin::Math::clamp(std::min(zx, zy) * 0.9f, minZoom, maxZoom);
        m_state.x    = worldW * 0.5f;
        m_state.y    = worldH * 0.5f;
    }

    void EditorCamera::centerOn(float worldX, float worldY) {
        m_state.x = worldX;
        m_state.y = worldY;
    }

    bool EditorCamera::isWorldVisible(float worldX, float worldY, uint32_t screenW, uint32_t screenH) const {
        ScreenPoint sp = worldToScreen({ worldX, worldY }, screenW, screenH);
        return sp.x >= 0.f && sp.x <= (float)screenW
            && sp.y >= 0.f && sp.y <= (float)screenH;
    }

    void EditorCamera::clampToBounds() {
        if (!bounds.enabled) return;
        m_state.x = Funkin::Math::clamp(m_state.x, bounds.minX, bounds.maxX);
        m_state.y = Funkin::Math::clamp(m_state.y, bounds.minY, bounds.maxY);
    }

    GameCamera::GameCamera() { name = "game"; m_targetZoom = baseZoom; }

    void GameCamera::update(float dt) {
        if (m_tween.active && !m_tween.done) {
            m_tween.update(dt, m_state.x, m_state.y, m_state.zoom);
        } else {
            m_state.x    = Funkin::Math::damp(m_state.x,    m_targetX,    followLerp, dt);
            m_state.y    = Funkin::Math::damp(m_state.y,    m_targetY,    followLerp, dt);
            m_state.zoom = Funkin::Math::damp(m_state.zoom, m_targetZoom, zoomLerp,   dt);

            if (m_targetZoom != baseZoom) {
                m_targetZoom = Funkin::Math::damp(m_targetZoom, baseZoom, zoomLerp, dt);
                if (std::abs(m_targetZoom - baseZoom) < 0.001f)
                    m_targetZoom = baseZoom;
            }
        }
        clampToBounds();
    }

    void GameCamera::setTarget(float x, float y) { m_targetX = x; m_targetY = y; }

    void GameCamera::snapToTarget() {
        m_state.x    = m_targetX;
        m_state.y    = m_targetY;
        m_state.zoom = m_targetZoom;
    }

    void GameCamera::tweenTo(float x, float y, float zoom, float duration, EaseType ease) {
        m_tween.start(m_state.x, m_state.y, m_state.zoom, x, y, zoom, duration, ease);
    }

    void GameCamera::tweenToTarget(float duration, EaseType ease) {
        tweenTo(m_targetX, m_targetY, m_targetZoom, duration, ease);
    }

    void GameCamera::onBeat(float intensity)  { m_targetZoom = baseZoom + intensity; }
    void GameCamera::bumpZoom(float amount)   { m_targetZoom = baseZoom + amount; }

    void GameCamera::clampToBounds() {
        if (!bounds.enabled) return;
        m_state.x = Funkin::Math::clamp(m_state.x, bounds.minX, bounds.maxX);
        m_state.y = Funkin::Math::clamp(m_state.y, bounds.minY, bounds.maxY);
    }

    void CameraLayer::apply(uint32_t screenW, uint32_t screenH) const {
        if (!camera || !visible) return;

        float proj[16];
        if (fixed) {
            bx::mtxOrtho(proj, 0.f, (float)screenW, (float)screenH, 0.f,
                0.f, 1000.f, 0.f, bgfx::getCaps()->homogeneousDepth);
            bgfx::setViewTransform(viewId, nullptr, proj);
            bgfx::setViewRect(viewId, 0, 0, (uint16_t)screenW, (uint16_t)screenH);
            return;
        }

        const auto& s = camera->state();
        float ex = s.x * scrollX;
        float ey = s.y * scrollY;
        float hw = (float)screenW * 0.5f;
        float hh = (float)screenH * 0.5f;

        bx::mtxOrtho(proj,
            ex - hw / s.zoom, ex + hw / s.zoom,
            ey + hh / s.zoom, ey - hh / s.zoom,
            0.f, 1000.f, 0.f, bgfx::getCaps()->homogeneousDepth);

        bgfx::setViewTransform(viewId, nullptr, proj);
        bgfx::setViewRect(viewId, 0, 0, (uint16_t)screenW, (uint16_t)screenH);
    }

    CameraManager& CameraManager::get() {
        static CameraManager s;
        return s;
    }

    void CameraManager::update(float dt) {
        m_editor.update(dt);
        m_game.update(dt);
    }

    void CameraManager::applyAll(uint32_t screenW, uint32_t screenH) const {
        for (const auto& l : m_layers) l.apply(screenW, screenH);
    }

    CameraLayer* CameraManager::addLayer(const std::string& name, uint16_t viewId, Camera* cam, bool fixed) {
        m_layers.push_back({ name, viewId, cam, 1.f, 1.f, fixed, true });
        return &m_layers.back();
    }

    CameraLayer* CameraManager::getLayer(const std::string& name) {
        for (auto& l : m_layers)
            if (l.name == name) return &l;
        return nullptr;
    }

    void CameraManager::removeLayer(const std::string& name) {
        m_layers.erase(std::remove_if(m_layers.begin(), m_layers.end(),
            [&](const CameraLayer& l) { return l.name == name; }), m_layers.end());
    }

    void CameraManager::clearLayers() { m_layers.clear(); }
}