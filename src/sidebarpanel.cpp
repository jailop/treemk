#include "sidebarpanel.h"
#include "ui_sidebarpanel.h"

#include <QListWidget>
#include <QLineEdit>
#include <QTabWidget>
#include <QVBoxLayout>

#include "filesystemtreeview.h"
#include "outlinepanel.h"

SidebarPanel::SidebarPanel(QWidget *parent)
    : QWidget(parent), ui(new Ui::SidebarPanel) {
    ui->setupUi(this);
}

SidebarPanel::~SidebarPanel() {
    delete ui;
}

QTabWidget* SidebarPanel::getLeftTabWidget() const {
    return ui->leftTabWidget;
}

QWidget* SidebarPanel::getTreePanel() const {
    return ui->treePanel;
}

QWidget* SidebarPanel::getOutlinePanel() const {
    return ui->outlinePanel;
}

QWidget* SidebarPanel::getBacklinksPanel() const {
    return ui->backlinksPanel;
}

QWidget* SidebarPanel::getHistoryPanel() const {
    return ui->historyPanel;
}

QListWidget* SidebarPanel::getBacklinksView() const {
    return ui->backlinksView;
}

QListWidget* SidebarPanel::getHistoryView() const {
    return ui->historyView;
}

QLineEdit* SidebarPanel::getHistoryFilterInput() const {
    return ui->historyFilterInput;
}

QLineEdit* SidebarPanel::getFileFilterInput() const {
    return ui->fileFilterInput;
}

void SidebarPanel::setTreeView(FileSystemTreeView* treeView) {
    if (treeView && ui->treePanel) {
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->treePanel->layout());
        if (layout) {
            layout->addWidget(treeView);
        }
    }
}

void SidebarPanel::setOutlineView(OutlinePanel* outlineView) {
    if (outlineView && ui->outlinePanel) {
        QVBoxLayout* layout = qobject_cast<QVBoxLayout*>(ui->outlinePanel->layout());
        if (layout) {
            layout->addWidget(outlineView);
        }
    }
}
