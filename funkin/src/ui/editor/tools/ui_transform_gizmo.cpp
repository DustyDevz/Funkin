// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_transform_gizmo.hpp"
#include <debugdraw/debugdraw.h>
#include <bx/math.h>
#include <cmath>

namespace Funkin::UI::Editor::Tools {
    static constexpr uint32_t COLOR_X        = 0xff3a5cf0;
    static constexpr uint32_t COLOR_Y        = 0xff4caf50;
    static constexpr uint32_t COLOR_CENTER   = 0xffffffff;
    static constexpr uint32_t COLOR_ROTATE   = 0xffe0a030;
    static constexpr uint32_t COLOR_HOVER    = 0xffffff00;

    static ScreenPoint toScreen(const Camera& cam, float wx, float wy, uint32_t w, uint32_t h) {
        return cam.worldToScreen({ wx, wy }, w, h);
    }

    static float screenLengthToWorld(const Camera& cam, float len) {
        return len / cam.state().zoom;
    }

    void TransformGizmo::draw(uint16_t viewId, const Camera& cam, const GizmoTarget& target,
                               uint32_t screenW, uint32_t screenH) const {
        DebugDrawEncoder dde;
        dde.begin(viewId);

        float zoom = cam.state().zoom;
        float axisWorld = axisLength / zoom;
        float ringWorld = ringRadius / zoom;

        bx::Vec3 origin = { target.x, target.y, 0.f };

        if (mode == GizmoMode::Move || mode == GizmoMode::Scale) {
            bx::Vec3 xEnd = { target.x + axisWorld, target.y, 0.f };
            bx::Vec3 yEnd = { target.x, target.y + axisWorld, 0.f };

            dde.setColor(m_activeHandle == GizmoHandle::AxisX ? COLOR_HOVER : COLOR_X);
            dde.moveTo(origin);
            dde.lineTo(xEnd);

            dde.setColor(m_activeHandle == GizmoHandle::AxisY ? COLOR_HOVER : COLOR_Y);
            dde.moveTo(origin);
            dde.lineTo(yEnd);

            float handleWorld = handleSize / zoom;

            if (mode == GizmoMode::Move) {
                float arrowLength = handleWorld * 1.5f;
                float arrowRadius = handleWorld * 0.7f;

                dde.setColor(m_activeHandle == GizmoHandle::AxisX ? COLOR_HOVER : COLOR_X);
                bx::Vec3 xTip = { xEnd.x + arrowLength, xEnd.y, 0.f };
                dde.drawCone(xEnd, xTip, arrowRadius);

                dde.setColor(m_activeHandle == GizmoHandle::AxisY ? COLOR_HOVER : COLOR_Y);
                bx::Vec3 yTip = { yEnd.x, yEnd.y + arrowLength, 0.f };
                dde.drawCone(yEnd, yTip, arrowRadius);
            } else {
                bx::Aabb xBox; xBox.min = bx::Vec3(xEnd.x - handleWorld, xEnd.y - handleWorld, -0.5f);
                xBox.max = bx::Vec3(xEnd.x + handleWorld, xEnd.y + handleWorld,  0.5f);
                dde.setColor(m_activeHandle == GizmoHandle::ScaleX ? COLOR_HOVER : COLOR_X);
                dde.draw(xBox);

                bx::Aabb yBox; yBox.min = bx::Vec3(yEnd.x - handleWorld, yEnd.y - handleWorld, -0.5f);
                yBox.max = bx::Vec3(yEnd.x + handleWorld, yEnd.y + handleWorld,  0.5f);
                dde.setColor(m_activeHandle == GizmoHandle::ScaleY ? COLOR_HOVER : COLOR_Y);
                dde.draw(yBox);
            }

            float centerWorld = (mode == GizmoMode::Move ? handleSize * 0.7f : handleSize * 0.9f) / zoom;
            bx::Aabb centerBox;
            centerBox.min = bx::Vec3(target.x - centerWorld, target.y - centerWorld, -0.5f);
            centerBox.max = bx::Vec3(target.x + centerWorld, target.y + centerWorld,  0.5f);
            dde.setColor(
                (m_activeHandle == GizmoHandle::Center || m_activeHandle == GizmoHandle::ScaleUniform)
                    ? COLOR_HOVER : COLOR_CENTER
            );
            dde.draw(centerBox);
        }

        if (mode == GizmoMode::Rotate) {
            dde.setColor(m_activeHandle == GizmoHandle::Rotate ? COLOR_HOVER : COLOR_ROTATE);
            dde.setWireframe(true);

            constexpr int segments = 48;
            bx::Vec3 prev = { target.x + ringWorld, target.y, 0.f };
            for (int i = 1; i <= segments; ++i) {
                float a = (float)i / (float)segments * bx::kPi2;
                bx::Vec3 cur = {
                    target.x + std::cosf(a) * ringWorld,
                    target.y + std::sinf(a) * ringWorld,
                    0.f
                };
                dde.moveTo(prev);
                dde.lineTo(cur);
                prev = cur;
            }

            bx::Vec3 handlePos = {
                target.x + std::cosf(target.rotation) * ringWorld,
                target.y + std::sinf(target.rotation) * ringWorld,
                0.f
            };
            float handleWorld = handleSize / zoom;
            bx::Aabb hBox;
            hBox.min = bx::Vec3(handlePos.x - handleWorld, handlePos.y - handleWorld, -0.5f);
            hBox.max = bx::Vec3(handlePos.x + handleWorld, handlePos.y + handleWorld,  0.5f);
            dde.draw(hBox);
        }

        dde.end();
    }

