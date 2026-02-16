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

void MainWindow::openFileInExplorer() {
    if (!historyView) {
        return;
    }

    QListWidgetItem* currentItem = historyView->currentItem();
    if (!currentItem) {
        return;
    }

    QString filePath = currentItem->data(Qt::UserRole).toString();
    if (filePath.isEmpty()) {
        return;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, tr("File Not Found"),
                             tr("The file no longer exists:\n%1").arg(filePath));
        return;
    }

    QString dirPath = fileInfo.absolutePath();

#ifdef Q_OS_LINUX
    // Try different file managers in order of preference
    QString desktopEnv = qgetenv("XDG_CURRENT_DESKTOP");
    
    // KDE Plasma - use Dolphin
    if (desktopEnv.contains("KDE", Qt::CaseInsensitive)) {
        if (QProcess::execute("which", QStringList() << "dolphin") == 0) {
            QProcess::startDetached("dolphin", QStringList() << "--select" << filePath);
            return;
        }
    }
    
    // GNOME - use Nautilus
    if (desktopEnv.contains("GNOME", Qt::CaseInsensitive)) {
        if (QProcess::execute("which", QStringList() << "nautilus") == 0) {
            QProcess::startDetached("nautilus", QStringList() << "--select" << filePath);
            return;
        }
    }
    
    // Try generic xdg-open to open the directory
    if (QProcess::execute("which", QStringList() << "xdg-open") == 0) {
        QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
        return;
    }
    
    // Fallback: just open the directory
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
    
#elif defined(Q_OS_WIN)
    // Windows: use explorer.exe with /select parameter
    QProcess::startDetached("explorer.exe", QStringList() << "/select," << QDir::toNativeSeparators(filePath));
    
#elif defined(Q_OS_MAC)
    // macOS: use open -R to reveal in Finder
    QProcess::startDetached("open", QStringList() << "-R" << filePath);
    
#else
    // Generic fallback: open the containing directory
    QDesktopServices::openUrl(QUrl::fromLocalFile(dirPath));
#endif
}
