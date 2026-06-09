// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_editor.hpp"

#include <QMenuBar>
#include <QMenu>
#include <QDockWidget>
#include <QTabBar>
#include <QFile>
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <windows.h>
#include <QWidget>
#include <dwmapi.h>
#include <QStatusBar>
#include <QWKWidgets/widgetwindowagent.h>
#include <bgfx/bgfx.h>

#pragma comment(lib, "dwmapi.lib")

#include "shared/log.hpp"
#include "app/project/project.hpp"

class StatusBar : public QStatusBar {
protected:
    void paintEvent(QPaintEvent* e) override {
        QPainter p(this);
        p.fillRect(rect(), QColor("#161616"));
        p.setPen(QColor("#2a2a2a"));
        p.drawLine(0, 0, width(), 0);
    }
};

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
        titleLabel->setText("Funkin Editor - " + QString::fromStdString(Funkin::App::Project::get().getName()));
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

        m_tabBar = new QTabBar(this);
        m_tabBar->setObjectName("EditorTabBar");
        m_tabBar->setDocumentMode(false);
        m_tabBar->setTabsClosable(true);
        m_tabBar->setMovable(true);
        m_tabBar->setFixedHeight(30);
        m_tabBar->setExpanding(false);
        m_tabBar->setDrawBase(false);
        
        m_tabBar->addTab("Game");
        m_tabBar->addTab("Test");
        m_tabBar->setTabButton(0, QTabBar::RightSide, nullptr);

        m_tabBar->setTabIcon(0, Funkin::UI::Icons::get("gamepad-2", QColor(0x66, 0x66, 0x66), 16));
        m_tabBar->setIconSize(QSize(16, 16));

        auto makeCloseBtn = [&](int tabIndex) {
            auto* btn = new QPushButton(m_tabBar);
            btn->setFixedSize(14, 14);
            btn->setFlat(true);
            btn->setObjectName("TabCloseBtn");
            btn->setIcon(Funkin::UI::Icons::get("x", QColor(0x88, 0x88, 0x88)));
            btn->setIconSize(QSize(10, 10));

            connect(btn, &QPushButton::clicked, this, [this, btn]() {
                for (int i = 0; i < m_tabBar->count(); i++) {
                    if (m_tabBar->tabButton(i, QTabBar::RightSide) == btn) {
                        m_tabBar->removeTab(i);
                        return;
                    }
                }
            });

            m_tabBar->setTabButton(tabIndex, QTabBar::RightSide, btn);
        };

        makeCloseBtn(1);

        connect(m_tabBar, &QTabBar::currentChanged, this, [this](int index) {
            emit activeTabChanged(m_tabBar->tabText(index));
        });
    }

    void EditorWindow::setRendererLabel(const char* name) {
        if (m_statusRenderer) m_statusRenderer->setText(name);
    }

    void EditorWindow::updateStats(float fps, float vramMB, float memMB) {
        if (m_statusFPS)    m_statusFPS->setText(QString::number((int)fps) + " FPS");
        if (m_statusVRAM)   m_statusVRAM->setText(QString::number(vramMB, 'f', 1) + " MB VRAM");
        if (m_statusMemory) m_statusMemory->setText(QString::number(memMB, 'f', 1) + " MB");
    }

    void EditorWindow::setGameViewport(QWidget* viewport) {
        auto* wrapper = new QWidget(this);
        wrapper->setObjectName("CentralWrapper");
        auto* wrapLayout = new QVBoxLayout(wrapper);
        wrapLayout->setContentsMargins(0, 0, 0, 0);
        wrapLayout->setSpacing(0);
        wrapLayout->addWidget(m_tabBar);

        m_blankWidget = new QWidget(wrapper);
        m_blankWidget->setObjectName("BlankArea");
        m_blankWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        m_blankWidget->setVisible(false);

        wrapLayout->addWidget(m_blankWidget, 1);
        wrapLayout->addWidget(viewport, 1);

        // status bar
        auto* statusBar = new FunkinStatusBar(this);
        statusBar->setObjectName("StatusBar");
        statusBar->setFixedHeight(24);
        statusBar->setSizeGripEnabled(false);
        statusBar->setStyleSheet("border: none;");

        auto makeStatusLabel = [&](const QString& text, const QString& color = "#858585") -> QLabel* {
            auto* label = new QLabel(text, statusBar);
            label->setStyleSheet(QString("color: %1; font-size: 11px; background: transparent;").arg(color));
            label->setContentsMargins(6, 0, 6, 0);
            return label;
        };

        auto makeSeparator = [&]() -> QWidget* {
            auto* sep = new QWidget(statusBar);
            sep->setFixedWidth(1);
            sep->setFixedHeight(12);
            sep->setStyleSheet("background: #3a3a3a;");
            return sep;
        };

        m_statusVersion = makeStatusLabel("v0.0.1 -dev", "#555555");
        m_statusRenderer = makeStatusLabel("...", "#555555");
        m_statusFPS = makeStatusLabel("0 FPS", "#98c379");
        m_statusMemory = makeStatusLabel("0 MB", "#858585");
        m_statusVRAM = makeStatusLabel("0 MB VRAM", "#858585");

        statusBar->addPermanentWidget(m_statusFPS);
        statusBar->addPermanentWidget(makeSeparator());
        statusBar->addPermanentWidget(m_statusMemory);
        statusBar->addPermanentWidget(makeSeparator());
        statusBar->addPermanentWidget(m_statusVRAM);
        statusBar->addPermanentWidget(makeSeparator());
        statusBar->addPermanentWidget(m_statusRenderer);
        statusBar->addPermanentWidget(makeSeparator());
        statusBar->addPermanentWidget(m_statusVersion);

        setStatusBar(statusBar);
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
        m_assets = new Funkin::UI::Editor::AssetsPanel(this);
        m_assetsDock = new QDockWidget("Scene Tree", this);
        m_assetsDock->setObjectName("SceneTreeDock");
        m_assetsDock->setWidget(m_assets);
        m_assetsDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        m_assetsDock->setMinimumWidth(200);
        m_assetsDock->setTitleBarWidget(new Funkin::UI::PanelTitleBar("Assets", m_assetsDock));
        addDockWidget(Qt::LeftDockWidgetArea, m_assetsDock);

        m_inspector = new Funkin::UI::Editor::InspectorPanel(this);
        m_inspectorDock = new QDockWidget("Inspector", this);
        m_inspectorDock->setObjectName("Inspector");
        m_inspectorDock->setWidget(m_inspector);
        m_inspectorDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        m_inspectorDock->setMinimumWidth(250);
        m_inspectorDock->setTitleBarWidget(new Funkin::UI::PanelTitleBar("Inspector", m_inspectorDock));
        addDockWidget(Qt::RightDockWidgetArea, m_inspectorDock);

        m_console = new Funkin::UI::Editor::ConsolePanel(this);
        m_consoleDock = new QDockWidget("Console", this);
        m_consoleDock->setObjectName("ConsoleDock");
        m_consoleDock->setWidget(m_console);
        m_consoleDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
        m_consoleDock->setTitleBarWidget(new Funkin::UI::PanelTitleBar("Console", m_consoleDock));
        addDockWidget(Qt::BottomDockWidgetArea, m_consoleDock);

        setCorner(Qt::BottomLeftCorner,  Qt::LeftDockWidgetArea);
        setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);
        resizeDocks({m_assetsDock, m_inspectorDock}, {220, 280}, Qt::Horizontal);
    }
}