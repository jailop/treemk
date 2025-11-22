#include "shortcutsdialog.h"
#include <QTableWidget>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QLabel>

ShortcutsDialog::ShortcutsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Keyboard Shortcuts"));
    resize(700, 500);
    setupUI();
}

void ShortcutsDialog::setupUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    QLabel *titleLabel = new QLabel(tr("<h2>Keyboard Shortcuts Reference</h2>"), this);
    layout->addWidget(titleLabel);
    
    shortcutsTable = new QTableWidget(this);
    shortcutsTable->setColumnCount(3);
    shortcutsTable->setHorizontalHeaderLabels({tr("Category"), tr("Action"), tr("Shortcut")});
    shortcutsTable->horizontalHeader()->setStretchLastSection(true);
    shortcutsTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    shortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    shortcutsTable->setAlternatingRowColors(true);
    
    // File operations
    addShortcut(tr("File"), tr("New File"), tr("Ctrl+N"));
    addShortcut(tr("File"), tr("Open Folder"), tr("Ctrl+Shift+O"));
    addShortcut(tr("File"), tr("Save"), tr("Ctrl+S"));
    addShortcut(tr("File"), tr("Save As"), tr("Ctrl+Shift+S"));
    addShortcut(tr("File"), tr("Close Tab"), tr("Ctrl+W"));
    addShortcut(tr("File"), tr("Close All Tabs"), tr("Ctrl+Shift+W"));
    addShortcut(tr("File"), tr("Quit"), tr("Ctrl+Q"));
    
    // Edit operations
    addShortcut(tr("Edit"), tr("Undo"), tr("Ctrl+Z"));
    addShortcut(tr("Edit"), tr("Redo"), tr("Ctrl+Shift+Z"));
    addShortcut(tr("Edit"), tr("Cut"), tr("Ctrl+X"));
    addShortcut(tr("Edit"), tr("Copy"), tr("Ctrl+C"));
    addShortcut(tr("Edit"), tr("Paste"), tr("Ctrl+V"));
    addShortcut(tr("Edit"), tr("Select All"), tr("Ctrl+A"));
    addShortcut(tr("Edit"), tr("Find"), tr("Ctrl+F"));
    addShortcut(tr("Edit"), tr("Find and Replace"), tr("Ctrl+H"));
    addShortcut(tr("Edit"), tr("Search in Files"), tr("Ctrl+Shift+F"));
    
    // Insert operations
    addShortcut(tr("Insert"), tr("Bold"), tr("Ctrl+B"));
    addShortcut(tr("Insert"), tr("Italic"), tr("Ctrl+I"));
    addShortcut(tr("Insert"), tr("Code"), tr("Ctrl+`"));
    addShortcut(tr("Insert"), tr("Link"), tr("Ctrl+K"));
    addShortcut(tr("Insert"), tr("Image"), tr("Ctrl+Shift+I"));
    addShortcut(tr("Insert"), tr("Header"), tr("Ctrl+1-6"));
    
    // Navigation
    addShortcut(tr("Navigation"), tr("Quick Open"), tr("Ctrl+P"));
    addShortcut(tr("Navigation"), tr("Go to Line"), tr("Ctrl+G"));
    addShortcut(tr("Navigation"), tr("Follow Link (Mouse)"), tr("Ctrl+Click"));
    addShortcut(tr("Navigation"), tr("Follow Link (Keyboard)"), tr("Ctrl+Enter"));
    
    // View operations
    addShortcut(tr("View"), tr("Toggle Tree View"), tr("Ctrl+\\"));
    addShortcut(tr("View"), tr("Toggle Preview"), tr("Ctrl+Shift+P"));
    
    shortcutsTable->resizeColumnsToContents();
    layout->addWidget(shortcutsTable);
    
    QPushButton *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    layout->addWidget(closeButton);
}

void ShortcutsDialog::addShortcut(const QString &category, const QString &action, const QString &shortcut)
{
    int row = shortcutsTable->rowCount();
    shortcutsTable->insertRow(row);
    shortcutsTable->setItem(row, 0, new QTableWidgetItem(category));
    shortcutsTable->setItem(row, 1, new QTableWidgetItem(action));
    shortcutsTable->setItem(row, 2, new QTableWidgetItem(shortcut));
}
