#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>

#include "fileutils.h"
#include "linkparser.h"
#include "mainwindow.h"
#include "managers/windowmanager.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "tabeditor.h"
// #include <QListWidget>
#include <QMessageBox>
#include <QStatusBar>
// #include <QTimer>
#include <QUrl>

void MainWindow::onMarkdownLinkClicked(const QString& linkTarget) {
    if (currentFolder.isEmpty()) {
        statusBar()->showMessage(
            tr("No folder opened. Cannot resolve markdown link."), 3000);
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
            shouldOpenInTreeMk =
                resolvedPath.endsWith(".md", Qt::CaseInsensitive);
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
                QString initialContent =
                    QString("# %1\n\nCreated from markdown link: %2\n")
                        .arg(QFileInfo(finalPath).baseName())
                        .arg(linkTarget);

                FileUtils::FileCreationResult result =
                    FileUtils::createFileWithDirectories(finalPath,
                                                         initialContent);

                if (result.success) {
                    // Load the newly created file
                    loadFile(finalPath);
                } else {
                    statusBar()->showMessage(result.errorMessage, 3000);
                }
            }
        } else {
            // For non-.md files that don't exist, show an error
            statusBar()->showMessage(tr("File not found: %1").arg(finalPath),
                                     3000);
        }
    }
}
