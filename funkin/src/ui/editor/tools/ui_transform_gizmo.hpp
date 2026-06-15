// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include "renderer/camera/camera.hpp"
#include <cstdint>

using namespace Funkin::Renderer::Camera;
namespace Funkin::UI::Editor::Tools {
    enum class GizmoMode {
        Move,
        Scale,
        Rotate,
    };

    enum class GizmoHandle {
        None,
        AxisX,
        AxisY,
        Center,
        ScaleX,
        ScaleY,
        ScaleUniform,
        Rotate,
    };

    struct GizmoTarget {
        float x        = 0.f;
        float y        = 0.f;
        float scaleX   = 1.f;
        float scaleY   = 1.f;
        float rotation = 0.f;
    };

    class TransformGizmo {
    public:
        void draw(uint16_t viewId, const Camera& cam, const GizmoTarget& target,
                  uint32_t screenW, uint32_t screenH) const;

        GizmoHandle hitTest(const Camera& cam, const GizmoTarget& target,
                             float mouseX, float mouseY,
                             uint32_t screenW, uint32_t screenH) const;

        void beginDrag(GizmoHandle handle, const GizmoTarget& target, float mouseX, float mouseY);
        void drag(GizmoTarget& target, const Camera& cam, float mouseX, float mouseY,
                  uint32_t screenW, uint32_t screenH);
        void endDrag();

        bool isDragging() const { return m_activeHandle != GizmoHandle::None; }
        GizmoHandle activeHandle() const { return m_activeHandle; }

        GizmoMode mode = GizmoMode::Move;

        float axisLength   = 80.f;
        float handleSize   = 8.f;
        float ringRadius   = 60.f;
        float pickRadius   = 10.f;

    private:
        GizmoHandle m_activeHandle = GizmoHandle::None;

        GizmoTarget m_dragStartTarget;
        float       m_dragStartMouseX = 0.f;
        float       m_dragStartMouseY = 0.f;
        float       m_dragStartAngle  = 0.f;
    };
}