// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "element_panel.hpp"
#include <ui/theme.hpp>
#include <ui/ui_renderer.hpp>

namespace Funkin::UI {
    void Panel::update(Vec2 mousePos, bool mouseDown) {
        for (auto& child : m_children)
            child->update(mousePos, mouseDown);
    }

    void Panel::draw() {
        if (!m_visible) return;
        auto& t = Theme::get();
        auto& r = UIRenderer::get();

        if (m_showBorder)
            r.drawRect(m_rect, t.bgPanel, t.border);
        else
            r.drawFilledRect(m_rect, t.bgPanel);

        for (auto& child : m_children)
            child->draw();
    }
}