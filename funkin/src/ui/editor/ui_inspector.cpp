// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_inspector.hpp"
#include "../ui_titlebar.hpp"
#include "../ui_style.hpp"
#include "../ui_icons.hpp"
#include "cache/project_cache.hpp"

#include <QScrollArea>
#include <QFrame>
#include <QFileInfo>
#include <QImageReader>
#include <QXmlStreamReader>
#include <QFile>
#include <QSet>
#include <QPixmap>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>

#include "shared/log.hpp"

namespace Funkin::UI::Editor {
    InspectorPanel::InspectorPanel(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName("InspectorPanel");
        Funkin::UI::UIStyle::load(this, ":/ui/inspector");

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, Funkin::UI::PanelTitleBar::CONTENT_OFFSET, 0, 0);
        root->setSpacing(0);

        auto* scroll = new QScrollArea(this);
        scroll->setObjectName("InspectorScroll");
        scroll->setWidgetResizable(true);
        scroll->setFrameShape(QFrame::NoFrame);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        m_content = new QWidget(scroll);
        m_content->setObjectName("InspectorContent");
        m_layout = new QVBoxLayout(m_content);
        m_layout->setContentsMargins(8, 8, 8, 8);
        m_layout->setSpacing(4);
        m_layout->addStretch();

        scroll->setWidget(m_content);
        root->addWidget(scroll, 1);

