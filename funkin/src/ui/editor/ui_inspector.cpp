#include "ui_inspector.hpp"
#include "../ui_titlebar.hpp"

#include <QApplication>
#include <QHeaderView>
#include <QSizePolicy>
#include <QMouseEvent>
#include <QPixmap>
#include <QImage>
#include <QPainterPath>
#include <QDockWidget>
#include <QWindow>
#include <QSvgRenderer>
#include <QHash>
#include <QXmlStreamReader>
#include <QFile>
#include <QLabel>

#include "shared/log.hpp"
#include "app/project/project.hpp"

namespace Funkin::UI::Editor {
    InspectorPanel::InspectorPanel(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName("Inspector");

        // QFile styleFile(":/ui/inspector");
        // if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        //     setStyleSheet(QLatin1String(styleFile.readAll()));
        //     styleFile.close();
        // }
    }

    void InspectorPanel::onItemSelected(const QString& path, bool isDirectory)
    {}

    void InspectorPanel::showEmpty()
    {}

    void InspectorPanel::showFolder(const QString& path)
    {}
}