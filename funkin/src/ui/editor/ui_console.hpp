// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QTreeWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include "../ui_log.hpp"

namespace Funkin::UI::Editor {
    class ConsolePanel : public QWidget {
        Q_OBJECT
    public:
        explicit ConsolePanel(QWidget* parent = nullptr);
        void appendEntry(const ConsoleEntry& e);

    private:
        void buildToolbar();
        void applyFilter();
        void updateCounts();

        QTreeWidget* m_tree    { nullptr };
        QLineEdit*   m_filter  { nullptr };
        QPushButton* m_btnInfo  { nullptr };
        QPushButton* m_btnWarn  { nullptr };
        QPushButton* m_btnError { nullptr };
        QPushButton* m_btnLock  { nullptr };

        bool m_showInfo  = true;
        bool m_showWarn  = true;
        bool m_showError = true;
    };
}