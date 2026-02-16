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
#include "regexpatterns.h"
#include "tabeditor.h"

static const char* HTTP_PREFIX = "http://";
static const char* HTTPS_PREFIX = "https://";

static const int WIKI_PREFIX_GROUP = 1;   // "!" or empty
static const int WIKI_TARGET_GROUP = 2;   // target filename
static const int WIKI_PIPE_GROUP = 3;     // pipe and display text (optional)
static const int WIKI_DISPLAY_GROUP = 4;  // display text after pipe

static const int MD_PREFIX_GROUP = 1;  // "!" or empty
static const int MD_TEXT_GROUP = 2;    // link text
static const int MD_URL_GROUP = 3;     // URL or path

static void updateLinksInDocument(QTextDocument* document,
                                  const QString& oldFileName,
                                  const QString& newFileName) {
    if (!document || oldFileName.isEmpty() || newFileName.isEmpty()) {
        return;
    }

    QString content = document->toPlainText();
    QString updatedContent = content;
    bool modified = false;

    QList<QPair<int, QPair<int, QString>>>
        replacements;  // position, length, replacement

    QRegularExpression wikiLinkPattern(RegexPatterns::WIKI_LINK);
    QRegularExpressionMatchIterator wikiIterator =
        wikiLinkPattern.globalMatch(content);

    while (wikiIterator.hasNext()) {
        QRegularExpressionMatch match = wikiIterator.next();
        QString prefix = match.captured(WIKI_PREFIX_GROUP);  // "!" or empty
        QString target = match.captured(WIKI_TARGET_GROUP).trimmed();

        // Check if this link references the old file
        // Extract just the filename without path for comparison
        QString targetFileName = QFileInfo(target).fileName();

        // Handle both with and without extension
        QString targetBase = targetFileName;
        QString targetExt;
        int dotPos = targetBase.lastIndexOf('.');
        if (dotPos > 0) {
            targetExt = targetBase.mid(dotPos);
            targetBase = targetBase.left(dotPos);
        }

        QString oldFileBase = oldFileName;
        QString oldFileExt;
        dotPos = oldFileBase.lastIndexOf('.');
        if (dotPos > 0) {
            oldFileExt = oldFileBase.mid(dotPos);
            oldFileBase = oldFileBase.left(dotPos);
        }

        if (targetBase == oldFileBase || targetFileName == oldFileName) {
            // Build replacement - preserve the path structure if present
            QString newTarget;
            if (target.contains('/') || target.contains('\\')) {
                // Has path, replace just the filename part
                QString path = QFileInfo(target).path();
                if (path == ".") {
                    newTarget = newFileName;
                } else {
                    newTarget = path + "/" + newFileName;
                }
            } else {
                newTarget = newFileName;
            }

            // Remove extension if original didn't have it
            if (targetExt.isEmpty() && newTarget.contains('.')) {
                int lastDot = newTarget.lastIndexOf('.');
                if (lastDot > 0) {
                    newTarget = newTarget.left(lastDot);
                }
            }

            QString replacement;
            if (match.captured(WIKI_PIPE_GROUP).isEmpty()) {
                // Simple link: [[target]] or ![[target]]
                replacement = QString("%1[[%2]]").arg(prefix, newTarget);
            } else {
                // Link with display: [[target|display]] or ![[target|display]]
                replacement = QString("%1[[%2|%3]]")
                                  .arg(prefix, newTarget,
                                       match.captured(WIKI_DISPLAY_GROUP));
            }

            replacements.append(
                qMakePair(match.capturedStart(),
                          qMakePair(match.capturedLength(), replacement)));
            modified = true;
        }
    }

    // Pattern for markdown links with optional !: ![text](url) or [text](url)
    QRegularExpression markdownLinkPattern(
        RegexPatterns::MARKDOWN_LINK_WITH_IMAGE);
    QRegularExpressionMatchIterator mdIterator =
        markdownLinkPattern.globalMatch(content);

    while (mdIterator.hasNext()) {
        QRegularExpressionMatch match = mdIterator.next();
        QString prefix = match.captured(MD_PREFIX_GROUP);  // "!" or empty
        QString text = match.captured(MD_TEXT_GROUP);
        QString url = match.captured(MD_URL_GROUP).trimmed();

        // Skip external links
        if (url.startsWith(HTTP_PREFIX) || url.startsWith(HTTPS_PREFIX)) {
            continue;
        }

        // Extract just the filename from the path
        QString urlFileName = QFileInfo(url).fileName();

        if (urlFileName == oldFileName) {
            // Build replacement with the same relative path structure
            QString newUrl = url;
            newUrl.replace(oldFileName, newFileName);
            QString replacement =
                QString("%1[%2](%3)").arg(prefix, text, newUrl);

            replacements.append(
                qMakePair(match.capturedStart(),
                          qMakePair(match.capturedLength(), replacement)));
            modified = true;
        }
    }

    if (!modified) {
        return;
    }

    // Apply replacements in reverse order to maintain positions
    std::sort(replacements.begin(), replacements.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (const auto& repl : replacements) {
        int pos = repl.first;
        int length = repl.second.first;
        QString replacement = repl.second.second;

        updatedContent.replace(pos, length, replacement);
    }

    // Update the document
    QTextCursor cursor(document);
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.insertText(updatedContent);
    cursor.endEditBlock();
}

// Helper function to remove links to a deleted file
static void removeLinksInDocument(QTextDocument* document,
                                  const QString& deletedFileName) {
    if (!document || deletedFileName.isEmpty()) {
        return;
    }

    QString content = document->toPlainText();
    QString updatedContent = content;
    bool modified = false;

    QList<QPair<int, QPair<int, QString>>> replacements;

    QRegularExpression wikiLinkPattern(RegexPatterns::WIKI_LINK);
    QRegularExpressionMatchIterator wikiIterator =
        wikiLinkPattern.globalMatch(content);

    while (wikiIterator.hasNext()) {
        QRegularExpressionMatch match = wikiIterator.next();
        QString prefix = match.captured(WIKI_PREFIX_GROUP);  // "!" or empty
        QString target = match.captured(WIKI_TARGET_GROUP).trimmed();

        // Check if this link references the deleted file
        QString targetFileName = QFileInfo(target).fileName();

        // Handle both with and without extension
        QString targetBase = targetFileName;
        QString targetExt;
        int dotPos = targetBase.lastIndexOf('.');
        if (dotPos > 0) {
            targetExt = targetBase.mid(dotPos);
            targetBase = targetBase.left(dotPos);
        }

        QString deletedFileBase = deletedFileName;
        QString deletedFileExt;
        dotPos = deletedFileBase.lastIndexOf('.');
        if (dotPos > 0) {
            deletedFileExt = deletedFileBase.mid(dotPos);
            deletedFileBase = deletedFileBase.left(dotPos);
        }

        if (targetBase == deletedFileBase ||
            targetFileName == deletedFileName) {
            // Remove the link
            QString replacement;
            if (prefix == "!") {
                // For image/inclusion links, just remove entirely (empty
                // string)
                replacement = "";
            } else if (match.captured(WIKI_PIPE_GROUP).isEmpty()) {
                // Simple link: [[target]] -> target
                replacement = targetBase;
            } else {
                // Link with display: [[target|display]] -> display
                replacement = match.captured(WIKI_DISPLAY_GROUP);
            }

            replacements.append(
                qMakePair(match.capturedStart(),
                          qMakePair(match.capturedLength(), replacement)));
            modified = true;
        }
    }

    QRegularExpression markdownLinkPattern(
        RegexPatterns::MARKDOWN_LINK_WITH_IMAGE);
    QRegularExpressionMatchIterator mdIterator =
        markdownLinkPattern.globalMatch(content);

    while (mdIterator.hasNext()) {
        QRegularExpressionMatch match = mdIterator.next();
        QString prefix = match.captured(MD_PREFIX_GROUP);  // "!" or empty
        QString text = match.captured(MD_TEXT_GROUP);
        QString url = match.captured(MD_URL_GROUP).trimmed();

        // Skip external links
        if (url.startsWith(HTTP_PREFIX) || url.startsWith(HTTPS_PREFIX)) {
            continue;
        }

        // Extract just the filename from the path
        QString urlFileName = QFileInfo(url).fileName();

        if (urlFileName == deletedFileName) {
            // Remove the link
            QString replacement;
            if (prefix == "!") {
                // For image links, remove entirely
                replacement = "";
            } else {
                // For regular links, keep the text: [text](url) -> text
                replacement = text;
            }

            replacements.append(
                qMakePair(match.capturedStart(),
                          qMakePair(match.capturedLength(), replacement)));
            modified = true;
        }
    }

    if (!modified) {
        return;
    }

    // Apply replacements in reverse order to maintain positions
    std::sort(replacements.begin(), replacements.end(),
              [](const auto& a, const auto& b) { return a.first > b.first; });

    for (const auto& repl : replacements) {
        int pos = repl.first;
        int length = repl.second.first;
        QString replacement = repl.second.second;

        updatedContent.replace(pos, length, replacement);
    }

    // Update the document
    QTextCursor cursor(document);
    cursor.beginEditBlock();
    cursor.select(QTextCursor::Document);
    cursor.insertText(updatedContent);
    cursor.endEditBlock();
}

void MainWindow::onEditorFileRenameRequested(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("File does not exist: %1").arg(filePath));
        return;
    }

    QString oldName = fileInfo.fileName();
    QString dirPath = fileInfo.absolutePath();

    bool ok;
    QString newName = QInputDialog::getText(
        this, tr("Rename File"), tr("Enter new name for '%1':").arg(oldName),
        QLineEdit::Normal, oldName, &ok);

    if (!ok || newName.isEmpty() || newName == oldName) {
        return;
    }

    QString newPath = dirPath + QDir::separator() + newName;

    if (QFile::exists(newPath)) {
        QMessageBox::warning(this, tr("Error"),
                             tr("A file with that name already exists!"));
        return;
    }

    bool success = QFile::rename(filePath, newPath);

    if (!success) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to rename file!"));
        return;
    }

    // Update links in all open tabs
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->editor()) {
            updateLinksInDocument(tab->editor()->document(), oldName, newName);
        }
    }

    onFileRenamed(filePath, newPath);

    if (!currentFolder.isEmpty()) {
        buildLinkIndexAsync();
    }

    statusBar()->showMessage(tr("File renamed to: %1").arg(newName), 3000);
}

void MainWindow::onEditorFileDeleteRequested(const QString& filePath) {
    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        QMessageBox::warning(this, tr("Error"),
                             tr("File does not exist: %1").arg(filePath));
        return;
    }

    QString fileName = fileInfo.fileName();

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("Delete File"),
        tr("Are you sure you want to delete this file?\n\n%1").arg(fileName),
        QMessageBox::Yes | QMessageBox::No);

    if (reply != QMessageBox::Yes) {
        return;
    }

    bool success = QFile::remove(filePath);

    if (!success) {
        QMessageBox::warning(this, tr("Error"), tr("Failed to delete file!"));
        return;
    }

    // Remove links in all open tabs
    for (int i = 0; i < tabWidget->count(); ++i) {
        TabEditor* tab = qobject_cast<TabEditor*>(tabWidget->widget(i));
        if (tab && tab->editor()) {
            removeLinksInDocument(tab->editor()->document(), fileName);
        }
    }

    onFileDeleted(filePath);

    if (!currentFolder.isEmpty()) {
        buildLinkIndexAsync();
    }

    statusBar()->showMessage(tr("File deleted: %1").arg(fileName), 3000);
}
