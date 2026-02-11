#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include <QMap>
#include <QTextEdit>

class QSyntaxHighlighter;
class LineNumberArea;
class MarkdownHighlighter;

class MarkdownEditor : public QTextEdit {
  Q_OBJECT

public:
  explicit MarkdownEditor(QWidget *parent = nullptr);
  ~MarkdownEditor();

  void lineNumberAreaPaintEvent(QPaintEvent *event);
  int lineNumberAreaWidth();

  bool isModified() const;
  void setModified(bool modified);

  MarkdownHighlighter *getHighlighter() const;
  MarkdownHighlighter *highlighter() const { return getHighlighter(); }

   QString getLinkAtPosition(int position) const;
   QString getExternalLinkAtPosition(int position) const;
   QString getMarkdownLinkAtPosition(int position) const;
  QString getTaskMarkerAtPosition(int position) const;
  bool isClickOnCheckbox(int position) const;
  void toggleTaskAtPosition(int position);

  void setCurrentFilePath(const QString &filePath);

signals:
  void wikiLinkClicked(const QString &linkTarget);
  void markdownLinkClicked(const QString &linkTarget);
  void openLinkInNewWindowRequested(const QString &linkTarget);
  void aiAssistRequested();
  void aiAssistWithPromptRequested(const QString &promptText);

protected:
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void keyPressEvent(QKeyEvent *event) override;
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dragMoveEvent(QDragMoveEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void insertFromMimeData(const QMimeData *source) override;
  void paintEvent(QPaintEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect &rect, int dy);
  void onTextChanged();
  void onThemeChanged();

private:
  void setupEditor();
  QString saveImageFromClipboard(const QImage &image);
  void updateWordFrequency();
  QString predictWord(const QString &prefix) const;
  QString predictWordUnigram(const QString &prefix) const;
  QString predictWordBigram(const QString &previousWord,
                            const QString &prefix) const;
  void showPrediction();
  void hidePrediction();
  void acceptPrediction();

    void updateParentTask(const QTextBlock &block);
    QTextBlock findParentBlock(const QTextBlock &block, int currentIndent);
    void updateTaskState(const QTextBlock &block);
    void uncheckChildTasks(const QTextBlock &block);
    QVector<QTextBlock> findChildBlocks(const QTextBlock &block);
    int getIndentLevel(const QTextBlock &block);
    void applyListHangingIndent(const QTextBlock &block);

  private slots:
    void applyDeferredFormatting();

  private:
    LineNumberArea *lineNumberArea;
    MarkdownHighlighter *m_highlighter;
    QString m_currentFilePath;
    class QTimer *m_formatTimer;

    // Word prediction
    QMap<QString, int> m_wordFrequency;                   // Unigram model
    QMap<QPair<QString, QString>, int> m_bigramFrequency; // Bigram model
    QString m_currentPrediction;
    bool m_predictionEnabled;
};

class LineNumberArea : public QWidget {
  Q_OBJECT

public:
  LineNumberArea(MarkdownEditor *editor)
      : QWidget(editor), codeEditor(editor) {}

  QSize sizeHint() const override {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
  }

protected:
  void paintEvent(QPaintEvent *event) override {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

private:
  MarkdownEditor *codeEditor;
};

#endif // MARKDOWNEDITOR_H
