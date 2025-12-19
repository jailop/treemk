#include "mainwindow.h"
#include "tabeditor.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "linkparser.h"
#include <QTimer>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QStatusBar>
#include <QListWidget>

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

  QString targetFile = linkParser->resolveLinkTarget(linkTarget, currentFilePath);

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
      QFile file(targetFile);
      if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        file.close();
        loadFile(targetFile);
      } else {
        QMessageBox::warning(this, tr("Error"),
                             tr("Could not create file: %1")
                                 .arg(QFileInfo(targetFile).fileName()));
      }
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
