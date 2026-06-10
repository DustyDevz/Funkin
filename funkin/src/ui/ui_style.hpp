// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QWidget>
#include <QFile>
#include <QString>
#include "shared/log.hpp"

namespace Funkin::UI {
    class UIStyle {
    public:
        static void load(QWidget* widget, const QString& path) {
            if (!widget) {
                LOG_ERR("Cannot load style: target widget is null for path {}", path.toStdString());
                return;
            }

            QFile file(path);
            if (!file.open(QFile::ReadOnly | QFile::Text)) {
                LOG_ERR("Failed to open style file at path: {}", path.toStdString());
                return;
            }

            QByteArray data = file.readAll();
            if (data.isEmpty()) {
                LOG_WARN("Style file at path is empty: {}", path.toStdString());
            }

            widget->setStyleSheet(widget->styleSheet() + "\n" + QString::fromUtf8(data));
            file.close();

            LOG_PRINT("Successfully loaded stylesheet: {} ({} bytes) onto target: {}", 
                     path.toStdString(), 
                     data.size(), 
                     widget->objectName().isEmpty() ? "UnnamedWidget" : widget->objectName().toStdString());
        }
    };
}