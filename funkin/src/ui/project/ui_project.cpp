// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#include "ui_project.hpp"
#include "app/project/project.hpp"

#include <QApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTabWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSizePolicy>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QMenu>
#include <QAction>
#include <QDateTime>
#include <QStyle>
#include <fstream>
#include <nlohmann/json.hpp>
#include <filesystem>

namespace Funkin::UI::Project {

    static QString formatTimestamp(int64_t ts) {
        if (ts == 0) return "Unknown";
        QDateTime dt = QDateTime::fromSecsSinceEpoch(ts);
        QDateTime now = QDateTime::currentDateTime();
        int64_t secs = dt.secsTo(now);

        if (secs < 60)          return "Just now";
        if (secs < 3600)        return QString("%1m ago").arg(secs / 60);
        if (secs < 86400)       return QString("%1h ago").arg(secs / 3600);
        if (secs < 86400 * 7)   return QString("%1d ago").arg(secs / 86400);
        return dt.toString("MMM d, yyyy");
    }

    bool RunLauncher(QWidget* parent) {
        LauncherDialog dlg(parent);
        return dlg.exec() == QDialog::Accepted;
    }

    LauncherDialog::LauncherDialog(QWidget* parent)
        : QDialog(parent)
    {
        setWindowTitle("Funkin Engine");
        setFixedSize(600, 420);
        setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

        auto* layout = new QVBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_contentWidget = new QWidget(this);
        layout->addWidget(m_contentWidget);

        auto* root = new QVBoxLayout(m_contentWidget);
        root->setContentsMargins(12, 12, 12, 12);
        root->setSpacing(8);

        m_errorLabel = new QLabel(m_contentWidget);
        m_errorLabel->setWordWrap(true);
        m_errorLabel->setStyleSheet("color: #ff6b6b; font-weight: bold;");
        m_errorLabel->hide();

        m_errorEffect = new QGraphicsOpacityEffect(m_errorLabel);
        m_errorLabel->setGraphicsEffect(m_errorEffect);

        root->addWidget(m_errorLabel);

        auto* tabs = new QTabWidget(m_contentWidget);
        root->addWidget(tabs, 1);

        auto* recentTab = new QWidget();
        buildRecentTab(recentTab);
        tabs->addTab(recentTab, "Recent Projects");

        auto* newTab = new QWidget();
        buildNewTab(newTab);
        tabs->addTab(newTab, "New Project");
    }

    void LauncherDialog::showEvent(QShowEvent* event) {
        QDialog::showEvent(event);

        auto* contentEffect = new QGraphicsOpacityEffect(m_contentWidget);
        m_contentWidget->setGraphicsEffect(contentEffect);

        auto* fadeAnim = new QPropertyAnimation(contentEffect, "opacity", this);
        fadeAnim->setDuration(250);
        fadeAnim->setStartValue(0.0);
        fadeAnim->setEndValue(1.0);
        fadeAnim->setEasingCurve(QEasingCurve::OutCubic);

        auto* posAnim = new QPropertyAnimation(m_contentWidget, "pos", this);
        posAnim->setDuration(300);
        posAnim->setStartValue(QPoint(0, 20));
        posAnim->setEndValue(QPoint(0, 0));
        posAnim->setEasingCurve(QEasingCurve::OutBack);

        auto* group = new QParallelAnimationGroup(this);
        group->addAnimation(fadeAnim);
        group->addAnimation(posAnim);

        group->start(QAbstractAnimation::DeleteWhenStopped);
    }

