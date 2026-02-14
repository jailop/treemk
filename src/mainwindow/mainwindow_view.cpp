#include "linkparser.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "tabeditor.h"
#include "managers/windowmanager.h"
#include "fileutils.h"
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QListWidget>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QUrl>

void MainWindow::toggleSidebar() {
  leftTabWidget->setVisible(!leftTabWidget->isVisible());
}

void MainWindow::togglePreview() {
  TabEditor *tab = currentTabEditor();
  if (tab) {
    tab->preview()->setVisible(!tab->preview()->isVisible());
  }
}

void MainWindow::applyViewMode(ViewMode mode, bool showStatusMessage) {
  TabEditor *tab = currentTabEditor();
  if (!tab) return;
  
  switch (mode) {
    case ViewMode_Both:
      tab->editor()->setVisible(true);
      tab->preview()->setVisible(true);
      if (showStatusMessage) {
        statusBar()->showMessage(tr("View Mode: Editor + Preview"), 2000);
      }
      break;
    case ViewMode_EditorOnly:
      tab->editor()->setVisible(true);
      tab->preview()->setVisible(false);
      if (showStatusMessage) {
        statusBar()->showMessage(tr("View Mode: Editor Only"), 2000);
      }
      break;
    case ViewMode_PreviewOnly:
      tab->editor()->setVisible(false);
      tab->preview()->setVisible(true);
      if (showStatusMessage) {
        statusBar()->showMessage(tr("View Mode: Preview Only"), 2000);
      }
      break;
  }
  
  currentViewMode = mode;
}

void MainWindow::cycleViewMode() {
  // Cycle through view modes
  switch (currentViewMode) {
    case ViewMode_Both:
      currentViewMode = ViewMode_EditorOnly;
      break;
    case ViewMode_EditorOnly:
      currentViewMode = ViewMode_PreviewOnly;
      break;
    case ViewMode_PreviewOnly:
      currentViewMode = ViewMode_Both;
      break;
  }
  
  applyViewMode(currentViewMode);
  
  // Update action text to show next mode
  QString nextModeText;
  switch (currentViewMode) {
    case ViewMode_Both:
      nextModeText = tr("Cycle View Mode (Next: Editor Only)");
      break;
    case ViewMode_EditorOnly:
      nextModeText = tr("Cycle View Mode (Next: Preview Only)");
      break;
    case ViewMode_PreviewOnly:
      nextModeText = tr("Cycle View Mode (Next: Both)");
      break;
  }
  if (cycleViewModeAction) {
    cycleViewModeAction->setText(nextModeText);
  }
  
  // Save to settings
  settings->setValue("viewMode", static_cast<int>(currentViewMode));
}

void MainWindow::onWikiLinkClicked(const QString &linkTarget) {
  if (currentFolder.isEmpty()) {
    statusBar()->showMessage(tr("No folder opened. Cannot resolve wiki link."),
                              3000);
    return;
  }

  int depth = getLinkSearchDepth();
  QString targetFile =
      linkParser->resolveLinkTarget(linkTarget, currentFilePath, depth);

  if (targetFile.isEmpty()) {
    statusBar()->showMessage(
        tr("Wiki link target not found: %1").arg(linkTarget), 3000);
    return;
  }

  if (QFileInfo(targetFile).exists()) {
    loadFile(targetFile);
  } else {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Create File"),
        tr("The file '%1' does not exist. Do you want to create it?")
            .arg(QFileInfo(targetFile).fileName()),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
      if (createFileFromLink(targetFile, linkTarget)) {
        // Adjust path with .md extension if it was added
        QString newFilePath = targetFile;
        if (QFileInfo(newFilePath).suffix().isEmpty()) {
          newFilePath += ".md";
        }
        loadFile(newFilePath);
      }
    }
  }
}

void MainWindow::onOpenLinkInNewWindow(const QString &linkTarget) {
  if (currentFolder.isEmpty()) {
    statusBar()->showMessage(tr("No folder opened. Cannot resolve link."), 3000);
    return;
  }
  
  QString actualTarget = linkTarget;
  if (actualTarget.startsWith("wiki:")) {
    actualTarget = actualTarget.mid(5);
  } else if (actualTarget.startsWith("markdown:")) {
    actualTarget = actualTarget.mid(9);
  }
  
  int depth = getLinkSearchDepth();
  QString targetFile = linkParser->resolveLinkTarget(actualTarget, currentFilePath, depth);
  
  if (targetFile.isEmpty()) {
    statusBar()->showMessage(tr("Cannot resolve link: %1").arg(actualTarget), 3000);
    return;
  }
  
  QFileInfo info(targetFile);
  
  if (!info.exists()) {
    if (!createFileFromLink(targetFile, actualTarget)) {
      return;
    }
  }
  
  // Open in new window
  WindowManager::instance()->createWindow(info.absolutePath(), targetFile);
}

bool MainWindow::createFileFromLink(const QString &targetFile, const QString &linkTarget) {
  QString newFilePath = targetFile;
  if (QFileInfo(newFilePath).suffix().isEmpty()) {
    newFilePath += ".md";
  }
  
  QString initialContent = QString("# %1\n\nCreated from link: %2\n")
      .arg(QFileInfo(newFilePath).baseName())
      .arg(linkTarget);
  
  FileUtils::FileCreationResult result = 
      FileUtils::createFileWithDirectories(newFilePath, initialContent);
  
  if (result.success) {
    return true;
  } else {
    statusBar()->showMessage(result.errorMessage, 3000);
    return false;
  }
}

void MainWindow::updateBacklinks() {
  backlinksView->clear();
  if (currentFilePath.isEmpty() || currentFolder.isEmpty()) {
    return;
  }
  
  QDir workspaceDir(currentFolder);
  QVector<QString> backlinks = linkParser->getBacklinks(currentFilePath);
  
  for (const QString &backlink : backlinks) {
    QString relativePath = workspaceDir.relativeFilePath(backlink);
    QListWidgetItem *item = new QListWidgetItem(relativePath, backlinksView);
    item->setData(Qt::UserRole, backlink);
  }
}

void MainWindow::updatePreview() {
  TabEditor *tab = currentTabEditor();
  if (tab) {
    tab->updatePreview();
  }
}

/* TODO to be removed. theme is general for all the app, not just
 * preview.
 *
void MainWindow::setPreviewThemeLight() {
  TabEditor *tab = currentTabEditor();
  if (tab) {
    tab->preview()->setTheme("light");
    settings->setValue("previewTheme", "light");
  }
}

void MainWindow::setPreviewThemeDark() {
  TabEditor *tab = currentTabEditor();
  if (tab) {
    tab->preview()->setTheme("dark");
    settings->setValue("previewTheme", "dark");
  }
}

void MainWindow::setPreviewThemeSepia() {
  TabEditor *tab = currentTabEditor();
  if (tab) {
    tab->preview()->setTheme("sepia");
    settings->setValue("previewTheme", "sepia");
  }
}
*/

void MainWindow::onDocumentModified() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  updatePreview();
}
