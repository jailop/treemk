#include "markdownpreview.h"

#include <md4c-html.h>

#include <QAction>
#include <QApplication>
#include <QContextMenuEvent>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QKeySequence>
#include <QMap>
#include <QMenu>
#include <QRegularExpression>
#include <QSettings>
#include <QShortcut>
#include <QStyleHints>
#include <QTextDocument>
#include <QUrl>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineSettings>

#include "defs.h"
#include "thememanager.h"

class WikiLinkPage : public QWebEnginePage {
   public:
    WikiLinkPage(MarkdownPreview* parent)
        : QWebEnginePage(parent), preview(parent) {}

   protected:
    bool acceptNavigationRequest(const QUrl& url, NavigationType type,
                                 bool isMainFrame) override {
        if (url.scheme() == "wiki") {
            emit preview->wikiLinkClicked(url.path());
            return false;
        }
        if (url.scheme() == "markdown") {
            emit preview->markdownLinkClicked(url.path());
            return false;
        }
        if (url.isLocalFile() &&
            url.toLocalFile().endsWith(".md", Qt::CaseInsensitive)) {
            emit preview->markdownLinkClicked(url.toLocalFile());
            return false;
        }
        if (url.scheme() == "http" || url.scheme() == "https") {
            QDesktopServices::openUrl(url);
            return false;
        }
        return QWebEnginePage::acceptNavigationRequest(url, type, isMainFrame);
    }

   private:
    MarkdownPreview* preview;
};

MarkdownPreview::MarkdownPreview(QWidget* parent)
    : QWebEngineView(parent),
      currentTheme("light"),
      basePath(QDir::homePath()),
      latexEnabled(true),
      lastScrollPercentage(0.0),
      lastMarkdownContent("") {
    setContextMenuPolicy(Qt::CustomContextMenu);
    // Set custom page to intercept link clicks
    WikiLinkPage* wikiPage = new WikiLinkPage(this);
    setPage(wikiPage);
    // Allow loading remote content (KaTeX CDN) from local HTML
    page()->settings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    // Connect context menu
    connect(this, &QWidget::customContextMenuRequested, this,
            &MarkdownPreview::showContextMenu);
    // Add F5 shortcut for reload
    QShortcut* reloadShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(reloadShortcut, &QShortcut::activated, this,
            &MarkdownPreview::reloadPreview);
    // Connect to theme changes
    if (ThemeManager::instance()) {
        connect(ThemeManager::instance(),
                &ThemeManager::previewColorSchemeChanged, this,
                &MarkdownPreview::onThemeChanged);
        connect(ThemeManager::instance(), &ThemeManager::themeChanged, this,
                &MarkdownPreview::onThemeChanged);
    }
}

MarkdownPreview::~MarkdownPreview() {}

void MarkdownPreview::setBasePath(const QString& path) { basePath = path; }

void MarkdownPreview::setLatexEnabled(bool enabled) { latexEnabled = enabled; }

