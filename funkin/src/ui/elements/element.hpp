// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include <math/math_rect.hpp>
#include <math/math_vec2.hpp>

namespace Funkin::UI {
    enum class ElementState { Normal, Hover, Pressed, Disabled };

    class Element {
    public:
        virtual ~Element() = default;

        virtual void update(Math::Vec2 mousePos, bool mouseDown) = 0;
        virtual void draw() = 0;

        void setRect(Math::Rect r)    { m_rect = r; }
        void setVisible(bool v) { m_visible = v; }
        void setEnabled(bool e) { m_enabled = e; }

        Math::Rect rect()    const { return m_rect; }
        bool visible() const { return m_visible; }
        bool enabled() const { return m_enabled; }

        void setFocused(bool f) { m_focused = f; }
        bool isFocused()  const { return m_focused; }

    protected:
        bool hitTest(Math::Vec2 p) const {
            return p.x >= m_rect.x && p.x <= m_rect.x + m_rect.w &&
                p.y >= m_rect.y && p.y <= m_rect.y + m_rect.h;
        }

        Math::Rect         m_rect{};
        ElementState m_state   = ElementState::Normal;
        bool         m_visible = true;
        bool         m_enabled = true;
        bool         m_focused = false;
    };
}