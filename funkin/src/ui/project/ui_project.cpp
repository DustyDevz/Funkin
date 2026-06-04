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
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QFrame>
#include <QSizePolicy>
#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <filesystem>

namespace Funkin::UI::Project {

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

        m_recentList = new QListWidget(tab);
        m_recentList->setAlternatingRowColors(true);

        auto recents = Funkin::App::Project::loadRecent();
        if (recents.empty()) {
            auto* placeholder = new QListWidgetItem("No recent projects.");
            placeholder->setFlags(Qt::NoItemFlags);
            m_recentList->addItem(placeholder);
        } else {
            for (auto& rp : recents) {
                auto* item = new QListWidgetItem(
                    QString("%1  —  %2")
                        .arg(QString::fromStdString(rp.name))
                        .arg(QString::fromStdString(rp.path))
                );
                item->setData(Qt::UserRole, QString::fromStdString(rp.path));
                m_recentList->addItem(item);
            }
        }

        connect(m_recentList, &QListWidget::itemDoubleClicked,
                this, &LauncherDialog::onOpenRecent);

        layout->addWidget(m_recentList, 1);

        auto* line = new QFrame(tab);
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        layout->addWidget(line);

        layout->addWidget(new QLabel("Open existing project:", tab));

        auto* row = new QHBoxLayout();
        m_openPath = new QLineEdit(tab);
        m_openPath->setPlaceholderText("Path to project folder...");

        auto* browseBtn = new QPushButton("Browse", tab);
        auto* openBtn   = new QPushButton("Open",   tab);
        openBtn->setDefault(true);

        connect(browseBtn, &QPushButton::clicked, this, &LauncherDialog::onOpenBrowse);
        connect(openBtn,   &QPushButton::clicked, this, &LauncherDialog::onOpenProject);

        row->addWidget(m_openPath, 1);
        row->addWidget(browseBtn);
        row->addWidget(openBtn);
        layout->addLayout(row);
    }

    void LauncherDialog::buildNewTab(QWidget* tab) {
        auto* layout = new QVBoxLayout(tab);
        layout->setContentsMargins(8, 8, 8, 8);
        layout->setSpacing(8);

        auto* nameRow = new QHBoxLayout();
        nameRow->addWidget(new QLabel("Name:", tab));
        m_newName = new QLineEdit(tab);
        m_newName->setPlaceholderText("My Shitty FNF Mod");
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

    void LauncherDialog::onOpenBrowse() {
        QString dir = QFileDialog::getExistingDirectory(
            this, "Select Project Folder", QString(),
            QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );
        if (!dir.isEmpty())
            m_openPath->setText(dir);
    }

    void LauncherDialog::onOpenProject() {
        clearError();
        std::string path = m_openPath->text().trimmed().toStdString();
        if (path.empty()) {
            setError("Please enter or browse to a project folder.");
            return;
        }

        auto projectFile = std::filesystem::path(path) / "project.funkin";
        if (Funkin::App::Project::get().load(projectFile)) {
            accept();
        } else {
            setError(QString("project.funkin not found in: %1").arg(m_openPath->text()));
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
            setError(QString("Failed to load project: %1").arg(path));
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