        showEmpty();
    }

    void InspectorPanel::clearContent() {
        while (m_layout->count() > 0) {
            auto* item = m_layout->takeAt(0);
            if (item->widget()) item->widget()->deleteLater();
            delete item;
        }
    }

    QLabel* InspectorPanel::makeLabel(const QString& text, bool dim) {
        auto* label = new QLabel(text, m_content);
        label->setStyleSheet(dim
            ? "color: #555555; font-size: 11px; background: transparent;"
            : "color: #888888; font-size: 11px; background: transparent;");
        return label;
    }

    QLabel* InspectorPanel::makeValue(const QString& text) {
        auto* label = new QLabel(text, m_content);
        label->setStyleSheet("color: #cccccc; font-size: 11px; background: transparent;");
        label->setWordWrap(true);
        return label;
    }

    QWidget* InspectorPanel::makeRow(const QString& key, const QString& value) {
        auto* row = new QWidget(m_content);
        row->setStyleSheet("background: transparent;");
        auto* layout = new QHBoxLayout(row);
        layout->setContentsMargins(0, 2, 0, 2);
        layout->setSpacing(8);
        auto* keyLabel = makeLabel(key);
        keyLabel->setFixedWidth(80);
        layout->addWidget(keyLabel);
        layout->addWidget(makeValue(value), 1);
        return row;
    }

    QFrame* InspectorPanel::makeDivider() {
        auto* line = new QFrame(m_content);
        line->setFrameShape(QFrame::HLine);
        line->setStyleSheet("background: #2a2a2a; border: none; max-height: 1px;");
        return line;
    }

    void InspectorPanel::showEmpty() {
        clearContent();
        auto* label = new QLabel("Nothing selected", m_content);
        label->setAlignment(Qt::AlignCenter);
        label->setStyleSheet("color: #444444; font-size: 12px; background: transparent;");
        m_layout->addWidget(label);
        m_layout->addStretch();
    }

    void InspectorPanel::showFolder(const QString& path) {
        clearContent();

        QFileInfo info(path);
        int count = 0;
        for (auto& e : std::filesystem::directory_iterator(path.toStdString()))
            count++;

        auto* nameLabel = new QLabel(info.fileName(), m_content);
        nameLabel->setStyleSheet("color: #cccccc; font-size: 13px; font-weight: bold; background: transparent;");
        m_layout->addWidget(nameLabel);
        m_layout->addWidget(makeDivider());
        m_layout->addWidget(makeRow("Type",  "Folder"));
        m_layout->addWidget(makeRow("Items", QString::number(count)));
        m_layout->addWidget(makeRow("Path",  path));
        m_layout->addStretch();
    }

    void InspectorPanel::showImage(const QString& path) {
        clearContent();

        QFileInfo info(path);
        QImageReader reader(path);
        QSize size = reader.size();

        qint64 bytes = info.size();
        QString sizeStr = bytes < 1024
            ? QString("%1 B").arg(bytes)
            : bytes < 1024 * 1024
                ? QString("%1 KB").arg(bytes / 1024)
                : QString("%1 MB").arg(bytes / (1024 * 1024));

        auto* thumb = new QLabel(m_content);
        thumb->setAlignment(Qt::AlignCenter);
        thumb->setFixedHeight(120);
        thumb->setStyleSheet("background: #1a1a1a; border: 1px solid #2a2a2a; border-radius: 4px;");
        thumb->setScaledContents(true);
        thumb->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        m_layout->addWidget(thumb);

        auto* nameLabel = new QLabel(info.fileName(), m_content);
        nameLabel->setStyleSheet("color: #cccccc; font-size: 13px; font-weight: bold; background: transparent; margin-top: 6px;");
        m_layout->addWidget(nameLabel);
        m_layout->addWidget(makeDivider());
        m_layout->addWidget(makeRow("Type",       "PNG Image"));
        m_layout->addWidget(makeRow("Dimensions", QString("%1 × %2").arg(size.width()).arg(size.height())));
        m_layout->addWidget(makeRow("Size",       sizeStr));
        m_layout->addWidget(makeRow("Path",       path));
        m_layout->addStretch();

        auto* watcher = new QFutureWatcher<QPixmap>(this);
        connect(watcher, &QFutureWatcher<QPixmap>::finished, this, [thumb, watcher]() {
            QPixmap px = watcher->result();
            if (!px.isNull())
                thumb->setPixmap(px);
            else
                thumb->setText("No preview");
            watcher->deleteLater();
        });

        QString pathCopy = path;
        watcher->setFuture(QtConcurrent::run([pathCopy]() -> QPixmap {
            std::string filename = std::filesystem::path(pathCopy.toStdString()).filename().string();
            
            auto thumbPath = Funkin::Cache::ProjectCache::get().getThumbPath(
                std::filesystem::path(pathCopy.toStdString()));

            if (!thumbPath.empty()) {
                std::ifstream f(thumbPath, std::ios::binary);
                if (f) {
                    int tw, th;
                    f.read(reinterpret_cast<char*>(&tw), 4);
                    f.read(reinterpret_cast<char*>(&th), 4);
                    if (tw > 0 && th > 0) {
                        std::vector<uint8_t> pixels(tw * th * 4);
                        f.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
                        if (f) {
                            QImage img(pixels.data(), tw, th, QImage::Format_RGBA8888);
                            return QPixmap::fromImage(img.copy());
                        }
                    }
                }
            }

            auto cachedPath = Funkin::Cache::ProjectCache::get().getCachedPath(
                std::filesystem::path(pathCopy.toStdString()));
            if (!cachedPath.empty()) {
                std::ifstream f(cachedPath, std::ios::binary);
                if (f) {
                    uint8_t magic[4]; uint8_t version; int w, h;
                    f.read(reinterpret_cast<char*>(magic),    4);
                    f.read(reinterpret_cast<char*>(&version), 1);
                    f.read(reinterpret_cast<char*>(&w),       4);
                    f.read(reinterpret_cast<char*>(&h),       4);
                    if (magic[0]=='F' && magic[1]=='K' && magic[2]=='T' && magic[3]=='X' && w > 0 && h > 0) {
                        std::vector<uint8_t> pixels(w * h * 4);
                        f.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
                        if (f) {
                            QImage img(pixels.data(), w, h, QImage::Format_RGBA8888);
                            return QPixmap::fromImage(
                                img.scaled(200, 110, Qt::KeepAspectRatio, Qt::SmoothTransformation));
                        }
                    }
                }
            }
            
            QImageReader reader(pathCopy);
            reader.setScaledSize(QSize(200, 110));
            return QPixmap::fromImage(reader.read());
        }));
    }

    void InspectorPanel::showXml(const QString& path) {
        clearContent();

        QFileInfo info(path);
        QFile file(path);

        int frameCount = 0;
        QSet<QString> prefixes;

        if (file.open(QFile::ReadOnly)) {
            QXmlStreamReader xml(&file);
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isStartElement() && xml.name() == QStringLiteral("SubTexture")) {
                    frameCount++;
                    QString name = xml.attributes().value("name").toString();
                    QString prefix = name;
                    while (!prefix.isEmpty() && prefix.back().isDigit())
                        prefix.chop(1);
                    prefix = prefix.trimmed();
                    if (!prefix.isEmpty())
                        prefixes.insert(prefix);
                }
            }
        }

        auto* nameLabel = new QLabel(info.fileName(), m_content);
        nameLabel->setStyleSheet("color: #cccccc; font-size: 13px; font-weight: bold; background: transparent;");
        m_layout->addWidget(nameLabel);
        m_layout->addWidget(makeDivider());

        m_layout->addWidget(makeRow("Type",   "Sparrow Atlas"));
        m_layout->addWidget(makeRow("Frames", QString::number(frameCount)));
        m_layout->addWidget(makeDivider());

        auto* animLabel = new QLabel("Animations", m_content);
        animLabel->setStyleSheet("color: #888888; font-size: 11px; font-weight: bold; background: transparent; margin-top: 4px;");
        m_layout->addWidget(animLabel);

        for (auto& prefix : prefixes) {
            auto* row = new QLabel("  " + prefix, m_content);
            row->setStyleSheet("color: #aaaaaa; font-size: 11px; font-family: Consolas; background: transparent;");
            m_layout->addWidget(row);
        }

        m_layout->addWidget(makeDivider());
        m_layout->addWidget(makeRow("Path", path));
        m_layout->addStretch();
    }

    void InspectorPanel::onItemSelected(const QString& path, bool isDirectory) {
        if (path.isEmpty()) { showEmpty(); return; }

        if (isDirectory) {
            showFolder(path);
            return;
        }

        QString lower = path.toLower();
        if (lower.endsWith(".png") || lower.endsWith(".jpg") || lower.endsWith(".jpeg"))
            showImage(path);
        else if (lower.endsWith(".xml"))
            showXml(path);
        else
            showEmpty();
    }
}