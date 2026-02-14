#include <QMessageBox>

#include "defs.h"
#include "mainwindow.h"
#include "navigationhistory.h"

void MainWindow::navigateBack() {
    QString filePath = navigationHistory->goBack();
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void MainWindow::navigateForward() {
    QString filePath = navigationHistory->goForward();
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void MainWindow::updateNavigationActions() {
    if (backAction) {
        backAction->setEnabled(navigationHistory->canGoBack());
    }
    if (forwardAction) {
        forwardAction->setEnabled(navigationHistory->canGoForward());
    }
}
