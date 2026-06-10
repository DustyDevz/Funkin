// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_console.hpp"
#include "../ui_icons.hpp"
#include "../ui_style.hpp"
#include "../ui_titlebar.hpp"
#include "../ui_contextmenu.hpp"
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QMetaObject>
#include <QHeaderView>
#include <QPainter>

class ConsoleTreeWidget : public QTreeWidget {
public:
    explicit ConsoleTreeWidget(QWidget* parent = nullptr) : QTreeWidget(parent) {
        setMouseTracking(true);
    }

    void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override {
        QTreeWidgetItem* item = itemFromIndex(index);
        if (!item || item->childCount() == 0) return;

        painter->save();
        const bool open = item->isExpanded();
        const int cellW = indentation();
        const int cellX = rect.right() - cellW;
        const QRect cell(cellX, rect.top(), cellW, rect.height());

        const float cx = cell.x() + cell.width()  * 0.5f;
        const float cy = cell.y() + cell.height()  * 0.5f;

        const QPoint cursorVp = viewport()->mapFromGlobal(QCursor::pos());
        const bool rowHover   = visualItemRect(item).contains(cursorVp);
        const bool selected   = item->isSelected();

        const QColor arrowColor = (rowHover || selected)
            ? QColor(0xcc, 0xcc, 0xcc)
            : QColor(0x55, 0x55, 0x55);

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
        painter->restore();
    }
};

namespace Funkin::UI::Editor {
    ConsolePanel::ConsolePanel(QWidget* parent)
        : QWidget(parent)
    {
        setObjectName("ConsolePanel");
        Funkin::UI::UIStyle::load(this, ":/ui/console");
        Funkin::UI::UIStyle::load(this, ":/ui/contextmenu");

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(0, Funkin::UI::PanelTitleBar::CONTENT_OFFSET, 0, 0);
        root->setSpacing(0);

        buildToolbar();

        m_tree = new ConsoleTreeWidget(this);
        m_tree->setObjectName("ConsoleTree");
        m_tree->setHeaderHidden(true);
        m_tree->setColumnCount(1);
        m_tree->setRootIsDecorated(true);
        m_tree->setIndentation(16);
        m_tree->setSelectionMode(QAbstractItemView::SingleSelection);
        m_tree->setContextMenuPolicy(Qt::CustomContextMenu);
        m_tree->setIconSize(QSize(12, 12));
        m_tree->setUniformRowHeights(false);
        m_tree->setAlternatingRowColors(false);
        m_tree->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        m_tree->setAnimated(false);
        m_tree->header()->setStretchLastSection(true);
        root->addWidget(m_tree, 1);

        connect(m_tree, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
            auto* item = m_tree->itemAt(pos);
            Funkin::UI::ContextMenu menu(this);

            if (item) {
                auto* top = item->parent() ? item->parent() : item;
                menu.addItem("Copy Message", "copy", QKeySequence::Copy, [top]() {
                    QApplication::clipboard()->setText(top->text(0));
                });
                menu.addItem("Copy All Details", "clipboard-list", {}, [top]() {
                    QStringList lines;
                    lines << top->text(0);
                    for (int i = 0; i < top->childCount(); i++)
                        lines << "  " + top->child(i)->text(0);
                    QApplication::clipboard()->setText(lines.join("\n"));
                });
                menu.addDivider();
                menu.addItem("Filter by This Level", "list-filter", {}, [this, top]() {
                    QString level = top->data(0, Qt::UserRole).toString();
                    m_btnInfo->setChecked(level == "info");
                    m_btnWarn->setChecked(level == "warn");
                    m_btnError->setChecked(level == "error");
                    applyFilter();
                });
                menu.addDivider();
            }

            menu.addItem("Select All", "check-check", QKeySequence::SelectAll, [this]() {
                m_tree->selectAll();
            });
            menu.addItem("Clear Console", "trash-2", {}, [this]() {
                ConsoleLog::get().clear();
                m_tree->clear();
                updateCounts();
            });

            if (item) {
                menu.addDivider();
                menu.addItem("Scroll to Top", "arrow-up-to-line", {}, [this]() {
                    m_tree->scrollToTop();
                });
                menu.addItem("Scroll to Bottom", "arrow-down-to-line", {}, [this]() {
                    m_tree->scrollToItem(m_tree->invisibleRootItem()->child(
                        m_tree->invisibleRootItem()->childCount() - 1));
                });
            }

            menu.exec(m_tree->viewport()->mapToGlobal(pos));
        });

