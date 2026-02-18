#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QStatusBar>
#include <QTimer>
#include <QUrl>

#include "fileutils.h"
#include "linkparser.h"
#include "mainwindow.h"
#include "managers/windowmanager.h"
#include "markdowneditor.h"
#include "markdownpreview.h"
#include "tabeditor.h"

void MainWindow::onMarkdownLinkClicked(const QString& linkTarget) {
    if (currentFolder.isEmpty()) {
        statusBar()->showMessage(
            tr("No folder opened. Cannot resolve markdown link."), 3000);
        return;
    }
    
    // Extract label text from current editor if available
    QString labelText;
    TabEditor* tab = currentTabEditor();
    if (tab) {
        QTextCursor cursor = tab->editor()->textCursor();
        tab->editor()->getMarkdownLinkAtPosition(cursor.position(), labelText);
    }
    
    LinkTarget parsed = LinkParser::parseLinkTarget(linkTarget);
    
    if (parsed.isInternalOnly) {
        onInternalLinkClicked(parsed.anchor);
        return;
    }
    
    QString resolvedPath;
    if (QFileInfo(parsed.filePath).isAbsolute()) {
        resolvedPath = parsed.filePath;
    } else {
        QFileInfo currentFileInfo(currentFilePath);
        QDir currentDir = currentFileInfo.dir();
        resolvedPath = currentDir.filePath(parsed.filePath);
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
            QString mdPath = resolvedPath + ".md";
            if (QFileInfo(mdPath).exists()) {
                finalPath = mdPath;
                shouldOpenInTreeMk = true;
            } else {
                finalPath = mdPath;
                shouldOpenInTreeMk = true;
            }
        } else {
            shouldOpenInTreeMk =
                resolvedPath.endsWith(".md", Qt::CaseInsensitive);
            finalPath = resolvedPath;
        }
    }
    if (QFileInfo(finalPath).exists()) {
        if (shouldOpenInTreeMk) {
            loadFile(finalPath);
            if (!parsed.anchor.isEmpty()) {
                QTimer::singleShot(100, [this, parsed]() {
                    onInternalLinkClicked(parsed.anchor);
                });
            }
        } else {
            QDesktopServices::openUrl(QUrl::fromLocalFile(finalPath));
        }
    } else {
        if (shouldOpenInTreeMk) {
            QMessageBox::StandardButton reply = QMessageBox::question(
                this, tr("Create File"),
                tr("The file '%1' does not exist. Do you want to create it?")
                    .arg(QFileInfo(finalPath).fileName()),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::Yes) {
                QString title = labelText.isEmpty() 
                    ? QFileInfo(finalPath).baseName() 
                    : labelText;
                
                QString initialContent = title.isEmpty()
                    ? QString("\n")
                    : QString("# %1\n\n").arg(title);

                FileUtils::FileCreationResult result =
                    FileUtils::createFileWithDirectories(finalPath,
                                                         initialContent);

                if (result.success) {
                    loadFile(finalPath);
                    if (!parsed.anchor.isEmpty()) {
                        QTimer::singleShot(100, [this, parsed]() {
                            onInternalLinkClicked(parsed.anchor);
                        });
                    }
                } else {
                    statusBar()->showMessage(result.errorMessage, 3000);
                }
            }
        } else {
            statusBar()->showMessage(tr("File not found: %1").arg(finalPath),
                                     3000);
        }
    }
}

void MainWindow::onInternalLinkClicked(const QString& anchor) {
    TabEditor* currentTab = currentTabEditor();
    if (!currentTab) {
        return;
    }
    
    currentTab->editor()->jumpToHeading(anchor);
    currentTab->preview()->scrollToAnchor(anchor);
}
