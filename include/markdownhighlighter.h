#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QRegularExpression>

class MarkdownHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    explicit MarkdownHighlighter(QTextDocument *parent = nullptr);
    
    void setRootPath(const QString &path);
    void setColorScheme(const QString &scheme);

public slots:
    void updateColorScheme();

protected:
    void highlightBlock(const QString &text) override;

private:
    struct HighlightingRule
    {
        QRegularExpression pattern;
        QTextCharFormat format;
    };
    
    void setupFormats();
    
    QVector<HighlightingRule> highlightingRules;
    
    QTextCharFormat h1Format;
    QTextCharFormat h2Format;
    QTextCharFormat h3Format;
    QTextCharFormat h4Format;
    QTextCharFormat h5Format;
    QTextCharFormat h6Format;
    QTextCharFormat boldFormat;
    QTextCharFormat italicFormat;
    QTextCharFormat codeFormat;
    QTextCharFormat inlineCodeFormat;
    QTextCharFormat linkFormat;
    QTextCharFormat urlFormat;
    QTextCharFormat listFormat;
    QTextCharFormat blockquoteFormat;
    QTextCharFormat horizontalRuleFormat;
    QTextCharFormat wikiLinkFormat;
    QTextCharFormat brokenWikiLinkFormat;
    QTextCharFormat inclusionLinkFormat;
    QTextCharFormat brokenInclusionLinkFormat;
    QTextCharFormat inlineLatexFormat;
    QTextCharFormat blockLatexFormat;
    
    QString rootPath;
    QString currentColorScheme;
    bool checkWikiLinkExists(const QString &linkText) const;
};

#endif // MARKDOWNHIGHLIGHTER_H