        ConsoleLog::get().addSink([this](const ConsoleEntry& e) {
            QMetaObject::invokeMethod(this, [this, e]() {
                appendEntry(e);
            }, Qt::QueuedConnection);
        });

        for (auto& e : ConsoleLog::get().entries())
            appendEntry(e);
    }

    void ConsolePanel::buildToolbar() {
        auto* toolbar = new QWidget(this);
        toolbar->setObjectName("ConsoleToolbar");
        toolbar->setFixedHeight(28);

        auto* layout = new QHBoxLayout(toolbar);
        layout->setContentsMargins(6, 0, 6, 0);
        layout->setSpacing(4);

        m_filter = new QLineEdit(toolbar);
        m_filter->setObjectName("ConsoleFilter");
        m_filter->setPlaceholderText("Filter...");
        m_filter->setFixedHeight(20);
        layout->addWidget(m_filter);

        auto makeToggle = [&](const QString& label, const QString& color, const QString& icon) -> QPushButton* {
            auto* btn = new QPushButton(toolbar);
            btn->setObjectName("ConsoleToggle");
            btn->setCheckable(true);
            btn->setChecked(true);
            btn->setFixedHeight(20);
            btn->setText(label + " 0");
            btn->setIcon(Funkin::UI::Icons::get(icon, QColor(color), 12));
            btn->setIconSize(QSize(12, 12));
            btn->setStyleSheet(QString(
                "QPushButton { color: %1; background: transparent; border: 1px solid #3a3a3a;"
                " border-radius: 3px; padding: 0 6px; font-size: 11px; }"
                "QPushButton:checked { background: #2a2a2a; border-color: %1; }"
                "QPushButton:hover { background: #222222; }"
            ).arg(color));
            return btn;
        };

        m_btnInfo  = makeToggle("Info",  "#858585", "info");
        m_btnWarn  = makeToggle("Warn",  "#e5c07b", "triangle-alert");
        m_btnError = makeToggle("Error", "#e06c75", "circle-x");
        layout->addWidget(m_btnInfo);
        layout->addWidget(m_btnWarn);
        layout->addWidget(m_btnError);

        m_btnLock = new QPushButton(toolbar);
        m_btnLock->setObjectName("ConsoleLock");
        m_btnLock->setCheckable(true);
        m_btnLock->setChecked(true);
        m_btnLock->setFlat(true);
        m_btnLock->setFixedSize(24, 20);
        m_btnLock->setIcon(Funkin::UI::Icons::get("lock", QColor(0x61, 0xaf, 0xef), 14));
        m_btnLock->setIconSize(QSize(14, 14));
        m_btnLock->setToolTip("Lock scroll to bottom");
        layout->addWidget(m_btnLock);

        connect(m_btnLock, &QPushButton::toggled, this, [this](bool locked) {
            m_btnLock->setIcon(Funkin::UI::Icons::get(
                locked ? "lock" : "lock-open",
                locked ? QColor(0x61, 0xaf, 0xef) : QColor(0x85, 0x85, 0x85),
                14
            ));
            if (locked) m_tree->scrollToBottom();
        });

        qobject_cast<QVBoxLayout*>(this->layout())->insertWidget(0, toolbar);

        connect(m_filter,   &QLineEdit::textChanged, this, &ConsolePanel::applyFilter);
        connect(m_btnInfo,  &QPushButton::toggled, this, [this](bool v) { m_showInfo  = v; applyFilter(); });
        connect(m_btnWarn,  &QPushButton::toggled, this, [this](bool v) { m_showWarn  = v; applyFilter(); });
        connect(m_btnError, &QPushButton::toggled, this, [this](bool v) { m_showError = v; applyFilter(); });
    }

    void ConsolePanel::appendEntry(const ConsoleEntry& e) {
        auto* top = new QTreeWidgetItem(m_tree);

        QColor fg;
        QColor bg;
        QString levelStr;
        QString iconId;
        QColor  iconColor;

        switch (e.level) {
            case ConsoleLevel::Info:
                fg        = QColor("#aaaaaa");
                bg        = QColor(0, 0, 0, 0);
                levelStr  = "info";
                iconId    = "info";
                iconColor = QColor(0x85, 0x85, 0x85);
                break;
            case ConsoleLevel::Warn:
                fg        = QColor("#e5c07b");
                bg        = QColor("#2a2510");
                levelStr  = "warn";
                iconId    = "triangle-alert";
                iconColor = QColor(0xe5, 0xc0, 0x7b);
                break;
            case ConsoleLevel::Error:
                fg        = QColor("#e06c75");
                bg        = QColor("#2a1515");
                levelStr  = "error";
                iconId    = "circle-x";
                iconColor = QColor(0xe0, 0x6c, 0x75);
                break;
        }

        top->setText(0, QString::fromStdString(e.message));
        top->setIcon(0, Funkin::UI::Icons::get(iconId, iconColor, 12));
        top->setForeground(0, fg);
        if (bg.alpha() > 0) top->setBackground(0, bg);
        top->setData(0, Qt::UserRole, levelStr);

        auto* timeItem = new QTreeWidgetItem(top);
        timeItem->setText(0, QString::fromStdString(e.timestamp));
        timeItem->setForeground(0, QColor("#555555"));
        timeItem->setIcon(0, Funkin::UI::Icons::get("clock", QColor(0x55, 0x55, 0x55), 12));
        timeItem->setToolTip(0, "Log Timestamp");

        if (!e.engineId.empty()) {
            auto* idItem = new QTreeWidgetItem(top);
            idItem->setText(0, QString::fromStdString(e.engineId));
            idItem->setForeground(0, QColor("#4a4a4a"));
            idItem->setIcon(0, Funkin::UI::Icons::get("cpu", QColor(0x4a, 0x4a, 0x4a), 12));
        }

        if (m_btnLock && m_btnLock->isChecked())
            m_tree->scrollToItem(top);

        updateCounts();
        applyFilter();
    }

    void ConsolePanel::updateCounts() {
        int info = 0, warn = 0, error = 0;
        auto* root = m_tree->invisibleRootItem();
        for (int i = 0; i < root->childCount(); i++) {
            QString level = root->child(i)->data(0, Qt::UserRole).toString();
            if      (level == "info")  info++;
            else if (level == "warn")  warn++;
            else if (level == "error") error++;
        }
        m_btnInfo->setText(QString("Info %1").arg(info));
        m_btnWarn->setText(QString("Warn %1").arg(warn));
        m_btnError->setText(QString("Error %1").arg(error));
    }

    void ConsolePanel::applyFilter() {
        const QString filter = m_filter ? m_filter->text() : "";
        auto* root = m_tree->invisibleRootItem();
        for (int i = 0; i < root->childCount(); i++) {
            auto* item = root->child(i);
            const QString level = item->data(0, Qt::UserRole).toString();
            bool levelVisible =
                (level == "info"  && m_showInfo)  ||
                (level == "warn"  && m_showWarn)  ||
                (level == "error" && m_showError);
            bool textVisible = filter.isEmpty() ||
                item->text(0).contains(filter, Qt::CaseInsensitive);
            item->setHidden(!levelVisible || !textVisible);
        }
    }
}