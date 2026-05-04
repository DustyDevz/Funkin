// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "element_button.hpp"
#include "element_text.hpp"
#include <ui/theme.hpp>
#include <ui/ui_renderer.hpp>

namespace Funkin::UI {
    void Button::update(Vec2 mousePos, bool mouseDown) {
        m_clicked = false;
        if (!m_enabled) { m_state = ElementState::Disabled; return; }

        bool hovered = hitTest(mousePos);
        if (!hovered) { m_state = ElementState::Normal; m_wasDown = false; return; }

        if (mouseDown) {
            m_state   = ElementState::Pressed;
            m_wasDown = true;
        } else {
            if (m_wasDown) {
                m_clicked = true;
                if (m_onClick) m_onClick();
            }
            m_wasDown = false;
            m_state   = ElementState::Hover;
        }
    }

    void Button::draw() {
        if (!m_visible) return;
        auto& t = Theme::get();
        auto& r = UIRenderer::get();

        Color bg;
        switch (m_style) {
            case ButtonStyle::Primary:
                bg = m_state == ElementState::Hover   ? t.btnBgPrimaryH :
                    m_state == ElementState::Pressed ? t.btnBgPress     : t.btnBgPrimary;
                break;
            case ButtonStyle::Ghost:
                bg = { 0, 0, 0, 0 };
                break;
            default:
                bg = m_state == ElementState::Hover   ? t.btnBgHover :
                    m_state == ElementState::Pressed ? t.btnBgPress  : t.btnBg;
                break;
        }

        r.drawRect(m_rect, bg, t.border, t.radiusMD);
        r.drawText(m_label, m_rect, t.textPrimary, t.fontSizeMD, TextAlign::Center);
    }
}