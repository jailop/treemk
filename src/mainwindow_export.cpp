#include "mainwindow.h"
#include "tabeditor.h"
#include "markdowneditor.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QProcess>
#include <QTextDocument>
#include <QPrinter>
#include <QPrintDialog>
#include <QStatusBar>

void MainWindow::exportToHtml() {
  TabEditor *tab = currentTabEditor();
  if (!tab || currentFilePath.isEmpty()) {
    QMessageBox::warning(this, tr("Export to HTML"),
                         tr("Please save the document before exporting."));
    return;
  }

  QString outputPath = QFileDialog::getSaveFileName(
      this, tr("Export to HTML"),
      currentFilePath.left(currentFilePath.lastIndexOf('.')) + ".html",
      tr("HTML Files (*.html)"));

  if (outputPath.isEmpty()) {
    return;
  }

  if (tab->isModified()) {
    save();
  }

  QStringList pandocArgs;
  pandocArgs << currentFilePath << "-o" << outputPath << "--standalone"
             << "--mathjax";

  QProcess pandoc;
  pandoc.start("pandoc", pandocArgs);

  if (!pandoc.waitForFinished(30000)) {
    QMessageBox::warning(this, tr("Export Failed"),
                         tr("Pandoc process timed out."));
    return;
  }

  if (pandoc.exitCode() != 0) {
    QMessageBox::warning(
        this, tr("Export Failed"),
        tr("Failed to export to HTML.\nError: %1")
            .arg(QString::fromUtf8(pandoc.readAllStandardError())));
  } else {
    statusBar()->showMessage(tr("Exported to HTML: %1").arg(outputPath), 3000);
  }
}

void MainWindow::exportToPdf() {
  TabEditor *tab = currentTabEditor();
  if (!tab || currentFilePath.isEmpty()) {
    QMessageBox::warning(this, tr("Export to PDF"),
                         tr("Please save the document before exporting."));
    return;
  }

  QString outputPath = QFileDialog::getSaveFileName(
      this, tr("Export to PDF"),
      currentFilePath.left(currentFilePath.lastIndexOf('.')) + ".pdf",
      tr("PDF Files (*.pdf)"));

  if (outputPath.isEmpty()) {
    return;
  }

  if (tab->isModified()) {
    save();
  }

  QStringList pandocArgs;
  pandocArgs << currentFilePath << "-o" << outputPath << "--pdf-engine=xelatex";

  QProcess pandoc;
  pandoc.start("pandoc", pandocArgs);

  if (!pandoc.waitForFinished(60000)) {
    QMessageBox::warning(this, tr("Export Failed"),
                         tr("Pandoc process timed out."));
    return;
  }

  if (pandoc.exitCode() != 0) {
    QMessageBox::warning(
        this, tr("Export Failed"),
        tr("Failed to export to PDF.\nError: %1")
            .arg(QString::fromUtf8(pandoc.readAllStandardError())));
  } else {
    statusBar()->showMessage(tr("Exported to PDF: %1").arg(outputPath), 3000);
  }
}

void MainWindow::exportToDocx() {
  TabEditor *tab = currentTabEditor();
  if (!tab || currentFilePath.isEmpty()) {
    QMessageBox::warning(this, tr("Export to Word"),
                         tr("Please save the document before exporting."));
    return;
  }

  QString outputPath = QFileDialog::getSaveFileName(
      this, tr("Export to Word"),
      currentFilePath.left(currentFilePath.lastIndexOf('.')) + ".docx",
      tr("Word Documents (*.docx)"));

  if (outputPath.isEmpty()) {
    return;
  }

  if (tab->isModified()) {
    save();
  }

  QStringList pandocArgs;
  pandocArgs << currentFilePath << "-o" << outputPath;

  QProcess pandoc;
  pandoc.start("pandoc", pandocArgs);

  if (!pandoc.waitForFinished(30000)) {
    QMessageBox::warning(this, tr("Export Failed"),
                         tr("Pandoc process timed out."));
    return;
  }

  if (pandoc.exitCode() != 0) {
    QMessageBox::warning(
        this, tr("Export Failed"),
        tr("Failed to export to Word.\nError: %1")
            .arg(QString::fromUtf8(pandoc.readAllStandardError())));
  } else {
    statusBar()->showMessage(tr("Exported to Word: %1").arg(outputPath), 3000);
  }
}

void MainWindow::exportToPlainText() {
  TabEditor *tab = currentTabEditor();
  if (!tab) {
    return;
  }

  QString outputPath = QFileDialog::getSaveFileName(
      this, tr("Export to Plain Text"),
      currentFilePath.isEmpty()
          ? QString("untitled.txt")
          : currentFilePath.left(currentFilePath.lastIndexOf('.')) + ".txt",
      tr("Text Files (*.txt)"));

  if (outputPath.isEmpty()) {
    return;
  }

  QFile file(outputPath);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
    QMessageBox::warning(this, tr("Export Failed"),
                         tr("Could not open file for writing."));
    return;
  }

  QTextStream out(&file);
  out << tab->editor()->toPlainText();
  file.close();

  statusBar()->showMessage(tr("Exported to plain text: %1").arg(outputPath),
                           3000);
}
