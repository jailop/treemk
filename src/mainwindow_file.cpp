#include "defs.h"
#include "filesystemtreeview.h"
#include "linkparser.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "tabeditor.h"
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMenu>
#include <QMessageBox>
#include <QStatusBar>

void MainWindow::newFile() { createNewTab(); }

void MainWindow::openFolder() {
  QString folder = QFileDialog::getExistingDirectory(
      this, tr("Open Folder"),
      currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!folder.isEmpty()) {
    treeView->setRootPath(folder);
    currentFolder = folder;

    TabEditor *tab = currentTabEditor();
    if (tab && tab->editor()->getHighlighter()) {
      tab->editor()->getHighlighter()->setRootPath(folder);
    }

    linkParser->buildLinkIndex(folder);

    updateRecentFolders(folder);

    statusBar()->showMessage(tr("Opened folder: %1").arg(folder));
  }
}

void MainWindow::openRecentFolder() {
  QAction *action = qobject_cast<QAction *>(sender());
  if (!action) {
    return;
  }

  QString folder = action->data().toString();
  if (!QDir(folder).exists()) {
    QMessageBox::warning(this, tr("Folder Not Found"),
                         tr("The folder '%1' no longer exists.").arg(folder));
    recentFolders.removeAll(folder);
    populateRecentFoldersMenu();
    return;
  }

  treeView->setRootPath(folder);
  currentFolder = folder;

  TabEditor *tab = currentTabEditor();
  if (tab && tab->editor()->getHighlighter()) {
    tab->editor()->getHighlighter()->setRootPath(folder);
  }

  linkParser->buildLinkIndex(folder);

  statusBar()->showMessage(tr("Opened folder: %1").arg(folder));
}

void MainWindow::clearRecentFolders() {
  recentFolders.clear();
  populateRecentFoldersMenu();
}

void MainWindow::updateRecentFolders(const QString &folder) {
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
    QAction *noRecent = recentFoldersMenu->addAction(tr("(No recent folders)"));
    noRecent->setEnabled(false);
  } else {
    for (const QString &folder : recentFolders) {
      QAction *action = recentFoldersMenu->addAction(folder);
      action->setData(folder);
      connect(action, &QAction::triggered, this, &MainWindow::openRecentFolder);
    }
    recentFoldersMenu->addSeparator();
    QAction *clearAction =
        recentFoldersMenu->addAction(tr("Clear Recent Folders"));
    connect(clearAction, &QAction::triggered, this,
            &MainWindow::clearRecentFolders);
  }
}

void MainWindow::save() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;
  if (tab->filePath().isEmpty()) {
    saveAs();
  } else {
    if (tab->saveFile()) {
      statusBar()->showMessage(tr("File saved"), 2000);
    }
  }
}

void MainWindow::saveAs() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

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
        linkParser->buildLinkIndex(currentFolder);
      }
      updateBacklinks();
      statusBar()->showMessage(
          tr("File saved as: %1").arg(QFileInfo(fileName).fileName()), 2000);
    }
  }
}

void MainWindow::onFileSelected(const QString &filePath) {
  // File selection logic if needed
}

void MainWindow::onFileDoubleClicked(const QString &filePath) {
  if (!filePath.isEmpty() && QFileInfo(filePath).isFile()) {
    loadFile(filePath);
  }
}

void MainWindow::onFileModifiedExternally(const QString &filePath) {
  TabEditor *tab = findTabByPath(filePath);
  if (!tab)
    return;
  if (tab->ownSaved()) {
    // The file was modified by this app, so there is nothing to do.
    // Just clear the flag
    tab->clearOwnSaved();
    return;
  }
  QMessageBox::StandardButton reply = QMessageBox::question(
      this, tr("File Modified"),
      tr("The file %1 has been modified externally. Do you want to reload it?")
          .arg(QFileInfo(filePath).fileName()),
      QMessageBox::Yes | QMessageBox::No);
  if (reply == QMessageBox::Yes) {
    tab->loadFile(filePath);
    statusBar()->showMessage(tr("File reloaded"), 2000);
  }
}

bool MainWindow::maybeSave() {
  for (int i = 0; i < tabWidget->count(); ++i) {
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && tab->isModified()) {
      tabWidget->setCurrentIndex(i);
      QMessageBox::StandardButton ret = QMessageBox::warning(
          this, tr("Unsaved Changes"),
          tr("The document '%1' has been modified.\n"
             "Do you want to save your changes?")
              .arg(tab->fileName()),
          QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

      if (ret == QMessageBox::Save) {
        save();
        if (tab->isModified()) {
          return false;
        }
      } else if (ret == QMessageBox::Cancel) {
        return false;
      }
    }
  }
  return true;
}

bool MainWindow::loadFile(const QString &filePath) {
  TabEditor *tab = findTabByPath(filePath);
  if (tab) {
    int index = tabWidget->indexOf(tab);
    tabWidget->setCurrentIndex(index);
    return true;
  }

  if (tabWidget->count() == 1) {
    TabEditor *firstTab = qobject_cast<TabEditor *>(tabWidget->widget(0));
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
    TabEditor *tab = qobject_cast<TabEditor *>(tabWidget->widget(i));
    if (tab && tab->isModified() && !tab->filePath().isEmpty()) {
      tab->saveFile();
    }
  }
}