    GizmoHandle TransformGizmo::hitTest(const Camera& cam, const GizmoTarget& target,
                                         float mouseX, float mouseY,
                                         uint32_t screenW, uint32_t screenH) const {
        ScreenPoint origin = toScreen(cam, target.x, target.y, screenW, screenH);

        auto dist = [&](ScreenPoint p) {
            float dx = p.x - mouseX;
            float dy = p.y - mouseY;
            return std::sqrtf(dx * dx + dy * dy);
        };

        if (mode == GizmoMode::Move || mode == GizmoMode::Scale) {
            ScreenPoint xEnd = toScreen(cam, target.x + screenLengthToWorld(cam, axisLength), target.y, screenW, screenH);
            ScreenPoint yEnd = toScreen(cam, target.x, target.y + screenLengthToWorld(cam, axisLength), screenW, screenH);

            float centerR = (mode == GizmoMode::Move ? handleSize * 0.7f : handleSize * 0.9f) + pickRadius;
            if (dist(origin) <= centerR)
                return mode == GizmoMode::Move ? GizmoHandle::Center : GizmoHandle::ScaleUniform;

            if (dist(xEnd) <= handleSize + pickRadius)
                return mode == GizmoMode::Move ? GizmoHandle::AxisX : GizmoHandle::ScaleX;

            if (dist(yEnd) <= handleSize + pickRadius)
                return mode == GizmoMode::Move ? GizmoHandle::AxisY : GizmoHandle::ScaleY;
        }

        if (mode == GizmoMode::Rotate) {
            float ringWorld = screenLengthToWorld(cam, ringRadius);
            ScreenPoint handlePos = toScreen(cam,
                target.x + std::cosf(target.rotation) * ringWorld,
                target.y + std::sinf(target.rotation) * ringWorld,
                screenW, screenH);

            if (dist(handlePos) <= handleSize + pickRadius)
                return GizmoHandle::Rotate;

            float d = dist(origin);
            float ringScreen = ringRadius * cam.state().zoom;
            if (std::fabs(d - ringScreen) <= pickRadius)
                return GizmoHandle::Rotate;
        }

        return GizmoHandle::None;
    }

    void TransformGizmo::beginDrag(GizmoHandle handle, const GizmoTarget& target, float mouseX, float mouseY) {
        m_activeHandle    = handle;
        m_dragStartTarget = target;
        m_dragStartMouseX = mouseX;
        m_dragStartMouseY = mouseY;

        if (handle == GizmoHandle::Rotate) {
            m_dragStartAngle = std::atan2f(mouseY, mouseX);
        }
    }

    void TransformGizmo::drag(GizmoTarget& target, const Camera& cam, float mouseX, float mouseY,
                               uint32_t screenW, uint32_t screenH) {
        if (m_activeHandle == GizmoHandle::None) return;

        WorldPoint startWorld = cam.screenToWorld({ m_dragStartMouseX, m_dragStartMouseY }, screenW, screenH);
        WorldPoint curWorld   = cam.screenToWorld({ mouseX, mouseY }, screenW, screenH);

        float dx = curWorld.x - startWorld.x;
        float dy = curWorld.y - startWorld.y;

        switch (m_activeHandle) {
            case GizmoHandle::Center:
                target.x = m_dragStartTarget.x + dx;
                target.y = m_dragStartTarget.y + dy;
                break;

            case GizmoHandle::AxisX:
                target.x = m_dragStartTarget.x + dx;
                break;

            case GizmoHandle::AxisY:
                target.y = m_dragStartTarget.y + dy;
                break;

            case GizmoHandle::ScaleX: {
                float factor = 1.f + dx / 100.f;
                target.scaleX = m_dragStartTarget.scaleX * factor;
                break;
            }

            case GizmoHandle::ScaleY: {
                float factor = 1.f + dy / 100.f;
                target.scaleY = m_dragStartTarget.scaleY * factor;
                break;
            }

            case GizmoHandle::ScaleUniform: {
                float dist  = std::sqrtf(dx * dx + dy * dy);
                float sign  = (dx + dy) >= 0.f ? 1.f : -1.f;
                float factor = 1.f + (dist * sign) / 100.f;
                target.scaleX = m_dragStartTarget.scaleX * factor;
                target.scaleY = m_dragStartTarget.scaleY * factor;
                break;
            }

            case GizmoHandle::Rotate: {
                float cx = m_dragStartTarget.x;
                float cy = m_dragStartTarget.y;

                ScreenPoint centerScreen = cam.worldToScreen({ cx, cy }, screenW, screenH);

                float startAngle = std::atan2f(m_dragStartMouseY - centerScreen.y, m_dragStartMouseX - centerScreen.x);
                float curAngle   = std::atan2f(mouseY - centerScreen.y, mouseX - centerScreen.x);

                target.rotation = m_dragStartTarget.rotation + (curAngle - startAngle);
                break;
            }

            default: break;
        }
    }

    void TransformGizmo::endDrag() {
        m_activeHandle = GizmoHandle::None;
    }

}