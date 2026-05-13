// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "element_text.hpp"
#include <drivers/d3d12/d3d12_ui/d3d12_ui.hpp>

namespace Funkin::UI {
    void Text::draw() {
        if (!m_visible) return;
        UIRenderer::get().drawText(m_text, m_rect, m_color, m_fontSize, m_align);
    }
}