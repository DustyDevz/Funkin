// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "element.hpp"
#include <string>
#include <vector>
#include <functional>

namespace Funkin::UI {
    struct ListItem {
        std::string title;
        std::string subtitle;
    };

    class List : public Element {
    public:
        void update(Math::Vec2 mousePos, bool mouseDown) override;
        void draw() override;

        void addItem(const ListItem& item) { m_items.push_back(item); }
        void clearItems()                  { m_items.clear(); m_selected = -1; }
        void setOnSelect(std::function<void(int)> cb) { m_onSelect = cb; }

        int  selectedIndex() const { return m_selected; }
        const ListItem* selectedItem() const {
            if (m_selected < 0 || m_selected >= (int)m_items.size()) return nullptr;
            return &m_items[m_selected];
        }

    private:
        std::vector<ListItem>    m_items;
        std::function<void(int)> m_onSelect;
        int   m_selected   = -1;
        int   m_hovered    = -1;
        float m_itemHeight = 56.0f;
        bool  m_wasDown    = false;
    };
}