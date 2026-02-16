#include <QDesktopServices>
#include <QKeyEvent>
#include <QSettings>
#include <QTextBlock>

#include "defs.h"
#include "markdowneditor.h"
#include "regexutils.h"
#include "shortcutmanager.h"

void MarkdownEditor::keyPressEvent(QKeyEvent* event) {
    // Handle Tab key for word prediction
    if (event->key() == Qt::Key_Tab && !m_currentPrediction.isEmpty()) {
        acceptPrediction();
        event->accept();
        return;
    }

    // Get shortcut manager instance
    ShortcutManager* sm = ShortcutManager::instance();
    QKeySequence pressed(event->key() | event->modifiers());

    // Check custom navigation shortcuts
    QTextCursor cursor = textCursor();
    QTextCursor::MoveMode moveMode = (event->modifiers() & Qt::ShiftModifier)
                                         ? QTextCursor::KeepAnchor
                                         : QTextCursor::MoveAnchor;

    // List item movement shortcuts
    if (pressed == sm->getShortcut(ShortcutManager::MoveListItemUp)) {
        moveListItemUp();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveListItemDown)) {
        moveListItemDown();
        event->accept();
        return;
    }

    // Navigation actions
    if (pressed == sm->getShortcut(ShortcutManager::MoveToStartOfLine)) {
        cursor.movePosition(QTextCursor::StartOfLine, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveToEndOfLine)) {
        cursor.movePosition(QTextCursor::EndOfLine, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed ==
               sm->getShortcut(ShortcutManager::MoveToStartOfDocument)) {
        cursor.movePosition(QTextCursor::Start, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed ==
               sm->getShortcut(ShortcutManager::MoveToEndOfDocument)) {
        cursor.movePosition(QTextCursor::End, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveWordLeft)) {
        cursor.movePosition(QTextCursor::PreviousWord, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveWordRight)) {
        cursor.movePosition(QTextCursor::NextWord, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed ==
               sm->getShortcut(ShortcutManager::MoveToPreviousParagraph)) {
        cursor.movePosition(QTextCursor::PreviousBlock, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    } else if (pressed ==
               sm->getShortcut(ShortcutManager::MoveToNextParagraph)) {
        cursor.movePosition(QTextCursor::NextBlock, moveMode);
        setTextCursor(cursor);
        event->accept();
        return;
    }

    // Editing actions
    if (pressed == sm->getShortcut(ShortcutManager::DeleteWordLeft)) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::PreviousWord, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.endEditBlock();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::DeleteWordRight)) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::NextWord, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.endEditBlock();
        event->accept();
        return;
    } else if (pressed ==
               sm->getShortcut(ShortcutManager::DeleteToStartOfLine)) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::StartOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.endEditBlock();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::DeleteToEndOfLine)) {
        cursor.beginEditBlock();
        cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
        cursor.removeSelectedText();
        cursor.endEditBlock();
        event->accept();
        return;
    }

    // Ctrl+Enter or Ctrl+Return to open wiki-link at cursor
    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() & Qt::ControlModifier)) {
        int position = cursor.position();

        // Try wiki link first
        QString linkTarget = getLinkAtPosition(position);
        if (!linkTarget.isEmpty()) {
            emit wikiLinkClicked(linkTarget);
            event->accept();
            return;
        }

        // Try markdown link
        QString markdownLink = getMarkdownLinkAtPosition(position);
        if (!markdownLink.isEmpty()) {
            emit markdownLinkClicked(markdownLink);
            event->accept();
            return;
        }

        // Try external link
        QString externalUrl = getExternalLinkAtPosition(position);
        if (!externalUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(externalUrl));
            event->accept();
            return;
        }
    }

    // Auto-indent on Enter
    QSettings settings(APP_LABEL, APP_LABEL);
    bool autoIndent = settings.value("editor/autoIndent", true).toBool();
    bool autoCloseBrackets =
        settings.value("editor/autoCloseBrackets", true).toBool();

    if (autoIndent &&
        (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        // Get current line text to calculate indentation
        QTextCursor cursor = textCursor();
        QString currentLine = cursor.block().text();
        int cursorPosInBlock = cursor.positionInBlock();

        // Check if current line is a task list item
        if (RegexUtils::isTaskItem(currentLine)) {
            RegexUtils::TaskItemInfo taskInfo =
                RegexUtils::parseTaskItem(currentLine);
            QString whitespace = taskInfo.indent;
            QString bullet = taskInfo.marker;

            // Check if cursor is at the end of the line
            bool isAtEnd = cursorPosInBlock >= currentLine.length();

            if (isAtEnd) {
                // Check if the task line contains only the checkbox (empty task
                // item)
                QString lineAfterCheckbox = taskInfo.content;
                bool isEmptyTaskItem = lineAfterCheckbox.isEmpty();

                if (isEmptyTaskItem) {
                    // User pressed Enter on an empty task item - exit task mode
                    cursor.select(QTextCursor::BlockUnderCursor);
                    cursor.removeSelectedText();
                    cursor.insertText("\n");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                } else {
                    // Auto-continue task list with new unchecked item
                    cursor.insertText("\n" + whitespace + bullet + " [ ] ");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                }
            }
        }

        // Check if current line is a regular list item (not task)
        if (RegexUtils::isListItem(currentLine)) {
            RegexUtils::ListItemInfo listInfo =
                RegexUtils::parseListItem(currentLine);
            QString whitespace = listInfo.indent;
            QString bullet = listInfo.marker;

            // Check if cursor is at the end of the line (after all content)
            bool isAtEnd = cursorPosInBlock >= currentLine.length();

            if (isAtEnd) {
                // Check if the line contains only the bullet (empty list item)
                QString lineAfterBullet = listInfo.content;
                bool isEmptyListItem = lineAfterBullet.isEmpty();

                if (isEmptyListItem) {
                    // User pressed Enter on an empty list item - exit list mode
                    // Replace the empty bullet with a blank line
                    cursor.select(QTextCursor::BlockUnderCursor);
                    cursor.removeSelectedText();
                    cursor.insertText("\n");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                } else {
                    // Auto-continue list with same bullet
                    cursor.insertText("\n" + whitespace + bullet + " ");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                }
            }
        }

        int indent = 0;

        if (RegexUtils::isListItem(currentLine)) {
            RegexUtils::ListItemInfo info =
                RegexUtils::parseListItem(currentLine);
            // For list items, indent continuation to align with the text after
            // the marker
            indent = info.indent.length() + info.marker.length() +
                     1;  // marker + space
        } else {
            // Calculate leading whitespace for other cases
            for (QChar ch : currentLine) {
                if (ch == ' ')
                    indent++;
                else if (ch == '\t')
                    indent += 4;  // Treat tab as 4 spaces
                else
                    break;
            }
        }

        // Insert newline and indentation
        cursor.insertText("\n" + QString(" ").repeated(indent));
        setTextCursor(cursor);
        event->accept();
        return;
    }

    // Auto-close brackets, parentheses, quotes
    if (autoCloseBrackets) {
        QString closingChar;
        bool shouldClose = false;

        if (event->text() == "(") {
            closingChar = ")";
            shouldClose = true;
        } else if (event->text() == "[") {
            // Regular bracket - auto-close with ]
            closingChar = "]";
            shouldClose = true;
        } else if (event->text() == "{") {
            closingChar = "}";
            shouldClose = true;
        } else if (event->text() == "`") {
            closingChar = "`";
            shouldClose = true;
        }

        if (shouldClose) {
            QTextCursor cursor = textCursor();
            cursor.insertText(event->text() + closingChar);
            cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
            event->accept();
            return;
        }

        // Skip over closing bracket if typed when already at one
        QTextCursor cursor = textCursor();
        QString nextChar =
            cursor.block().text().mid(cursor.positionInBlock(), 1);
        if ((event->text() == ")" && nextChar == ")") ||
            (event->text() == "]" && nextChar == "]") ||
            (event->text() == "}" && nextChar == "}") ||
            (event->text() == "`" && nextChar == "`")) {
            cursor.movePosition(QTextCursor::Right, QTextCursor::MoveAnchor);
            setTextCursor(cursor);
            event->accept();
            return;
        }
    }

    // Handle Tab and Shift+Tab for indenting/dedenting lists and tasks
    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        QTextCursor cursor = textCursor();
        QString line = cursor.block().text();

        // Check if current line is a list or task item
        if (RegexUtils::isListItem(line)) {
            RegexUtils::ListItemInfo listInfo = RegexUtils::parseListItem(line);
            cursor.beginEditBlock();

            if (event->key() == Qt::Key_Tab &&
                !(event->modifiers() & Qt::ShiftModifier)) {
                // Tab - indent (add 2 spaces)
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText("  ");
            } else if (event->key() == Qt::Key_Backtab ||
                       (event->modifiers() & Qt::ShiftModifier)) {
                // Shift+Tab - dedent (remove up to 2 spaces)
                QString whitespace = listInfo.indent;
                if (!whitespace.isEmpty()) {
                    cursor.movePosition(QTextCursor::StartOfBlock);
                    int charsToRemove = qMin(2, whitespace.length());
                    for (int i = 0; i < charsToRemove; i++) {
                        cursor.deleteChar();
                    }
                }
            }

            cursor.endEditBlock();
            event->accept();
            return;
        }
    }

    // Ctrl+Space to toggle task on current line
    // IMPORTANT: Handle this before any other space processing
    if (event->key() == Qt::Key_Space &&
        (event->modifiers() == Qt::ControlModifier ||
         event->modifiers() == (Qt::ControlModifier | Qt::KeypadModifier))) {
        QTextCursor cursor = textCursor();
        QString line = cursor.block().text();

        if (RegexUtils::isTaskItem(line)) {
            // Calculate the position of the marker character inside the
            // brackets
            int checkboxStart = line.indexOf('[');
            if (checkboxStart >= 0) {
                int markerPos = checkboxStart + 1;
                toggleTaskAtPosition(cursor.block().position() + markerPos);
            }
        }
        // Always accept and return - don't insert space
        event->accept();
        return;
    }

    // Hide prediction on certain keys
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
        event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
        event->key() == Qt::Key_Escape) {
        hidePrediction();
    }

    QTextEdit::keyPressEvent(event);
}
