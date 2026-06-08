// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_icons.hpp"
#include <QSvgRenderer>
#include <QImage>
#include <QPainter>
#include <QHash>
#include <QPixmap>

namespace Funkin::UI::Icons {
    QIcon get(const QString& iconId, const QColor& color, int size) {
        QString cacheKey = iconId + color.name() + QString::number(size);
        static QHash<QString, QIcon> cache;
        if (cache.contains(cacheKey))
            return cache.value(cacheKey);

        QString path = QStringLiteral(":/icons/") + iconId;
        QSvgRenderer renderer(path);
        if (!renderer.isValid()) return QIcon();

        QImage img(size, size, QImage::Format_ARGB32_Premultiplied);
        img.fill(Qt::transparent);

        QPainter p(&img);
        p.setRenderHint(QPainter::Antialiasing);
        renderer.render(&p);
        p.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        p.fillRect(img.rect(), color);
        p.end();

        QIcon icon(QPixmap::fromImage(img));
        cache.insert(cacheKey, icon);
        return icon;
    }
}