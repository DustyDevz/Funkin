// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "element.hpp"

namespace Funkin::UI {
    class Panel : public Element {
    public:
        void update(Math::Vec2 mousePos, bool mouseDown) override;
        void draw() override;

        void addChild(std::shared_ptr<Element> e) { m_children.push_back(e); }
        void clearChildren()                       { m_children.clear(); }
        void setShowBorder(bool b)                 { m_showBorder = b; }

    private:
        std::vector<std::shared_ptr<Element>> m_children;
        bool m_showBorder = true;
    };
}