#include <QRegularExpression>
#include <QTextBlock>

#include "defs.h"
#include "markdowneditor.h"

const char* LIST_ITEM_PATTERN = R"(^(\s*)([-*+]|[0-9]+\.)\s+)";
// const QString LIST_ITEM_PATTERN("(^(\\s*)([-*+]|[0-9]+\\.)\\s+)");

bool MarkdownEditor::isCurrentLineListItem() const {
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text();
    QRegularExpression listPattern(LIST_ITEM_PATTERN);
    return listPattern.match(currentLine).hasMatch();
}

void MarkdownEditor::moveListItemUp() {
    if (!isCurrentLineListItem()) return;
    QTextBlock currentBlock = textCursor().block();
    QTextBlock previousBlock = currentBlock.previous();
    if (!previousBlock.isValid()) return;
    QRegularExpression listPattern(LIST_ITEM_PATTERN);
    if (!listPattern.match(previousBlock.text()).hasMatch()) return;
    int currentIndent = getIndentLevel(currentBlock);
    int previousIndent = getIndentLevel(previousBlock);
    if (previousIndent < currentIndent) return;
    swapTextBlocks(currentBlock, previousBlock);
}

void MarkdownEditor::moveListItemDown() {
    if (!isCurrentLineListItem()) return;
    QTextBlock currentBlock = textCursor().block();
    QTextBlock nextBlock = currentBlock.next();
    if (!nextBlock.isValid()) return;
    QRegularExpression listPattern(LIST_ITEM_PATTERN);
    if (!listPattern.match(nextBlock.text()).hasMatch()) return;
    int currentIndent = getIndentLevel(currentBlock);
    int nextIndent = getIndentLevel(nextBlock);
    if (nextIndent < currentIndent) return;
    swapTextBlocks(currentBlock, nextBlock);
}
