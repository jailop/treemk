#include "mainwindow.h"
#include "tabeditor.h"
#include "markdowneditor.h"
#include "formuladialog.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextCursor>
#include <QTextBlock>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QStatusBar>

void MainWindow::insertImage() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QString imagePath = QFileDialog::getOpenFileName(
      this, tr("Select Image"), currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
      tr("Images (*.png *.jpg *.jpeg *.gif *.bmp *.svg)"));

  if (imagePath.isEmpty()) {
    return;
  }

  QString relPath = imagePath;
  if (!currentFilePath.isEmpty()) {
    QDir currentDir = QFileInfo(currentFilePath).absoluteDir();
    relPath = currentDir.relativeFilePath(imagePath);
  }

  bool ok;
  QString altText = QInputDialog::getText(
      this, tr("Image Alt Text"),
      tr("Enter alternative text for the image (optional):"), QLineEdit::Normal,
      QFileInfo(imagePath).baseName(), &ok);

  if (!ok) {
    return;
  }

  QString markdownImage = QString("![%1](%2)").arg(altText, relPath);

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.insertText(markdownImage);
}

void MainWindow::attachDocument() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QString documentPath = QFileDialog::getOpenFileName(
      this, tr("Select Document to Attach"),
      currentFolder.isEmpty() ? QDir::homePath() : currentFolder,
      tr("All Files (*)"));

  if (documentPath.isEmpty()) {
    return;
  }

  QFileInfo fileInfo(documentPath);
  QString fileName = fileInfo.fileName();

  QDir attachmentsDir;
  if (!currentFilePath.isEmpty()) {
    QFileInfo currentFileInfo(currentFilePath);
    attachmentsDir = QDir(currentFileInfo.absolutePath());
  } else if (!currentFolder.isEmpty()) {
    attachmentsDir = QDir(currentFolder);
  } else {
    QMessageBox::warning(
        this, tr("No Folder Open"),
        tr("Please save the document or open a folder first."));
    return;
  }

  if (!attachmentsDir.exists("attachments")) {
    attachmentsDir.mkdir("attachments");
  }

  attachmentsDir.cd("attachments");
  QString targetPath = attachmentsDir.absoluteFilePath(fileName);

  int counter = 1;
  while (QFile::exists(targetPath)) {
    QString baseName = fileInfo.completeBaseName();
    QString suffix = fileInfo.suffix();
    fileName = QString("%1_%2.%3").arg(baseName).arg(counter).arg(suffix);
    targetPath = attachmentsDir.absoluteFilePath(fileName);
    counter++;
  }

  if (QFile::copy(documentPath, targetPath)) {
    QString relPath;
    if (!currentFilePath.isEmpty()) {
      QDir currentDir = QFileInfo(currentFilePath).absoluteDir();
      relPath = currentDir.relativeFilePath(targetPath);
    } else {
      relPath = QString("attachments/%1").arg(fileName);
    }

    QString markdownLink = QString("[%1](%2)").arg(fileName, relPath);

    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(markdownLink);

    statusBar()->showMessage(tr("Document attached: %1").arg(fileName), 3000);
  } else {
    QMessageBox::warning(this, tr("Error"),
                         tr("Could not copy the document."));
  }
}

void MainWindow::insertFormula() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  FormulaDialog dialog(this);
  if (dialog.exec() == QDialog::Accepted) {
    QString formula = dialog.getFormula();
    bool isBlock = dialog.isBlockFormula();

    QString markdownFormula;
    if (isBlock) {
      markdownFormula = QString("\n$$\n%1\n$$\n").arg(formula);
    } else {
      markdownFormula = QString("$%1$").arg(formula);
    }

    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(markdownFormula);
  }
}

void MainWindow::insertWikiLink() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  QString selectedText = cursor.selectedText();

  bool ok;
  QString linkTarget = QInputDialog::getText(
      this, tr("Insert Wiki Link"), tr("Enter target page name:"),
      QLineEdit::Normal, selectedText, &ok);

  if (!ok || linkTarget.isEmpty()) {
    return;
  }

  QString displayText = selectedText;
  if (displayText.isEmpty()) {
    displayText = QInputDialog::getText(
        this, tr("Insert Wiki Link"),
        tr("Enter display text (optional, leave empty to use target):"),
        QLineEdit::Normal, linkTarget, &ok);
    if (!ok) {
      return;
    }
  }

  QString wikiLink;
  if (displayText.isEmpty() || displayText == linkTarget) {
    wikiLink = QString("[[%1]]").arg(linkTarget);
  } else {
    wikiLink = QString("[[%1|%2]]").arg(linkTarget, displayText);
  }

  if (cursor.hasSelection()) {
    cursor.removeSelectedText();
  }
  cursor.insertText(wikiLink);
}

