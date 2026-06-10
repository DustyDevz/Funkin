// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once
#include <QWidget>
#include <QAbstractNativeEventFilter>
#include <QPaintEngine>
#include <functional>
#include <windows.h>

class ViewportWidget : public QWidget {
public:
    explicit ViewportWidget(QWidget* parent = nullptr);
    QPaintEngine* paintEngine() const override { return nullptr; }

protected:
    void paintEvent(QPaintEvent*) override {}
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override;
};

class RawInputFilter : public QAbstractNativeEventFilter {
public:
    std::function<void()> onRenderFrame;
    bool     windowMoving  = false;
    QWidget* viewportWidget = nullptr;

    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;
};