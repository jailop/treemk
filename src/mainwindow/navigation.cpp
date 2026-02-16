#include <QMessageBox>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QListWidget>
#include <QLineEdit>
#include <QProcess>
#include <QUrl>

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

void MainWindow::filterHistoryList() {
    if (!historyView || !historyFilterInput) {
        return;
    }

    QString filterText = historyFilterInput->text().toLower();
    historyView->clear();

    QVector<QString> history = navigationHistory->getHistory();
    
    // Iterate in reverse order to show most recent at the top
    for (int i = history.size() - 1; i >= 0; --i) {
        const QString& filePath = history[i];
        
        // Apply filter
        if (!filterText.isEmpty() && 
            !filePath.toLower().contains(filterText)) {
            continue;
        }

        QFileInfo fileInfo(filePath);

        // Show relative path if within current folder, otherwise show full path
        QString displayText;
        if (!currentFolder.isEmpty() &&
            filePath.startsWith(currentFolder + "/")) {
            displayText = filePath.mid(currentFolder.length() + 1);
        } else {
            displayText = filePath;
        }

        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, filePath);
        item->setToolTip(filePath);
        historyView->addItem(item);
    }
}
