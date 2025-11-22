#ifndef MARKDOWNEDITOR_H
#define MARKDOWNEDITOR_H

#include <QPlainTextEdit>

class QSyntaxHighlighter;
class LineNumberArea;
class MarkdownHighlighter;

class MarkdownEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit MarkdownEditor(QWidget *parent = nullptr);
    ~MarkdownEditor();

    void lineNumberAreaPaintEvent(QPaintEvent *event);
    int lineNumberAreaWidth();
    
    bool isModified() const;
    void setModified(bool modified);
    
    MarkdownHighlighter* getHighlighter() const;
    MarkdownHighlighter* highlighter() const { return getHighlighter(); }
    
    QString getLinkAtPosition(int position) const;
    
    void setCurrentFilePath(const QString &filePath);

signals:
    void wikiLinkClicked(const QString &linkTarget);

protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void keyPressEvent(QKeyEvent *event) override;
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void insertFromMimeData(const QMimeData *source) override;

private slots:
    void updateLineNumberAreaWidth(int newBlockCount);
    void highlightCurrentLine();
    void updateLineNumberArea(const QRect &rect, int dy);

private:
    void setupEditor();
    QString saveImageFromClipboard(const QImage &image);
    
    LineNumberArea *lineNumberArea;
    MarkdownHighlighter *m_highlighter;
    QString m_currentFilePath;
};

class LineNumberArea : public QWidget
{
    Q_OBJECT

public:
    LineNumberArea(MarkdownEditor *editor) : QWidget(editor), codeEditor(editor) {}

    QSize sizeHint() const override
    {
        return QSize(codeEditor->lineNumberAreaWidth(), 0);
    }

protected:
    void paintEvent(QPaintEvent *event) override
    {
        codeEditor->lineNumberAreaPaintEvent(event);
    }

private:
    MarkdownEditor *codeEditor;
};

#endif // MARKDOWNEDITOR_H
