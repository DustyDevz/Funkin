// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_editor.hpp"
#include "ui/editor/ui_scene_tree.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QDockWidget>
#include <QFile>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <windows.h>
#include <dwmapi.h>
#include <QWKWidgets/widgetwindowagent.h>

#pragma comment(lib, "dwmapi.lib")

#include "shared/log.hpp"
#include "app/project/project.hpp"

namespace Funkin::UI::Editor {
    EditorWindow::EditorWindow(QWidget* parent)
        : QMainWindow(parent)
    {
        setObjectName("EditorWindow");
        setDockNestingEnabled(true);
        setAnimated(true);

        m_windowAgent = new QWK::WidgetWindowAgent(this);
        m_windowAgent->setup(this);

        applyTheme();
        buildTitleBar();
        buildDocks();

        HWND hwnd = (HWND)winId();
        if (hwnd) {
            BOOL dark = TRUE;
            DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
        }
    }

    void EditorWindow::applyTheme() {
        QFile styleFile(":/styles/src/ui/editor/ui_editor.qss");
        if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
            setStyleSheet(styleFile.readAll());
            styleFile.close();
        }
    }

    void EditorWindow::buildTitleBar() {
        m_titleBar = new QWidget(this);
        m_titleBar->setObjectName("CustomTitleBar");
        m_titleBar->setFixedHeight(32);

        auto* layout = new QHBoxLayout(m_titleBar);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        auto* appIcon = new QLabel(m_titleBar);
        appIcon->setObjectName("AppIcon");
        appIcon->setFixedSize(32, 32);
        appIcon->setAlignment(Qt::AlignCenter);
        QPixmap iconPx(":/styles/assets/images/icon.ico");
        if (!iconPx.isNull())
            appIcon->setPixmap(iconPx.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        layout->addWidget(appIcon);

        m_menuBar = new QMenuBar(m_titleBar);
        m_menuBar->setObjectName("EditorMenuBar");
        m_menuBar->addMenu("File");
        m_menuBar->addMenu("Edit");
        m_menuBar->addMenu("Help");
        layout->addWidget(m_menuBar);

        layout->addStretch();

        auto* centerWidget = new QWidget(m_titleBar);
        centerWidget->setAttribute(Qt::WA_TransparentForMouseEvents);
        auto* centerLayout = new QHBoxLayout(centerWidget);
        centerLayout->setContentsMargins(0, 0, 0, 0);

        auto* titleLabel = new QLabel(centerWidget);
        titleLabel->setObjectName("TitleLabel");
        titleLabel->setText("Funkin Editor");
        titleLabel->setAlignment(Qt::AlignCenter);
        titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
        centerLayout->addWidget(titleLabel);
        layout->addWidget(centerWidget, 1);
        layout->addStretch();

        auto makeBtn = [&](const QString& id, QWK::WindowAgentBase::SystemButton role, const QString& glyph) -> QPushButton* {
            auto* btn = new QPushButton(m_titleBar);
            btn->setObjectName(id);
            btn->setFixedSize(46, 32);
            btn->setFlat(true);
            btn->setText(glyph);
            m_windowAgent->setSystemButton(role, btn);
            layout->addWidget(btn);
            return btn;
        };

        m_btnMinimize = makeBtn("WinBtnMinimize", QWK::WindowAgentBase::Minimize, QString(QChar(0xE921)));
        m_btnMaximize = makeBtn("WinBtnMaximize", QWK::WindowAgentBase::Maximize, QString(QChar(0xE922)));
        m_btnClose    = makeBtn("WinBtnClose",    QWK::WindowAgentBase::Close,    QString(QChar(0xE8BB)));

        connect(m_btnMinimize, &QPushButton::clicked, this, &QMainWindow::showMinimized);
        connect(m_btnMaximize, &QPushButton::clicked, this, [this]() {
            isMaximized() ? showNormal() : showMaximized();
        });
        connect(m_btnClose, &QPushButton::clicked, this, &QMainWindow::close);

        setMenuWidget(m_titleBar);
        m_windowAgent->setTitleBar(m_titleBar);
        m_windowAgent->setHitTestVisible(m_menuBar, true);
        m_windowAgent->setHitTestVisible(appIcon, false);
    }

    void EditorWindow::changeEvent(QEvent* event) {
        QMainWindow::changeEvent(event);
        if (event->type() == QEvent::WindowStateChange && m_btnMaximize) {
            m_btnMaximize->setText(
                isMaximized()
                    ? QString(QChar(0xE923))
                    : QString(QChar(0xE922))
            );
        }
    }

    void EditorWindow::buildDocks() {
        m_sceneTree = new Funkin::UI::Editor::SceneTreePanel(this);

        m_sceneTreeDock = new QDockWidget("Scene Tree", this);
        m_sceneTreeDock->setObjectName("SceneTreeDock");
        m_sceneTreeDock->setWidget(m_sceneTree);
        m_sceneTreeDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        addDockWidget(Qt::LeftDockWidgetArea, m_sceneTreeDock);
    }
}