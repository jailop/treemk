#include "systempromptsdialog.h"
#include "ui_systempromptsdialog.h"
#include "logic/systemprompts.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QCheckBox>
#include <QDialogButtonBox>

SystemPromptsDialog::SystemPromptsDialog(QWidget *parent)
    : QDialog(parent),
      ui(new Ui::SystemPromptsDialog),
      selectedRow(-1) {
  ui->setupUi(this);
  
  connect(ui->addButton, &QPushButton::clicked, this, &SystemPromptsDialog::addPrompt);
  connect(ui->editButton, &QPushButton::clicked, this, &SystemPromptsDialog::editPrompt);
  connect(ui->deleteButton, &QPushButton::clicked, this, &SystemPromptsDialog::deletePrompt);
  connect(ui->moveUpButton, &QPushButton::clicked, this, &SystemPromptsDialog::movePromptUp);
  connect(ui->moveDownButton, &QPushButton::clicked, this, &SystemPromptsDialog::movePromptDown);
  connect(ui->resetButton, &QPushButton::clicked, this, &SystemPromptsDialog::resetToDefaults);
  
  connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &SystemPromptsDialog::saveChanges);
  connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  
  connect(ui->promptsList, &QListWidget::currentRowChanged,
          this, &SystemPromptsDialog::onPromptSelectionChanged);
  connect(ui->promptsList, &QListWidget::itemChanged,
          this, &SystemPromptsDialog::onPromptToggled);
  
  loadPrompts();
  updateButtonStates();
}

SystemPromptsDialog::~SystemPromptsDialog() {
  delete ui;
}

void SystemPromptsDialog::loadPrompts() {
  ui->promptsList->clear();
  currentPrompts = SystemPrompts::instance()->getAllPrompts();
  
  for (const SystemPrompt &prompt : currentPrompts) {
    QListWidgetItem *item = new QListWidgetItem(prompt.name);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(prompt.enabled ? Qt::Checked : Qt::Unchecked);
    
    if (prompt.isCustom) {
      item->setText(prompt.name + " (Custom)");
      QFont font = item->font();
      font.setItalic(true);
      item->setFont(font);
    }
    
    ui->promptsList->addItem(item);
  }
  
  if (!currentPrompts.isEmpty()) {
    ui->promptsList->setCurrentRow(0);
  }
}

void SystemPromptsDialog::addPrompt() {
  QDialog dialog(this);
  dialog.setWindowTitle("Add Custom Prompt");
  dialog.resize(500, 300);
  
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  
  QLabel *nameLabel = new QLabel("Name:");
  QLineEdit *nameEdit = new QLineEdit();
  
  QLabel *promptLabel = new QLabel("Prompt:");
  QPlainTextEdit *promptEdit = new QPlainTextEdit();
  promptEdit->setPlaceholderText("Enter the instruction for the AI...");
  promptEdit->setMinimumHeight(100);
  
  QCheckBox *enabledCheck = new QCheckBox("Enable by default");
  enabledCheck->setChecked(true);
  
  QLabel *tipLabel = new QLabel("Tip: Use clear, concise instructions for best AI results.");
  QFont tipFont = tipLabel->font();
  tipFont.setItalic(true);
  tipLabel->setFont(tipFont);
  tipLabel->setStyleSheet("color: gray;");
  
  QDialogButtonBox *buttons = new QDialogButtonBox(
    QDialogButtonBox::Save | QDialogButtonBox::Cancel);
  
  layout->addWidget(nameLabel);
  layout->addWidget(nameEdit);
  layout->addWidget(promptLabel);
  layout->addWidget(promptEdit);
  layout->addWidget(enabledCheck);
  layout->addWidget(tipLabel);
  layout->addWidget(buttons);
  
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  
  if (dialog.exec() == QDialog::Accepted) {
    QString name = nameEdit->text().trimmed();
    QString prompt = promptEdit->toPlainText().trimmed();
    
    if (!validatePrompt(name, prompt)) {
      return;
    }
    
    SystemPrompts::instance()->addCustomPrompt(name, prompt);
    if (!enabledCheck->isChecked()) {
      QList<SystemPrompt> all = SystemPrompts::instance()->getAllPrompts();
      if (!all.isEmpty()) {
        SystemPrompts::instance()->setPromptEnabled(all.last().id, false);
      }
    }
    
    loadPrompts();
    ui->promptsList->setCurrentRow(ui->promptsList->count() - 1);
  }
}

void SystemPromptsDialog::editPrompt() {
  if (selectedRow < 0 || selectedRow >= currentPrompts.size()) {
    return;
  }
  
  const SystemPrompt &selected = currentPrompts[selectedRow];
  
  if (!selected.isCustom) {
    QMessageBox::information(this, "Edit Prompt",
                            "Built-in prompts cannot be edited.\n"
                            "You can disable them or create a custom variant.");
    return;
  }
  
  QDialog dialog(this);
  dialog.setWindowTitle("Edit Custom Prompt");
  dialog.resize(500, 300);
  
  QVBoxLayout *layout = new QVBoxLayout(&dialog);
  
  QLabel *nameLabel = new QLabel("Name:");
  QLineEdit *nameEdit = new QLineEdit(selected.name);
  
  QLabel *promptLabel = new QLabel("Prompt:");
  QPlainTextEdit *promptEdit = new QPlainTextEdit(selected.prompt);
  promptEdit->setMinimumHeight(100);
  
  QCheckBox *enabledCheck = new QCheckBox("Enabled");
  enabledCheck->setChecked(selected.enabled);
  
  QDialogButtonBox *buttons = new QDialogButtonBox(
    QDialogButtonBox::Save | QDialogButtonBox::Cancel);
  
  layout->addWidget(nameLabel);
  layout->addWidget(nameEdit);
  layout->addWidget(promptLabel);
  layout->addWidget(promptEdit);
  layout->addWidget(enabledCheck);
  layout->addWidget(buttons);
  
  connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
  connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
  
  if (dialog.exec() == QDialog::Accepted) {
    QString name = nameEdit->text().trimmed();
    QString prompt = promptEdit->toPlainText().trimmed();
    
    if (!validatePrompt(name, prompt)) {
      return;
    }
    
    SystemPrompts::instance()->updateCustomPrompt(selected.id, name, prompt);
    SystemPrompts::instance()->setPromptEnabled(selected.id, enabledCheck->isChecked());
    
    loadPrompts();
    ui->promptsList->setCurrentRow(selectedRow);
  }
}

