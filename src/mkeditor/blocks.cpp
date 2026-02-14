#include "defs.h"
#include "markdowneditor.h"
#include <QTextBlock>

void MarkdownEditor::swapTextBlocks(QTextBlock &block1, QTextBlock &block2) {
    if (!block1.isValid() || !block2.isValid()) {
        return;
    }
    
    QString text1 = block1.text();
    QString text2 = block2.text();
    int block1Num = block1.blockNumber();
    int block2Num = block2.blockNumber();
    
    // Save cursor info
    QTextCursor cursor = textCursor();
    int currentBlockNum = cursor.block().blockNumber();
    int relativePos = cursor.position() - cursor.block().position();
    
    // Determine which block comes first in document
    bool block1IsFirst = (block1Num < block2Num);
    QTextBlock firstBlock = block1IsFirst ? block1 : block2;
    QTextBlock secondBlock = block1IsFirst ? block2 : block1;
    QString firstText = block1IsFirst ? text1 : text2;
    QString secondText = block1IsFirst ? text2 : text1;
    
    // Calculate character positions
    int firstStart = firstBlock.position();
    int firstEnd = firstStart + firstBlock.length() - 1;  // -1 to exclude newline
    int secondStart = secondBlock.position();
    int secondEnd = secondStart + secondBlock.length() - 1;  // -1 to exclude newline
    
    QTextCursor tempCursor(document());
    tempCursor.beginEditBlock();
    
    // Replace second block's text (do this first so first block position stays valid)
    tempCursor.setPosition(secondStart);
    tempCursor.setPosition(secondEnd, QTextCursor::KeepAnchor);
    tempCursor.insertText(firstText);
    
    // Replace first block's text
    tempCursor.setPosition(firstStart);
    tempCursor.setPosition(firstEnd, QTextCursor::KeepAnchor);
    tempCursor.insertText(secondText);
    
    tempCursor.endEditBlock();
    
    // Restore cursor to correct block
    int targetBlockNum = (currentBlockNum == block1Num) ? block2Num : block1Num;
    QTextBlock targetBlock = document()->findBlockByNumber(targetBlockNum);
    int newPos = targetBlock.position() + qMin(relativePos, targetBlock.text().length());
    cursor.setPosition(newPos);
    setTextCursor(cursor);
}
