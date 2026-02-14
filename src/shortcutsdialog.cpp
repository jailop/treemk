#include "shortcutsdialog.h"
#include "shortcutmanager.h"
#include <QHBoxLayout>
#include <QHeaderView>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QVBoxLayout>

ShortcutsDialog::ShortcutsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Keyboard Shortcuts"));
  resize(800, 600);
  setupUI();
  loadShortcuts();
}

void ShortcutsDialog::setupUI() {
  QVBoxLayout *layout = new QVBoxLayout(this);

  QLabel *titleLabel =
      new QLabel(tr("<h2>Keyboard Shortcuts Configuration</h2>"), this);
  layout->addWidget(titleLabel);

  QLabel *infoLabel = new QLabel(tr("Click on a shortcut to edit it. Press "
                                    "Enter to confirm or Esc to cancel."),
                                 this);
  layout->addWidget(infoLabel);

  shortcutsTable = new QTableWidget(this);
  shortcutsTable->setColumnCount(3);
  shortcutsTable->setHorizontalHeaderLabels(
      {tr("Category"), tr("Action"), tr("Shortcut")});
  shortcutsTable->horizontalHeader()->setStretchLastSection(true);
  shortcutsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
  shortcutsTable->setAlternatingRowColors(true);
  shortcutsTable->horizontalHeader()->setSectionResizeMode(
      0, QHeaderView::ResizeToContents);
  shortcutsTable->horizontalHeader()->setSectionResizeMode(
      1, QHeaderView::Stretch);

  connect(shortcutsTable, &QTableWidget::cellDoubleClicked, this,
          &ShortcutsDialog::onCellClicked);

  layout->addWidget(shortcutsTable);

  // Button bar
  QHBoxLayout *buttonLayout = new QHBoxLayout();

  resetButton = new QPushButton(tr("Reset to Defaults"), this);
  connect(resetButton, &QPushButton::clicked, this,
          &ShortcutsDialog::onResetDefaults);
  buttonLayout->addWidget(resetButton);

  buttonLayout->addStretch();

  saveButton = new QPushButton(tr("Save"), this);
  connect(saveButton, &QPushButton::clicked, this, &ShortcutsDialog::onSave);
  buttonLayout->addWidget(saveButton);

  cancelButton = new QPushButton(tr("Cancel"), this);
  connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
  buttonLayout->addWidget(cancelButton);

  layout->addLayout(buttonLayout);
}

void ShortcutsDialog::loadShortcuts() {
  shortcutsTable->setRowCount(0);

  ShortcutManager *sm = ShortcutManager::instance();
  QList<ShortcutManager::Action> actions = sm->getAllActions();

  for (ShortcutManager::Action action : actions) {
    QString category = sm->getActionCategory(action);
    QString description = sm->getActionDescription(action);
    QString shortcut = sm->getShortcut(action).toString();

    int row = shortcutsTable->rowCount();
    shortcutsTable->insertRow(row);

    QTableWidgetItem *categoryItem = new QTableWidgetItem(category);
    categoryItem->setFlags(categoryItem->flags() & ~Qt::ItemIsEditable);
    shortcutsTable->setItem(row, 0, categoryItem);

    QTableWidgetItem *actionItem = new QTableWidgetItem(description);
    actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
    actionItem->setData(Qt::UserRole, static_cast<int>(action));
    shortcutsTable->setItem(row, 1, actionItem);

    QTableWidgetItem *shortcutItem = new QTableWidgetItem(shortcut);
    shortcutsTable->setItem(row, 2, shortcutItem);
  }

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
  addShortcut(tr("Edit"), tr("Move List Item Up"), tr("Ctrl+Up"));
  addShortcut(tr("Edit"), tr("Move List Item Down"), tr("Ctrl+Down"));
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

  // Navigation (non-configurable app-level)
  addShortcut(tr("App Navigation"), tr("Quick Open"), tr("Ctrl+P"));
  addShortcut(tr("App Navigation"), tr("Go to Line"), tr("Ctrl+G"));
  addShortcut(tr("App Navigation"), tr("Follow Link (Mouse)"),
              tr("Ctrl+Click"));
  addShortcut(tr("App Navigation"), tr("Follow Link (Keyboard)"),
              tr("Ctrl+Enter"));

  // View operations
  addShortcut(tr("View"), tr("Toggle Tree View"), tr("Ctrl+\\"));
  addShortcut(tr("View"), tr("Toggle Preview"), tr("Ctrl+Shift+P"));

  shortcutsTable->resizeColumnsToContents();
}

