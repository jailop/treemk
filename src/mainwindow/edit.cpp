#include <QApplication>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QStatusBar>
#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

#include "aiassistdialog.h"
#include "logic/aiprovider.h"
#include "logic/systemprompts.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "quickopendialog.h"
#include "searchdialog.h"
#include "tabeditor.h"

void MainWindow::find() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    bool ok;
    QString searchText = QInputDialog::getText(
        this, tr("Find"), tr("Find text:"), QLineEdit::Normal, "", &ok);

    if (!ok || searchText.isEmpty()) {
        return;
    }

    QTextCursor cursor = tab->editor()->textCursor();
    QTextDocument::FindFlags flags;

    QTextCursor foundCursor =
        tab->editor()->document()->find(searchText, cursor, flags);

    if (!foundCursor.isNull()) {
        tab->editor()->setTextCursor(foundCursor);
    } else {
        QTextCursor startCursor = tab->editor()->textCursor();
        startCursor.movePosition(QTextCursor::Start);
        tab->editor()->setTextCursor(startCursor);

        foundCursor =
            tab->editor()->document()->find(searchText, startCursor, flags);

        if (!foundCursor.isNull()) {
            tab->editor()->setTextCursor(foundCursor);
        } else {
            statusBar()->showMessage(tr("Text not found"), 2000);
        }
    }
}

void MainWindow::findAndReplace() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    bool okFind, okReplace;
    QString searchText =
        QInputDialog::getText(this, tr("Find and Replace"), tr("Find text:"),
                              QLineEdit::Normal, "", &okFind);

    if (!okFind || searchText.isEmpty()) {
        return;
    }

    QString replaceText =
        QInputDialog::getText(this, tr("Find and Replace"), tr("Replace with:"),
                              QLineEdit::Normal, "", &okReplace);

    if (!okReplace) {
        return;
    }

    QTextCursor cursor = tab->editor()->textCursor();
    cursor.movePosition(QTextCursor::Start);
    tab->editor()->setTextCursor(cursor);

    int replaceCount = 0;
    QTextDocument::FindFlags flags;

    while (true) {
        QTextCursor foundCursor =
            tab->editor()->document()->find(searchText, cursor, flags);

        if (foundCursor.isNull()) {
            break;
        }

        foundCursor.insertText(replaceText);
        cursor = foundCursor;
        replaceCount++;
    }

    if (replaceCount > 0) {
        statusBar()->showMessage(
            tr("Replaced %1 occurrence(s)").arg(replaceCount), 3000);
    } else {
        statusBar()->showMessage(tr("Text not found"), 2000);
    }
}

void MainWindow::searchInFiles() {
    if (currentFolder.isEmpty()) {
        QMessageBox::information(
            this, tr("Search in Files"),
            tr("Please open a folder first to search within."));
        return;
    }

    SearchDialog dialog(currentFolder, this);
    connect(&dialog, &SearchDialog::fileSelected, this,
            &MainWindow::onSearchResultSelected);
    dialog.exec();
}

void MainWindow::onSearchResultSelected(const QString& filePath,
                                        int lineNumber) {
    if (loadFile(filePath)) {
        jumpToLine(lineNumber);
    }
}

void MainWindow::quickOpen() {
    if (currentFolder.isEmpty()) {
        QMessageBox::information(this, tr("Quick Open"),
                                 tr("Please open a folder first."));
        return;
    }

    QuickOpenDialog dialog(currentFolder, recentFiles, this);
    if (dialog.exec() == QDialog::Accepted) {
        QString selectedFile = dialog.getSelectedFile();
        if (!selectedFile.isEmpty()) {
            loadFile(selectedFile);
        }
    }
}

void MainWindow::jumpToLine(int lineNumber) {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    QTextBlock block =
        tab->editor()->document()->findBlockByLineNumber(lineNumber - 1);
    QTextCursor cursor(block);
    tab->editor()->setTextCursor(cursor);
    tab->editor()->ensureCursorVisible();
}

void MainWindow::openAIAssist() {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    MarkdownEditor* editor = tab->editor();
    QTextCursor cursor = editor->textCursor();

    QString selectedText = cursor.selectedText();
    selectedText.replace(QChar(0x2029), '\n');

    int cursorPos = cursor.position();
    int selStart = cursor.selectionStart();
    int selEnd = cursor.selectionEnd();

    AIAssistDialog dialog(this, selectedText, cursorPos, selStart, selEnd);

    connect(&dialog, &AIAssistDialog::insertText,
            [editor](int position, const QString& text) {
                QTextCursor cursor = editor->textCursor();
                cursor.setPosition(position);
                cursor.insertText(text);
                editor->setTextCursor(cursor);
            });

    connect(&dialog, &AIAssistDialog::replaceText,
            [editor](int start, int end, const QString& text) {
                QTextCursor cursor = editor->textCursor();
                cursor.setPosition(start);
                cursor.setPosition(end, QTextCursor::KeepAnchor);
                cursor.insertText(text);
                editor->setTextCursor(cursor);
            });

    dialog.exec();
}

void MainWindow::processAIWithPrompt(const QString& promptText) {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    MarkdownEditor* editor = tab->editor();
    QTextCursor cursor = editor->textCursor();

    QString selectedText = cursor.selectedText();
    selectedText.replace(QChar(0x2029), '\n');

    bool hasSelection = cursor.hasSelection();
    int selStart = cursor.selectionStart();
    int selEnd = cursor.selectionEnd();
    int cursorPos = cursor.position();

    // Show wait cursor and processing status
    QApplication::setOverrideCursor(Qt::WaitCursor);
    statusBar()->showMessage(tr("Processing with AI..."), 0);

    // Process with AI provider
    AIProviderManager::instance()->process(
        promptText, selectedText,
        [this, editor, hasSelection, selStart, selEnd,
         cursorPos](const QString& result) {
            QTextCursor cursor = editor->textCursor();

            cursor.beginEditBlock();
            if (hasSelection) {
                // Replace selected text
                cursor.setPosition(selStart);
                cursor.setPosition(selEnd, QTextCursor::KeepAnchor);
                cursor.insertText(result);
            } else {
                // Insert at cursor position
                cursor.setPosition(cursorPos);
                cursor.insertText(result);
            }
            cursor.endEditBlock();

            editor->setTextCursor(cursor);

            // Restore cursor and show completion message
            QApplication::restoreOverrideCursor();
            statusBar()->showMessage(tr("AI processing completed"), 3000);
        },
        [this](const QString& error) {
            QApplication::restoreOverrideCursor();
            QMessageBox::warning(this, tr("AI Error"), error);
            statusBar()->clearMessage();
        });
}

void MainWindow::processAIWithSystemPrompt(const QString& promptId) {
    SystemPrompt prompt = SystemPrompts::instance()->getPromptById(promptId);
    if (prompt.id.isEmpty()) {
        return;
    }

    processAIWithPrompt(prompt.prompt);
}