void SystemPromptsDialog::deletePrompt() {
  if (selectedRow < 0 || selectedRow >= currentPrompts.size()) {
    return;
  }
  
  const SystemPrompt &selected = currentPrompts[selectedRow];
  
  if (!selected.isCustom) {
    QMessageBox::information(this, "Delete Prompt",
                            "Built-in prompts cannot be deleted.\n"
                            "You can disable them instead.");
    return;
  }
  
  QMessageBox::StandardButton reply = QMessageBox::question(
    this, "Delete Prompt",
    QString("Are you sure you want to delete '%1'?").arg(selected.name),
    QMessageBox::Yes | QMessageBox::No);
  
  if (reply == QMessageBox::Yes) {
    SystemPrompts::instance()->removeCustomPrompt(selected.id);
    loadPrompts();
  }
}

void SystemPromptsDialog::movePromptUp() {
  if (selectedRow <= 0) return;
  
  currentPrompts.swapItemsAt(selectedRow, selectedRow - 1);
  SystemPrompts::instance()->reorderPrompts(currentPrompts);
  int newRow = selectedRow - 1;
  loadPrompts();
  ui->promptsList->setCurrentRow(newRow);
}

void SystemPromptsDialog::movePromptDown() {
  if (selectedRow < 0 || selectedRow >= currentPrompts.size() - 1) return;
  
  currentPrompts.swapItemsAt(selectedRow, selectedRow + 1);
  SystemPrompts::instance()->reorderPrompts(currentPrompts);
  int newRow = selectedRow + 1;
  loadPrompts();
  ui->promptsList->setCurrentRow(newRow);
}

void SystemPromptsDialog::resetToDefaults() {
  QMessageBox::StandardButton reply = QMessageBox::question(
    this, "Reset Prompts",
    "This will:\n"
    "• Re-enable all built-in prompts\n"
    "• Delete all custom prompts\n"
    "• Reset prompt order\n\n"
    "Are you sure?",
    QMessageBox::Yes | QMessageBox::No);
  
  if (reply == QMessageBox::Yes) {
    QList<SystemPrompt> allPrompts = SystemPrompts::instance()->getAllPrompts();
    for (const SystemPrompt &p : allPrompts) {
      if (p.isCustom) {
        SystemPrompts::instance()->removeCustomPrompt(p.id);
      } else {
        SystemPrompts::instance()->setPromptEnabled(p.id, true);
      }
    }
    
    loadPrompts();
    QMessageBox::information(this, "Reset Complete",
                            "System prompts have been reset to defaults.");
  }
}

void SystemPromptsDialog::onPromptSelectionChanged() {
  selectedRow = ui->promptsList->currentRow();
  updatePreview();
  updateButtonStates();
}

void SystemPromptsDialog::onPromptToggled() {
  int row = ui->promptsList->currentRow();
  if (row >= 0 && row < currentPrompts.size()) {
    QListWidgetItem *item = ui->promptsList->item(row);
    bool enabled = (item->checkState() == Qt::Checked);
    SystemPrompts::instance()->setPromptEnabled(currentPrompts[row].id, enabled);
    currentPrompts[row].enabled = enabled;
  }
}

void SystemPromptsDialog::updatePreview() {
  if (selectedRow >= 0 && selectedRow < currentPrompts.size()) {
    const SystemPrompt &prompt = currentPrompts[selectedRow];
    
    QString preview = QString("Prompt: \"%1\"\n\n").arg(prompt.prompt);
    preview += QString("Type: %1\n").arg(prompt.isCustom ? "Custom" : "Built-in");
    preview += QString("Status: %1\n").arg(prompt.enabled ? "Enabled" : "Disabled");
    preview += QString("Order: %1").arg(selectedRow + 1);
    
    ui->previewText->setPlainText(preview);
  } else {
    ui->previewText->clear();
  }
}

void SystemPromptsDialog::updateButtonStates() {
  bool hasSelection = selectedRow >= 0 && selectedRow < currentPrompts.size();
  bool isCustom = hasSelection && currentPrompts[selectedRow].isCustom;
  
  ui->editButton->setEnabled(hasSelection);
  ui->deleteButton->setEnabled(isCustom);
  ui->moveUpButton->setEnabled(hasSelection && selectedRow > 0);
  ui->moveDownButton->setEnabled(hasSelection && selectedRow < currentPrompts.size() - 1);
}

bool SystemPromptsDialog::validatePrompt(const QString &name, const QString &prompt) {
  if (name.isEmpty()) {
    QMessageBox::warning(this, "Invalid Input", "Please enter a prompt name.");
    return false;
  }
  
  if (prompt.isEmpty()) {
    QMessageBox::warning(this, "Invalid Input", "Please enter a prompt text.");
    return false;
  }
  
  if (name.length() > 100) {
    QMessageBox::warning(this, "Invalid Input", "Prompt name is too long (max 100 characters).");
    return false;
  }
  
  return true;
}

void SystemPromptsDialog::saveChanges() {
  SystemPrompts::instance()->saveToSettings();
  accept();
}
