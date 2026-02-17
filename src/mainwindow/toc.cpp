#include "mainwindow.h"
#include "markdowneditor.h"
#include "tabeditor.h"
#include "tocgenerator.h"
#include <QTextCursor>
#include <QTextDocument>
#include <QStatusBar>

void MainWindow::insertToc() {
    TabEditor* tab = currentTabEditor();
    if (!tab) {
        return;
    }

    MarkdownEditor* editor = tab->editor();
    QString currentText = editor->toPlainText();
    
    QString newText = TocGenerator::generateToc(currentText);
    
    if (newText != currentText) {
        QTextCursor cursor = editor->textCursor();
        int originalPosition = cursor.position();
        
        editor->selectAll();
        editor->insertPlainText(newText);
        
        cursor.setPosition(qMin(originalPosition, editor->document()->characterCount() - 1));
        editor->setTextCursor(cursor);
        
        statusBar()->showMessage(tr("Table of Contents generated"), 3000);
    } else {
        statusBar()->showMessage(tr("No headers found to generate TOC"), 3000);
    }
}