void MarkdownPreview::setMarkdownContent(const QString& markdown) {
    lastMarkdownContent = markdown;
    QString html = convertMarkdownToHtml(markdown);
    html = processWikiLinks(html);
    if (latexEnabled) {
        html = processLatexFormulas(html);
    }
    // Get the appropriate stylesheet with max-width for images
    QString baseStyleSheet = ThemeManager::instance()->getPreviewStyleSheet();
    QString imageStyleSheet = "img { max-width: 100%; height: auto; }";
    QString taskStyles;
    QFile taskFile(":/css/task-list.css");
    if (taskFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        taskStyles = QString::fromUtf8(taskFile.readAll());
        taskFile.close();
    }
    QString previewStyleSheet =
        baseStyleSheet + "\n" + imageStyleSheet + "\n" + taskStyles;
    // Choose highlight.js theme based on current preview scheme
    QString highlightTheme = "highlight-github.min.css";
    // Determine if we're in dark mode
    QSettings appSettings(APP_LABEL, APP_LABEL);
    QString previewScheme =
        appSettings.value("appearance/previewColorScheme", "auto").toString();
    bool isDark = false;
    if (previewScheme == "auto") {
        QString appTheme =
            appSettings.value("appearance/appTheme", "system").toString();
        if (appTheme == "dark") {
            isDark = true;
        }
        /*
        else if (appTheme == "system") {
          QStyleHints *hints = QApplication::styleHints();
          isDark = (hints->colorScheme() == Qt::ColorScheme::Dark);
        }
        */
    } else if (previewScheme == "dark") {
        isDark = true;
    }

    // Set dark theme if needed
    if (isDark) {
        highlightTheme = "highlight-github-dark.min.css";
    }
    // Add custom CSS
    QString customCSS = appSettings.value("preview/customCSS", "").toString();
    previewStyleSheet += "\n" + customCSS;
    // Load HTML template from resources
    QFile templateFile(":/templates/preview-template.html");
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load preview template";
        return;
    }
    QString fullHtml = QString::fromUtf8(templateFile.readAll());
    templateFile.close();
    // Replace placeholders in template
    QString mermaidTheme = isDark ? "dark" : "default";
    fullHtml.replace("HIGHLIGHT_THEME", highlightTheme);
    fullHtml.replace("MERMAID_THEME", mermaidTheme);
    fullHtml.replace("CUSTOM_STYLESHEET", previewStyleSheet);
    fullHtml.replace("SCROLL_PERCENTAGE",
                     QString::number(lastScrollPercentage));
    fullHtml.replace("MARKDOWN_CONTENT", html);
    // Use setHtml with baseUrl to allow loading local images
    QUrl baseUrl;
    if (basePath.startsWith("qrc:") || basePath.startsWith(":/")) {
        // For Qt resources, use QUrl directly
        QString qrcPath = basePath;
        if (qrcPath.startsWith(":/")) {
            qrcPath = "qrc" + qrcPath;
        }
        if (!qrcPath.endsWith("/")) {
            qrcPath += "/";
        }
        baseUrl = QUrl(qrcPath);
    } else {
        // For local files, use fromLocalFile
        baseUrl = QUrl::fromLocalFile(basePath + "/");
    }
    setHtml(fullHtml, baseUrl);
}
void MarkdownPreview::setTheme(const QString& theme) { currentTheme = theme; }
void MarkdownPreview::scrollToPercentage(double percentage) {
    lastScrollPercentage = percentage;
    // Use JavaScript to scroll the preview to a specific percentage
    QString script =
        QString("window.scrollTo(0, document.body.scrollHeight * %1);")
            .arg(percentage);
    page()->runJavaScript(script);
}

double MarkdownPreview::currentScrollPercentage() const {
    return lastScrollPercentage;
}

QString MarkdownPreview::convertMarkdownToHtml(const QString& markdown) {
    // Callback function for md4c to accumulate HTML output
    struct OutputBuffer {
        QString html;
    };
    auto processOutput = [](const MD_CHAR* data, MD_SIZE size, void* userdata) {
        OutputBuffer* buffer = static_cast<OutputBuffer*>(userdata);
        buffer->html.append(QString::fromUtf8(data, size));
    };
    OutputBuffer buffer;
    QByteArray utf8Data = markdown.toUtf8();
    // Configure parser flags for CommonMark + extensions
    unsigned parserFlags =
        MD_FLAG_TABLES |                    // Enable tables
        MD_FLAG_STRIKETHROUGH |             // Enable ~~strikethrough~~
        MD_FLAG_TASKLISTS |                 // Enable task lists [ ] [x]
        MD_FLAG_LATEXMATHSPANS |            // Enable $math$ and $$math$$
        MD_FLAG_WIKILINKS |                 // Enable [[wiki links]]
        MD_FLAG_PERMISSIVEURLAUTOLINKS |    // Auto-link URLs
        MD_FLAG_PERMISSIVEEMAILAUTOLINKS |  // Auto-link emails
        MD_FLAG_PERMISSIVEWWWAUTOLINKS;     // Auto-link www.example.com
    unsigned rendererFlags = 0;
    // Parse markdown to HTML using md4c
    int result = md_html(utf8Data.constData(), utf8Data.size(), processOutput,
                         &buffer, parserFlags, rendererFlags);
    if (result != 0) {
        return "<p style=\"color: red;\">Error parsing markdown</p>";
    }
    return buffer.html;
}

QString MarkdownPreview::getStyleSheet(const QString& theme) {
    // Load base stylesheet
    QFile baseFile(":/css/preview-base.css");
    if (!baseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load base CSS";
        return "";
    }
    QString baseStyle = QString::fromUtf8(baseFile.readAll());
    baseFile.close();
    // Load theme-specific stylesheet
    QString themeFileName;
    if (theme == "dark") {
        themeFileName = ":/css/preview-dark.css";
    } else if (theme == "sepia") {
        themeFileName = ":/css/preview-sepia.css";
    } else {  // light (default)
        themeFileName = ":/css/preview-light.css";
    }
    QFile themeFile(themeFileName);
    if (!themeFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load theme CSS:" << themeFileName;
        return baseStyle;
    }
    QString themeStyle = QString::fromUtf8(themeFile.readAll());
    themeFile.close();
    return baseStyle + "\n" + themeStyle;
}

