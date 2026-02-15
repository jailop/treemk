#include <QDir>
#include <QFileInfo>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QRegularExpression>

#include "defs.h"
#include "markdowneditor.h"

QString MarkdownEditor::saveImageFromClipboard(const QImage& image) {
    // Check if we have a current file path
    if (m_currentFilePath.isEmpty()) {
        QMessageBox::warning(
            this, tr("Cannot Save Image"),
            tr("Please save the document first before pasting images."));
        return QString();
    }

    // Get the directory of the current file
    QFileInfo fileInfo(m_currentFilePath);
    QDir fileDir = fileInfo.absoluteDir();

    // Use a single 'images' directory for all files
    QString imagesDirName = "images";
    QString imagesDirPath = fileDir.filePath(imagesDirName);
    QDir imagesDir(imagesDirPath);

    if (!imagesDir.exists()) {
        if (!fileDir.mkdir(imagesDirName)) {
            QMessageBox::warning(
                this, tr("Cannot Create Directory"),
                tr("Failed to create directory '%1'.").arg(imagesDirName));
            return QString();
        }
    }

    // Find the next available number in sequence
    int nextNumber = 1;
    QRegularExpression imagePattern("^image_(\\d+)\\.png$");

    QStringList existingFiles =
        imagesDir.entryList(QStringList() << "image_*.png", QDir::Files);
    for (const QString& file : existingFiles) {
        QRegularExpressionMatch match = imagePattern.match(file);
        if (match.hasMatch()) {
            int num = match.captured(1).toInt();
            if (num >= nextNumber) {
                nextNumber = num + 1;
            }
        }
    }

    // Suggest filename
    QString suggestedName =
        QString("image_%1.png").arg(nextNumber, 3, 10, QChar('0'));

    // Ask user for filename
    bool ok;
    QString fileName = QInputDialog::getText(
        this, tr("Save Image"), tr("Enter image filename:"), QLineEdit::Normal,
        suggestedName, &ok);

    if (!ok || fileName.isEmpty()) {
        return QString();  // User cancelled
    }

    // Ensure .png extension
    if (!fileName.endsWith(".png", Qt::CaseInsensitive)) {
        fileName += ".png";
    }

    // Check if file already exists
    QString fullPath = imagesDir.filePath(fileName);
    if (QFileInfo::exists(fullPath)) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, tr("File Exists"),
            tr("File '%1' already exists. Overwrite?").arg(fileName),
            QMessageBox::Yes | QMessageBox::No);

        if (reply != QMessageBox::Yes) {
            return QString();  // User chose not to overwrite
        }
    }

    // Save the image
    if (image.save(fullPath, "PNG")) {
        return imagesDirName + "/" +
               fileName;  // Return relative path with subdirectory
    }

    QMessageBox::warning(this, tr("Save Failed"),
                         tr("Failed to save image to '%1'.").arg(fileName));
    return QString();
}
