#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QProcess>
#include <QStatusBar>
#include <QTextDocument>
#include <QTextStream>

#include "fileutils.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "tabeditor.h"

void MainWindow::exportToHtml() {
    TabEditor* tab = currentTabEditor();
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
    // Add Mermaid filter if available with proper working directory
    QProcess checkFilter;
    checkFilter.start("which", QStringList() << "mermaid-filter");
    if (checkFilter.waitForFinished(2000) && checkFilter.exitCode() == 0) {
        pandocArgs << "--filter" << "mermaid-filter";
    }
    QProcess pandoc;
    // Set working directory to a writable location for mermaid-filter
    QString workDir = QFileInfo(currentFilePath).absolutePath();
    pandoc.setWorkingDirectory(workDir);
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
        statusBar()->showMessage(tr("Exported to HTML: %1").arg(outputPath),
                                 3000);
    }
}

void MainWindow::exportToPdf() {
    TabEditor* tab = currentTabEditor();
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
    pandocArgs << currentFilePath << "-o" << outputPath
               << "--pdf-engine=xelatex";

    // Add Mermaid filter if available with proper working directory
    QProcess checkFilter;
    checkFilter.start("which", QStringList() << "mermaid-filter");
    if (checkFilter.waitForFinished(2000) && checkFilter.exitCode() == 0) {
        pandocArgs.insert(pandocArgs.size() - 2, "--filter");
        pandocArgs.insert(pandocArgs.size() - 2, "mermaid-filter");
    }

    QProcess pandoc;
    // Set working directory to a writable location for mermaid-filter
    QString workDir = QFileInfo(currentFilePath).absolutePath();
    pandoc.setWorkingDirectory(workDir);
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
        statusBar()->showMessage(tr("Exported to PDF: %1").arg(outputPath),
                                 3000);
    }
}

void MainWindow::exportToDocx() {
    TabEditor* tab = currentTabEditor();
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

    // Add Mermaid filter if available with proper working directory
    QProcess checkFilter;
    checkFilter.start("which", QStringList() << "mermaid-filter");
    if (checkFilter.waitForFinished(2000) && checkFilter.exitCode() == 0) {
        pandocArgs.insert(pandocArgs.size() - 2, "--filter");
        pandocArgs.insert(pandocArgs.size() - 2, "mermaid-filter");
    }

    QProcess pandoc;
    // Set working directory to a writable location for mermaid-filter
    QString workDir = QFileInfo(currentFilePath).absolutePath();
    pandoc.setWorkingDirectory(workDir);
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
        statusBar()->showMessage(tr("Exported to Word: %1").arg(outputPath),
                                 3000);
    }
}

void MainWindow::exportToPlainText() {
    TabEditor* tab = currentTabEditor();
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

    FileUtils::FileCreationResult result = FileUtils::createFileWithDirectories(
        outputPath, tab->editor()->toPlainText());

    if (!result.success) {
        QMessageBox::warning(this, tr("Export Failed"), result.errorMessage);
        return;
    }

    statusBar()->showMessage(tr("Exported to plain text: %1").arg(outputPath),
                             3000);
}
