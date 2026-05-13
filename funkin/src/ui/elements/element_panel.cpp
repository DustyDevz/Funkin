// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include <ui/ui_theme.hpp>
#include <drivers/d3d12/d3d12_ui/d3d12_ui.hpp>
#include "element_panel.hpp"

namespace Funkin::UI {
    void Panel::update(Math::Vec2 mousePos, bool mouseDown) {
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