QString MarkdownPreview::processLatexFormulas(const QString& html) {
    QString result = html;
    result.replace(
        QRegularExpression("<x-equation type=\"display\">([^<]*)</x-equation>"),
        "$$\\1$$");
    result.replace(QRegularExpression("<x-equation>([^<]*)</x-equation>"),
                   "$\\1$");
    return result;
}

QString MarkdownPreview::processWikiLinks(const QString& html) {
    QString result = html;
    
    // First, process inclusion links inside code blocks [[!filename]]
    // These are preserved as literal text by md4c inside <code> tags
    // We need to find and replace just the [[!filename]] pattern within code blocks
    QRegularExpression codeBlockPattern(
        "<code([^>]*)>([^<]*)</code>", 
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator codeIt = codeBlockPattern.globalMatch(result);
    QList<QRegularExpressionMatch> codeMatches;
    while (codeIt.hasNext()) {
        codeMatches.prepend(codeIt.next());
    }
    
    for (const QRegularExpressionMatch& match : codeMatches) {
        QString codeAttrs = match.captured(1);
        QString codeContent = match.captured(2);
        
        // Check if this code block contains [[!filename]]
        QRegularExpression inclusionPattern("\\[\\[!([^\\]]+)\\]\\]");
        QRegularExpressionMatch inclMatch = inclusionPattern.match(codeContent);
        if (!inclMatch.hasMatch()) {
            continue;  // No inclusion in this code block, skip it
        }
        
        // Process all inclusions in this code block by repeatedly replacing
        QString processedContent = codeContent;
        while (true) {
            QRegularExpressionMatch inclMatch = inclusionPattern.match(processedContent);
            if (!inclMatch.hasMatch()) {
                break;  // No more inclusions
            }
            
            QString target = inclMatch.captured(1).trimmed();
            
            // Read the file content
            QString errorMsg;
            QString fileContent = readFileContent(target, errorMsg);
            
            if (!errorMsg.isEmpty()) {
                // Show error in code block
                fileContent = QString("Error: %1").arg(errorMsg);
            }
            
            // HTML escape the file content for display in code block
            fileContent = fileContent.toHtmlEscaped();
            
            // Replace this specific [[!filename]] with the content
            processedContent.replace(inclMatch.capturedStart(), 
                                    inclMatch.capturedLength(), 
                                    fileContent);
        }
        
        // Replace the entire code block with the processed version
        QString replacement = QString("<code%1>%2</code>").arg(codeAttrs, processedContent);
        result.replace(match.capturedStart(), match.capturedLength(), replacement);
    }
    
    // md4c converts [[!target]] or [[!target|display]] to 
    // <x-wikilink data-target="!target">display</x-wikilink>
    // Process inclusion links first (before converting regular wiki links to <a> tags)
    QRegularExpression inclusionPattern(
        "<x-wikilink data-target=\"!([^\"]+)\">([^<]+)</x-wikilink>");
    QRegularExpressionMatchIterator inclusionIt =
        inclusionPattern.globalMatch(result);
    // Collect matches in reverse order to avoid position shifts
    QList<QRegularExpressionMatch> inclusionMatches;
    while (inclusionIt.hasNext()) {
        inclusionMatches.prepend(inclusionIt.next());
    }
    for (const QRegularExpressionMatch& match : inclusionMatches) {
        QString target = match.captured(1).trimmed();
        QString display = match.captured(2).trimmed();
        // md4c already extracted the display text correctly from [[!target|display]]
        // If no pipe was used, display will be "!target", so we need to clean it
        if (display.startsWith("!")) {
            display = target;  // Use target as display if no custom display was provided
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

    // Process standard markdown links [text](url)
    // md4c outputs: <a href="url">text</a>
    QRegularExpression linkPattern("<a href=\"([^\"]*)\">([^<]*)</a>");
    QRegularExpressionMatchIterator linkIt = linkPattern.globalMatch(result);
    // Collect matches in reverse order to avoid position shifts
    QList<QRegularExpressionMatch> linkMatches;
    while (linkIt.hasNext()) {
        linkMatches.prepend(linkIt.next());
    }
    for (const QRegularExpressionMatch& match : linkMatches) {
        QString href = match.captured(1);
        QString text = match.captured(2);
        QString replacement;
        if (href.startsWith("http://") || href.startsWith("https://")) {
            replacement = QString("<a href=\"%1\">%2</a>").arg(href, text);
        } else {
            replacement =
                QString(
                    "<a href=\"markdown:%1\" class=\"markdown-link\">%2</a>")
                    .arg(href, text);
        }
        result.replace(match.capturedStart(), match.capturedLength(),
                       replacement);
    }
    return result;
}

QString MarkdownPreview::resolveAndIncludeFile(const QString& linkTarget,
                                               const QString& displayText) {
    // Resolve the file path relative to basePath
    QDir baseDir(basePath);
    QString cleanTarget = linkTarget.trimmed();
    // Try different file extensions
    QStringList possibleFiles;
    possibleFiles << cleanTarget + ".md" << cleanTarget + ".markdown"
                  << cleanTarget;
    QString resolvedPath;
    for (const QString& fileName : possibleFiles) {
        QString fullPath = baseDir.filePath(fileName);
        if (QFileInfo::exists(fullPath)) {
            resolvedPath = fullPath;
            break;
        }
    }
    if (resolvedPath.isEmpty()) {
        // File not found, return error message
        return QString(
                   "<div class=\"inclusion-error\" style=\"border: 1px solid "
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
        return QString(
                   "<div class=\"inclusion-error\" style=\"border: 1px solid "
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
    QString wrappedContent =
        QString(
            "<div class=\"included-content\" "
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

QString MarkdownPreview::readFileContent(const QString& linkTarget,
                                         QString& errorMsg) {
    // Resolve the file path relative to basePath
    QDir baseDir(basePath);
    QString cleanTarget = linkTarget.trimmed();
    // Try different file extensions
    QStringList possibleFiles;
    possibleFiles << cleanTarget << cleanTarget + ".md"
                  << cleanTarget + ".markdown" << cleanTarget + ".txt"
                  << cleanTarget + ".py" << cleanTarget + ".cpp"
                  << cleanTarget + ".java" << cleanTarget + ".js"
                  << cleanTarget + ".rs" << cleanTarget + ".go";
    QString resolvedPath;
    for (const QString& fileName : possibleFiles) {
        QString fullPath = baseDir.filePath(fileName);
        if (QFileInfo::exists(fullPath)) {
            resolvedPath = fullPath;
            break;
        }
    }
    if (resolvedPath.isEmpty()) {
        errorMsg = QString("File not found: %1").arg(linkTarget);
        return QString();
    }
    // Read file content
    QFile file(resolvedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        errorMsg = QString("Cannot read file: %1").arg(linkTarget);
        return QString();
    }
    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString content = in.readAll();
    file.close();
    errorMsg.clear();
    return content;
}

void MarkdownPreview::showContextMenu(const QPoint& pos) {
    // Use JavaScript to check if cursor is on a link
    QString jsCode = QString(
                         "(function() {"
                         "  var elem = document.elementFromPoint(%1, %2);"
                         "  if (elem && elem.tagName === 'A') {"
                         "    return { isLink: true, href: "
                         "elem.getAttribute('href'), text: elem.textContent };"
                         "  }"
                         "  return { isLink: false };"
                         "})();")
                         .arg(pos.x())
                         .arg(pos.y());

    page()->runJavaScript(jsCode, [this, pos](const QVariant& result) {
        QMenu contextMenu(this);

        QVariantMap linkInfo = result.toMap();
        bool isLink = linkInfo.value("isLink").toBool();
        QString href = linkInfo.value("href").toString();

        if (isLink && !href.isEmpty()) {
            // Add "Open Link in New Window" for links
            QAction* openNewWindowAction =
                contextMenu.addAction(tr("Open Link in New Window"));
            connect(openNewWindowAction, &QAction::triggered, this,
                    [this, href]() {
                        // Emit signal with the link target
                        emit openLinkInNewWindowRequested(href);
                    });
            contextMenu.addSeparator();
        }

        // Add copy action using the web page's standard action
        QAction* copyAction = page()->action(QWebEnginePage::Copy);
        if (copyAction) {
            contextMenu.addAction(copyAction);
            contextMenu.addSeparator();
        }

        QAction* reloadAction = contextMenu.addAction(tr("Reload"));
        connect(reloadAction, &QAction::triggered, this,
                &MarkdownPreview::reloadPreview);

        contextMenu.exec(mapToGlobal(pos));
    });
}

void MarkdownPreview::reloadPreview() { reload(); }

void MarkdownPreview::onThemeChanged() {
    // Trigger a re-render with the new theme
    if (!lastMarkdownContent.isEmpty()) {
        setMarkdownContent(lastMarkdownContent);
    }
}
