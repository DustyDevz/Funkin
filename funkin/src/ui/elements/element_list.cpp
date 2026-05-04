// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "element_list.hpp"
#include "element_text.hpp"
#include <ui/theme.hpp>
#include <ui/ui_renderer.hpp>

namespace Funkin::UI {
    void List::update(Vec2 mousePos, bool mouseDown) {
        if (!m_enabled) return;

        m_hovered = -1;

        for (int i = 0; i < (int)m_items.size(); ++i) {
            Rect itemRect = {
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
        if (!m_visible) return;
        auto& t = Theme::get();
        auto& r = UIRenderer::get();

        for (int i = 0; i < (int)m_items.size(); ++i) {
            Rect itemRect = {
                m_rect.x,
                m_rect.y + i * m_itemHeight,
                m_rect.w,
                m_itemHeight
            };

            Color bg = t.bgItem;
            if (i == m_selected) bg = t.bgItemSelect;
            else if (i == m_hovered) bg = t.bgItemHover;

            r.drawFilledRect(itemRect, bg);
            r.drawLine(
                { itemRect.x, itemRect.y + itemRect.h },
                { itemRect.x + itemRect.w, itemRect.y + itemRect.h },
                t.border, 1.0f
            );

            r.drawText(m_items[i].title,
                { itemRect.x + t.paddingMD, itemRect.y + 10.0f, itemRect.w - t.paddingMD * 2, 20.0f },
                t.textPrimary, t.fontSizeMD, TextAlign::Left);

            if (!m_items[i].subtitle.empty()) {
                r.drawText(m_items[i].subtitle,
                    { itemRect.x + t.paddingMD, itemRect.y + 30.0f, itemRect.w - t.paddingMD * 2, 16.0f },
                    t.textSecondary, t.fontSizeSM, TextAlign::Left);
            }
        }
    }
}