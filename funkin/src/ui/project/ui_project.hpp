// © 2026 Dusty | https://github.com/DustyDevz/Funkin
// Licensed under GNU GPL v3.0

#pragma once

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QLabel>

class QGraphicsOpacityEffect;
namespace Funkin::UI::Project {
    bool RunLauncher(QWidget* parent = nullptr);

    class LauncherDialog : public QDialog {
        Q_OBJECT

    public:
        explicit LauncherDialog(QWidget* parent = nullptr);

    protected:
        void showEvent(QShowEvent* event) override;

    private slots:
        void onOpenBrowse();
        void onOpenProject();
        void onOpenRecent(QListWidgetItem* item);
        void onRecentContextMenu(const QPoint& pos);
        void onNewBrowse();
        void onCreateProject();

    private:
        void buildRecentTab(QWidget* tab);
        void buildNewTab(QWidget* tab);
        void populateRecentList();
        void setError(const QString& msg);
        void clearError();

        QWidget*               m_contentWidget = nullptr;

        QListWidget*           m_recentList    = nullptr;
        QLabel*                m_emptyLabel    = nullptr;
        QLineEdit*             m_openPath      = nullptr;

        QLineEdit*             m_newName       = nullptr;
        QLineEdit*             m_newFolder     = nullptr;

        QLabel*                m_errorLabel    = nullptr;
        QGraphicsOpacityEffect* m_errorEffect  = nullptr;
    };
}