// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <QMenu>
#include <QAction>
#include <QString>
#include <QKeySequence>
#include "ui_icons.hpp"

namespace Funkin::UI {
    class ContextMenu : public QMenu {
        Q_OBJECT
    public:
        explicit ContextMenu(QWidget* parent = nullptr);

        QAction* addItem(const QString& label,
                         const QString& icon = "",
                         const QKeySequence& shortcut = {},
                         std::function<void()> callback = nullptr);

        QAction* addDanger(const QString& label,
                           const QString& icon = "",
                           std::function<void()> callback = nullptr);

        ContextMenu* addSubmenu(const QString& label, const QString& icon = "");

        void addDivider();
    };
}