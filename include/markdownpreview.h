#ifndef MARKDOWNPREVIEW_H
#define MARKDOWNPREVIEW_H

#include <QWebEngineView>

class MarkdownPreview : public QWebEngineView
{
    Q_OBJECT

public:
    explicit MarkdownPreview(QWidget *parent = nullptr);
    ~MarkdownPreview();

    void setMarkdownContent(const QString &markdown);
    void setTheme(const QString &theme);
    void setBasePath(const QString &path);
    void setLatexEnabled(bool enabled);

private:
    QString convertMarkdownToHtml(const QString &markdown);
    QString getStyleSheet(const QString &theme);
    QString processLatexFormulas(const QString &html);
    
    QString currentTheme;
    QString basePath;
    bool latexEnabled;
};

#endif // MARKDOWNPREVIEW_H
