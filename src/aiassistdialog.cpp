#include "aiassistdialog.h"
#include "ui_aiassistdialog.h"
#include "logic/aiprovider.h"
#include "logic/systemprompts.h"
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QShortcut>

AIAssistDialog::AIAssistDialog(QWidget *parent,
                                 const QString &selectedText,
                                 int cursorPos,
                                 int selStart,
                                 int selEnd)
    : QDialog(parent),
      ui(new Ui::AIAssistDialog),
      historyIndex(-1),
      wasTextSelected(!selectedText.isEmpty()),
      cursorPosition(cursorPos),
      selectionStart(selStart),
      selectionEnd(selEnd) {
  ui->setupUi(this);
  setupUI();
  
  if (!selectedText.isEmpty()) {
    ui->contentEdit->setPlainText(selectedText);
    HistoryEntry entry;
    entry.prompt = "";
    entry.content = selectedText;
    entry.timestamp = QDateTime::currentDateTime();
    historyStack.push(entry);
    historyIndex = 0;
  } else {
    HistoryEntry entry;
    entry.prompt = "";
    entry.content = "";
    entry.timestamp = QDateTime::currentDateTime();
    historyStack.push(entry);
    historyIndex = 0;
  }
  
  updateApplyButtonText();
  updateHistoryButtons();
  
  ui->promptEdit->setFocus();
}

AIAssistDialog::~AIAssistDialog() {
  delete ui;
}

void AIAssistDialog::setupUI() {
  connect(ui->systemPromptCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &AIAssistDialog::onSystemPromptChanged);
  connect(ui->processButton, &QPushButton::clicked,
          this, &AIAssistDialog::processWithAI);
  connect(ui->applyButton, &QPushButton::clicked,
          this, &AIAssistDialog::applyToEditor);
  connect(ui->copyButton, &QPushButton::clicked,
          this, &AIAssistDialog::copyToClipboard);
  connect(ui->prevButton, &QPushButton::clicked,
          this, &AIAssistDialog::navigatePrevious);
  connect(ui->nextButton, &QPushButton::clicked,
          this, &AIAssistDialog::navigateNext);
  
  connect(ui->promptEdit, &QLineEdit::textChanged, [this]() {
    if (ui->systemPromptCombo->currentIndex() != 0) {
      ui->systemPromptCombo->setCurrentIndex(0);
    }
  });
  
  QShortcut *processShortcut = new QShortcut(QKeySequence("Ctrl+Return"), this);
  connect(processShortcut, &QShortcut::activated,
          this, &AIAssistDialog::processWithAI);
  
  loadSystemPrompts();
  setProcessingState(false);
}

void AIAssistDialog::loadSystemPrompts() {
  ui->systemPromptCombo->clear();
  ui->systemPromptCombo->addItem("Custom");
  
  QList<SystemPrompt> prompts = SystemPrompts::instance()->getEnabledPrompts();
  for (const SystemPrompt &prompt : prompts) {
    ui->systemPromptCombo->addItem(prompt.name, prompt.id);
  }
}

void AIAssistDialog::onSystemPromptChanged(int index) {
  if (index == 0) {
    return;
  }
  
  QString promptId = ui->systemPromptCombo->itemData(index).toString();
  SystemPrompt prompt = SystemPrompts::instance()->getPromptById(promptId);
  
  if (!prompt.id.isEmpty()) {
    ui->promptEdit->setText(prompt.prompt);
    
    // Auto-process when a predefined prompt is selected
    processWithAI();
  }
}

void AIAssistDialog::updateHistoryButtons() {
  ui->prevButton->setEnabled(historyIndex > 0);
  ui->nextButton->setEnabled(historyIndex < historyStack.size() - 1);
  
  if (historyStack.isEmpty()) {
    ui->historyLabel->setText("0/0");
  } else {
    ui->historyLabel->setText(QString("%1/%2")
                              .arg(historyIndex + 1)
                              .arg(historyStack.size()));
  }
}

void AIAssistDialog::updateApplyButtonText() {
  if (wasTextSelected) {
    ui->applyButton->setText("Replace");
    ui->applyButton->setToolTip("Replace selected text in editor");
  } else {
    ui->applyButton->setText("Insert");
    ui->applyButton->setToolTip("Insert content at cursor position");
  }
}

void AIAssistDialog::setProcessingState(bool processing) {
  ui->processButton->setEnabled(!processing);
  ui->applyButton->setEnabled(!processing);
  ui->promptEdit->setEnabled(!processing);
  ui->contentEdit->setReadOnly(processing);
  
  if (processing) {
    ui->processButton->setText("Processing...");
  } else {
    ui->processButton->setText("Process");
  }
}

void AIAssistDialog::saveCurrentState() {
  if (historyIndex >= 0 && historyIndex < historyStack.size()) {
    historyStack[historyIndex].prompt = ui->promptEdit->text();
    historyStack[historyIndex].content = ui->contentEdit->toPlainText();
  }
}

void AIAssistDialog::restoreHistoryEntry(int index) {
  if (index >= 0 && index < historyStack.size()) {
    const HistoryEntry &entry = historyStack[index];
    ui->promptEdit->setText(entry.prompt);
    ui->contentEdit->setPlainText(entry.content);
  }
}

void AIAssistDialog::processWithAI() {
  QString prompt = ui->promptEdit->text().trimmed();
  QString content = ui->contentEdit->toPlainText();
  
  if (prompt.isEmpty()) {
    QMessageBox::warning(this, "AI Assist", 
                        "Please enter a prompt for the AI.");
    return;
  }
  
  saveCurrentState();
  setProcessingState(true);
  
  AIProviderManager::instance()->process(prompt, content,
    [this](const QString &result) {
      ui->contentEdit->setPlainText(result);
      
      HistoryEntry entry;
      entry.prompt = ui->promptEdit->text();
      entry.content = result;
      entry.timestamp = QDateTime::currentDateTime();
      
      while (historyIndex < historyStack.size() - 1) {
        historyStack.pop();
      }
      
      historyStack.push(entry);
      historyIndex = historyStack.size() - 1;
      
      updateHistoryButtons();
      setProcessingState(false);
    },
    [this](const QString &error) {
      QMessageBox::warning(this, "AI Error", error);
      setProcessingState(false);
    }
  );
}

void AIAssistDialog::applyToEditor() {
  QString content = ui->contentEdit->toPlainText();
  
  if (content.isEmpty()) {
    QMessageBox::warning(this, "AI Assist",
                        "Content is empty. Nothing to apply.");
    return;
  }
  
  if (wasTextSelected) {
    emit replaceText(selectionStart, selectionEnd, content);
  } else {
    emit insertText(cursorPosition, content);
  }
  
  accept();
}

void AIAssistDialog::copyToClipboard() {
  QString content = ui->contentEdit->toPlainText();
  
  if (content.isEmpty()) {
    QMessageBox::information(this, "AI Assist",
                            "Content is empty. Nothing to copy.");
    return;
  }
  
  QClipboard *clipboard = QApplication::clipboard();
  clipboard->setText(content);
  
  QMessageBox::information(this, "AI Assist",
                          "Content copied to clipboard.");
}

void AIAssistDialog::navigatePrevious() {
  if (historyIndex > 0) {
    saveCurrentState();
    historyIndex--;
    restoreHistoryEntry(historyIndex);
    updateHistoryButtons();
  }
}

void AIAssistDialog::navigateNext() {
  if (historyIndex < historyStack.size() - 1) {
    saveCurrentState();
    historyIndex++;
    restoreHistoryEntry(historyIndex);
    updateHistoryButtons();
  }
}
