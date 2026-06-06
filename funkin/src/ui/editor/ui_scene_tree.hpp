// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QWidget>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QAction>
#include <QPainter>
#include <QDockWidget>
#include <filesystem>
#include <string>

namespace Funkin::UI::Editor {
    class FunkinTreeWidget : public QTreeWidget {
        Q_OBJECT
    public:
        explicit FunkinTreeWidget(QWidget* parent = nullptr);

    protected:
        void drawBranches(QPainter* painter,
                          const QRect& rect,
                          const QModelIndex& index) const override;
        void enterEvent(QEnterEvent* event) override;
        void leaveEvent(QEvent* event) override;

    private:
        mutable bool m_isHovered { false };
    };

    class SceneTreeTitleBar : public QWidget {
        Q_OBJECT
    public:
        explicit SceneTreeTitleBar(QDockWidget* parent = nullptr);
    };

    class SceneTreePanel : public QWidget {
        Q_OBJECT

    public:
        explicit SceneTreePanel(QWidget* parent = nullptr);
        void refresh();

        static QIcon getIcon(const QString& iconId, const QColor& color = QColor(Qt::white));

    signals:
        void itemSelected(const QString& path, bool isDirectory);
        void fileActivated(const QString& path);
        void newFolderRequested();

    protected:
        bool eventFilter(QObject* obj, QEvent* event) override;

    private slots:
        void onFilterChanged(const QString& text);
        void onItemClicked(QTreeWidgetItem* item, int column);
        void onItemDoubleClicked(QTreeWidgetItem* item, int column);
        void onContextMenu(const QPoint& pos);
        void onRefreshClicked();

    private:
        void populateNode(const std::filesystem::path& dir, QTreeWidgetItem* parent);
        bool applyFilter(QTreeWidgetItem* item, const QString& filter);
        inline bool isItemExpanded(QTreeWidgetItem* item) const { return item && item->isExpanded(); }

        QLabel*           m_projectLabel { nullptr };
        QLineEdit*        m_filterEdit   { nullptr };
        FunkinTreeWidget* m_tree         { nullptr };

        QIcon m_iconFolder;
        QIcon m_iconFolderOpen;
        QIcon m_iconFile;
        QIcon m_iconScene;
        QIcon m_iconAudio;
        QIcon m_iconImage;
        QIcon m_iconScript;

        void  loadIcons();
        QIcon iconForPath(const std::filesystem::path& p, bool isDir) const;
    };
}