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

    /* Here we handle shortcuts for moving list items up and down. It is
     * at the top of the function because we want these shortcuts to
     * have precedence over the default cursor movement shortcuts.
     */
    if (pressed == sm->getShortcut(ShortcutManager::MoveListItemUp)) {
        moveListItemUp();
        event->accept();
        return;
    } else if (pressed == sm->getShortcut(ShortcutManager::MoveListItemDown)) {
        moveListItemDown();
        event->accept();
        return;
    }

    /* Here is the basic navigation through the document. We have
     * movements for moving to the start and end of lines, words,
     * paragraphs, and the whole document. These are all implemented by
     * moving the cursor to the appropriate position.
     */
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

    /* These are delete shortcuts. They are implemented by first moving
     * the cursor to select the text that should be deleted, and then
     * removing the selected text.
     */
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

    /* Here we're handling a critical feature of the editor, opening
     * links by pressing Ctrl+Enter. The cursor position is used to
     * check if the context is a wiki link, a markdown link, or an
     * external link, and the appropriate signal is emitted or action
     * taken.
     */
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

        /* For task items, we want to continue the task item if the
         * cursor is at the end of the line, but if it is in the middle,
         * we just want to insert a newline with the same indentation,
         * without adding a new task item. If the cursor is at the end
         * of a task item, we want to check if the content after the
         * checkbox is empty. If it is empty, we should exit the task
         * item by inserting a blank line. If it is not empty, we should
         * continue the task item by inserting a new task item with the
         * same bullet and an unchecked checkbox.
         */
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

        /* For lists, we want to continue the list if the cursor is at
         * the end of the line, but if it is in the middle, we just want
         * to insert a newline with the same indentation, without adding
         * a new list item.
         *
         * If the cursor is at the end of a list item, we want to check
         * if the content after the bullet is empty. If it is empty, we
         * should exit the list by inserting a blank line.  If it is not
         * empty, we should continue the list by inserting a new list
         * item with the same bullet or incremented number for ordered
         * lists.
         */
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
                    if (listInfo.isOrdered) {
                        int number = listInfo.marker
                            .left(listInfo.marker.length() - 1).toInt();
                        bullet = QString::number(number + 1) + ".";
                    }
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

    /* Here we're handling the auto-closing of brackets and quotes. When
     * the user types an opening bracket or quote, we check if the
     * character after the cursor is whitespace or the end of the line.
     * If it is, we insert the corresponding closing character and move
     * the cursor back to be between the two characters. If the user
     * types a closing bracket or quote and the character after the
     * cursor is the same closing character, we move the cursor over it
     * instead of inserting a new character.
     */
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

    /* Indenting and unindenting list items with Tab and Shift+Tab. If
     * the current line is a list item, pressing Tab will add two spaces
     * at the beginning of the line to indent it, and pressing Shift+Tab
     * will remove up to two spaces from the beginning of the line.
     */
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

    /* Toggling task items with Ctrl+Space. If the current line is a
     * task item, pressing Ctrl+Space will toggle the checkbox between
     * checked and unchecked. This is done by finding the position of
     * the checkbox marker in the line and replacing it with the
     * opposite character.
     */
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

    /* Hiding the autocomplete prediction when pressing navigation keys
     * or Escape. This is important to ensure that the prediction
     * doesn't interfere with normal navigation and can be dismissed
     * easily when the user wants to continue typing without accepting
     * the prediction.
     */
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right ||
        event->key() == Qt::Key_Up || event->key() == Qt::Key_Down ||
        event->key() == Qt::Key_Home || event->key() == Qt::Key_End ||
        event->key() == Qt::Key_PageUp || event->key() == Qt::Key_PageDown ||
        event->key() == Qt::Key_Escape) {
        hidePrediction();
    }

    QTextEdit::keyPressEvent(event);
}
