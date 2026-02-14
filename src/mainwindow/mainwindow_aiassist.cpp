#include "mainwindow.h"
#include "logic/systemprompts.h"
#include <QMenu>

void MainWindow::createAIAssistMenu() {
  // Clear existing menu if it exists
  if (aiAssistMenu) {
    aiAssistMenu->clear();
  } else {
    aiAssistMenu = new QMenu(this);
  }
  
  // Add predefined prompts
  QList<SystemPrompt> prompts = SystemPrompts::instance()->getEnabledPrompts();
  for (const SystemPrompt &prompt : prompts) {
    QAction *action = aiAssistMenu->addAction(prompt.name);
    QString promptId = prompt.id;  // Capture by value
    connect(action, &QAction::triggered, this, [this, promptId]() {
      processAIWithSystemPrompt(promptId);
    });
  }
  
  // Add separator before Custom option
  if (!prompts.isEmpty()) {
    aiAssistMenu->addSeparator();
  }
  
  // Add "Custom..." option that opens the dialog
  QAction *customAction = aiAssistMenu->addAction(tr("Custom..."));
  customAction->setShortcut(QKeySequence(tr("Ctrl+Shift+A")));
  connect(customAction, &QAction::triggered, this, &MainWindow::openAIAssist);
}
