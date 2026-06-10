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

        m_list = new QListWidget(this);
        m_list->setObjectName("ConsoleList");
        m_list->setSelectionMode(QAbstractItemView::SingleSelection);
        m_list->setContextMenuPolicy(Qt::CustomContextMenu);
        m_list->setSpacing(0);
        m_list->setIconSize(QSize(12, 12));
        m_list->setUniformItemSizes(true);
        m_list->setAlternatingRowColors(true);
        m_list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
        root->addWidget(m_list, 1);

        connect(m_list, &QListWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
            auto* item = m_list->itemAt(pos);
            
            Funkin::UI::ContextMenu menu(this);

            if (item) {
                menu.addItem("Copy", "copy", QKeySequence::Copy, [this, item]() {
                    QApplication::clipboard()->setText(item->text());
                });
                menu.addItem("Copy All", "clipboard-list", {}, [this]() {
                    QStringList lines;
                    for (int i = 0; i < m_list->count(); i++)
                        if (!m_list->item(i)->isHidden())
                            lines << m_list->item(i)->text();
                    QApplication::clipboard()->setText(lines.join("\n"));
                });
                menu.addDivider();
                menu.addItem("Filter by This Level", "filter", {}, [this, item]() {
                    QString level = item->data(Qt::UserRole).toString();
                    m_btnInfo->setChecked(level == "info");
                    m_btnWarn->setChecked(level == "warn");
                    m_btnError->setChecked(level == "error");
                    applyFilter();
                });
                menu.addDivider();
            }

            menu.addItem("Select All", "check-check", QKeySequence::SelectAll, [this]() {
                m_list->selectAll();
            });
            menu.addItem("Clear Console", "trash-2", {}, [this]() {
                ConsoleLog::get().clear();
                m_list->clear();
                updateCounts();
            });

            if (item) {
                menu.addDivider();
                menu.addItem("Scroll to Top", "arrow-up-to-line", {}, [this]() {
                    m_list->scrollToTop();
                });
                menu.addItem("Scroll to Bottom", "arrow-down-to-line", {}, [this]() {
                    m_list->scrollToBottom();
                });
            }

            menu.exec(m_list->viewport()->mapToGlobal(pos));
        });

        ConsoleLog::get().addSink([this](const ConsoleEntry& e) {
            QMetaObject::invokeMethod(this, [this, e]() {
                appendEntry(e);
            }, Qt::QueuedConnection);
        });

        for (auto& e : ConsoleLog::get().entries())
            appendEntry(e);
    }

    void ConsolePanel::updateCounts() {
        int info = 0, warn = 0, error = 0;
        for (int i = 0; i < m_list->count(); i++) {
            QString level = m_list->item(i)->data(Qt::UserRole).toString();
            if      (level == "info")  info++;
            else if (level == "warn")  warn++;
            else if (level == "error") error++;
        }
        m_btnInfo->setText(QString("Info %1").arg(info));
        m_btnWarn->setText(QString("Warn %1").arg(warn));
        m_btnError->setText(QString("Error %1").arg(error));
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
        m_btnLock->setFixedSize(18, 18);
        m_btnLock->setIcon(Funkin::UI::Icons::get("lock", QColor(0x85, 0x85, 0x85), 12));
        m_btnLock->setIconSize(QSize(12, 12));
        m_btnLock->setToolTip("Lock scroll to bottom");
        layout->addWidget(m_btnLock);

        qobject_cast<QVBoxLayout*>(this->layout())->insertWidget(0, toolbar);
        connect(m_filter,   &QLineEdit::textChanged, this, &ConsolePanel::applyFilter);
        connect(m_btnInfo,  &QPushButton::toggled, this, [this](bool v) { m_showInfo  = v; applyFilter(); });
        connect(m_btnWarn,  &QPushButton::toggled, this, [this](bool v) { m_showWarn  = v; applyFilter(); });
        connect(m_btnError, &QPushButton::toggled, this, [this](bool v) { m_showError = v; applyFilter(); });
        connect(m_btnLock,  &QPushButton::toggled, this, [this](bool locked) {
            m_btnLock->setIcon(Funkin::UI::Icons::get(
                locked ? "lock" : "lock-open",
                locked ? QColor(0x61, 0xaf, 0xef) : QColor(0x85, 0x85, 0x85),
                14
            ));
            if (locked) m_list->scrollToBottom();
        });
    }

    void ConsolePanel::appendEntry(const ConsoleEntry& e) {
        auto* item = new QListWidgetItem();

        switch (e.level) {
            case ConsoleLevel::Warn:
                item->setText(QString::fromStdString(e.message));
                item->setIcon(Funkin::UI::Icons::get("triangle-alert", QColor(0xe5, 0xc0, 0x7b), 12));
                item->setForeground(QColor("#e5c07b"));
                item->setBackground(QColor("#2a2510"));
                item->setData(Qt::UserRole, "warn");
                break;
            case ConsoleLevel::Error:
                item->setText(QString::fromStdString(e.message));
                item->setIcon(Funkin::UI::Icons::get("circle-x", QColor(0xe0, 0x6c, 0x75), 12));
                item->setForeground(QColor("#e06c75"));
                item->setBackground(QColor("#2a1515"));
                item->setData(Qt::UserRole, "error");
                break;
            case ConsoleLevel::Info:
                item->setText(QString::fromStdString(e.message));
                item->setIcon(Funkin::UI::Icons::get("info", QColor(0x85, 0x85, 0x85), 12));
                item->setForeground(QColor("#aaaaaa"));
                item->setData(Qt::UserRole, "info");
                break;
        }

        m_list->addItem(item);
        if (m_btnLock && m_btnLock->isChecked())
            m_list->scrollToBottom();
            
        updateCounts();
        applyFilter();
    }

    void ConsolePanel::applyFilter() {
        const QString filter = m_filter ? m_filter->text() : "";
        for (int i = 0; i < m_list->count(); i++) {
            auto* item = m_list->item(i);
            const QString level = item->data(Qt::UserRole).toString();
            bool levelVisible =
                (level == "info"  && m_showInfo)  ||
                (level == "warn"  && m_showWarn)  ||
                (level == "error" && m_showError);
            bool textVisible = filter.isEmpty() ||
                item->text().contains(filter, Qt::CaseInsensitive);
            item->setHidden(!levelVisible || !textVisible);
        }
    }
}