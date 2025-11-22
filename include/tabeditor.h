#ifndef TABEDITOR_H
#define TABEDITOR_H

#include <QWidget>
#include <QString>

class MarkdownEditor;
class MarkdownPreview;
class OutlinePanel;
class QSplitter;
class QTimer;

class TabEditor : public QWidget
{
    Q_OBJECT

public:
    explicit TabEditor(QWidget *parent = nullptr);
    ~TabEditor();

    QString filePath() const { return m_filePath; }
    void setFilePath(const QString &path) { m_filePath = path; }
    
    QString fileName() const;
    
    bool isModified() const { return m_isModified; }
    void setModified(bool modified);
    
    bool loadFile(const QString &filePath);
    bool saveFile();
    bool saveFileAs(const QString &filePath);
    
    MarkdownEditor* editor() const { return m_editor; }
    MarkdownPreview* preview() const { return m_preview; }
    OutlinePanel* outline() const { return m_outline; }
    
    QString content() const;
    void setContent(const QString &content);

signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString &filePath);

public slots:
    void updatePreview();

private slots:
    void onDocumentModified();
    void syncPreviewScroll();

private:
    void setupUI();

    MarkdownEditor *m_editor;
    MarkdownPreview *m_preview;
    OutlinePanel *m_outline;
    QSplitter *m_splitter;
    QTimer *m_previewTimer;
    
    QString m_filePath;
    bool m_isModified;
};

#endif // TABEDITOR_H
