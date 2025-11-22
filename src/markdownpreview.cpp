#include "markdownpreview.h"
#include <QRegularExpression>
#include <QTextDocument>
#include <QDir>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QUrl>

// Custom page class to intercept link navigation
class WikiLinkPage : public QWebEnginePage {
public:
    WikiLinkPage(MarkdownPreview *parent) : QWebEnginePage(parent), preview(parent) {}
    
protected:
    bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override {
        if (url.scheme() == "wiki") {
            emit preview->wikiLinkClicked(url.path());
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }
    
private:
    MarkdownPreview *preview;
};

MarkdownPreview::MarkdownPreview(QWidget *parent)
    : QWebEngineView(parent), currentTheme("light"), basePath(QDir::homePath()), latexEnabled(true)
{
    setContextMenuPolicy(Qt::NoContextMenu);
    
    // Set custom page to intercept link clicks
    setPage(new WikiLinkPage(this));
}

MarkdownPreview::~MarkdownPreview()
{
}

void MarkdownPreview::setBasePath(const QString &path)
{
    basePath = path;
}

void MarkdownPreview::setLatexEnabled(bool enabled)
{
    latexEnabled = enabled;
}

void MarkdownPreview::setMarkdownContent(const QString &markdown)
{
    QString html = convertMarkdownToHtml(markdown);
    
    // Process wiki links (including inclusions)
    html = processWikiLinks(html);
    
    if (latexEnabled) {
        html = processLatexFormulas(html);
    }
    
    QString styleSheet = getStyleSheet(currentTheme);
    
    QString fullHtml = QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<base href=\"file://%1/\">"
        "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css\">"
        "<script src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js\"></script>"
        "<script src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js\"></script>"
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
        "});"
        "</script>"
        "</head>"
        "<body>%3</body>"
        "</html>"
    ).arg(basePath, styleSheet, html);
    
    setHtml(fullHtml);
}

void MarkdownPreview::setTheme(const QString &theme)
{
    currentTheme = theme;
}

QString MarkdownPreview::convertMarkdownToHtml(const QString &markdown)
{
    QString html = markdown;
    
    // Escape HTML special characters first
    html.replace("&", "&amp;");
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");
    
    QStringList lines = html.split('\n');
    QStringList outputLines;
    bool inCodeBlock = false;
    QString codeBlockContent;
    
    for (const QString &line : lines) {
        // Code blocks
        if (line.startsWith("```")) {
            if (inCodeBlock) {
                outputLines.append("<pre><code>" + codeBlockContent + "</code></pre>");
                codeBlockContent.clear();
                inCodeBlock = false;
            } else {
                inCodeBlock = true;
            }
            continue;
        }
        
        if (inCodeBlock) {
            codeBlockContent += line + "\n";
            continue;
        }
        
        QString processedLine = line;
        
        // Headers
        if (processedLine.startsWith("# ")) {
            processedLine = "<h1>" + processedLine.mid(2) + "</h1>";
        } else if (processedLine.startsWith("## ")) {
            processedLine = "<h2>" + processedLine.mid(3) + "</h2>";
        } else if (processedLine.startsWith("### ")) {
            processedLine = "<h3>" + processedLine.mid(4) + "</h3>";
        } else if (processedLine.startsWith("#### ")) {
            processedLine = "<h4>" + processedLine.mid(5) + "</h4>";
        } else if (processedLine.startsWith("##### ")) {
            processedLine = "<h5>" + processedLine.mid(6) + "</h5>";
        } else if (processedLine.startsWith("###### ")) {
            processedLine = "<h6>" + processedLine.mid(7) + "</h6>";
        }
        // Blockquotes
        else if (processedLine.startsWith("> ")) {
            processedLine = "<blockquote>" + processedLine.mid(2) + "</blockquote>";
        }
        // Horizontal rules
        else if (processedLine.trimmed().contains(QRegularExpression("^([-*_]\\s*){3,}$"))) {
            processedLine = "<hr>";
        }
        // Unordered lists
        else if (processedLine.trimmed().startsWith("- ") || 
                 processedLine.trimmed().startsWith("* ") ||
                 processedLine.trimmed().startsWith("+ ")) {
            int indent = processedLine.indexOf(QRegularExpression("[-*+]"));
            QString content = processedLine.mid(indent + 2);
            processedLine = "<li>" + content + "</li>";
        }
        // Ordered lists
        else if (processedLine.trimmed().contains(QRegularExpression("^\\d+\\.\\s"))) {
            QRegularExpression re("^(\\s*)(\\d+)\\.\\s(.*)$");
            QRegularExpressionMatch match = re.match(processedLine);
            if (match.hasMatch()) {
                QString content = match.captured(3);
                processedLine = "<li>" + content + "</li>";
            }
        }
        
        // Inline formatting (apply to processed line)
        // Bold **text** or __text__
        processedLine.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "<strong>\\1</strong>");
        processedLine.replace(QRegularExpression("__(.+?)__"), "<strong>\\1</strong>");
        
        // Italic *text* or _text_
        processedLine.replace(QRegularExpression("(?<!\\*)\\*(?!\\*)(.+?)(?<!\\*)\\*(?!\\*)"), "<em>\\1</em>");
        processedLine.replace(QRegularExpression("(?<!_)_(?!_)(.+?)(?<!_)_(?!_)"), "<em>\\1</em>");
        
        // Inline code `code`
        processedLine.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
        
        // Links [text](url)
        processedLine.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^\\)]+)\\)"), 
                             "<a href=\"\\2\">\\1</a>");
        
        // Images ![alt](url) - must be before links
        processedLine.replace(QRegularExpression("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)"),
                             "<img src=\"\\2\" alt=\"\\1\" style=\"max-width: 100%; height: auto;\" />");
        
        // Note: Wiki links are now processed separately in processWikiLinks()
        // to handle inclusions properly
        
        // URLs
        processedLine.replace(QRegularExpression("(https?://[^\\s]+)"), 
                             "<a href=\"\\1\">\\1</a>");
        
        // Paragraph wrapping for non-empty, non-header, non-list lines
        if (!processedLine.isEmpty() && 
            !processedLine.startsWith("<h") && 
            !processedLine.startsWith("<li") &&
            !processedLine.startsWith("<blockquote") &&
            !processedLine.startsWith("<hr>")) {
            processedLine = "<p>" + processedLine + "</p>";
        }
        
        outputLines.append(processedLine);
    }
    
    return outputLines.join("\n");
}

