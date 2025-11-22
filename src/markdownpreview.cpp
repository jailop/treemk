#include "markdownpreview.h"
#include <QRegularExpression>
#include <QTextDocument>
#include <QDir>
#include <QWebEngineProfile>
#include <QWebEnginePage>
#include <QWebEngineSettings>
#include <QUrl>
#include <QMap>

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
    WikiLinkPage *wikiPage = new WikiLinkPage(this);
    setPage(wikiPage);
    
    // Allow loading remote content (KaTeX CDN) from local HTML
    page()->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
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
    
    // Choose highlight.js theme based on current theme
    QString highlightTheme = "github.min.css";
    if (currentTheme == "dark") {
        highlightTheme = "github-dark.min.css";
    } else if (currentTheme == "sepia") {
        highlightTheme = "github.min.css"; // Use light theme for sepia
    }
    
    QString fullHtml = QString(
        "<!DOCTYPE html>"
        "<html>"
        "<head>"
        "<meta charset=\"UTF-8\">"
        "<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.css\">"
        "<link rel=\"stylesheet\" href=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/styles/%1\">"
        "<script src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/katex.min.js\"></script>"
        "<script src=\"https://cdn.jsdelivr.net/npm/katex@0.16.9/dist/contrib/auto-render.min.js\"></script>"
        "<script src=\"https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.9.0/highlight.min.js\"></script>"
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
        "</html>"
    ).arg(highlightTheme, styleSheet, html);
    
    // Use setHtml with baseUrl to allow loading local images
    QUrl baseUrl = QUrl::fromLocalFile(basePath + "/");
    setHtml(fullHtml, baseUrl);
}

void MarkdownPreview::setTheme(const QString &theme)
{
    currentTheme = theme;
}

void MarkdownPreview::scrollToPercentage(double percentage)
{
    // Use JavaScript to scroll the preview to a specific percentage
    QString script = QString("window.scrollTo(0, document.body.scrollHeight * %1);").arg(percentage);
    page()->runJavaScript(script);
}

