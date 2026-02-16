#include <QRegularExpression>

#include "defs.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "regexpatterns.h"
#include "regexutils.h"
#include "shortcutmanager.h"

QString MarkdownEditor::getTaskMarkerAtPosition(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);
    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<QRegularExpressionMatch> matches = 
        RegexUtils::getAllMatches(line, RegexPatterns::TASK_CHECKBOX);

    for (const auto& match : matches) {
        int start = match.capturedStart();
        int end = start + match.capturedLength();
        if (posInBlock >= start && posInBlock <= end) {
            return match.captured(1);
        }
    }
    return QString();
}

bool MarkdownEditor::isClickOnCheckbox(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);
    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    if (!RegexUtils::isTaskItem(line)) {
        return false;
    }

    RegexUtils::TaskItemInfo taskInfo = RegexUtils::parseTaskItem(line);
    
    int bracketStart = line.indexOf('[');
    int bracketEnd = line.indexOf(']', bracketStart);

    if (bracketStart >= 0 && bracketEnd >= 0) {
        return (posInBlock >= bracketStart && posInBlock <= bracketEnd + 1);
    }

    return false;
}

void MarkdownEditor::toggleTaskAtPosition(int position) {
    QTextCursor cursor(document());
    cursor.setPosition(position);
    QTextBlock block = cursor.block();
    QString line = block.text();

    QRegularExpression taskPattern("([-*+])\\s+\\[([ xX.]?)\\]");
    QRegularExpressionMatch match = taskPattern.match(line);
    if (match.hasMatch()) {
        QString marker = match.captured(2);  // The character inside brackets
        QString newMarker;

        if (marker == " ") {
            newMarker = "X";
        } else if (marker.toUpper() == "X") {
            newMarker = " ";
        } else if (marker == ".") {
            newMarker = "X";
        } else {
            newMarker = "X";
        }

        QTextCursor editorCursor = textCursor();
        int cursorPos = editorCursor.position();
        int blockPos = block.position();
        int relativePos = cursorPos - blockPos;

        cursor.beginEditBlock();
        cursor.setPosition(block.position() + match.capturedStart(2));
        cursor.deleteChar();
        cursor.insertText(newMarker);
        cursor.endEditBlock();

        editorCursor.setPosition(blockPos + relativePos);
        setTextCursor(editorCursor);

        updateParentTask(block);

        if (newMarker == " ") {
            uncheckChildTasks(block);
        }
    }
}

void MarkdownEditor::uncheckChildTasks(const QTextBlock& block) {
    QVector<QTextBlock> children = findChildBlocks(block);
    for (const QTextBlock& child : children) {
        QString line = child.text();
        QRegularExpression taskPattern("([-*+])\\s+\\[([xX.]?)\\]");
        QRegularExpressionMatch match = taskPattern.match(line);
        if (match.hasMatch()) {
            QString marker = match.captured(2);
            if (marker != " ") {
                QTextCursor cursor(document());
                cursor.setPosition(child.position());
                cursor.beginEditBlock();
                cursor.select(QTextCursor::BlockUnderCursor);
                QString newLine = line;
                int markerPos = match.capturedStart(2);
                newLine.replace(markerPos, match.capturedLength(2), " ");
                cursor.insertText(newLine);
                cursor.endEditBlock();
            }
        }
    }
}

void MarkdownEditor::updateParentTask(const QTextBlock& block) {
    int currentIndent = getIndentLevel(block);
    QTextBlock parentBlock = findParentBlock(block, currentIndent);
    if (parentBlock.isValid()) {
        updateTaskState(parentBlock);
        updateParentTask(parentBlock);
    }
}

QTextBlock MarkdownEditor::findParentBlock(const QTextBlock& block,
                                           int currentIndent) {
    QTextBlock current = block.previous();
    while (current.isValid()) {
        int indent = getIndentLevel(current);
        if (indent < currentIndent) {
            QString line = current.text();
            QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
            if (taskPattern.match(line).hasMatch()) {
                return current;
            }
        }
        current = current.previous();
    }
    return QTextBlock();
}

void MarkdownEditor::updateTaskState(const QTextBlock& block) {
    QVector<QTextBlock> children = findChildBlocks(block);
    bool hasDone = false;
    bool hasPending = false;
    for (const QTextBlock& child : children) {
        QString line = child.text();
        QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
        QRegularExpressionMatch match = taskPattern.match(line);
        if (match.hasMatch()) {
            QString marker = match.captured(1);
            if (marker == "X") {
                hasDone = true;
            } else if (marker == " ") {
                hasPending = true;
            } else if (marker == ".") {
                hasDone = true;
                hasPending = true;
            }
        }
    }
    QString newMarker;
    if (hasDone && hasPending) {
        newMarker = ".";
    } else if (hasDone) {
        newMarker = "X";
    } else {
        newMarker = " ";
    }

    QString line = block.text();
    QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
    QRegularExpressionMatch match = taskPattern.match(line);
    if (match.hasMatch()) {
        QString oldMarker = match.captured(1);
        if (oldMarker != newMarker) {
            QTextCursor cursor(document());
            cursor.setPosition(block.position() + match.capturedStart(1));
            cursor.setPosition(block.position() + match.capturedStart(1) +
                                   match.capturedLength(1),
                               QTextCursor::KeepAnchor);
            cursor.insertText(newMarker);
        }
    }
}

