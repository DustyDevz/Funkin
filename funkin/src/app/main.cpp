// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include <QApplication>
#include <QGuiApplication>
#include "app/app.hpp"

int main(int argc, char** argv) {
    QGuiApplication::setAttribute(Qt::AA_DontCreateNativeWidgetSiblings);
    QApplication qtApp(argc, argv);
    return Funkin::App::run(argc, argv, qtApp);
}