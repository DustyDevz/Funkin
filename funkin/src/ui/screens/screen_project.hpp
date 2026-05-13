// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#pragma once

#include "screen.hpp"
#include <ui/elements/element_button.hpp>
#include <ui/elements/element_text.hpp>
#include <ui/elements/element_list.hpp>
#include <ui/elements/element_panel.hpp>

namespace Funkin::UI {
    class ProjectUI : public Screen {
    public:
        void init()                                override;
        void update(Math::Vec2 mousePos, bool mouseDown) override;
        void draw()                                override;
        void shutdown()                            override;
        void onResize(uint32_t w, uint32_t h)      override;

    private:
        void layout(uint32_t w, uint32_t h);

        std::shared_ptr<Panel>  m_sidebar;
        std::shared_ptr<Button> m_btnNew;
        std::shared_ptr<Button> m_btnOpenFolder;
        std::shared_ptr<Button> m_btnOpenFile;
        std::shared_ptr<Panel>  m_content;
        std::shared_ptr<Text>   m_recentLabel;
        std::shared_ptr<List>   m_recentList;

        uint32_t m_width  = 800;
        uint32_t m_height = 600;
    };
}