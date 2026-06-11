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

    public slots:
        void onItemSelected(const QString& path, bool isDirectory);

    private:
        void showEmpty();
        void showFolder(const QString& path);
        void showImage(const QString& path);
        void showXml(const QString& path);
        void clearContent();

        QWidget*     m_content { nullptr };
        QVBoxLayout* m_layout  { nullptr };

        QLabel* makeLabel(const QString& text, bool dim = false);
        QLabel* makeValue(const QString& text);
        QWidget* makeRow(const QString& key, const QString& value);
        QFrame* makeDivider();
    };
}