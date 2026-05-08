// © 2026 Dusty | https://github.com/DustyDevz/FNFCPP
// Licensed under GNU GPL v3.0

#include "screen_project.hpp"
#include <ui/theme.hpp>
#include <ui/ui_renderer.hpp>
#include <algorithm>
#include <core/engine.hpp>
#include <platform/window/window_win32.hpp>

namespace Funkin::UI {
    void ProjectUI::init() {
        m_sidebar    = std::make_shared<Panel>();
        m_content    = std::make_shared<Panel>();
        m_btnNew     = std::make_shared<Button>();
        m_btnOpenFile = std::make_shared<Button>();
        m_recentList  = std::make_shared<List>();

        m_btnNew->setLabel("New Project");
        m_btnNew->setStyle(ButtonStyle::Primary);
        m_btnNew->setOnClick([]{ /* TODO */ });

        m_btnOpenFile->setLabel("Open .funkinProj");
        m_btnOpenFile->setOnClick([]{ /* TODO */ });

        m_recentList->addItem({ "My 111111 Mod",   "C:/Projects/111" });
        m_recentList->addItem({ "Test 11111",       "C:/Projects/Test" });
        m_recentList->addItem({ "Another Mod",      "C:/Projects/Another" });

        m_recentList->setOnSelect([](int i) { /* TODO */ });

        layout(m_width, m_height);
    }

    void ProjectUI::layout(uint32_t w, uint32_t h) {
        auto& t = Theme::get();

        const float sidebarW = 220.0f;
        const float pad      = t.paddingMD;
        const float btnW     = sidebarW - pad * 2;
        const float btnH     = 32.0f;
        const float btnX     = pad;

        m_sidebar->setRect({ 0, 0, sidebarW, (float)h });
        
        float btnBaseY = (float)h - (btnH * 2) - t.paddingSM - t.paddingMD;
        m_btnNew->setRect(      { btnX, btnBaseY,                      btnW, btnH });
        m_btnOpenFile->setRect( { btnX, btnBaseY + btnH + t.paddingSM, btnW, btnH });

        const float cx = sidebarW + 1.0f;
        const float cw = std::max(0.0f, (float)w - cx);
        m_content->setRect({ cx, 0, cw, (float)h });
        m_recentList->setRect({ cx + pad, 32.0f + 52.0f + 24.0f, std::max(0.0f, cw - pad * 2),
                                std::max(0.0f, (float)h - 32.0f - 52.0f - 24.0f - pad) });
    }

    void ProjectUI::update(Vec2 mousePos, bool mouseDown) {
        auto& r = UIRenderer::get();
        float W = (float)m_width;
        const float wbtnSize = 32.0f;
        const float titleH   = 32.0f;

        // close
        if (mouseDown &&
            mousePos.x >= W - wbtnSize && mousePos.x <= W &&
            mousePos.y >= 0 && mousePos.y <= titleH) {
            Core::Engine::get().quit();
        }

        // maximize
        if (mouseDown &&
            mousePos.x >= W - wbtnSize*2 && mousePos.x <= W - wbtnSize &&
            mousePos.y >= 0 && mousePos.y <= titleH) {
            HWND hwnd = Funkin::Platform::Window_Win32::get().hwnd();
            if (IsZoomed(hwnd))
                ShowWindow(hwnd, SW_RESTORE);
            else
                ShowWindow(hwnd, SW_MAXIMIZE);
        }

        // minimize
        if (mouseDown &&
            mousePos.x >= W - wbtnSize*3 && mousePos.x <= W - wbtnSize*2 &&
            mousePos.y >= 0 && mousePos.y <= titleH) {
            ShowWindow(Funkin::Platform::Window_Win32::get().hwnd(), SW_MINIMIZE);
        }

        m_btnNew->update(mousePos, mouseDown);
        m_btnOpenFile->update(mousePos, mouseDown);
        m_recentList->update(mousePos, mouseDown);
    }

