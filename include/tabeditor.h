#ifndef TABEDITOR_H
#define TABEDITOR_H

#include <QString>
#include <QWidget>

class MarkdownEditor;
class MarkdownPreview;
class OutlinePanel;
class QSplitter;
class QTimer;

class TabEditor : public QWidget {
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
  MarkdownEditor *editor() const { return m_editor; }
  MarkdownPreview *preview() const { return m_preview; }
  QString content() const;
  void setContent(const QString &content);
  bool ownSaved() { return m_ownSaved; }
  void clearOwnSaved() { m_ownSaved = false; }
  
signals:
  void modificationChanged(bool modified);
  void filePathChanged(const QString &filePath);
  void fileModified(bool modified);
  void documentModified();
  void wikiLinkClicked(const QString &linkTarget);
  void markdownLinkClicked(const QString &linkTarget);
  void openLinkInNewWindowRequested(const QString &linkTarget);
  void internalLinkClicked(const QString &anchor);
  void aiAssistRequested();
  
public slots:
  void updatePreview();
private slots:
  void onDocumentModified();
  void syncPreviewScroll();

private:
  void setupUI();
  MarkdownEditor *m_editor;
  MarkdownPreview *m_preview;
  QSplitter *m_splitter;
  QTimer *m_previewTimer;
  QString m_filePath;
  bool m_isModified;
  bool m_ownSaved;
};

#endif // TABEDITOR_H
