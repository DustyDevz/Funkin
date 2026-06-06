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
    class SceneTreePanel;
    class EditorWindow : public QMainWindow {
        Q_OBJECT
    
    protected:
        void changeEvent(QEvent* event) override;

    public:
        explicit EditorWindow(QWidget* parent = nullptr);
        ~EditorWindow() override = default;

        Funkin::UI::Editor::SceneTreePanel* sceneTree() const { return m_sceneTree; }

    private:
        void buildMenuBar();
        void buildTitleBar();
        void buildDocks();
        void applyTheme();

        QWidget* m_titleBar { nullptr };
        QMenuBar* m_menuBar { nullptr };
        QPushButton* m_btnMinimize { nullptr };
        QPushButton* m_btnMaximize { nullptr };
        QPushButton* m_btnClose { nullptr };

        QDockWidget* m_sceneTreeDock { nullptr };
        Funkin::UI::Editor::SceneTreePanel* m_sceneTree { nullptr };

        QWK::WidgetWindowAgent* m_windowAgent { nullptr };
    };
}