    void ProjectUI::draw() {
        auto& r = UIRenderer::get();
        auto& t = Theme::get();
        const float W = (float)m_width;
        const float H = (float)m_height;

        r.drawFilledRect({ 0, 0, W, H }, t.bgBase);

        const float titleH = 32.0f;
        r.drawFilledRect({ 0, 0, W, titleH }, t.bgPanel);
        r.drawLine({ 0, titleH }, { W, titleH }, t.border, 1.0f);

        r.drawText("Funkin Project", { W * 0.5f - 60, 8, 120, 18 },
            t.textSecondary, t.fontSizeSM, TextAlign::Left);

        const float wbtnSize = 32.0f;
        const float wbtnY    = 0.0f;

        // close
        r.drawFilledRect({ W - wbtnSize, wbtnY, wbtnSize, titleH }, { 0,0,0,0 });
        r.drawLine({ W - 22, 11 }, { W - 10, 23 }, t.textSecondary, 1.5f);
        r.drawLine({ W - 10, 11 }, { W - 22, 23 }, t.textSecondary, 1.5f);

        // maximize
        r.drawLine({ W - wbtnSize*2 + 10, 11 }, { W - wbtnSize*2 + 22, 11 }, t.textSecondary, 1.5f);
        r.drawLine({ W - wbtnSize*2 + 22, 11 }, { W - wbtnSize*2 + 22, 23 }, t.textSecondary, 1.5f);
        r.drawLine({ W - wbtnSize*2 + 22, 23 }, { W - wbtnSize*2 + 10, 23 }, t.textSecondary, 1.5f);
        r.drawLine({ W - wbtnSize*2 + 10, 23 }, { W - wbtnSize*2 + 10, 11 }, t.textSecondary, 1.5f);

        // minimize
        r.drawLine({ W - wbtnSize*3 + 10, 17 }, { W - wbtnSize*3 + 22, 17 }, t.textSecondary, 1.5f);

        // sidebar
        const float sidebarW = 220.0f;
        r.drawFilledRect({ 0, titleH, sidebarW, H - titleH }, t.bgPanel);
        r.drawLine({ sidebarW, titleH }, { sidebarW, H }, t.border, 1.0f);

        // app name
        r.drawFilledRect({ 0, titleH, sidebarW, 56.0f }, t.bgBase);
        r.drawLine({ 0, titleH + 56.0f }, { sidebarW, titleH + 56.0f }, t.border, 1.0f);
        r.drawText("Funkin Project", { 16, titleH + 10, 188, 22 },
            t.textPrimary, t.fontSizeLG, TextAlign::Left);
        r.drawText("v0.1.0-alpha", { 16, titleH + 34, 188, 14 },
            t.textDisabled, t.fontSizeSM, TextAlign::Left);

        // nav items
        auto drawNavItem = [&](const char* label, float y, bool active) {
            float itemY = titleH + 56.0f + y;
            if (active) {
                r.drawRoundedRect({ 8.0f, itemY + 2.0f, sidebarW - 16.0f, 32.0f },
                    t.navActiveBg, t.radiusMD);
                r.drawFilledRect({ 8.0f, itemY + 8.0f, 3.0f, 20.0f }, t.navActiveBar);
            }
            r.drawText(label, { 24, itemY + 8, 180, 18 },
                active ? t.textPrimary : t.textSecondary,
                t.fontSizeMD, TextAlign::Left);
        };

        drawNavItem("Projects",  8,   true);
        drawNavItem("Community", 46,  false);

        // bottom buttons
        r.drawLine({ 0, H - (32.0f * 2) - t.paddingSM - t.paddingMD - 8 },
                   { sidebarW, H - (32.0f * 2) - t.paddingSM - t.paddingMD - 8 },
                   t.border, 1.0f);
        m_btnNew->draw();
        m_btnOpenFile->draw();

        const float cx = sidebarW + 1.0f;
        const float cw = W - cx;
        const float contentTop = titleH;

        // content header
        r.drawFilledRect({ cx, contentTop, cw, 52.0f }, t.bgPanel);
        r.drawLine({ cx, contentTop + 52.0f }, { W, contentTop + 52.0f }, t.border, 1.0f);
        r.drawText("Projects", { cx + t.paddingMD, contentTop + 14, 300, 28 },
            t.textPrimary, t.fontSizeXL, TextAlign::Left);

        // content body
        r.drawFilledRect({ cx, contentTop + 52.0f, cw, H - contentTop - 52.0f }, t.bgBase);

        // section label
        r.drawText("Recent", { cx + t.paddingMD, contentTop + 64, 60, 16 },
            t.textSecondary, t.fontSizeSM, TextAlign::Left);
        r.drawLine({ cx + t.paddingMD + 48, contentTop + 72 },
                   { W - t.paddingMD,       contentTop + 72 },
                   t.border, 1.0f);

        m_recentList->draw();
    }

    void ProjectUI::shutdown() {}

    void ProjectUI::onResize(uint32_t w, uint32_t h) {
        m_width  = w;
        m_height = h;
        layout(w, h);
    }
}