    void LauncherDialog::buildRecentTab(QWidget* tab) {
        auto* layout = new QVBoxLayout(tab);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(6);

        m_emptyLabel = new QLabel(
            "No projects found — create a new one or import an existing project setup.", tab);
        m_emptyLabel->setAlignment(Qt::AlignCenter);
        m_emptyLabel->setWordWrap(true);
        m_emptyLabel->setStyleSheet("color: gray; padding: 24px;");
        m_emptyLabel->hide();

        m_recentList = new QListWidget(tab);
        m_recentList->setAlternatingRowColors(true);
        m_recentList->setIconSize(QSize(20, 20));
        m_recentList->setContextMenuPolicy(Qt::CustomContextMenu);
        m_recentList->setSpacing(2);

        connect(m_recentList, &QListWidget::itemActivated,
                this, &LauncherDialog::onOpenRecent);
        connect(m_recentList, &QListWidget::customContextMenuRequested,
                this, &LauncherDialog::onRecentContextMenu);

        populateRecentList();

        auto* listContainer = new QWidget(tab);
        auto* stackLayout   = new QVBoxLayout(listContainer);
        stackLayout->setContentsMargins(0, 0, 0, 0);
        stackLayout->addWidget(m_recentList);
        stackLayout->addWidget(m_emptyLabel);

        layout->addWidget(listContainer, 1);

        auto* line = new QFrame(tab);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);

