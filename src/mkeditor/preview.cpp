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
#include "regexpatterns.h"
#include "regexutils.h"
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
            if (url.path().isEmpty() && url.hasFragment()) {
                emit preview->internalLinkClicked(url.fragment());
                return false;
            }
            if (url.hasFragment()) {
                QString linkTarget = url.path() + "#" + url.fragment();
                emit preview->markdownLinkClicked(linkTarget);
                return false;
            }
            emit preview->markdownLinkClicked(url.path());
            return false;
        }
        if (url.isLocalFile() &&
            url.toLocalFile().endsWith(".md", Qt::CaseInsensitive)) {
            emit preview->markdownLinkClicked(url.toLocalFile());
            return false;
        }
        if (url.hasFragment()) {
            QString fragment = url.fragment();
            QString path = url.path();
            
            if (path.isEmpty() || path == "/" || url.scheme().isEmpty()) {
                emit preview->internalLinkClicked(fragment);
                return false;
            }
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
    WikiLinkPage* wikiPage = new WikiLinkPage(this);
    setPage(wikiPage);
    page()->settings()->setAttribute(
        QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);
    connect(this, &QWidget::customContextMenuRequested, this,
            &MarkdownPreview::showContextMenu);
    QShortcut* reloadShortcut = new QShortcut(QKeySequence(Qt::Key_F5), this);
    connect(reloadShortcut, &QShortcut::activated, this,
            &MarkdownPreview::reloadPreview);
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
    html = addHeadingIds(html);
    if (latexEnabled) {
        html = processLatexFormulas(html);
    }
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
    QString highlightTheme = "highlight-github.min.css";
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

    if (isDark) {
        highlightTheme = "highlight-github-dark.min.css";
    }
    QString customCSS = appSettings.value("preview/customCSS", "").toString();
    previewStyleSheet += "\n" + customCSS;
    QFile templateFile(":/templates/preview-template.html");
    if (!templateFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load preview template";
        return;
    }
    QString fullHtml = QString::fromUtf8(templateFile.readAll());
    templateFile.close();
    QString mermaidTheme = isDark ? "dark" : "default";
    fullHtml.replace("HIGHLIGHT_THEME", highlightTheme);
    fullHtml.replace("MERMAID_THEME", mermaidTheme);
    fullHtml.replace("CUSTOM_STYLESHEET", previewStyleSheet);
    fullHtml.replace("SCROLL_PERCENTAGE",
                     QString::number(lastScrollPercentage));
    fullHtml.replace("MARKDOWN_CONTENT", html);
    QUrl baseUrl;
    if (basePath.startsWith("qrc:") || basePath.startsWith(":/")) {
        QString qrcPath = basePath;
        if (qrcPath.startsWith(":/")) {
            qrcPath = "qrc" + qrcPath;
        }
        if (!qrcPath.endsWith("/")) {
            qrcPath += "/";
        }
        baseUrl = QUrl(qrcPath);
    } else {
        baseUrl = QUrl::fromLocalFile(basePath + "/");
    }
    setHtml(fullHtml, baseUrl);
}

void MarkdownPreview::scrollToAnchor(const QString& anchor) {
    QString cleanAnchor = anchor;
    if (cleanAnchor.startsWith("#")) {
        cleanAnchor = cleanAnchor.mid(1);
    }
    
    QString script = QString(
        "var element = document.getElementById('%1');"
        "if (element) {"
        "    element.scrollIntoView({behavior: 'smooth', block: 'start'});"
        "    true;"
        "} else {"
        "    false;"
        "}"
    ).arg(cleanAnchor);
    
    page()->runJavaScript(script, [anchor](const QVariant& result) {
        if (!result.toBool()) {
            qWarning() << "Anchor not found:" << anchor;
        }
    });
}

void MarkdownPreview::setTheme(const QString& theme) { currentTheme = theme; }
void MarkdownPreview::scrollToPercentage(double percentage) {
    lastScrollPercentage = percentage;
    QString script =
        QString("window.scrollTo(0, document.body.scrollHeight * %1);")
            .arg(percentage);
    page()->runJavaScript(script);
}

double MarkdownPreview::currentScrollPercentage() const {
    return lastScrollPercentage;
}

QString MarkdownPreview::convertMarkdownToHtml(const QString& markdown) {
    struct OutputBuffer {
        QString html;
    };
    auto processOutput = [](const MD_CHAR* data, MD_SIZE size, void* userdata) {
        OutputBuffer* buffer = static_cast<OutputBuffer*>(userdata);
        buffer->html.append(QString::fromUtf8(data, size));
    };
    OutputBuffer buffer;
    QByteArray utf8Data = markdown.toUtf8();
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
    int result = md_html(utf8Data.constData(), utf8Data.size(), processOutput,
                         &buffer, parserFlags, rendererFlags);
    if (result != 0) {
        return "<p style=\"color: red;\">Error parsing markdown</p>";
    }
    return buffer.html;
}

QString MarkdownPreview::getStyleSheet(const QString& theme) {
    QFile baseFile(":/css/preview-base.css");
    if (!baseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to load base CSS";
        return "";
    }
    QString baseStyle = QString::fromUtf8(baseFile.readAll());
    baseFile.close();
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
        QRegularExpression(RegexPatterns::LATEX_DISPLAY_EQUATION),
        "$$\\1$$");
    result.replace(
        QRegularExpression(RegexPatterns::LATEX_INLINE_EQUATION),
        "$\\1$");
    return result;
}