void ShortcutsDialog::addShortcut(const QString &category,
                                  const QString &action,
                                  const QString &shortcut) {
  int row = shortcutsTable->rowCount();
  shortcutsTable->insertRow(row);

  QTableWidgetItem *categoryItem = new QTableWidgetItem(category);
  categoryItem->setFlags(categoryItem->flags() & ~Qt::ItemIsEditable);
  shortcutsTable->setItem(row, 0, categoryItem);

  QTableWidgetItem *actionItem = new QTableWidgetItem(action);
  actionItem->setFlags(actionItem->flags() & ~Qt::ItemIsEditable);
  shortcutsTable->setItem(row, 1, actionItem);

  QTableWidgetItem *shortcutItem = new QTableWidgetItem(shortcut);
  shortcutItem->setFlags(shortcutItem->flags() & ~Qt::ItemIsEditable);
  shortcutsTable->setItem(row, 2, shortcutItem);
}

void ShortcutsDialog::onCellClicked(int row, int column) {
  if (column != 2) {
    return; // Only shortcut column is editable
  }

  QTableWidgetItem *actionItem = shortcutsTable->item(row, 1);
  if (!actionItem) {
    return;
  }

  // Check if this row has a UserRole (meaning it's configurable)
  QVariant actionData = actionItem->data(Qt::UserRole);
  if (!actionData.isValid()) {
    return; // Not a configurable shortcut
  }

  QTableWidgetItem *shortcutItem = shortcutsTable->item(row, 2);
  QString currentShortcut = shortcutItem->text();

  // Create a key sequence editor in a dialog
  QDialog *editDialog = new QDialog(this);
  editDialog->setWindowTitle(tr("Edit Shortcut"));
  QVBoxLayout *layout = new QVBoxLayout(editDialog);

  QLabel *label = new QLabel(
      tr("Press the key combination for: %1").arg(actionItem->text()),
      editDialog);
  layout->addWidget(label);

  QKeySequenceEdit *keyEdit = new QKeySequenceEdit(
      QKeySequence::fromString(currentShortcut), editDialog);
  layout->addWidget(keyEdit);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  QPushButton *okButton = new QPushButton(tr("OK"), editDialog);
  QPushButton *clearButton = new QPushButton(tr("Clear"), editDialog);
  QPushButton *cancelButton = new QPushButton(tr("Cancel"), editDialog);

  connect(okButton, &QPushButton::clicked, editDialog, &QDialog::accept);
  connect(clearButton, &QPushButton::clicked,
          [keyEdit]() { keyEdit->clear(); });
  connect(cancelButton, &QPushButton::clicked, editDialog, &QDialog::reject);

  buttonLayout->addWidget(okButton);
  buttonLayout->addWidget(clearButton);
  buttonLayout->addWidget(cancelButton);
  layout->addLayout(buttonLayout);

  if (editDialog->exec() == QDialog::Accepted) {
    QKeySequence newSequence = keyEdit->keySequence();
    shortcutItem->setText(newSequence.toString());
  }

  delete editDialog;
}

void ShortcutsDialog::onResetDefaults() {
  QMessageBox::StandardButton reply =
      QMessageBox::question(this, tr("Reset Shortcuts"),
                            tr("Are you sure you want to reset all shortcuts "
                               "to their default values?"),
                            QMessageBox::Yes | QMessageBox::No);

  if (reply == QMessageBox::Yes) {
    ShortcutManager::instance()->resetToDefaults();
    loadShortcuts();
  }
}

void ShortcutsDialog::onSave() {
  ShortcutManager *sm = ShortcutManager::instance();

  // Update shortcuts in the manager
  for (int row = 0; row < shortcutsTable->rowCount(); ++row) {
    QTableWidgetItem *actionItem = shortcutsTable->item(row, 1);
    if (!actionItem) {
      continue;
    }

    QVariant actionData = actionItem->data(Qt::UserRole);
    if (!actionData.isValid()) {
      continue; // Not a configurable shortcut
    }

    ShortcutManager::Action action =
        static_cast<ShortcutManager::Action>(actionData.toInt());
    QTableWidgetItem *shortcutItem = shortcutsTable->item(row, 2);
    QKeySequence sequence = QKeySequence::fromString(shortcutItem->text());

    sm->setShortcut(action, sequence);
  }

  sm->saveSettings();
  accept();
}