QVector<QTextBlock> MarkdownEditor::findChildBlocks(const QTextBlock& block) {
    QVector<QTextBlock> children;
    int parentIndent = getIndentLevel(block);
    int childIndent = -1;
    QTextBlock current = block.next();
    while (current.isValid()) {
        int indent = getIndentLevel(current);
        if (indent <= parentIndent) {
            break;
        }
        QString line = current.text();
        QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
        if (taskPattern.match(line).hasMatch()) {
            if (childIndent == -1) {
                childIndent = indent;
            }
            if (indent == childIndent) {
                children.append(current);
            } else if (indent < childIndent) {
                break;
            }
        }
        current = current.next();
    }
    return children;
}

int MarkdownEditor::getIndentLevel(const QTextBlock& block) {
    QString text = block.text();
    int indent = 0;
    for (QChar ch : text) {
        if (ch == ' ') {
            indent++;
        } else if (ch == '\t') {
            indent += 4;
        } else {
            break;
        }
    }
    return indent;
}

void MarkdownEditor::applyListHangingIndent(const QTextBlock& block) {
    QString text = block.text();
    QRegularExpression listPattern("^(\\s*)([-*+]|[0-9]+\\.)\\s+");
    QRegularExpressionMatch match = listPattern.match(text);

    QTextCursor cursor(document());
    cursor.setPosition(block.position());
    QTextBlockFormat blockFormat = cursor.blockFormat();

    if (!match.hasMatch()) {
        if (blockFormat.leftMargin() != 0 || blockFormat.textIndent() != 0) {
            blockFormat.setLeftMargin(0);
            blockFormat.setTextIndent(0);
            cursor.setBlockFormat(blockFormat);
        }
        return;
    }

    QFont font = this->font();
    QFontMetrics fm(font);
    QString indent = match.captured(1);  // Leading whitespace
    QString bullet = match.captured(2);  // Bullet marker (-, *, +, or number.)
    int bulletWidth = fm.horizontalAdvance(indent + bullet + " ");
    blockFormat.setLeftMargin(bulletWidth);  // Total left margin
    blockFormat.setTextIndent(
        -bulletWidth);  // Negative indent for hanging effect
    cursor.setBlockFormat(blockFormat);
}

void MarkdownEditor::applyListHangingIndentToCurrentBlock() {

    QTextCursor cursor = textCursor();
    QTextBlock currentBlock = cursor.block();

    QString text = currentBlock.text();
    QRegularExpression listPattern("^(\\s*)([-*+]|[0-9]+\\.)\\s+");
    QRegularExpressionMatch match = listPattern.match(text);

    QTextBlockFormat currentFormat = cursor.blockFormat();

    bool needsUpdate = false;
    qreal targetLeftMargin = 0;
    qreal targetTextIndent = 0;

    if (match.hasMatch()) {
        QFont font = this->font();
        QFontMetrics fm(font);
        QString indent = match.captured(1);
        QString bullet = match.captured(2);
        int bulletWidth = fm.horizontalAdvance(indent + bullet + " ");
        targetLeftMargin = bulletWidth;
        targetTextIndent = -bulletWidth;

        if (currentFormat.leftMargin() != targetLeftMargin ||
            currentFormat.textIndent() != targetTextIndent) {
            needsUpdate = true;
        }
    } else {
        if (currentFormat.leftMargin() != 0 ||
            currentFormat.textIndent() != 0) {
            needsUpdate = true;
        }
    }

    if (needsUpdate) {
        bool undoEnabled = document()->isUndoRedoEnabled();
        document()->setUndoRedoEnabled(false);

        QTextBlockFormat newFormat = currentFormat;
        newFormat.setLeftMargin(targetLeftMargin);
        newFormat.setTextIndent(targetTextIndent);
        cursor.setBlockFormat(newFormat);

        document()->setUndoRedoEnabled(undoEnabled);
    }
}

void MarkdownEditor::applyDeferredFormatting() {
    bool wasModified = document()->isModified();

    QTextCursor batchCursor(document());

    batchCursor.beginEditBlock();

    QTextBlock block = document()->firstBlock();
    while (block.isValid()) {
        applyListHangingIndent(block);
        block = block.next();
    }

    batchCursor.endEditBlock();

    document()->undo();

    document()->setModified(wasModified);
}
