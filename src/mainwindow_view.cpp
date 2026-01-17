#include "linkparser.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "tabeditor.h"
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
      // Create new file
      QString newFilePath = targetFile;
      if (QFileInfo(newFilePath).suffix().isEmpty()) {
        newFilePath += ".md";
      }
      // Create the file with a basic template
      QFile file(newFilePath);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << "# " << QFileInfo(newFilePath).baseName() << "\n\n";
        out << "Created from link: " << linkTarget << "\n";
        file.close();
        // Load the newly created file
        loadFile(newFilePath);
      } else {
        statusBar()->showMessage(tr("Failed to create file: %1").arg(newFilePath),
                                 3000);
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
