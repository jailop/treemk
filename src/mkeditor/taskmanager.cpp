#include "defs.h"
#include "markdowneditor.h"
#include "markdownhighlighter.h"
#include "shortcutmanager.h"

QString MarkdownEditor::getTaskMarkerAtPosition(int position) const {
  QTextCursor cursor(document());
  cursor.setPosition(position);
  QString line = cursor.block().text();
  int posInBlock = cursor.positionInBlock();

  QRegularExpression taskPattern("[-*+] \\[([ xX.]?)\\]");
  QRegularExpressionMatchIterator matchIterator = taskPattern.globalMatch(line);

  while (matchIterator.hasNext()) {
    QRegularExpressionMatch match = matchIterator.next();
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

  // Pattern to match task list: - [ ], * [ ], + [ ]
  QRegularExpression taskPattern("([-*+])\\s+\\[([ xX.]?)\\]");
  QRegularExpressionMatch match = taskPattern.match(line);

  if (match.hasMatch()) {
    // Check if click is on the checkbox part: [ ] or [X] or [.]
    int checkboxStart = match.capturedStart();
    
    // Allow clicking anywhere on "[ ]" part (including the brackets)
    int bracketStart = line.indexOf('[', checkboxStart);
    int bracketEnd = line.indexOf(']', bracketStart);
    
    if (bracketStart >= 0 && bracketEnd >= 0) {
      return (posInBlock >= bracketStart && posInBlock <= bracketEnd + 1);
    }
  }
  return false;
}

void MarkdownEditor::toggleTaskAtPosition(int position) {
  QTextCursor cursor(document());
  cursor.setPosition(position);
  QTextBlock block = cursor.block();
  QString line = block.text();

  // Match any list marker (-, *, +) followed by [ ]
  QRegularExpression taskPattern("([-*+])\\s+\\[([ xX.]?)\\]");
  QRegularExpressionMatch match = taskPattern.match(line);
  if (match.hasMatch()) {
    QString marker = match.captured(2); // The character inside brackets
    QString newMarker;
    
    // Toggle: space -> X, X/x -> space, . -> X
    if (marker == " ") {
      newMarker = "X";
    } else if (marker.toUpper() == "X") {
      newMarker = " ";
    } else if (marker == ".") {
      newMarker = "X";
    } else {
      newMarker = "X";
    }

    // Save current cursor position in the editor
    QTextCursor editorCursor = textCursor();
    int cursorPos = editorCursor.position();
    int blockPos = block.position();
    int relativePos = cursorPos - blockPos;

    // Replace the marker character
    cursor.beginEditBlock();
    cursor.setPosition(block.position() + match.capturedStart(2));
    cursor.deleteChar();
    cursor.insertText(newMarker);
    cursor.endEditBlock();

    // Restore cursor position
    editorCursor.setPosition(blockPos + relativePos);
    setTextCursor(editorCursor);

    // Update parent task states
    updateParentTask(block);
    
    // Update child task states if this task was marked as done/pending
    if (newMarker == " ") {
      // Parent was unchecked - uncheck all children
      uncheckChildTasks(block);
    }
  }
}

void MarkdownEditor::uncheckChildTasks(const QTextBlock &block) {
  QVector<QTextBlock> children = findChildBlocks(block);
  for (const QTextBlock &child : children) {
    QString line = child.text();
    QRegularExpression taskPattern("([-*+])\\s+\\[([xX.]?)\\]");
    QRegularExpressionMatch match = taskPattern.match(line);
    if (match.hasMatch()) {
      QString marker = match.captured(2);
      if (marker != " ") {
        // Change to unchecked
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

void MarkdownEditor::updateParentTask(const QTextBlock &block) {
  int currentIndent = getIndentLevel(block);
  QTextBlock parentBlock = findParentBlock(block, currentIndent);
  if (parentBlock.isValid()) {
    updateTaskState(parentBlock);
    updateParentTask(parentBlock);
  }
}

QTextBlock MarkdownEditor::findParentBlock(const QTextBlock &block, int currentIndent) {
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



void MarkdownEditor::updateTaskState(const QTextBlock &block) {
  QVector<QTextBlock> children = findChildBlocks(block);
  bool hasDone = false;
  bool hasPending = false;
  for (const QTextBlock &child : children) {
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
      cursor.setPosition(block.position() + match.capturedStart(1) + match.capturedLength(1), QTextCursor::KeepAnchor);
      cursor.insertText(newMarker);
    }
  }
}

QVector<QTextBlock> MarkdownEditor::findChildBlocks(const QTextBlock &block) {
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

int MarkdownEditor::getIndentLevel(const QTextBlock &block) {
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

void MarkdownEditor::applyListHangingIndent(const QTextBlock &block) {
  QString text = block.text();
  QRegularExpression listPattern("^(\\s*)([-*+]|[0-9]+\\.)\\s+");
  QRegularExpressionMatch match = listPattern.match(text);
  if (!match.hasMatch()) {
    return; // Not a list item
  }
  QFont font = this->font();
  QFontMetrics fm(font);
  QString indent = match.captured(1); // Leading whitespace
  QString bullet = match.captured(2); // Bullet marker (-, *, +, or number.)
  int bulletWidth = fm.horizontalAdvance(indent + bullet + " ");
  QTextCursor cursor(document());
  cursor.setPosition(block.position());
  QTextBlockFormat blockFormat = cursor.blockFormat();
  blockFormat.setLeftMargin(bulletWidth);     // Total left margin
  blockFormat.setTextIndent(-bulletWidth);    // Negative indent for hanging effect
  cursor.setBlockFormat(blockFormat);
}

void MarkdownEditor::applyDeferredFormatting() {
  // This method is called after user stops typing (via QTimer)
  // It safely applies hanging indent formatting to all list items
  
  // Store the current modified state
  bool wasModified = document()->isModified();
  
  // Block signals to prevent triggering textChanged and modificationChanged
  document()->blockSignals(true);
  
  QTextBlock block = document()->firstBlock();
  while (block.isValid()) {
    applyListHangingIndent(block);
    block = block.next();
  }
  
  // Restore signals
  document()->blockSignals(false);
  
  // Restore the modified state (formatting shouldn't count as modification)
  document()->setModified(wasModified);
}
