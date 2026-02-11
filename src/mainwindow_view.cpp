#include "linkparser.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "tabeditor.h"
#include "managers/windowmanager.h"
#include <QDesktopServices>
#include <QDir>
#include <QFile>
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

void MainWindow::applyViewMode(ViewMode mode) {
  TabEditor *tab = currentTabEditor();
  if (!tab) return;
  
  switch (mode) {
    case ViewMode_Both:
      tab->editor()->setVisible(true);
      tab->preview()->setVisible(true);
      statusBar()->showMessage(tr("View Mode: Editor + Preview"), 2000);
      break;
    case ViewMode_EditorOnly:
      tab->editor()->setVisible(true);
      tab->preview()->setVisible(false);
      statusBar()->showMessage(tr("View Mode: Editor Only"), 2000);
      break;
    case ViewMode_PreviewOnly:
      tab->editor()->setVisible(false);
      tab->preview()->setVisible(true);
      statusBar()->showMessage(tr("View Mode: Preview Only"), 2000);
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

  QString targetFile =
      linkParser->resolveLinkTarget(linkTarget, currentFilePath);

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

void MainWindow::onMarkdownLinkClicked(const QString &linkTarget) {
  if (currentFolder.isEmpty()) {
    statusBar()->showMessage(tr("No folder opened. Cannot resolve markdown link."),
                              3000);
    return;
  }
  QString resolvedPath;
  if (QFileInfo(linkTarget).isAbsolute()) {
    resolvedPath = linkTarget;
  } else {
    QFileInfo currentFileInfo(currentFilePath);
    QDir currentDir = currentFileInfo.dir();
    resolvedPath = currentDir.filePath(linkTarget);
  }
  resolvedPath = QFileInfo(resolvedPath).absoluteFilePath();
  QString finalPath = resolvedPath;
  bool shouldOpenInTreeMk = false;
  if (QFileInfo(resolvedPath).exists()) {
    finalPath = resolvedPath;
    shouldOpenInTreeMk = resolvedPath.endsWith(".md", Qt::CaseInsensitive);
  } else {
    QFileInfo resolvedInfo(resolvedPath);
    if (resolvedInfo.suffix().isEmpty()) {
      // No extension - try .md first
      QString mdPath = resolvedPath + ".md";
      if (QFileInfo(mdPath).exists()) {
        finalPath = mdPath;
        shouldOpenInTreeMk = true;
      } else {
        // Use .md for creation
        finalPath = mdPath;
        shouldOpenInTreeMk = true;
      }
    } else {
      // Has extension but doesn't exist - offer to create if it's .md
      shouldOpenInTreeMk = resolvedPath.endsWith(".md", Qt::CaseInsensitive);
      finalPath = resolvedPath;
    }
  }
  if (QFileInfo(finalPath).exists()) {
    if (shouldOpenInTreeMk) {
      loadFile(finalPath);
    } else {
      QDesktopServices::openUrl(QUrl::fromLocalFile(finalPath));
    }
  } else {
    // File doesn't exist - only offer to create .md files
    if (shouldOpenInTreeMk) {
      QMessageBox::StandardButton reply = QMessageBox::question(
          this, tr("Create File"),
          tr("The file '%1' does not exist. Do you want to create it?")
              .arg(QFileInfo(finalPath).fileName()),
          QMessageBox::Yes | QMessageBox::No);
      if (reply == QMessageBox::Yes) {
        // Create the file with a basic template
        QFile file(finalPath);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
          QTextStream out(&file);
          out.setEncoding(QStringConverter::Utf8);
          out << "# " << QFileInfo(finalPath).baseName() << "\n\n";
          out << "Created from markdown link: " << linkTarget << "\n";
          file.close();
          // Load the newly created file
          loadFile(finalPath);
        } else {
          statusBar()->showMessage(tr("Failed to create file: %1").arg(finalPath),
                                   3000);
        }
      }
    } else {
      // For non-.md files that don't exist, show an error
      statusBar()->showMessage(tr("File not found: %1").arg(finalPath), 3000);
    }
  }
}

void MainWindow::onOpenLinkInNewWindow(const QString &linkTarget) {
  if (currentFolder.isEmpty()) {
    statusBar()->showMessage(tr("No folder opened. Cannot resolve link."), 3000);
    return;
  }
  
  // Strip wiki: or markdown: scheme prefix if present
  QString actualTarget = linkTarget;
  if (actualTarget.startsWith("wiki:")) {
    actualTarget = actualTarget.mid(5); // Remove "wiki:" prefix
  } else if (actualTarget.startsWith("markdown:")) {
    actualTarget = actualTarget.mid(9); // Remove "markdown:" prefix
  }
  
  QString targetFile = linkParser->resolveLinkTarget(actualTarget, currentFilePath);
  
  if (targetFile.isEmpty()) {
    statusBar()->showMessage(tr("Cannot resolve link: %1").arg(actualTarget), 3000);
    return;
  }
  
  QFileInfo info(targetFile);
  
  // If file doesn't exist, create it
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
  
  // Ensure directory exists
  QFileInfo info(newFilePath);
  QDir dir = info.dir();
  if (!dir.exists()) {
    dir.mkpath(".");
  }
  
  // Create the file with basic content
  QFile file(newFilePath);
  if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << "# " << info.baseName() << "\n\n";
    out << "Created from link: " << linkTarget << "\n";
    file.close();
    return true;
  } else {
    statusBar()->showMessage(tr("Failed to create file: %1").arg(newFilePath), 3000);
    return false;
  }
}

void MainWindow::updateBacklinks() {
  backlinksView->clear();
  if (currentFilePath.isEmpty() || currentFolder.isEmpty()) {
    return;
  }
  QVector<QString> backlinks = linkParser->getBacklinks(currentFilePath);
  for (const QString &backlink : backlinks) {
    QListWidgetItem *item =
        new QListWidgetItem(QFileInfo(backlink).fileName(), backlinksView);
    item->setData(Qt::UserRole, backlink);
  }
}

void MainWindow::updatePreview() {
  TabEditor *tab = currentTabEditor();
  if (tab) {
    tab->updatePreview();
  }
}

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

void MainWindow::onDocumentModified() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  updatePreview();
}
