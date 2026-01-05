#ifndef MARKDOWNPREVIEW_H
#define MARKDOWNPREVIEW_H

#include <QWebEngineView>

class MarkdownPreview : public QWebEngineView {
  Q_OBJECT

public:
  explicit MarkdownPreview(QWidget *parent = nullptr);
  ~MarkdownPreview();

  void setMarkdownContent(const QString &markdown);
  void setTheme(const QString &theme);
  void setBasePath(const QString &path);
  void setLatexEnabled(bool enabled);
  void scrollToPercentage(double percentage);
  double currentScrollPercentage() const;

signals:
  void wikiLinkClicked(const QString &linkTarget);

private slots:
  void showContextMenu(const QPoint &pos);
  void reloadPreview();
  void onThemeChanged();

private:
  QString convertMarkdownToHtml(const QString &markdown);
  QString getStyleSheet(const QString &theme);
  QString processLatexFormulas(const QString &html);
  QString processWikiLinks(const QString &html);
  QString resolveAndIncludeFile(const QString &linkTarget,
                                const QString &displayText);

  QString currentTheme;
  QString basePath;
  bool latexEnabled;
  double lastScrollPercentage;
};

#endif // MARKDOWNPREVIEW_H
