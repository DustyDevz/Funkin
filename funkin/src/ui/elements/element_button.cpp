// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "element_button.hpp"
#include "element_text.hpp"
#include <ui/ui_theme.hpp>
#include <drivers/d3d12/d3d12_ui/d3d12_ui.hpp>

namespace Funkin::UI {
    void Button::update(Math::Vec2 mousePos, bool mouseDown) {
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

        Math::Color bg;
        switch (m_style) {
            case ButtonStyle::Primary:
                bg = m_state == ElementState::Hover   ? t.btnBgPrimaryH :
                    m_state == ElementState::Pressed ? t.btnBgPress    : t.btnBgPrimary;
                break;
            case ButtonStyle::Ghost:
                bg = { 0, 0, 0, 0 };
                break;
            default:
                bg = m_state == ElementState::Hover   ? t.btnBgHover :
                    m_state == ElementState::Pressed ? t.btnBgPress  : t.btnBg;
                break;
        }

        r.drawRoundedRect(m_rect, bg, t.radiusMD);
        
        if (m_focused || m_state == ElementState::Hover) {
            float fw = t.focusWidth;
            r.drawLine(
                { m_rect.x - fw,           m_rect.y - fw },
                { m_rect.x + m_rect.w + fw, m_rect.y - fw },
                m_focused ? t.focusRing : Math::Color{ t.focusRing.r, t.focusRing.g, t.focusRing.b, 0.3f },
                fw);
            r.drawLine(
                { m_rect.x + m_rect.w + fw, m_rect.y - fw },
                { m_rect.x + m_rect.w + fw, m_rect.y + m_rect.h + fw },
                m_focused ? t.focusRing : Math::Color{ t.focusRing.r, t.focusRing.g, t.focusRing.b, 0.3f },
                fw);
            r.drawLine(
                { m_rect.x + m_rect.w + fw, m_rect.y + m_rect.h + fw },
                { m_rect.x - fw,             m_rect.y + m_rect.h + fw },
                m_focused ? t.focusRing : Math::Color{ t.focusRing.r, t.focusRing.g, t.focusRing.b, 0.3f },
                fw);
            r.drawLine(
                { m_rect.x - fw, m_rect.y + m_rect.h + fw },
                { m_rect.x - fw, m_rect.y - fw },
                m_focused ? t.focusRing : Math::Color{ t.focusRing.r, t.focusRing.g, t.focusRing.b, 0.3f },
                fw);
        }

        r.drawText(m_label, m_rect, t.textPrimary, t.fontSizeMD, TextAlign::Center);
    }
}