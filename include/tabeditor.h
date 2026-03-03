#ifndef TABEDITOR_H
#define TABEDITOR_H

#include <QString>
#include <QWidget>

class MarkdownEditor;
class MarkdownPreview;
class NavigationHistory;
class OutlinePanel;
class QSplitter;
class QTimer;

class TabEditor : public QWidget {
    Q_OBJECT
   public:
    explicit TabEditor(QWidget* parent = nullptr);
    ~TabEditor();
    QString filePath() const { return m_filePath; }
    void setFilePath(const QString& path) { m_filePath = path; }
    QString fileName() const;
    bool isModified() const { return m_isModified; }
    void setModified(bool modified);
    bool loadFile(const QString& filePath);
    bool saveFile();
    bool saveFileAs(const QString& filePath);
    MarkdownEditor* editor() const { return m_editor; }
    QString content() const;
    void setContent(const QString& content);
    bool ownSaved() { return m_ownSaved; }
    void clearOwnSaved() { m_ownSaved = false; }
    void setSharedPreview(MarkdownPreview* preview);
    void updatePreviewContent(MarkdownPreview* preview);
    double scrollPercentage() const { return m_lastScrollPercentage; }
    NavigationHistory* navigationHistory() const { return m_navigationHistory; }

   signals:
    void modificationChanged(bool modified);
    void filePathChanged(const QString& filePath);
    void fileModified(bool modified);
    void documentModified();
    void wikiLinkClicked(const QString& linkTarget);
    void markdownLinkClicked(const QString& linkTarget);
    void openLinkInNewTabRequested(const QString& linkTarget);
    void openLinkInNewWindowRequested(const QString& linkTarget);
    void internalLinkClicked(const QString& anchor);
    void aiAssistRequested();

   public slots:
    void updatePreview();
    void onEditorScrolled();

   private slots:
    void onDocumentModified();

   private:
    void setupUI();
    MarkdownEditor* m_editor;
    MarkdownPreview* m_sharedPreview;
    QTimer* m_previewTimer;
    NavigationHistory* m_navigationHistory;
    QString m_filePath;
    bool m_isModified;
    bool m_ownSaved;
    double m_lastScrollPercentage;
};

#endif  // TABEDITOR_H