QString MarkdownPreview::getStyleSheet(const QString &theme)
{
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

QString MarkdownPreview::processLatexFormulas(const QString &html)
{
    // KaTeX will handle the rendering via auto-render.js
    // We just need to make sure the delimiters are preserved in the HTML
    // The actual rendering happens in the JavaScript
    return html;
}

QString MarkdownPreview::processWikiLinks(const QString &html)
{
    QString result = html;
    
    // Process inclusion links first [[!target]] or [[!target|display]]
    QRegularExpression inclusionPattern("\\[\\[!([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator inclusionIt = inclusionPattern.globalMatch(result);
    
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
        result.replace(match.capturedStart(), match.capturedLength(), includedContent);
    }
    
    // Process regular wiki links [[link]] or [[link|text]]
    // Use wiki: scheme to handle clicks
    result.replace(QRegularExpression("\\[\\[([^\\]|!]+)\\]\\]"), 
                   "<a href=\"wiki:\\1\" class=\"wiki-link\">\\1</a>");
    
    result.replace(QRegularExpression("\\[\\[([^\\]|!]+)\\|([^\\]]+)\\]\\]"), 
                   "<a href=\"wiki:\\1\" class=\"wiki-link\">\\2</a>");
    
    return result;
}

QString MarkdownPreview::resolveAndIncludeFile(const QString &linkTarget, const QString &displayText)
{
    // Resolve the file path relative to basePath
    QDir baseDir(basePath);
    QString cleanTarget = linkTarget.trimmed();
    
    // Try different file extensions
    QStringList possibleFiles;
    possibleFiles << cleanTarget + ".md"
                  << cleanTarget + ".markdown"
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
        return QString("<div class=\"inclusion-error\" style=\"border: 1px solid #ff6b6b; "
                      "background-color: #ffe0e0; padding: 10px; margin: 10px 0; border-radius: 5px;\">"
                      "<strong>Inclusion Error:</strong> File not found: %1"
                      "</div>").arg(linkTarget.toHtmlEscaped());
    }
    
    // Read file content
    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return QString("<div class=\"inclusion-error\" style=\"border: 1px solid #ff6b6b; "
                      "background-color: #ffe0e0; padding: 10px; margin: 10px 0; border-radius: 5px;\">"
                      "<strong>Inclusion Error:</strong> Cannot read file: %1"
                      "</div>").arg(linkTarget.toHtmlEscaped());
    }
    
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    
    // Convert the included markdown to HTML
    QString includedHtml = convertMarkdownToHtml(content);
    
    // Wrap the included content in a styled div with optional title
    QString wrappedContent = QString(
        "<div class=\"included-content\" style=\"border-left: 3px solid #4ade80; "
        "padding-left: 15px; margin: 20px 0;\">"
        "<div class=\"inclusion-title\" style=\"font-weight: bold; color: #0a8a0a; "
        "margin-bottom: 10px;\">%1</div>"
        "%2"
        "</div>"
    ).arg(displayText.toHtmlEscaped(), includedHtml);
    
    return wrappedContent;
}
