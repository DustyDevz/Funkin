// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "screen_project.hpp"
#include <ui/theme.hpp>
#include <ui/ui_renderer.hpp>

namespace Funkin::UI {
    void ProjectUI::init() {
        m_sidebar       = std::make_shared<Panel>();
        m_content       = std::make_shared<Panel>();
        m_btnNew        = std::make_shared<Button>();
        m_btnOpenFolder = std::make_shared<Button>();
        m_btnOpenFile   = std::make_shared<Button>();
        m_recentLabel   = std::make_shared<Text>();
        m_recentList    = std::make_shared<List>();

        m_btnNew->setLabel("New Project");
        m_btnNew->setStyle(ButtonStyle::Primary);
        m_btnNew->setOnClick([]{ /* TODO: hi :3 */ });

        m_btnOpenFolder->setLabel("Open Folder");
        m_btnOpenFolder->setOnClick([]{ /* TODO: hi :3 */ });

        m_btnOpenFile->setLabel("Open .project");
        m_btnOpenFile->setOnClick([]{ /* TODO: hi :3 */ });

        m_recentLabel->setText("Recent Projects");
        m_recentLabel->setFontSize(Theme::get().fontSizeSM);
        m_recentLabel->setColor(Theme::get().textSecondary);

        m_recentList->addItem({ "My FNF Mod",   "C:/Projects/MyFNFMod" });
        m_recentList->addItem({ "Test Project", "C:/Projects/Test" });

        m_recentList->setOnSelect([](int i) {
            // TODO: hi :3
        });

        layout(m_width, m_height);
    }

    void ProjectUI::layout(uint32_t w, uint32_t h) {
        auto& t = Theme::get();

        const float sidebarW = 200.0f;
        const float pad      = t.paddingMD;
        const float btnW     = sidebarW - pad * 2;
        const float btnH     = 36.0f;
        const float btnX     = pad;

        m_sidebar->setRect({ 0, 0, sidebarW, (float)h });
        m_btnNew->setRect(       { btnX, 80.0f,               btnW, btnH });
        m_btnOpenFolder->setRect({ btnX, 80.0f + btnH + 8,    btnW, btnH });
        m_btnOpenFile->setRect(  { btnX, 80.0f + (btnH+8)*2,  btnW, btnH });

        const float cx = sidebarW + 1.0f;
        const float cw = (float)w - cx;
        m_content->setRect({ cx, 0, cw, (float)h });
        m_recentLabel->setRect({ cx + pad, 20.0f,  cw - pad*2, 20.0f });
        m_recentList->setRect({ cx + pad,  46.0f,  cw - pad*2, (float)h - 46.0f - pad });
    }

    void ProjectUI::update(Vec2 mousePos, bool mouseDown) {
        m_btnNew->update(mousePos, mouseDown);
        m_btnOpenFolder->update(mousePos, mouseDown);
        m_btnOpenFile->update(mousePos, mouseDown);
        m_recentList->update(mousePos, mouseDown);
    }

    void ProjectUI::draw() {
        auto& r = UIRenderer::get();
        auto& t = Theme::get();

        // background
        r.drawFilledRect({ 0, 0, (float)m_width, (float)m_height }, t.bgBase);

        // sidebar
        r.drawFilledRect({ 0, 0, 200.0f, (float)m_height }, t.bgPanel);
        r.drawLine({ 200, 0 }, { 200, (float)m_height }, t.border, 1.0f);

        // sidebar header
        r.drawText("bla bla", { 16, 20, 168, 30 },
                t.textPrimary, t.fontSizeXL, TextAlign::Left);
        r.drawText("alb alb", { 16, 48, 168, 18 },
                t.textAccent, t.fontSizeSM, TextAlign::Left);
        r.drawLine({ 0, 72 }, { 200, 72 }, t.border, 1.0f);

        m_btnNew->draw();
        m_btnOpenFolder->draw();
        m_btnOpenFile->draw();

        // content
        r.drawFilledRect({ 201, 0, (float)m_width - 201, (float)m_height }, t.bgBase);
        m_recentLabel->draw();
        m_recentList->draw();
    }

    void ProjectUI::shutdown() {}

    void ProjectUI::onResize(uint32_t w, uint32_t h) {
        m_width  = w;
        m_height = h;
        layout(w, h);
    }
}