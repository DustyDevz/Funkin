// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "element.hpp"
#include <math/color.hpp>
#include <string>

namespace Funkin::UI {
    enum class TextAlign { Left, Center, Right };

    class Text : public Element {
    public:
        void update(Vec2, bool) override {}
        void draw() override;

        void setText(const std::string& t) { m_text = t; }
        void setColor(Color c)             { m_color = c; }
        void setFontSize(float s)          { m_fontSize = s; }
        void setAlign(TextAlign a)         { m_align = a; }
        void setBold(bool b)               { m_bold = b; }

        const std::string& text() const { return m_text; }

    private:
        std::string m_text;
        Color       m_color    = { 0.92f, 0.92f, 0.92f, 1.0f };
        float       m_fontSize = 13.0f;
        TextAlign   m_align    = TextAlign::Left;
        bool        m_bold     = false;
    };
}