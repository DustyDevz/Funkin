// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QMainWindow>

class QMenuBar;
class QDockWidget;
class QPushButton;

namespace QWK {
    class WidgetWindowAgent;
}

namespace Funkin::UI::Editor {
    class AssetsPanel;
    class InspectorPanel;

    class EditorWindow : public QMainWindow {
        Q_OBJECT

    protected:
        void changeEvent(QEvent* event) override;

    signals:
        void activeTabChanged(const QString& tabName);

    public:
        explicit EditorWindow(QWidget* parent = nullptr);
        ~EditorWindow() override = default;

        QWidget* blankWidget() const { return m_blankWidget; }
        void setGameViewport(QWidget* viewport);
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

        QDockWidget*                      m_assetsDock  { nullptr };
        Funkin::UI::Editor::AssetsPanel*  m_assets      { nullptr };

        QDockWidget*                        m_inspectorDock  { nullptr };
        Funkin::UI::Editor::InspectorPanel* m_inspector      { nullptr };

        QTabBar* m_tabBar { nullptr };
        QWidget* m_blankWidget  { nullptr };

        QWK::WidgetWindowAgent* m_windowAgent { nullptr };
    };
}