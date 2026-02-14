#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QStatusBar>
#include <QTextCursor>
#include <QTextDocument>
#include <algorithm>

#include "defs.h"
#include "filesystemtreeview.h"
#include "linkparser.h"
#include "logic/mainfilelocator.h"
#include "mainwindow.h"
#include "managers/windowmanager.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "navigationhistory.h"
#include "tabeditor.h"

void MainWindow::newFile() { createNewTab(); }

void MainWindow::openFolder() {
    QString folder = QFileDialog::getExistingDirectory(
        this, tr("Open Folder"),
        currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (!folder.isEmpty()) {
        // Create new window with the selected folder
        WindowManager::instance()->createWindow(folder, QString());
    }
}

void MainWindow::openRecentFolder() {
    QAction* action = qobject_cast<QAction*>(sender());
    if (!action) {
        return;
    }

    QString folder = action->data().toString();
    if (!QDir(folder).exists()) {
        QMessageBox::warning(
            this, tr("Folder Not Found"),
            tr("The folder '%1' no longer exists.").arg(folder));
        recentFolders.removeAll(folder);
        populateRecentFoldersMenu();
        return;
    }

    // Close tabs from the previous folder
    closeTabsFromOtherFolders();

    treeView->setRootPath(folder);
    currentFolder = folder;

    TabEditor* tab = currentTabEditor();
    if (tab && tab->editor()->getHighlighter()) {
        tab->editor()->getHighlighter()->setRootPath(folder);
    }

    buildLinkIndexAsync();

    updateRecentFolders(folder);

    statusBar()->showMessage(tr("Opened folder: %1").arg(folder));

    closeTabsFromOtherFolders();
}

void MainWindow::clearRecentFolders() {
    recentFolders.clear();
    settings->setValue("recentFolders", recentFolders);
    populateRecentFoldersMenu();
}

void MainWindow::updateRecentFolders(const QString& folder) {
    recentFolders.removeAll(folder);
    recentFolders.prepend(folder);
    if (recentFolders.size() > 10) {
        recentFolders.removeLast();
    }
    populateRecentFoldersMenu();
}

void MainWindow::populateRecentFoldersMenu() {
    recentFoldersMenu->clear();

    if (recentFolders.isEmpty()) {
        QAction* noRecent =
            recentFoldersMenu->addAction(tr("(No recent folders)"));
        noRecent->setEnabled(false);
    } else {
        for (const QString& folder : recentFolders) {
            QAction* action = recentFoldersMenu->addAction(folder);
            action->setData(folder);
            connect(action, &QAction::triggered, this,
                    &MainWindow::openRecentFolder);
        }
        recentFoldersMenu->addSeparator();
        QAction* clearAction =
            recentFoldersMenu->addAction(tr("Clear Recent Folders"));
        connect(clearAction, &QAction::triggered, this,
                &MainWindow::clearRecentFolders);
    }
}

void MainWindow::save() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;
    if (tab->filePath().isEmpty()) {
        saveAs();
    } else {
        if (tab->saveFile()) {
            statusBar()->showMessage(tr("File saved"), 2000);
        }
    }
}

void MainWindow::saveAs() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    QString initialPath =
        currentFolder.isEmpty() ? QDir::homePath() : currentFolder;
    if (!tab->filePath().isEmpty()) {
        initialPath = tab->filePath();
    }

    QString fileName = QFileDialog::getSaveFileName(
        this, tr("Save File"), initialPath,
        tr("Markdown Files (*.md);;Text Files (*.txt);;All Files (*)"));

    if (!fileName.isEmpty()) {
        if (tab->saveFileAs(fileName)) {
            currentFilePath = fileName;
            if (!currentFolder.isEmpty()) {
                buildLinkIndexAsync();
            }
            updateBacklinks();
            statusBar()->showMessage(
                tr("File saved as: %1").arg(QFileInfo(fileName).fileName()),
                2000);
        }
    }
}

void MainWindow::onFileSelected(const QString& filePath) {
    // File selection logic if needed
}

void MainWindow::onFileDoubleClicked(const QString& filePath) {
    if (!filePath.isEmpty() && QFileInfo(filePath).isFile()) {
        loadFile(filePath);
    }
}

void MainWindow::onFileModifiedExternally(const QString& filePath) {
    TabEditor* tab = findTabByPath(filePath);
    if (!tab) return;
    if (tab->ownSaved()) {
        // The file was modified by this app, so there is nothing to do.
        // Just clear the flag
        tab->clearOwnSaved();
        return;
    }
    QMessageBox::StandardButton reply =
        QMessageBox::question(this, tr("File Modified"),
                              tr("The file %1 has been modified externally. Do "
                                 "you want to reload it?")
                                  .arg(QFileInfo(filePath).fileName()),
                              QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        tab->loadFile(filePath);
        statusBar()->showMessage(tr("File reloaded"), 2000);
    }
}

void MainWindow::onFolderChanged(const QString& folderPath) {
    // Update recent folders
    updateRecentFolders(folderPath);

    // Close all tabs that are not in the new folder
    QDir newFolder(folderPath);
    QString canonicalFolderPath = newFolder.canonicalPath();

    for (int i = tabWidget->count() - 1; i >= 0; --i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab) {
            QString tabFilePath = tab->filePath();
            if (!tabFilePath.isEmpty()) {
                QFileInfo fileInfo(tabFilePath);
                QString fileFolder = fileInfo.canonicalPath();

                // Check if file is in the new folder or its subfolders
                if (!fileFolder.startsWith(canonicalFolderPath)) {
                    tabWidget->removeTab(i);
                }
            }
        }
    }

    statusBar()->showMessage(
        tr("Current folder changed to: %1").arg(folderPath), 3000);
}

