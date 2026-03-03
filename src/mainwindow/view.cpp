#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QListWidget>
#include <QMessageBox>
#include <QScrollBar>
#include <QSplitter>
#include <QStatusBar>
#include <QTimer>
#include <QUrl>

#include "fileutils.h"
#include "linkparser.h"
#include "mainwindow.h"
#include "managers/windowmanager.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "sidebarpanel.h"
#include "tabeditor.h"

void MainWindow::toggleSidebar() {
    sidebarPanel->setVisible(!sidebarPanel->isVisible());
}

void MainWindow::toggleEditor() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    bool editorVisible = tabWidget->isVisible();
    bool previewVisible = sharedPreview->isVisible();

    // Prevent hiding both
    if (editorVisible && !previewVisible) {
        // Editor is the only visible, can't hide it
        statusBar()->showMessage(
            tr("Cannot hide editor when preview is already hidden"), 2000);
        return;
    }

    // Toggle editor (entire tabWidget)
    bool newEditorVisible = !editorVisible;
    tabWidget->setVisible(newEditorVisible);

    // Update splitter sizes to force re-layout
    if (!newEditorVisible && previewVisible) {
        // Editor hidden - give all space to preview
        QList<int> sizes = editorPreviewSplitter->sizes();
        int total = sizes[0] + sizes[1];
        editorPreviewSplitter->setSizes(QList<int>() << 0 << total);
        statusBar()->showMessage(tr("Editor hidden"), 2000);
    } else if (newEditorVisible && previewVisible) {
        // Both visible - split equally
        QList<int> sizes = editorPreviewSplitter->sizes();
        int total = sizes[0] + sizes[1];
        editorPreviewSplitter->setSizes(QList<int>() << total/2 << total/2);
        statusBar()->showMessage(tr("Editor visible"), 2000);
    }

    // Update action checkstate
    if (toggleEditorAction) {
        toggleEditorAction->setChecked(newEditorVisible);
    }
    
    // Enable/disable the preview toggle based on editor visibility
    if (togglePreviewAction) {
        togglePreviewAction->setEnabled(newEditorVisible);
    }
}

void MainWindow::togglePreview() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    bool editorVisible = tabWidget->isVisible();
    bool previewVisible = sharedPreview->isVisible();

    // Prevent hiding both
    if (!editorVisible && previewVisible) {
        // Preview is the only visible, can't hide it
        statusBar()->showMessage(
            tr("Cannot hide preview when editor is already hidden"), 2000);
        return;
    }

    // Toggle preview
    bool newPreviewVisible = !previewVisible;
    sharedPreview->setVisible(newPreviewVisible);

    // Update splitter sizes to force re-layout
    if (editorVisible && !newPreviewVisible) {
        // Preview hidden - give all space to editor
        QList<int> sizes = editorPreviewSplitter->sizes();
        int total = sizes[0] + sizes[1];
        editorPreviewSplitter->setSizes(QList<int>() << total << 0);
        statusBar()->showMessage(tr("Preview hidden"), 2000);
    } else if (editorVisible && newPreviewVisible) {
        // Both visible - split equally
        QList<int> sizes = editorPreviewSplitter->sizes();
        int total = sizes[0] + sizes[1];
        editorPreviewSplitter->setSizes(QList<int>() << total/2 << total/2);
        statusBar()->showMessage(tr("Preview visible"), 2000);
    }

    // Update action checkstate
    if (togglePreviewAction) {
        togglePreviewAction->setChecked(newPreviewVisible);
    }
    
    // Enable/disable the editor toggle based on preview visibility
    if (toggleEditorAction) {
        toggleEditorAction->setEnabled(newPreviewVisible);
    }
}

void MainWindow::onWikiLinkClicked(const QString& linkTarget) {
    if (currentFolder.isEmpty()) {
        statusBar()->showMessage(
            tr("No folder opened. Cannot resolve wiki link."), 3000);
        return;
    }

    // Extract display text from current editor if available
    QString displayText;
    TabEditor* tab = currentTabEditor();
    if (tab) {
        QTextCursor cursor = tab->editor()->textCursor();
        tab->editor()->getLinkAtPosition(cursor.position(), displayText);
    }

    int depth = getLinkSearchDepth();
    QString targetFile =
        linkParser->resolveLinkTarget(linkTarget, currentFilePath, depth);

    if (targetFile.isEmpty()) {
        // File not found, construct path for potential creation
        QFileInfo currentFileInfo(currentFilePath);
        QDir currentDir = currentFileInfo.dir();
        targetFile = currentDir.filePath(linkTarget);
        
        // Add .md extension if not present
        if (QFileInfo(targetFile).suffix().isEmpty()) {
            targetFile += ".md";
        }
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
            QString title = displayText.isEmpty() 
                ? QFileInfo(targetFile).baseName() 
                : displayText;
            
            QString initialContent = title.isEmpty()
                ? QString("\n")
                : QString("# %1\n\n").arg(title);

            FileUtils::FileCreationResult result =
                FileUtils::createFileWithDirectories(targetFile, initialContent);

            if (result.success) {
                loadFile(targetFile);
            } else {
                statusBar()->showMessage(result.errorMessage, 3000);
            }
        }
    }
}

void MainWindow::onOpenLinkInNewWindow(const QString& linkTarget) {
    if (currentFolder.isEmpty()) {
        statusBar()->showMessage(tr("No folder opened. Cannot resolve link."),
                                 3000);
        return;
    }

    QString actualTarget = linkTarget;
    if (actualTarget.startsWith("wiki:")) {
        actualTarget = actualTarget.mid(5);
    } else if (actualTarget.startsWith("markdown:")) {
        actualTarget = actualTarget.mid(9);
    }

    int depth = getLinkSearchDepth();
    QString targetFile =
        linkParser->resolveLinkTarget(actualTarget, currentFilePath, depth);

    if (targetFile.isEmpty()) {
        statusBar()->showMessage(
            tr("Cannot resolve link: %1").arg(actualTarget), 3000);
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

bool MainWindow::createFileFromLink(const QString& targetFile,
                                    const QString& linkTarget,
                                    const QString& label) {
    QString newFilePath = targetFile;
    if (QFileInfo(newFilePath).suffix().isEmpty()) {
        newFilePath += ".md";
    }

    QString title = label.isEmpty() 
        ? QFileInfo(newFilePath).baseName() 
        : label;
    
    QString initialContent = title.isEmpty()
        ? QString("\n")
        : QString("# %1\n\n").arg(title);

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

    for (const QString& backlink : backlinks) {
        QString relativePath = workspaceDir.relativeFilePath(backlink);
        QListWidgetItem* item =
            new QListWidgetItem(relativePath, backlinksView);
        item->setData(Qt::UserRole, backlink);
    }
}

void MainWindow::updatePreview() {
    TabEditor* tab = currentTabEditor();
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
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    updatePreview();
}
