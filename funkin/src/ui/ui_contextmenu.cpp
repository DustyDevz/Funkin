// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_contextmenu.hpp"

namespace Funkin::UI {
    ContextMenu::ContextMenu(QWidget* parent) : QMenu(parent) {
        setObjectName("ContextMenu");
    }

    QAction* ContextMenu::addItem(const QString& label, const QString& icon,
                                   const QKeySequence& shortcut,
                                   std::function<void()> callback) {
        auto* action = new QAction(label, this);
        if (!icon.isEmpty())
            action->setIcon(Icons::get(icon, QColor(0x85, 0x85, 0x85), 14));
        if (!shortcut.isEmpty())
            action->setShortcut(shortcut);
        if (callback)
            connect(action, &QAction::triggered, this, [callback]() { callback(); });
        addAction(action);
        return action;
    }

    QAction* ContextMenu::addDanger(const QString& label, const QString& icon,
                                     std::function<void()> callback) {
        auto* action = new QAction(label, this);
        if (!icon.isEmpty())
            action->setIcon(Icons::get(icon, QColor(0xe0, 0x6c, 0x75), 14));
        action->setProperty("danger", true);
        if (callback)
            connect(action, &QAction::triggered, this, [callback]() { callback(); });
        addAction(action);
        return action;
    }

    ContextMenu* ContextMenu::addSubmenu(const QString& label, const QString& icon) {
        auto* sub = new ContextMenu(this);
        sub->setTitle(label);
        if (!icon.isEmpty())
            sub->setIcon(Icons::get(icon, QColor(0x85, 0x85, 0x85), 14));
        addMenu(sub);
        return sub;
    }

    void ContextMenu::addDivider() {
        addSeparator();
    }
}