QString MarkdownPreview::convertMarkdownToHtml(const QString &markdown)
{
    QString html = markdown;
    
    // Protect LaTeX formulas from HTML escaping and markdown processing
    QMap<QString, QString> latexFormulas;
    int formulaIndex = 0;
    
    // Extract block formulas ($$...$$) - must be before inline
    QRegularExpression blockLatex("\\$\\$(.+?)\\$\\$");
    blockLatex.setPatternOptions(QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator blockIt = blockLatex.globalMatch(html);
    QList<QRegularExpressionMatch> blockMatches;
    while (blockIt.hasNext()) {
        blockMatches.append(blockIt.next());
    }
    // Process in reverse to maintain positions
    for (int i = blockMatches.size() - 1; i >= 0; --i) {
        const QRegularExpressionMatch &match = blockMatches[i];
        // Use a marker that survives HTML escaping
        QString placeholder = QString("LATEXBLOCK%1PLACEHOLDER").arg(formulaIndex);
        latexFormulas[placeholder] = match.captured(0);
        html.replace(match.capturedStart(), match.capturedLength(), placeholder);
        formulaIndex++;
    }
    
    // Extract inline formulas ($...$)
    QRegularExpression inlineLatex("\\$([^$\n]+)\\$");
    QRegularExpressionMatchIterator inlineIt = inlineLatex.globalMatch(html);
    QList<QRegularExpressionMatch> inlineMatches;
    while (inlineIt.hasNext()) {
        inlineMatches.append(inlineIt.next());
    }
    // Process in reverse to maintain positions
    for (int i = inlineMatches.size() - 1; i >= 0; --i) {
        const QRegularExpressionMatch &match = inlineMatches[i];
        QString placeholder = QString("LATEXINLINE%1PLACEHOLDER").arg(formulaIndex);
        latexFormulas[placeholder] = match.captured(0);
        html.replace(match.capturedStart(), match.capturedLength(), placeholder);
        formulaIndex++;
    }
    
    // Escape HTML special characters
    html.replace("&", "&amp;");
    html.replace("<", "&lt;");
    html.replace(">", "&gt;");
    
    QStringList lines = html.split('\n');
    QStringList outputLines;
    bool inCodeBlock = false;
    QString codeBlockContent;
    QString codeLanguage;
    
    for (const QString &line : lines) {
        // Code blocks
        if (line.startsWith("```")) {
            if (inCodeBlock) {
                // Closing fence - add the code block with language class
                QString langClass = codeLanguage.isEmpty() ? "" : QString(" class=\"language-%1\"").arg(codeLanguage);
                outputLines.append("<pre><code" + langClass + ">" + codeBlockContent + "</code></pre>");
                codeBlockContent.clear();
                codeLanguage.clear();
                inCodeBlock = false;
            } else {
                // Opening fence - extract language if present
                inCodeBlock = true;
                if (line.length() > 3) {
                    codeLanguage = line.mid(3).trimmed();
                }
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
        
        // Images ![alt](url) - must be before links
        processedLine.replace(QRegularExpression("!\\[([^\\]]*)\\]\\(([^\\)]+)\\)"),
                             "<img src=\"\\2\" alt=\"\\1\" style=\"max-width: 100%; height: auto;\" />");
        
        // Links [text](url) - detect and render media files
        QRegularExpression linkPattern("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
        QRegularExpressionMatchIterator linkIt = linkPattern.globalMatch(processedLine);
        QList<QPair<int, int>> replacements;
        QStringList replacementTexts;
        
        while (linkIt.hasNext()) {
            QRegularExpressionMatch linkMatch = linkIt.next();
            QString linkText = linkMatch.captured(1);
            QString linkUrl = linkMatch.captured(2);
            QString lowerUrl = linkUrl.toLower();
            QString replacement;
            
            // Check for audio files
            if (lowerUrl.endsWith(".mp3") || lowerUrl.endsWith(".wav") || 
                lowerUrl.endsWith(".ogg") || lowerUrl.endsWith(".m4a") ||
                lowerUrl.endsWith(".flac") || lowerUrl.endsWith(".aac")) {
                replacement = QString("<div style=\"margin: 10px 0;\">"
                                    "<p>%1</p>"
                                    "<audio controls style=\"width: 100%; max-width: 600px;\">"
                                    "<source src=\"%2\">"
                                    "Your browser does not support the audio element."
                                    "</audio></div>").arg(linkText.toHtmlEscaped(), linkUrl);
            }
            // Check for video files
            else if (lowerUrl.endsWith(".mp4") || lowerUrl.endsWith(".webm") || 
                     lowerUrl.endsWith(".ogv") || lowerUrl.endsWith(".avi") ||
                     lowerUrl.endsWith(".mov") || lowerUrl.endsWith(".mkv")) {
                replacement = QString("<div style=\"margin: 10px 0;\">"
                                    "<p>%1</p>"
                                    "<video controls style=\"width: 100%; max-width: 800px;\">"
                                    "<source src=\"%2\">"
                                    "Your browser does not support the video element."
                                    "</video></div>").arg(linkText.toHtmlEscaped(), linkUrl);
            }
            // Regular link
            else {
                replacement = QString("<a href=\"%2\">%1</a>").arg(linkText, linkUrl);
            }
            
            replacements.append(qMakePair(linkMatch.capturedStart(), linkMatch.capturedLength()));
            replacementTexts.append(replacement);
        }
        
        // Apply replacements in reverse order to maintain positions
        for (int i = replacements.size() - 1; i >= 0; --i) {
            processedLine.replace(replacements[i].first, replacements[i].second, replacementTexts[i]);
        }
        
        // Note: Wiki links are now processed separately in processWikiLinks()
        // to handle inclusions properly
        
        // URLs - detect and render media URLs
        QRegularExpression urlPattern("(https?://[^\\s]+)");
        QRegularExpressionMatchIterator urlIt = urlPattern.globalMatch(processedLine);
        QList<QPair<int, int>> urlReplacements;
        QStringList urlReplacementTexts;
        
        while (urlIt.hasNext()) {
            QRegularExpressionMatch urlMatch = urlIt.next();
            QString url = urlMatch.captured(1);
            QString lowerUrl = url.toLower();
            QString replacement;
            
            // Check for audio URLs
            if (lowerUrl.endsWith(".mp3") || lowerUrl.endsWith(".wav") || 
                lowerUrl.endsWith(".ogg") || lowerUrl.endsWith(".m4a") ||
                lowerUrl.endsWith(".flac") || lowerUrl.endsWith(".aac")) {
                replacement = QString("<div style=\"margin: 10px 0;\">"
                                    "<p><a href=\"%1\">%1</a></p>"
                                    "<audio controls style=\"width: 100%; max-width: 600px;\">"
                                    "<source src=\"%1\">"
                                    "Your browser does not support the audio element."
                                    "</audio></div>").arg(url);
            }
            // Check for video URLs
            else if (lowerUrl.endsWith(".mp4") || lowerUrl.endsWith(".webm") || 
                     lowerUrl.endsWith(".ogv") || lowerUrl.endsWith(".avi") ||
                     lowerUrl.endsWith(".mov") || lowerUrl.endsWith(".mkv")) {
                replacement = QString("<div style=\"margin: 10px 0;\">"
                                    "<p><a href=\"%1\">%1</a></p>"
                                    "<video controls style=\"width: 100%; max-width: 800px;\">"
                                    "<source src=\"%1\">"
                                    "Your browser does not support the video element."
                                    "</video></div>").arg(url);
            }
            // Regular URL link
            else {
                replacement = QString("<a href=\"%1\">%1</a>").arg(url);
            }
            
            urlReplacements.append(qMakePair(urlMatch.capturedStart(), urlMatch.capturedLength()));
            urlReplacementTexts.append(replacement);
        }
        
        // Apply URL replacements in reverse order to maintain positions
        for (int i = urlReplacements.size() - 1; i >= 0; --i) {
            processedLine.replace(urlReplacements[i].first, urlReplacements[i].second, urlReplacementTexts[i]);
        }
        
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
    
    QString result = outputLines.join("\n");
    
    // Restore LaTeX formulas
    for (auto it = latexFormulas.constBegin(); it != latexFormulas.constEnd(); ++it) {
        result.replace(it.key(), it.value());
    }
    
    return result;
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
