// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "element.hpp"
#include <math/color.hpp>
#include <string>
#include <functional>

namespace Funkin::UI {
    enum class ButtonStyle { Default, Primary, Ghost };

    class Button : public Element {
    public:
        void update(Vec2 mousePos, bool mouseDown) override;
        void draw() override;

        void setLabel(const std::string& l)       { m_label = l; }
        void setStyle(ButtonStyle s)              { m_style = s; }
        void setOnClick(std::function<void()> cb) { m_onClick = cb; }

        bool clicked() const { return m_clicked; }

    private:
        std::string           m_label;
        ButtonStyle           m_style   = ButtonStyle::Default;
        std::function<void()> m_onClick;
        bool                  m_wasDown = false;
        bool                  m_clicked = false;
    };
}