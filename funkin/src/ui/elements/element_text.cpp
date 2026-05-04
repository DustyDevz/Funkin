// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "element_text.hpp"
#include <ui/ui_renderer.hpp>

namespace Funkin::UI {
    void Text::draw() {
        if (!m_visible) return;
        UIRenderer::get().drawText(m_text, m_rect, m_color, m_fontSize, m_align);
    }
}