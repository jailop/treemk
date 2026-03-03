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
#include "tabeditor.h"

void MainWindow::navigateBack() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    QString filePath = tab->navigationHistory()->goBack();
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void MainWindow::navigateForward() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    QString filePath = tab->navigationHistory()->goForward();
    if (!filePath.isEmpty()) {
        loadFile(filePath);
    }
}

void MainWindow::updateNavigationActions() {
    TabEditor* tab = currentTabEditor();
    if (!tab) {
        if (backAction) backAction->setEnabled(false);
        if (forwardAction) forwardAction->setEnabled(false);
        return;
    }

    if (backAction) {
        backAction->setEnabled(tab->navigationHistory()->canGoBack());
    }
    if (forwardAction) {
        forwardAction->setEnabled(tab->navigationHistory()->canGoForward());
    }
}

void MainWindow::filterHistoryList() {
    if (!historyView || !historyFilterInput) {
        return;
    }

    TabEditor* tab = currentTabEditor();
    if (!tab) {
        historyView->clear();
        return;
    }

    QString filterText = historyFilterInput->text().toLower();
    historyView->clear();

    QVector<QString> history = tab->navigationHistory()->getHistory();

    for (int i = history.size() - 1; i >= 0; --i) {
        const QString& filePath = history[i];

        if (!filterText.isEmpty() &&
            !filePath.toLower().contains(filterText)) {
            continue;
        }

        QFileInfo fileInfo(filePath);

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
