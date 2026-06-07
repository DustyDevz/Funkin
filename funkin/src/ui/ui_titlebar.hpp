// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QWidget>
#include <QLabel>
#include <QHBoxLayout>
#include <QDockWidget>

namespace Funkin::UI {
    class PanelTitleBar : public QWidget {
        Q_OBJECT

    public:
        static constexpr int CONTENT_OFFSET = 16; // I really don't know a better solution D:
        explicit PanelTitleBar(const QString& title, QDockWidget* parent = nullptr);

    private:
        QLabel* m_title = nullptr;
    };
}