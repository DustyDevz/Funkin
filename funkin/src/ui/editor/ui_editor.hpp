// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QMainWindow>

class QMenuBar;
class QDockWidget;
class QPushButton;

#include <QMainWindow>
#include <QStatusBar>
#include <QPainter>
#include <QTabBar>
#include <QEvent>

#include "ui/editor/ui_assets.hpp"
#include "ui_inspector.hpp"
#include "ui_console.hpp"
#include "../ui_icons.hpp"
#include "../ui_titlebar.hpp"
#include "../ui_style.hpp"

namespace QWK {
    class WidgetWindowAgent;
}

namespace Funkin::UI::Editor {
    class AssetsPanel;
    class InspectorPanel;

    class FunkinStatusBar : public QStatusBar {
        Q_OBJECT
    public:
        explicit FunkinStatusBar(QWidget* parent = nullptr) : QStatusBar(parent) {
            setSizeGripEnabled(false);
            setFixedHeight(24);
        }
    protected:
        void paintEvent(QPaintEvent*) override {
            QPainter p(this);
            p.fillRect(rect(), QColor("#161616"));
            p.setPen(QColor("#2a2a2a"));
            p.drawLine(0, 0, width(), 0);
        }
    };

    class EditorWindow : public QMainWindow {
        Q_OBJECT

    protected:
        void paintEvent(QPaintEvent*) override {}
        void changeEvent(QEvent* event) override;

    signals:
        void activeTabChanged(const QString& tabName);

    public:
        explicit EditorWindow(QWidget* parent = nullptr);
        ~EditorWindow() override = default;

        QWidget* blankWidget() const { return m_blankWidget; }

        void setGameViewport(QWidget* viewport);
        void setRendererLabel(const char* name);
        void updateStats(float fps, float vramMB, float memMB);

        Funkin::UI::Editor::AssetsPanel*    assets()    const { return m_assets;     }
        Funkin::UI::Editor::InspectorPanel* inspector() const { return m_inspector;  }

    private:
        void buildMenuBar();
        void buildTitleBar();
        void buildCentralArea();
        void buildDocks();
        void applyTheme();

        QWidget*     m_titleBar  { nullptr };
        QMenuBar*    m_menuBar   { nullptr };
        QPushButton* m_btnMinimize { nullptr };
        QPushButton* m_btnMaximize { nullptr };
        QPushButton* m_btnClose    { nullptr };

        QLabel* m_statusVersion  { nullptr };
        QLabel* m_statusFPS      { nullptr };
        QLabel* m_statusMemory   { nullptr };
        QLabel* m_statusVRAM     { nullptr };
        QLabel* m_statusRenderer { nullptr };

        QDockWidget*                      m_assetsDock  { nullptr };
        Funkin::UI::Editor::AssetsPanel*  m_assets      { nullptr };

        QDockWidget*                        m_inspectorDock  { nullptr };
        Funkin::UI::Editor::InspectorPanel* m_inspector      { nullptr };

        QDockWidget*                      m_consoleDock { nullptr };
        Funkin::UI::Editor::ConsolePanel* m_console     { nullptr };

        QTabBar* m_tabBar { nullptr };
        QWidget* m_blankWidget  { nullptr };

        FunkinStatusBar* m_statusBar = nullptr;
        QWK::WidgetWindowAgent* m_windowAgent { nullptr };
    };
}