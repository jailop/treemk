#include "markdownpreview.h"
#include <QAction>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QDir>
#include <QKeySequence>
#include <QMap>
#include <QMenu>
#include <QRegularExpression>
#include <QShortcut>
#include <QTextDocument>
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <md4c-html.h>

class WikiLinkPage : public QWebEnginePage {
public:
  WikiLinkPage(MarkdownPreview *parent)
      : QWebEnginePage(parent), preview(parent) {}

protected:
  bool acceptNavigationRequest(const QUrl &url, NavigationType type,
                               bool isMainFrame) override {
    if (url.scheme() == "wiki") {
      emit preview->wikiLinkClicked(url.path());
      return false;
    }
    // Open external links (http/https) in system browser
    if (url.scheme() == "http" || url.scheme() == "https") {
      QDesktopServices::openUrl(url);
      return false;
    }
    return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
  }

private:
  MarkdownPreview *preview;
};

MarkdownPreview::MarkdownPreview(QWidget *parent)
    : QWebEngineView(parent), currentTheme("light"), basePath(QDir::homePath()),
      latexEnabled(true) {
  setContextMenuPolicy(Qt::CustomContextMenu);
  // Set custom page to intercept link clicks
  WikiLinkPage *wikiPage = new WikiLinkPage(this);
  setPage(wikiPage);
  // Allow loading remote content (KaTeX CDN) from local HTML
  page()->settings()->setAttribute(
      QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
  // Connect context menu
  connect(this, &QWidget::customContextMenuRequested, this,
          &MarkdownPreview::showContextMenu);
  // Add F5 shortcut for reload
  QShortcut *reloadShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
  connect(reloadShortcut, &QShortcut::activated, this,
          &MarkdownPreview::reloadPreview);
}

MarkdownPreview::~MarkdownPreview() {}

void MarkdownPreview::setBasePath(const QString &path) { basePath = path; }

void MarkdownPreview::setLatexEnabled(bool enabled) { latexEnabled = enabled; }

void MarkdownPreview::setMarkdownContent(const QString &markdown) {
  QString html = convertMarkdownToHtml(markdown);
  html = processWikiLinks(html);
  if (latexEnabled) {
    html = processLatexFormulas(html);
  }
  QString styleSheet = getStyleSheet(currentTheme);
  // Choose highlight.js theme based on current theme
  QString highlightTheme = "github.min.css";
  if (currentTheme == "dark") {
    highlightTheme = "github-dark.min.css";
  } else if (currentTheme == "sepia") {
    highlightTheme = "github.min.css"; // Use light theme for sepia
  }

  QString fullHtml =
      QString("<!DOCTYPE html>"
              "<html>"
              "<head>"
              "<meta charset=\"UTF-8\">"
              "<link rel=\"stylesheet\" "
              "href=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/"
              "katex.min.css\">"
              "<link rel=\"stylesheet\" "
              "href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/"
              "11.9.0/styles/%1\">"
              "<script "
              "src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/"
              "katex.min.js\"></script>"
              "<script "
              "src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/"
              "auto-render.min.js\"></script>"
              "<script "
              "src=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/"
              "11.9.0/highlight.min.js\"></script>"
              "<style>%2</style>"
              "<script>"
              "document.addEventListener('DOMContentLoaded', function() {"
              "  renderMathInElement(document.body, {"
              "    delimiters: ["
              "      {left: '$$', right: '$$', display: true},"
              "      {left: '$', right: '$', display: false}"
              "    ],"
              "    throwOnError: false"
              "  });"
              "  document.querySelectorAll('pre code').forEach((block) => {"
              "    hljs.highlightElement(block);"
              "  });"
              "});"
              "</script>"
              "</head>"
              "<body>%3</body>"
              "</html>")
          .arg(highlightTheme, styleSheet, html);
  // Use setHtml with baseUrl to allow loading local images
  QUrl baseUrl = QUrl::fromLocalFile(basePath + "/");
  setHtml(fullHtml, baseUrl);
}

void MarkdownPreview::setTheme(const QString &theme) { currentTheme = theme; }

void MarkdownPreview::scrollToPercentage(double percentage) {
  // Use JavaScript to scroll the preview to a specific percentage
  QString script =
      QString("window.scrollTo(0, document.body.scrollHeight * %1);")
          .arg(percentage);
  page()->runJavaScript(script);
}

QString MarkdownPreview::convertMarkdownToHtml(const QString &markdown) {
  // Callback function for md4c to accumulate HTML output
  struct OutputBuffer {
    QString html;
  };

  auto processOutput = [](const MD_CHAR *data, MD_SIZE size, void *userdata) {
    OutputBuffer *buffer = static_cast<OutputBuffer *>(userdata);
    buffer->html.append(QString::fromUtf8(data, size));
  };

  OutputBuffer buffer;
  QByteArray utf8Data = markdown.toUtf8();

  // Configure parser flags for CommonMark + extensions
  unsigned parserFlags =
      MD_FLAG_TABLES |                   // Enable tables
      MD_FLAG_STRIKETHROUGH |            // Enable ~~strikethrough~~
      MD_FLAG_TASKLISTS |                // Enable task lists [ ] [x]
      MD_FLAG_LATEXMATHSPANS |           // Enable $math$ and $$math$$
      MD_FLAG_WIKILINKS |                // Enable [[wiki links]]
      MD_FLAG_PERMISSIVEURLAUTOLINKS |   // Auto-link URLs
      MD_FLAG_PERMISSIVEEMAILAUTOLINKS | // Auto-link emails
      MD_FLAG_PERMISSIVEWWWAUTOLINKS;    // Auto-link www.example.com

  unsigned rendererFlags = 0;

  // Parse markdown to HTML using md4c
  int result = md_html(utf8Data.constData(), utf8Data.size(), processOutput,
                       &buffer, parserFlags, rendererFlags);

  if (result != 0) {
    return "<p style=\"color: red;\">Error parsing markdown</p>";
  }

  return buffer.html;
}

QString MarkdownPreview::getStyleSheet(const QString &theme) {
  QString baseStyle = R"(
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Helvetica, Arial, sans-serif;
            font-size: 14px;
            line-height: 1.6;
            padding: 20px;
            max-width: 900px;
            margin: 0 auto;
        }
        h1, h2, h3, h4, h5, h6 {
            margin-top: 24px;
            margin-bottom: 16px;
            font-weight: 600;
            line-height: 1.25;
        }
        h1 { font-size: 2em; border-bottom: 1px solid; padding-bottom: 0.3em; }
        h2 { font-size: 1.5em; border-bottom: 1px solid; padding-bottom: 0.3em; }
        h3 { font-size: 1.25em; }
        h4 { font-size: 1em; }
        h5 { font-size: 0.875em; }
        h6 { font-size: 0.85em; }
        
        p { margin-top: 0; margin-bottom: 16px; }
        
        a { color: #0366d6; text-decoration: none; }
        a:hover { text-decoration: underline; }
        a.wiki-link { color: #0a8a0a; font-weight: 500; }
        
        code {
            padding: 0.2em 0.4em;
            margin: 0;
            font-size: 85%;
            border-radius: 3px;
            font-family: 'Courier New', Courier, monospace;
        }
        
        pre {
            padding: 16px;
            overflow: auto;
            font-size: 85%;
            line-height: 1.45;
            border-radius: 3px;
            font-family: 'Courier New', Courier, monospace;
        }
        
        pre code {
            padding: 0;
            margin: 0;
            font-size: 100%;
            background-color: transparent;
            border: 0;
        }
        
        blockquote {
            padding: 0 1em;
            border-left: 0.25em solid;
            margin: 0 0 16px 0;
        }
        
        ul, ol {
            padding-left: 2em;
            margin-top: 0;
            margin-bottom: 16px;
        }
        
        li { margin-bottom: 4px; }
        
        hr {
            height: 0.25em;
            padding: 0;
            margin: 24px 0;
            border: 0;
        }
        
        table {
            border-collapse: collapse;
            margin-bottom: 16px;
        }
        
        table th, table td {
            padding: 6px 13px;
            border: 1px solid;
        }
        
        img {
            max-width: 100%;
            height: auto;
        }
    )";

  QString themeStyle;

  if (theme == "dark") {
    themeStyle = R"(
            body { background-color: #1e1e1e; color: #d4d4d4; }
            h1, h2 { border-bottom-color: #4d4d4d; }
            code { background-color: #2d2d2d; color: #d4d4d4; }
            pre { background-color: #2d2d2d; }
            blockquote { border-left-color: #4d4d4d; color: #b4b4b4; }
            hr { background-color: #4d4d4d; }
            table th, table td { border-color: #4d4d4d; }
            a { color: #4da6ff; }
            a.wiki-link { color: #4ade80; }
        )";
  } else if (theme == "sepia") {
    themeStyle = R"(
            body { background-color: #f4ecd8; color: #5c4b37; }
            h1, h2 { border-bottom-color: #c9b896; }
            code { background-color: #e8dcc0; color: #5c4b37; }
            pre { background-color: #e8dcc0; }
            blockquote { border-left-color: #c9b896; color: #7d6a54; }
            hr { background-color: #c9b896; }
            table th, table td { border-color: #c9b896; }
            a { color: #0366d6; }
            a.wiki-link { color: #0a8a0a; }
        )";
  } else { // light (default)
    themeStyle = R"(
            body { background-color: #ffffff; color: #24292e; }
            h1, h2 { border-bottom-color: #eaecef; }
            code { background-color: #f6f8fa; color: #24292e; }
            pre { background-color: #f6f8fa; }
            blockquote { border-left-color: #dfe2e5; color: #6a737d; }
            hr { background-color: #e1e4e8; }
            table th, table td { border-color: #dfe2e5; }
            a { color: #0366d6; }
            a.wiki-link { color: #0a8a0a; }
        )";
  }

  return baseStyle + themeStyle;
}

QString MarkdownPreview::processLatexFormulas(const QString &html) {
  QString result = html;

  // md4c outputs LaTeX as <x-equation type="display">...</x-equation> for block
  // formulas and <x-equation>...</x-equation> for inline formulas

  // Convert display (block) equations to $$...$$
  result.replace(
      QRegularExpression("<x-equation type=\"display\">([^<]*)</x-equation>"),
      "$$\\1$$");

  // Convert inline equations to $...$
  result.replace(QRegularExpression("<x-equation>([^<]*)</x-equation>"),
                 "$\\1$");

  return result;
}

QString MarkdownPreview::processWikiLinks(const QString &html) {
  QString result = html;

  // Process inclusion links [[!target]] or [[!target|display]]
  QRegularExpression inclusionPattern("\\[\\[!([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
  QRegularExpressionMatchIterator inclusionIt =
      inclusionPattern.globalMatch(result);

  // Collect matches in reverse order to avoid position shifts
  QList<QRegularExpressionMatch> inclusionMatches;
  while (inclusionIt.hasNext()) {
    inclusionMatches.prepend(inclusionIt.next());
  }

  for (const QRegularExpressionMatch &match : inclusionMatches) {
    QString target = match.captured(1).trimmed();
    QString display = match.captured(3).trimmed();
    if (display.isEmpty()) {
      display = target;
    }

    QString includedContent = resolveAndIncludeFile(target, display);
    result.replace(match.capturedStart(), match.capturedLength(),
                   includedContent);
  }

  // md4c already handles regular wiki links [[link]] with MD_FLAG_WIKILINKS
  // but we need to convert them to use wiki: scheme and add CSS class
  // md4c outputs: <x-wikilink data-target="link">link</x-wikilink>
  result.replace(
      QRegularExpression(
          "<x-wikilink data-target=\"([^\"]+)\">([^<]+)</x-wikilink>"),
      "<a href=\"wiki:\\1\" class=\"wiki-link\">\\2</a>");

  return result;
}

QString MarkdownPreview::resolveAndIncludeFile(const QString &linkTarget,
                                               const QString &displayText) {
  // Resolve the file path relative to basePath
  QDir baseDir(basePath);
  QString cleanTarget = linkTarget.trimmed();

  // Try different file extensions
  QStringList possibleFiles;
  possibleFiles << cleanTarget + ".md" << cleanTarget + ".markdown"
                << cleanTarget;

  QString resolvedPath;
  for (const QString &fileName : possibleFiles) {
    QString fullPath = baseDir.filePath(fileName);
    if (QFileInfo::exists(fullPath)) {
      resolvedPath = fullPath;
      break;
    }
  }

  if (resolvedPath.isEmpty()) {
    // File not found, return error message
    return QString("<div class=\"inclusion-error\" style=\"border: 1px solid "
                   "#ff6b6b; "
                   "background-color: #ffe0e0; padding: 10px; margin: 10px 0; "
                   "border-radius: 5px;\">"
                   "<strong>Inclusion Error:</strong> File not found: %1"
                   "</div>")
        .arg(linkTarget.toHtmlEscaped());
  }

  // Read file content
  QFile file(resolvedPath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return QString("<div class=\"inclusion-error\" style=\"border: 1px solid "
                   "#ff6b6b; "
                   "background-color: #ffe0e0; padding: 10px; margin: 10px 0; "
                   "border-radius: 5px;\">"
                   "<strong>Inclusion Error:</strong> Cannot read file: %1"
                   "</div>")
        .arg(linkTarget.toHtmlEscaped());
  }

  QTextStream in(&file);
  in.setEncoding(QStringConverter::Utf8);
  QString content = in.readAll();
  file.close();

  // Convert the included markdown to HTML
  QString includedHtml = convertMarkdownToHtml(content);

  // Wrap the included content in a styled div with optional title
  QString wrappedContent = QString("<div class=\"included-content\" "
                                   "style=\"border-left: 3px solid #4ade80; "
                                   "padding-left: 15px; margin: 20px 0;\">"
                                   "<div class=\"inclusion-title\" "
                                   "style=\"font-weight: bold; color: #0a8a0a; "
                                   "margin-bottom: 10px;\">%1</div>"
                                   "%2"
                                   "</div>")
                               .arg(displayText.toHtmlEscaped(), includedHtml);

  return wrappedContent;
}

void MarkdownPreview::showContextMenu(const QPoint &pos) {
  QMenu contextMenu(this);

  QAction *reloadAction = contextMenu.addAction(tr("Reload"));
  connect(reloadAction, &QAction::triggered, this,
          &MarkdownPreview::reloadPreview);

  contextMenu.exec(mapToGlobal(pos));
}

void MarkdownPreview::reloadPreview() { reload(); }
