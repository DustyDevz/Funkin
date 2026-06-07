// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_scene_tree.hpp"
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
    FunkinTreeWidget::FunkinTreeWidget(QWidget* parent)
        : QTreeWidget(parent)
    {
        setMouseTracking(true);
    }

    void FunkinTreeWidget::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const {
        QTreeWidgetItem* item = itemFromIndex(index);
        if (!item) return;

        painter->save();

        if (item->childCount() > 0) {
            const bool open = item->isExpanded();
            const int cellW = indentation();
            const int cellX = rect.right() - cellW;
            const QRect cell(cellX, rect.top(), cellW, rect.height());

            const float cx = cell.x() + cell.width()  * 0.5f;
            const float cy = cell.y() + cell.height() * 0.5f;

            const QPoint cursorVp = viewport()->mapFromGlobal(QCursor::pos());
            const QRect  rowRect  = visualItemRect(item);
            const bool   rowHover = rowRect.contains(cursorVp);
            const bool   selected = item->isSelected();

            const QColor arrowColor = (rowHover || selected)
                ? QColor(0xcc, 0xcc, 0xcc)
                : QColor(0x70, 0x70, 0x70);

            painter->setRenderHint(QPainter::Antialiasing, true);
            QPen pen(arrowColor);
            pen.setWidthF(1.3f);
            pen.setCapStyle(Qt::RoundCap);
            pen.setJoinStyle(Qt::RoundJoin);
            painter->setPen(pen);
            painter->setBrush(Qt::NoBrush);

            const float s = 3.0f;

            if (open) {
                QPointF pts[3] = {
                    { cx - s,        cy - s * 0.4f },
                    { cx,            cy + s * 0.7f },
                    { cx + s,        cy - s * 0.4f }
                };
                painter->drawPolyline(pts, 3);
            } else {
                QPointF pts[3] = {
                    { cx - s * 0.4f, cy - s        },
                    { cx + s * 0.7f, cy            },
                    { cx - s * 0.4f, cy + s        }
                };
                painter->drawPolyline(pts, 3);
            }
        }

        painter->restore();
    }

    void FunkinTreeWidget::enterEvent(QEnterEvent* event) {
        QTreeWidget::enterEvent(event);
        m_isHovered = true;
        viewport()->update();
    }

    void FunkinTreeWidget::leaveEvent(QEvent* event) {
        QTreeWidget::leaveEvent(event);
        m_isHovered = false;
        viewport()->update();
    }

    SceneTreePanel::SceneTreePanel(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName("SceneTreePanel");

        QFile styleFile(":/styles/src/ui/editor/ui_scene_tree.qss");
        if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
            setStyleSheet(QLatin1String(styleFile.readAll()));
            styleFile.close();
        }

        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, Funkin::UI::PanelTitleBar::CONTENT_OFFSET, 0, 0);
        root->setSpacing(0);

        auto* filterWrapper = new QWidget(this);
        filterWrapper->setFixedHeight(24);
        filterWrapper->setStyleSheet("background-color: #161616;");
        auto* filterRow = new QHBoxLayout(filterWrapper);
        filterRow->setContentsMargins(8, 1, 8, 1);
        filterRow->setSpacing(0);

        auto* searchIconLabel = new QLabel(filterWrapper);
        searchIconLabel->setPixmap(getIcon("search", QColor(0x6b, 0x6b, 0x6b)).pixmap(12, 12));
        searchIconLabel->setFixedSize(16, 16);
        searchIconLabel->setStyleSheet("background: transparent;");
        filterRow->addWidget(searchIconLabel);

        m_filterEdit = new QLineEdit(filterWrapper);
        m_filterEdit->setObjectName("FilterEdit");
        m_filterEdit->setPlaceholderText("Filter assets...");
        filterRow->addWidget(m_filterEdit);

        root->addWidget(filterWrapper);

        m_tree = new FunkinTreeWidget(this);
        m_tree->setObjectName("SceneTree");
        m_tree->setHeaderHidden(true);
        m_tree->setColumnCount(1);
        m_tree->setAlternatingRowColors(false);
        m_tree->setIndentation(14);
        m_tree->setRootIsDecorated(true);
        m_tree->setAnimated(false);
        m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
        m_tree->setUniformRowHeights(true);
        m_tree->setIconSize(QSize(14, 14));
        m_tree->header()->setStretchLastSection(true);
        root->addWidget(m_tree, 1);

        loadIcons();

        m_tree->viewport()->installEventFilter(this);

        connect(m_filterEdit, &QLineEdit::textChanged, this, &SceneTreePanel::onFilterChanged);
        connect(m_tree, &QTreeWidget::itemClicked, this, &SceneTreePanel::onItemClicked);
        connect(m_tree, &QTreeWidget::itemDoubleClicked, this, &SceneTreePanel::onItemDoubleClicked);
        connect(m_tree, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem* item) {
            if (item->data(0, Qt::UserRole + 1).toBool())
                item->setIcon(0, getIcon("folder-open", QColor(0xdc, 0xb6, 0x7a)));
        });
        connect(m_tree, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem* item) {
            if (item->data(0, Qt::UserRole + 1).toBool())
                item->setIcon(0, getIcon("folder", QColor(0xdc, 0xb6, 0x7a)));
        });
        connect(m_tree, &QTreeWidget::customContextMenuRequested, this, &SceneTreePanel::onContextMenu);

        refresh();
    }

    void SceneTreePanel::loadIcons() {
        m_iconFolder     = getIcon("folder",      QColor(0xdc, 0xb6, 0x7a));
        m_iconFolderOpen = getIcon("folder-open", QColor(0xdc, 0xb6, 0x7a));
        m_iconFile       = getIcon("file",        QColor(0x85, 0x85, 0x85));
        m_iconScene      = getIcon("file-code",   QColor(0x8f, 0xa1, 0xb3));
        m_iconAudio      = getIcon("file-music",  QColor(0x85, 0x85, 0x85));
        m_iconImage      = getIcon("image",       QColor(0x85, 0x85, 0x85));
        m_iconScript     = getIcon("file",        QColor(0x85, 0x85, 0x85));
    }

    bool SceneTreePanel::eventFilter(QObject* obj, QEvent* event) {
        if (obj == m_tree->viewport()) {
            if (event->type() == QEvent::MouseButtonPress) {
                auto* me = static_cast<QMouseEvent*>(event);
                if (!m_tree->indexAt(me->pos()).isValid()) {
                    m_tree->clearSelection();
                    m_tree->setCurrentItem(nullptr);
                }
            }
        }
        return QWidget::eventFilter(obj, event);
    }

    QIcon SceneTreePanel::iconForPath(const std::filesystem::path&, bool isDir) const {
        return isDir ? m_iconFolder : m_iconFile;
    }

    void SceneTreePanel::refresh() {
        m_tree->clear();

        const std::filesystem::path& rootPath = Funkin::App::Project::get().getRoot();
        if (rootPath.empty()) {
            auto* ph = new QTreeWidgetItem(m_tree);
            ph->setText(0, "No project loaded");
            ph->setForeground(0, QColor("#6b6b6b"));
            ph->setFlags(Qt::NoItemFlags);
            return;
        }

        if (!std::filesystem::exists(rootPath))
            return;

        populateNode(rootPath, nullptr);

        for (int i = 0; i < m_tree->topLevelItemCount(); ++i)
            m_tree->topLevelItem(i)->setExpanded(true);

        if (!m_filterEdit->text().isEmpty())
            onFilterChanged(m_filterEdit->text());
    }

    QIcon SceneTreePanel::getIcon(const QString& iconId, const QColor& color) {
        QString cacheKey = iconId + color.name();
        static QHash<QString, QIcon> iconCache;
        if (iconCache.contains(cacheKey))
            return iconCache.value(cacheKey);

        QString rccPath = QStringLiteral(":/icons/assets/images/icons/") + iconId + QStringLiteral(".svg");
        QSvgRenderer renderer(rccPath);

        if (!renderer.isValid())
            return QIcon();

        QImage baseImage(24, 24, QImage::Format_ARGB32_Premultiplied);
        baseImage.fill(Qt::transparent);

        QPainter imgPainter(&baseImage);
        imgPainter.setRenderHint(QPainter::Antialiasing, true);
        renderer.render(&imgPainter);
        imgPainter.setCompositionMode(QPainter::CompositionMode_SourceAtop);
        imgPainter.fillRect(baseImage.rect(), color);
        imgPainter.end();

        QIcon finalizedIcon(QPixmap::fromImage(baseImage));
        iconCache.insert(cacheKey, finalizedIcon);
        return finalizedIcon;
    }

    void SceneTreePanel::populateNode(const std::filesystem::path& dir, QTreeWidgetItem* parentItem) {
        std::vector<std::filesystem::directory_entry> dirs, files;
        std::error_code ec;

        for (auto& e : std::filesystem::directory_iterator(dir, ec)) {
            if (ec) { ec.clear(); continue; }
            const auto name = e.path().filename().string();
            if (!name.empty() && name[0] == '.') continue;
            if (name == "project.funkin") continue;
            if (e.is_directory(ec)) dirs.push_back(e);
            else                    files.push_back(e);
        }

        auto byName = [](const auto& a, const auto& b) {
            return a.path().filename().string() < b.path().filename().string();
        };
        std::sort(dirs.begin(),  dirs.end(),  byName);
        std::sort(files.begin(), files.end(), byName);

        auto makeItem = [&](const std::filesystem::path& p, bool isDir) {
            auto* item = parentItem
                ? new QTreeWidgetItem(parentItem)
                : new QTreeWidgetItem(m_tree);

            item->setText(0, QString::fromStdString(p.filename().string()));

            QIcon itemIcon;
            if (isDir) {
                itemIcon = getIcon("folder", QColor(0xdc, 0xb6, 0x7a));
            } else {
                const std::string ext = p.extension().string();

                struct IconSpec { QString id; QColor color; };
                static const std::unordered_map<std::string, IconSpec> extMap = {
                    { ".funkin", { "file-code",  QColor(0x8f, 0xa1, 0xb3) } },
                    { ".png",    { "image",      QColor(0x85, 0x85, 0x85) } },
                    { ".jpg",    { "image",      QColor(0x85, 0x85, 0x85) } },
                    { ".jpeg",   { "image",      QColor(0x85, 0x85, 0x85) } },
                    { ".ogg",    { "file-music", QColor(0x85, 0x85, 0x85) } },
                    { ".mp3",    { "file-music", QColor(0x85, 0x85, 0x85) } },
                    { ".wav",    { "file-music", QColor(0x85, 0x85, 0x85) } },
                    { ".json",   { "file-json",  QColor(0x85, 0x85, 0x85) } },
                    { ".xml",    { "code-xml",   QColor(0x85, 0x85, 0x85) } },
                    { ".glsl",   { "file-code",  QColor(0x85, 0x85, 0x85) } },
                    { ".frag",   { "file-code",  QColor(0x85, 0x85, 0x85) } },
                    { ".vert",   { "file-code",  QColor(0x85, 0x85, 0x85) } },
                };

                auto it = extMap.find(ext);
                itemIcon = (it != extMap.end())
                    ? getIcon(it->second.id, it->second.color)
                    : getIcon("file", QColor(0x85, 0x85, 0x85));
            }

            item->setIcon(0, itemIcon);
            item->setData(0, Qt::UserRole,     QString::fromStdString(p.string()));
            item->setData(0, Qt::UserRole + 1, isDir);

            item->setForeground(0, QColor("#909090"));

            return item;
        };

        for (auto& e : dirs)  { auto* n = makeItem(e.path(), true); populateNode(e.path(), n); }
        for (auto& e : files) { makeItem(e.path(), false); }
    }

    void SceneTreePanel::onFilterChanged(const QString& text) {
        for (int i = 0; i < m_tree->topLevelItemCount(); ++i)
            applyFilter(m_tree->topLevelItem(i), text);
    }

    bool SceneTreePanel::applyFilter(QTreeWidgetItem* item, const QString& filter) {
        if (!item) return false;

        bool selfMatch = filter.isEmpty() || item->text(0).contains(filter, Qt::CaseInsensitive);
        bool anyChild  = false;

        for (int i = 0; i < item->childCount(); ++i)
            if (applyFilter(item->child(i), filter)) anyChild = true;

        bool visible = selfMatch || anyChild;
        item->setHidden(!visible);
        if (anyChild && !filter.isEmpty()) item->setExpanded(true);
        return visible;
    }

    void SceneTreePanel::onItemClicked(QTreeWidgetItem* item, int) {
        if (!item) return;
        QString path  = item->data(0, Qt::UserRole).toString();
        bool    isDir = item->data(0, Qt::UserRole + 1).toBool();
        if (isDir)
            item->setExpanded(!item->isExpanded());
        emit itemSelected(path, isDir);
    }

    void SceneTreePanel::onItemDoubleClicked(QTreeWidgetItem* item, int) {
        if (!item) return;
        if (!item->data(0, Qt::UserRole + 1).toBool())
            emit fileActivated(item->data(0, Qt::UserRole).toString());
    }

    void SceneTreePanel::onRefreshClicked() {
        refresh();
    }

    void SceneTreePanel::onContextMenu(const QPoint& pos) {
        QTreeWidgetItem* item = m_tree->itemAt(pos);
        QMenu menu(this);

        if (item) {
            bool    isDir = item->data(0, Qt::UserRole + 1).toBool();
            QString path  = item->data(0, Qt::UserRole).toString();

            if (isDir) {
                menu.addAction("New Folder");
                menu.addAction("New Scene");
                menu.addSeparator();
                QAction* reveal = menu.addAction("Reveal in Explorer");
                menu.addSeparator();
                menu.addAction("Rename");
                menu.addAction("Delete");

                if (menu.exec(m_tree->viewport()->mapToGlobal(pos)) == reveal) {}
            } else {
                QAction* open   = menu.addAction("Open");
                QAction* reveal = menu.addAction("Reveal in Explorer");
                menu.addSeparator();
                menu.addAction("Rename");
                menu.addAction("Delete");

                QAction* chosen = menu.exec(m_tree->viewport()->mapToGlobal(pos));
                if      (chosen == open)   emit fileActivated(path);
                else if (chosen == reveal) {}
            }
        } else {
            QAction* r = menu.addAction("Refresh");
            if (menu.exec(m_tree->viewport()->mapToGlobal(pos)) == r)
                this->refresh();
        }
    }
}