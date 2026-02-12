#ifndef MARKDOWNHIGHLIGHTER_H
#define MARKDOWNHIGHLIGHTER_H

#include <QRegularExpression>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class MarkdownHighlighter : public QSyntaxHighlighter {
  Q_OBJECT

public:
  explicit MarkdownHighlighter(QTextDocument *parent = nullptr);

  void setRootPath(const QString &path);
  void setColorScheme(const QString &scheme);
  void setCodeSyntaxEnabled(bool enabled);
  QString getColorScheme() const { return currentColorScheme; }
  void setCurrentCursorLine(int lineNumber);

public slots:
  void updateColorScheme();

protected:
  void highlightBlock(const QString &text) override;

private:
  struct HighlightingRule {
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
  QTextCharFormat strikethroughFormat;
  /*
  QTextCharFormat taskPendingFormat;
  QTextCharFormat taskDoneFormat;
  QTextCharFormat taskPartialFormat;
  */

  QString rootPath;
  QString currentColorScheme;
  QString currentCodeLanguage;
  bool codeSyntaxEnabled;
  int currentCursorLine;
  bool checkWikiLinkExists(const QString &linkText) const;
  void highlightCodeLine(const QString &text, const QString &language);
  QColor getSubtleColor() const;

  enum BlockState { Normal = -1, InCodeBlock = 1 };

  // Code syntax formats
  QTextCharFormat codeKeywordFormat;
  QTextCharFormat codeStringFormat;
  QTextCharFormat codeCommentFormat;
  QTextCharFormat codeNumberFormat;
  QTextCharFormat codeFunctionFormat;
  QTextCharFormat codeTypeFormat;
};

#endif // MARKDOWNHIGHLIGHTER_H
