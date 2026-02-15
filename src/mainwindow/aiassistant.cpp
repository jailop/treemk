#include <QMenu>

#include "logic/systemprompts.h"
#include "mainwindow.h"

void MainWindow::createAIAssistMenu() {
    if (aiAssistMenu) {
        aiAssistMenu->clear();
    } else {
        aiAssistMenu = new QMenu(this);
    }

    QList<SystemPrompt> prompts =
        SystemPrompts::instance()->getEnabledPrompts();
    for (const SystemPrompt& prompt : prompts) {
        QAction* action = aiAssistMenu->addAction(prompt.name);
        QString promptId = prompt.id;
        connect(action, &QAction::triggered, this,
                [this, promptId]() { processAIWithSystemPrompt(promptId); });
    }

    if (!prompts.isEmpty()) {
        aiAssistMenu->addSeparator();
    }

    QAction* customAction = aiAssistMenu->addAction(tr("Custom..."));
    connect(customAction, &QAction::triggered, this, &MainWindow::openAIAssist);

    if (aiAssistEditSubmenu) {
        aiAssistEditSubmenu->clear();
        aiAssistEditSubmenu->addActions(aiAssistMenu->actions());
    }
}
