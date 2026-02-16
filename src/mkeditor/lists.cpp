#include <QTextBlock>

#include "defs.h"
#include "markdowneditor.h"
#include "regexutils.h"

bool MarkdownEditor::isCurrentLineListItem() const {
    QTextCursor cursor = textCursor();
    QString currentLine = cursor.block().text();
    return RegexUtils::isListItem(currentLine);
}

void MarkdownEditor::moveListItemUp() {
    if (!isCurrentLineListItem()) return;
    QTextBlock currentBlock = textCursor().block();
    QTextBlock previousBlock = currentBlock.previous();
    if (!previousBlock.isValid()) return;
    if (!RegexUtils::isListItem(previousBlock.text())) return;
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
    if (!RegexUtils::isListItem(nextBlock.text())) return;
    int currentIndent = getIndentLevel(currentBlock);
    int nextIndent = getIndentLevel(nextBlock);
    if (nextIndent < currentIndent) return;
    swapTextBlocks(currentBlock, nextBlock);
}