void MainWindow::onFileDeleted(const QString& filePath) {
    int tabIndex = findTabIndexByPath(filePath);
    if (tabIndex >= 0) {
        tabWidget->removeTab(tabIndex);
        statusBar()->showMessage(tr("Closed tab for deleted file: %1")
                                     .arg(QFileInfo(filePath).fileName()),
                                 3000);
    }
}

void MainWindow::onFileRenamed(const QString& oldPath, const QString& newPath) {
    TabEditor* tab = findTabByPath(oldPath);
    if (tab) {
        int tabIndex = findTabIndexByPath(oldPath);
        tab->setFilePath(newPath);
        if (tabIndex >= 0) {
            tabWidget->setTabText(tabIndex, QFileInfo(newPath).fileName());
            tabWidget->setTabToolTip(tabIndex, newPath);
        }
        statusBar()->showMessage(
            tr("File renamed: %1").arg(QFileInfo(newPath).fileName()), 3000);
    }
}

bool MainWindow::maybeSave() {
    // First pass: count modified documents
    QList<TabEditor*> modifiedTabs;
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->isModified()) {
            // Skip empty documents - they can be discarded without prompting
            bool isDocumentEmpty =
                tab->editor()->toPlainText().trimmed().isEmpty();
            if (!isDocumentEmpty) {
                modifiedTabs.append(tab);
            }
        }
    }

    // If no modified tabs, just close
    if (modifiedTabs.isEmpty()) {
        return true;
    }

    // If multiple modified tabs, offer "Save All" option
    bool saveAll = false;

    for (int i = 0; i < modifiedTabs.count(); ++i) {
        TabEditor* tab = modifiedTabs[i];
        int tabIndex = tabWidget->indexOf(tab);
        tabWidget->setCurrentIndex(tabIndex);

        QMessageBox msgBox(this);
        msgBox.setWindowTitle(tr("Unsaved Changes"));
        msgBox.setText(tr("The document '%1' has been modified.\n"
                          "Do you want to save your changes?")
                           .arg(tab->fileName()));
        msgBox.setIcon(QMessageBox::Warning);

        auto* saveButton =
            msgBox.addButton(tr("Save"), QMessageBox::AcceptRole);
        msgBox.addButton(tr("Discard"), QMessageBox::DestructiveRole);
        auto* cancelButton =
            msgBox.addButton(tr("Cancel"), QMessageBox::RejectRole);
        QPushButton* saveAllButton = nullptr;

        // Add "Save All" button only if multiple modified tabs and not yet
        // triggered
        if (modifiedTabs.count() > 1 && !saveAll) {
            saveAllButton =
                msgBox.addButton(tr("Save All"), QMessageBox::AcceptRole);
        }

        msgBox.exec();

        auto* clicked = msgBox.clickedButton();

        if (clicked == static_cast<QAbstractButton*>(saveButton)) {
            save();
            if (tab->isModified()) {
                return false;  // Save failed
            }
        } else if (saveAllButton &&
                   clicked == static_cast<QAbstractButton*>(saveAllButton)) {
            saveAll = true;
            // Save this tab and all remaining modified tabs
            for (int j = i; j < modifiedTabs.count(); ++j) {
                TabEditor* t = modifiedTabs[j];
                int idx = tabWidget->indexOf(t);
                tabWidget->setCurrentIndex(idx);
                save();
                if (t->isModified()) {
                    return false;  // Save failed
                }
            }
            return true;  // All saved successfully
        } else if (clicked == static_cast<QAbstractButton*>(cancelButton)) {
            return false;
        }
        // If Discard was clicked, continue to next tab
    }

    return true;
}

bool MainWindow::loadFile(const QString& filePath) {
    TabEditor* tab = findTabByPath(filePath);
    if (tab) {
        int index = tabWidget->indexOf(tab);
        tabWidget->setCurrentIndex(index);
        navigationHistory->addFile(filePath);
        return true;
    }

    if (tabWidget->count() == 1) {
        TabEditor* firstTab = qobject_cast<TabEditor*>(tabWidget->widget(0));
        if (firstTab && firstTab->filePath().isEmpty() &&
            !firstTab->editor()->isModified()) {
            tab = firstTab;
        }
    }

    if (!tab) {
        tab = createNewTab();
    }

    if (tab->loadFile(filePath)) {
        currentFilePath = filePath;
        navigationHistory->addFile(filePath);

        if (!recentFiles.contains(filePath)) {
            recentFiles.prepend(filePath);
            if (recentFiles.size() > 10) {
                recentFiles.removeLast();
            }
        }

        updateBacklinks();
        setWindowTitle(
            QString("%1 - %2").arg(QFileInfo(filePath).fileName(), APP_LABEL));
        statusBar()->showMessage(
            tr("Loaded: %1").arg(QFileInfo(filePath).fileName()), 3000);
        return true;
    }

    QMessageBox::warning(this, tr("Error"), tr("Could not load file"));
    return false;
}

void MainWindow::autoSave() {
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->isModified() && !tab->filePath().isEmpty()) {
            tab->saveFile();
        }
    }
}
