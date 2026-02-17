#include <QDesktopServices>
#include <QKeyEvent>
#include <QSettings>
#include <QTextBlock>

#include "defs.h"
#include "markdowneditor.h"
#include "regexutils.h"
#include "shortcutmanager.h"

void MarkdownEditor::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Tab && !m_currentPrediction.isEmpty()) {
        acceptPrediction();
        event->accept();
        return;
    }

    ShortcutManager* sm = ShortcutManager::instance();
    QKeySequence pressed(event->key() | event->modifiers());

    QTextCursor cursor = textCursor();
    QTextCursor::MoveMode moveMode = (event->modifiers() & Qt::ShiftModifier)
                                         ? QTextCursor::KeepAnchor
                                         : QTextCursor::MoveAnchor;

    if (pressed == sm->getShortcut(ShortcutManager::MoveListItemUp)) {
        moveListItemUp();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveListItemDown)) {
        moveListItemDown();
        event->accept();
        return;
    }

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

    if ((event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) &&
        (event->modifiers() & Qt::ControlModifier)) {
        int position = cursor.position();

        QString linkTarget = getLinkAtPosition(position);
        if (!linkTarget.isEmpty()) {
            emit wikiLinkClicked(linkTarget);
            event->accept();
            return;
        }

        QString markdownLink = getMarkdownLinkAtPosition(position);
        if (!markdownLink.isEmpty()) {
            emit markdownLinkClicked(markdownLink);
            event->accept();
            return;
        }

        QString externalUrl = getExternalLinkAtPosition(position);
        if (!externalUrl.isEmpty()) {
            QDesktopServices::openUrl(QUrl(externalUrl));
            event->accept();
            return;
        }
    }

    QSettings settings(APP_LABEL, APP_LABEL);
    bool autoIndent = settings.value("editor/autoIndent", true).toBool();
    bool autoCloseBrackets =
        settings.value("editor/autoCloseBrackets", true).toBool();

    if (autoIndent &&
        (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)) {
        QTextCursor cursor = textCursor();
        QString currentLine = cursor.block().text();
        int cursorPosInBlock = cursor.positionInBlock();

        if (RegexUtils::isTaskItem(currentLine)) {
            RegexUtils::TaskItemInfo taskInfo =
                RegexUtils::parseTaskItem(currentLine);
            QString whitespace = taskInfo.indent;
            QString bullet = taskInfo.marker;

            bool isAtEnd = cursorPosInBlock >= currentLine.length();

            if (isAtEnd) {
                QString lineAfterCheckbox = taskInfo.content;
                bool isEmptyTaskItem = lineAfterCheckbox.isEmpty();

                if (isEmptyTaskItem) {
                    cursor.select(QTextCursor::BlockUnderCursor);
                    cursor.removeSelectedText();
                    cursor.insertText("\n");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                } else {
                    cursor.insertText("\n" + whitespace + bullet + " [ ] ");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                }
            }
        }

        if (RegexUtils::isListItem(currentLine)) {
            RegexUtils::ListItemInfo listInfo =
                RegexUtils::parseListItem(currentLine);
            QString whitespace = listInfo.indent;
            QString bullet = listInfo.marker;

            bool isAtEnd = cursorPosInBlock >= currentLine.length();

            if (isAtEnd) {
                QString lineAfterBullet = listInfo.content;
                bool isEmptyListItem = lineAfterBullet.isEmpty();

                if (isEmptyListItem) {
                    cursor.select(QTextCursor::BlockUnderCursor);
                    cursor.removeSelectedText();
                    cursor.insertText("\n");
                    setTextCursor(cursor);
                    event->accept();
                    return;
                } else {
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
            indent = info.indent.length() + info.marker.length() +
                     1;  // marker + space
        } else {
            for (QChar ch : currentLine) {
                if (ch == ' ')
                    indent++;
                else if (ch == '\t')
                    indent += 4;  // Treat tab as 4 spaces
                else
                    break;
            }
        }

        cursor.insertText("\n" + QString(" ").repeated(indent));
        setTextCursor(cursor);
        event->accept();
        return;
    }

    if (autoCloseBrackets) {
        QString closingChar;
        bool shouldClose = false;

        if (event->text() == "(") {
            closingChar = ")";
            shouldClose = true;
        } else if (event->text() == "[") {
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
            QString textAfterCursor = 
                cursor.block().text().mid(cursor.positionInBlock());
            
            bool atEndOrWhitespace = textAfterCursor.isEmpty() || 
                                     textAfterCursor.at(0).isSpace();
            
            if (atEndOrWhitespace) {
                cursor.insertText(event->text() + closingChar);
                cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor);
                setTextCursor(cursor);
                event->accept();
                return;
            }
        }

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

    if (event->key() == Qt::Key_Tab || event->key() == Qt::Key_Backtab) {
        QTextCursor cursor = textCursor();
        QString line = cursor.block().text();

        if (RegexUtils::isListItem(line)) {
            RegexUtils::ListItemInfo listInfo = RegexUtils::parseListItem(line);
            cursor.beginEditBlock();

            if (event->key() == Qt::Key_Tab &&
                !(event->modifiers() & Qt::ShiftModifier)) {
                cursor.movePosition(QTextCursor::StartOfBlock);
                cursor.insertText("  ");
            } else if (event->key() == Qt::Key_Backtab ||
                       (event->modifiers() & Qt::ShiftModifier)) {
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

    if (event->key() == Qt::Key_Space &&
        (event->modifiers() == Qt::ControlModifier ||
         event->modifiers() == (Qt::ControlModifier | Qt::KeypadModifier))) {
        QTextCursor cursor = textCursor();
        QString line = cursor.block().text();

        if (RegexUtils::isTaskItem(line)) {
            int checkboxStart = line.indexOf('[');
            if (checkboxStart >= 0) {
                int markerPos = checkboxStart + 1;
                toggleTaskAtPosition(cursor.block().position() + markerPos);
            }
        }
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
        event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
        event->key() == Qt::Key_Escape) {
        hidePrediction();
    }

    QTextEdit::keyPressEvent(event);
}