        auto* importBtn = new QPushButton("Import Existing Project...", tab);
        importBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(importBtn, &QPushButton::clicked, this, &LauncherDialog::onOpenProject);
        layout->addWidget(importBtn);
    }

    void LauncherDialog::buildNewTab(QWidget* tab) {
        auto* layout = new QVBoxLayout(tab);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(8);

        auto* nameRow = new QHBoxLayout();
        nameRow->addWidget(new QLabel("Name:", tab));
        m_newName = new QLineEdit(tab);
        m_newName->setPlaceholderText(":3");
        nameRow->addWidget(m_newName, 1);
        layout->addLayout(nameRow);

        auto* folderRow = new QHBoxLayout();
        folderRow->addWidget(new QLabel("Folder:", tab));
        m_newFolder = new QLineEdit(tab);
        m_newFolder->setPlaceholderText("Parent directory for the project...");
        auto* browseBtn = new QPushButton("Browse", tab);
        connect(browseBtn, &QPushButton::clicked, this, &LauncherDialog::onNewBrowse);
        folderRow->addWidget(m_newFolder, 1);
        folderRow->addWidget(browseBtn);
        layout->addLayout(folderRow);

        layout->addStretch(1);

        auto* createBtn = new QPushButton("Create Project", tab);
        createBtn->setDefault(true);
        createBtn->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        connect(createBtn, &QPushButton::clicked, this, &LauncherDialog::onCreateProject);
        layout->addWidget(createBtn);
    }

    void LauncherDialog::populateRecentList() {
        m_recentList->clear();

        auto recents = Funkin::App::Project::loadRecent();

        if (recents.empty()) {
            m_recentList->hide();
            m_emptyLabel->show();
            return;
        }

        m_emptyLabel->hide();
        m_recentList->show();

        QIcon folderIcon = QApplication::style()->standardIcon(QStyle::SP_FileIcon);

        for (auto& rp : recents) {
            auto* item   = new QListWidgetItem(m_recentList);
            auto* widget = new QWidget();
            auto* vbox   = new QVBoxLayout(widget);
            vbox->setContentsMargins(4, 4, 4, 4);
            vbox->setSpacing(2);

            auto* nameLabel = new QLabel(QString::fromStdString(rp.name));
            QFont nameFont  = nameLabel->font();
            nameFont.setBold(true);
            nameLabel->setFont(nameFont);

            auto projectFile = std::filesystem::path(rp.path) / "project.funkin";
            auto displayPath = QString::fromStdString(projectFile.string());

            auto* metaLabel = new QLabel(
                QString("%1  ·  %2")
                    .arg(displayPath)
                    .arg(formatTimestamp(rp.lastOpened))
            );
            metaLabel->setStyleSheet("color: gray; font-size: 11px;");

            vbox->addWidget(nameLabel);
            vbox->addWidget(metaLabel);
            widget->setLayout(vbox);

            item->setSizeHint(widget->sizeHint());
            item->setIcon(folderIcon);
            item->setData(Qt::UserRole, QString::fromStdString(rp.path));
            item->setToolTip(displayPath);

            m_recentList->setItemWidget(item, widget);
        }
    }

    void LauncherDialog::onRecentContextMenu(const QPoint& pos) {
        QListWidgetItem* item = m_recentList->itemAt(pos);
        if (!item) return;

        QMenu menu(this);
        QAction* removeAction = menu.addAction("Remove from recents");

        if (menu.exec(m_recentList->mapToGlobal(pos)) == removeAction) {
            QString path = item->data(Qt::UserRole).toString();

            auto recents = Funkin::App::Project::loadRecent();
            recents.erase(
                std::remove_if(recents.begin(), recents.end(),
                    [&](const Funkin::App::RecentProject& rp) {
                        return rp.path == path.toStdString();
                    }),
                recents.end()
            );
            Funkin::App::Project::saveRecent(recents);

            populateRecentList();
        }
    }

    void LauncherDialog::onOpenBrowse() {
    }

    void LauncherDialog::onOpenProject() {
        clearError();

        QString file = QFileDialog::getOpenFileName(
            this, "Import Project Configuration", QString(),
            "Funkin Project Files (project.funkin);;All Files (*)"
        );

        if (file.isEmpty()) return;

        std::filesystem::path projectFile(file.toStdString());
        if (!std::filesystem::exists(projectFile)) {
            setError("The specified project file does not exist.");
            return;
        }

        try {
            std::ifstream f(projectFile);
            auto j = nlohmann::json::parse(f);
            std::string pName = j["name"].get<std::string>();
            std::string pRoot = projectFile.parent_path().string();

            Funkin::App::Project::get().addRecent({ pName, pRoot });
            populateRecentList();
        } catch (...) {
            setError("Failed to parse the chosen project file structure.");
        }
    }

    void LauncherDialog::onOpenRecent(QListWidgetItem* item) {
        clearError();
        QString path = item->data(Qt::UserRole).toString();
        if (path.isEmpty()) return;

        auto projectFile = std::filesystem::path(path.toStdString()) / "project.funkin";
        if (Funkin::App::Project::get().load(projectFile)) {
            accept();
        } else {
            setError(QString("Failed to load project at: %1\n\nRight-click to remove it from recents.").arg(QString::fromStdString(projectFile.string())));
        }
    }

    void LauncherDialog::onNewBrowse() {
        QString dir = QFileDialog::getExistingDirectory(
            this, "Select Parent Folder", QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        
        if (!dir.isEmpty())
            m_newFolder->setText(dir);
    }

    void LauncherDialog::onCreateProject() {
        clearError();
        QString name   = m_newName->text().trimmed();
        QString folder = m_newFolder->text().trimmed();

        if (name.isEmpty()) {
            setError("Name cannot be empty.");
            return;
        }
        if (folder.isEmpty()) {
            setError("Folder cannot be empty.");
            return;
        }
        if (!std::filesystem::exists(folder.toStdString())) {
            setError("Folder does not exist.");
            return;
        }

        if (Funkin::App::Project::get().create(folder.toStdString(), name.toStdString())) {
            accept();
        } else {
            setError("Failed to create project.");
        }
    }

    void LauncherDialog::setError(const QString& msg) {
        m_errorLabel->setText(msg);

        if (m_errorLabel->isHidden()) {
            m_errorLabel->show();
            auto* anim = new QPropertyAnimation(m_errorEffect, "opacity");
            anim->setDuration(180);
            anim->setStartValue(0.0);
            anim->setEndValue(1.0);
            anim->setEasingCurve(QEasingCurve::OutQuad);
            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }

    void LauncherDialog::clearError() {
        if (!m_errorLabel->isHidden()) {
            auto* anim = new QPropertyAnimation(m_errorEffect, "opacity");
            anim->setDuration(120);
            anim->setStartValue(m_errorEffect->opacity());
            anim->setEndValue(0.0);
            anim->setEasingCurve(QEasingCurve::InQuad);

            connect(anim, &QPropertyAnimation::finished, [this]() {
                m_errorLabel->hide();
                m_errorLabel->clear();
            });

            anim->start(QAbstractAnimation::DeleteWhenStopped);
        }
    }
}