void MainWindow::insertHeader() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.movePosition(QTextCursor::StartOfLine);

  QString line = cursor.block().text();
  if (line.startsWith("#")) {
    int headerLevel = 0;
    while (headerLevel < line.length() && line[headerLevel] == '#') {
      headerLevel++;
    }
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(line.mid(headerLevel).trimmed());
  } else {
    cursor.insertText("# ");
  }
}

void MainWindow::insertBold() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  if (cursor.hasSelection()) {
    QString selected = cursor.selectedText();
    cursor.insertText(QString("**%1**").arg(selected));
  } else {
    cursor.insertText("****");
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 2);
    tab->editor()->setTextCursor(cursor);
  }
}

void MainWindow::insertItalic() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  if (cursor.hasSelection()) {
    QString selected = cursor.selectedText();
    cursor.insertText(QString("*%1*").arg(selected));
  } else {
    cursor.insertText("**");
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    tab->editor()->setTextCursor(cursor);
  }
}

void MainWindow::insertCode() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  if (cursor.hasSelection()) {
    QString selected = cursor.selectedText();
    cursor.insertText(QString("`%1`").arg(selected));
  } else {
    cursor.insertText("``");
    cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 1);
    tab->editor()->setTextCursor(cursor);
  }
}

void MainWindow::insertCodeBlock() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  QString text = "```\n\n```";
  cursor.insertText(text);
  cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 4);
  tab->editor()->setTextCursor(cursor);
}

void MainWindow::insertList() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.insertText("- ");
}

void MainWindow::insertNumberedList() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.insertText("1. ");
}

void MainWindow::insertBlockquote() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.movePosition(QTextCursor::StartOfLine);

  QString line = cursor.block().text();
  if (line.startsWith("> ")) {
    cursor.movePosition(QTextCursor::EndOfLine, QTextCursor::KeepAnchor);
    cursor.removeSelectedText();
    cursor.insertText(line.mid(2));
  } else {
    cursor.insertText("> ");
  }
}

void MainWindow::insertHorizontalRule() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.insertText("\n---\n");
}

void MainWindow::insertLink() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  QTextCursor cursor = tab->editor()->textCursor();
  QString selectedText = cursor.selectedText();

  bool ok;
  QString url = QInputDialog::getText(this, tr("Insert Link"), tr("Enter URL:"),
                                      QLineEdit::Normal, "", &ok);

  if (!ok || url.isEmpty()) {
    return;
  }

  QString linkText =
      selectedText.isEmpty()
          ? QInputDialog::getText(this, tr("Insert Link"),
                                  tr("Enter link text:"), QLineEdit::Normal,
                                  url, &ok)
          : selectedText;

  if (!ok || linkText.isEmpty()) {
    return;
  }

  QString markdownLink = QString("[%1](%2)").arg(linkText, url);
  if (cursor.hasSelection()) {
    cursor.removeSelectedText();
  }
  cursor.insertText(markdownLink);
}

void MainWindow::insertTable() {
  TabEditor *tab = currentTabEditor();
  if (!tab)
    return;

  bool ok;
  int rows = QInputDialog::getInt(this, tr("Insert Table"),
                                  tr("Number of rows:"), 3, 1, 100, 1, &ok);
  if (!ok)
    return;

  int cols = QInputDialog::getInt(this, tr("Insert Table"),
                                  tr("Number of columns:"), 3, 1, 100, 1, &ok);
  if (!ok)
    return;

  QString table;

  for (int c = 0; c < cols; ++c) {
    table += "| Header ";
  }
  table += "|\n";

  for (int c = 0; c < cols; ++c) {
    table += "|--------";
  }
  table += "|\n";

  for (int r = 0; r < rows; ++r) {
    for (int c = 0; c < cols; ++c) {
      table += "| Cell   ";
    }
    table += "|\n";
  }

  QTextCursor cursor = tab->editor()->textCursor();
  cursor.insertText(table);
}
