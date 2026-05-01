// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once
#include <math/rect.hpp>
#include <math/vec2.hpp>
#include <functional>

namespace Funkin::UI {
    enum class ElementState {
        Normal,
        Hover,
        Pressed,
        Disabled
    };

    class Element {
    public:
        virtual ~Element() = default;

        virtual void update(Funkin::Vec2 mousePos, bool mouseDown) = 0;
        virtual void draw()                                = 0;

        void setRect(Funkin::Rect r)        { m_rect = r; }
        void setVisible(bool v)     { m_visible = v; }
        void setEnabled(bool e)     { m_enabled = e; }

        Funkin::Rect rect()     const { return m_rect; }
        bool visible()  const { return m_visible; }
        bool enabled()  const { return m_enabled; }

    protected:
        bool hitTest(Vec2 p) const {
            return p.x >= m_rect.x && p.x <= m_rect.x + m_rect.w &&
                p.y >= m_rect.y && p.y <= m_rect.y + m_rect.h;
        }

        Funkin::Rect         m_rect{};
        ElementState m_state   = ElementState::Normal;
        bool         m_visible = true;
        bool         m_enabled = true;
    };
}