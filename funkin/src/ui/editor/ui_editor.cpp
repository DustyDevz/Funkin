// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_editor.hpp"
#include "ui/editor/ui_scene_tree.hpp"
#include "ui_inspector.hpp"
#include "../ui_icons.hpp"
#include "../ui_titlebar.hpp"
#include "../ui_style.hpp"

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
#include <QWidget>
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

        Funkin::UI::UIStyle::load(this, ":/ui/editor");
        buildTitleBar();
        buildDocks();

        HWND hwnd = (HWND)winId();
        if (hwnd) {
            BOOL dark = TRUE;
            DwmSetWindowAttribute(hwnd, 20, &dark, sizeof(dark));
        }
    }

    void EditorWindow::buildTitleBar() {
        m_titleBar = new QWidget(this);
        m_titleBar->setObjectName("CustomTitleBar");
        m_titleBar->setFixedHeight(32);

        auto* outerLayout = new QVBoxLayout(m_titleBar);
        outerLayout->setContentsMargins(0, 0, 0, 0);
        outerLayout->setSpacing(0);

        auto* titleRow = new QWidget(m_titleBar);
        titleRow->setFixedHeight(32);
        auto* layout = new QHBoxLayout(titleRow);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        auto* appIcon = new QLabel(titleRow);
        appIcon->setObjectName("AppIcon");
        appIcon->setFixedSize(32, 32);
        appIcon->setAlignment(Qt::AlignCenter);
        QPixmap iconPx(":/ui/icon");
        if (!iconPx.isNull())
            appIcon->setPixmap(iconPx.scaled(16, 16, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        layout->addWidget(appIcon);

        m_menuBar = new QMenuBar(titleRow);
        m_menuBar->setObjectName("EditorMenuBar");
        m_menuBar->addMenu("File");
        m_menuBar->addMenu("Edit");
        m_menuBar->addMenu("Help");
        layout->addWidget(m_menuBar);

        layout->addStretch();

        auto* centerWidget = new QWidget(titleRow);
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
            auto* btn = new QPushButton(titleRow);
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

        outerLayout->addWidget(titleRow);

        setMenuWidget(m_titleBar);
        m_windowAgent->setTitleBar(m_titleBar);
        m_windowAgent->setHitTestVisible(m_menuBar, true);
        m_windowAgent->setHitTestVisible(appIcon, false);

        m_tabWidget = new QTabWidget(this);
        m_tabWidget->setObjectName("EditorTabWidget");
        m_tabWidget->setDocumentMode(true);
        m_tabWidget->setTabsClosable(true);
        m_tabWidget->setMovable(true);
        m_tabWidget->setFixedHeight(30);

        m_tabWidget->addTab(new QWidget(), "Game");
        m_tabWidget->addTab(new QWidget(), "Test");
        m_tabWidget->tabBar()->setTabButton(0, QTabBar::RightSide, nullptr); // 0 = game

        auto makeCloseBtn = [&](int tabIndex) {
            auto* btn = new QPushButton(m_tabWidget);
            btn->setFixedSize(14, 14);
            btn->setFlat(true);
            btn->setObjectName("TabCloseBtn");
            btn->setIcon(Funkin::UI::Icons::get("x", QColor(0x88, 0x88, 0x88)));
            btn->setIconSize(QSize(10, 10));

            connect(btn, &QPushButton::clicked, this, [this, btn]() {
                for (int i = 0; i < m_tabWidget->count(); i++) {
                    if (m_tabWidget->tabBar()->tabButton(i, QTabBar::RightSide) == btn) {
                        m_tabWidget->removeTab(i);
                        return;
                    }
                }
            });

            m_tabWidget->tabBar()->setTabButton(tabIndex, QTabBar::RightSide, btn);
        };

        makeCloseBtn(1);

        connect(m_tabWidget, &QTabWidget::tabCloseRequested, m_tabWidget, &QTabWidget::removeTab);
        connect(m_tabWidget, &QTabWidget::currentChanged, this, [this](int index) {
            emit activeTabChanged(m_tabWidget->tabText(index));
        });
    }

    void EditorWindow::setGameViewport(QWidget* viewport) {
        auto* wrapper = new QWidget(this);
        wrapper->setObjectName("CentralWrapper");
        auto* wrapLayout = new QVBoxLayout(wrapper);
        wrapLayout->setContentsMargins(0, 0, 0, 0);
        wrapLayout->setSpacing(0);
        wrapLayout->addWidget(m_tabWidget);

        m_blankWidget = new QWidget(wrapper);
        m_blankWidget->setObjectName("BlankArea");
        m_blankWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        wrapLayout->addWidget(m_blankWidget, 1);
        wrapLayout->addWidget(viewport, 1);

        setCentralWidget(wrapper);
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
        m_sceneTreeDock->setMinimumWidth(200);
        m_sceneTreeDock->setTitleBarWidget(new Funkin::UI::PanelTitleBar("Scene Tree", m_sceneTreeDock));
        addDockWidget(Qt::LeftDockWidgetArea, m_sceneTreeDock);

        m_inspector = new Funkin::UI::Editor::InspectorPanel(this);
        m_inspectorDock = new QDockWidget("Inspector", this);
        m_inspectorDock->setObjectName("Inspector");
        m_inspectorDock->setWidget(m_inspector);
        m_inspectorDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        m_inspectorDock->setMinimumWidth(250);
        m_inspectorDock->setTitleBarWidget(new Funkin::UI::PanelTitleBar("Inspector", m_inspectorDock));
        addDockWidget(Qt::RightDockWidgetArea, m_inspectorDock);

        resizeDocks({m_sceneTreeDock, m_inspectorDock}, {220, 280}, Qt::Horizontal);
    }
}