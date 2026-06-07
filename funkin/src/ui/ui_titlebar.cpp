// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_titlebar.hpp"

namespace Funkin::UI {
    PanelTitleBar::PanelTitleBar(const QString& title, QDockWidget* parent)
        : QWidget(parent)
    {
        setFixedHeight(30);
        setContentsMargins(0, 0, 0, 0);
        setAttribute(Qt::WA_StyledBackground, true);
        setStyleSheet(
            "QWidget {"
            "  background-color: #161616;"
            "  border-bottom: 1px solid #2a2a2a;"
            "  border-top: none;"
            "  border-left: none;"
            "  border-right: none;"
            "  margin: 0px;"
            "  padding: 0px;"
            "}"
        );

        auto* layout = new QHBoxLayout(this);
        layout->setContentsMargins(8, 0, 4, 0);
        layout->setSpacing(0);

        m_title = new QLabel(title.toUpper(), this);
        m_title->setStyleSheet(
            "color: #cccccc;"
            "font-size: 11px;"
            "font-weight: 700;"
            "letter-spacing: 1px;"
            "background: transparent;"
            "border: none;"
        );

        layout->addWidget(m_title);
        layout->addStretch();
    }
}