QString MarkdownPreview::processWikiLinks(const QString& html) {
    QString result = html;
    
    QRegularExpression codeBlockPattern(
        RegexPatterns::HTML_CODE,
        QRegularExpression::DotMatchesEverythingOption);
    QRegularExpressionMatchIterator codeIt = codeBlockPattern.globalMatch(result);
    QList<QRegularExpressionMatch> codeMatches;
    while (codeIt.hasNext()) {
        codeMatches.prepend(codeIt.next());
    }
    
    for (const QRegularExpressionMatch& match : codeMatches) {
        QString codeAttrs = match.captured(1);
        QString codeContent = match.captured(2);
        
        QRegularExpression inclusionPattern(RegexPatterns::INCLUSION_PATTERN);
        QRegularExpressionMatch inclMatch = inclusionPattern.match(codeContent);
        if (!inclMatch.hasMatch()) {
            continue;
        }
        
        QString processedContent = codeContent;
        while (true) {
            QRegularExpressionMatch inclMatch = inclusionPattern.match(processedContent);
            if (!inclMatch.hasMatch()) {
                break;
            }
            
            QString target = inclMatch.captured(1).trimmed();
            
            QString errorMsg;
            QString fileContent = readFileContent(target, errorMsg);
            
            if (!errorMsg.isEmpty()) {
                fileContent = QString("Error: %1").arg(errorMsg);
            }
            
            fileContent = fileContent.toHtmlEscaped();
            
            processedContent.replace(inclMatch.capturedStart(), 
                                    inclMatch.capturedLength(), 
                                    fileContent);
        }
        
        QString replacement = QString("<code%1>%2</code>").arg(codeAttrs, processedContent);
        result.replace(match.capturedStart(), match.capturedLength(), replacement);
    }
    
    QRegularExpression inclusionPattern(RegexPatterns::MD4C_WIKILINK_INCLUSION);
    QRegularExpressionMatchIterator inclusionIt =
        inclusionPattern.globalMatch(result);
    QList<QRegularExpressionMatch> inclusionMatches;
    while (inclusionIt.hasNext()) {
        inclusionMatches.prepend(inclusionIt.next());
    }
    for (const QRegularExpressionMatch& match : inclusionMatches) {
        QString target = match.captured(1).trimmed();
        QString display = match.captured(2).trimmed();
        if (display.startsWith("!")) {
            display = target;
        }
        QString includedContent = resolveAndIncludeFile(target, display);
        result.replace(match.capturedStart(), match.capturedLength(),
                       includedContent);
    }
    result.replace(
        QRegularExpression(RegexPatterns::MD4C_WIKILINK),
        "<a href=\"wiki:\\1\" class=\"wiki-link\">\\2</a>");

    QRegularExpression linkPattern(RegexPatterns::HTML_ANCHOR);
    QRegularExpressionMatchIterator linkIt = linkPattern.globalMatch(result);
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
    QDir baseDir(basePath);
    QString cleanTarget = linkTarget.trimmed();
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
        return QString(
                   "<div class=\"inclusion-error\" style=\"border: 1px solid "
                   "#ff6b6b; "
                   "background-color: #ffe0e0; padding: 10px; margin: 10px 0; "
                   "border-radius: 5px;\">"
                   "<strong>Inclusion Error:</strong> File not found: %1"
                   "</div>")
            .arg(linkTarget.toHtmlEscaped());
    }
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
    QString includedHtml = convertMarkdownToHtml(content);
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
    QDir baseDir(basePath);
    QString cleanTarget = linkTarget.trimmed();
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
            QAction* openNewWindowAction =
                contextMenu.addAction(tr("Open Link in New Window"));
            connect(openNewWindowAction, &QAction::triggered, this,
                    [this, href]() {
                        emit openLinkInNewWindowRequested(href);
                    });
            contextMenu.addSeparator();
        }

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
    if (!lastMarkdownContent.isEmpty()) {
        setMarkdownContent(lastMarkdownContent);
    }
}

QString MarkdownPreview::addHeadingIds(const QString& html) {
    QString result = html;
    QMap<QString, int> anchorCounts;
    
    QRegularExpression headingPattern(
        RegexPatterns::HTML_HEADING,
        QRegularExpression::CaseInsensitiveOption
    );
    
    QRegularExpressionMatchIterator it = headingPattern.globalMatch(html);
    QList<QRegularExpressionMatch> matches;
    
    while (it.hasNext()) {
        matches.prepend(it.next());
    }
    
    for (const QRegularExpressionMatch& match : matches) {
        QString tag = match.captured(1);
        QString attributes = match.captured(2);
        QString headingText = match.captured(3);
        
        QString slug = RegexUtils::generateSlug(headingText);
        
        if (anchorCounts.contains(slug)) {
            anchorCounts[slug]++;
            slug += QString("-%1").arg(anchorCounts[slug]);
        } else {
            anchorCounts[slug] = 1;
        }
        
        QString newHeading = QString(
            "<%1%2 id=\"%3\">%4</%1>"
        ).arg(tag, attributes, slug, headingText);
        
        result.replace(match.capturedStart(), match.capturedLength(), newHeading);
    }
    
    return result;
}
