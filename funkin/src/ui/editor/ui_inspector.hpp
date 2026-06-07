// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <QDockWidget>
#include <filesystem>
#include <string>

namespace Funkin::UI::Editor {
    class InspectorPanel : public QWidget {
        Q_OBJECT

    public:
        explicit InspectorPanel(QWidget* parent = nullptr);

    signals:
        void inspectorCleared();

    public slots:
        void onItemSelected(const QString& path, bool isDirectory);

    private:
        void showEmpty();
        void showFolder(const QString& path);

        QScrollArea* m_scrollArea    { nullptr };
        QWidget*     m_contentWidget { nullptr };
        QVBoxLayout* m_contentLayout { nullptr };
    };
}