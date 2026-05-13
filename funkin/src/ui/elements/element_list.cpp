// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include <ui/ui_theme.hpp>
#include <drivers/d3d12/d3d12_ui/d3d12_ui.hpp>
#include "element_list.hpp"
#include "element_text.hpp"

namespace Funkin::UI {
    void List::update(Math::Vec2 mousePos, bool mouseDown) {
        if (!m_enabled) return;

        m_hovered = -1;

        for (int i = 0; i < (int)m_items.size(); ++i) {
            Math::Rect itemRect = {
                m_rect.x,
                m_rect.y + i * m_itemHeight,
                m_rect.w,
                m_itemHeight
            };

            if (mousePos.x >= itemRect.x && mousePos.x <= itemRect.x + itemRect.w &&
                mousePos.y >= itemRect.y && mousePos.y <= itemRect.y + itemRect.h) {
                m_hovered = i;

                if (!mouseDown && m_wasDown) {
                    m_selected = i;
                    if (m_onSelect) m_onSelect(i);
                }
            }
        }

        m_wasDown = mouseDown;
    }

   void List::draw() {
    auto& r = UIRenderer::get();
    auto& t = Theme::get();

    float y = m_rect.y;

    for (int i = 0; i < (int)m_items.size(); ++i) {
        bool hovered  = (i == m_hovered);
        bool selected = (i == m_selected);

        Math::Color baseBg = (i % 2 == 0) ? t.bgItem
                                     : Math::Color{ t.bgItem.r - 0.03f,
                                              t.bgItem.g - 0.03f,
                                              t.bgItem.b - 0.03f, 1.0f };
        Math::Color bg = selected ? t.bgItemSelect
                 : hovered  ? t.bgItemHover
                            : baseBg;

        r.drawRoundedRect({ m_rect.x, y + 2.0f, m_rect.w, m_itemHeight - 4.0f }, bg, t.radiusMD);

        if (selected) {
            r.drawLine({ m_rect.x, y + 4.0f },
                       { m_rect.x, y + m_itemHeight - 4.0f },
                       t.accent, 3.0f);
        }

        r.drawText(m_items[i].title,    { m_rect.x + 16, y + 14, m_rect.w - 32, 20 },
            t.textPrimary,   t.fontSizeMD, TextAlign::Left);
        r.drawText(m_items[i].subtitle, { m_rect.x + 16, y + 38, m_rect.w - 32, 16 },
            t.textSecondary, t.fontSizeSM, TextAlign::Left);

        y += m_itemHeight;
    }
}
}