#include "markdownhighlighter.h"
#include <QColor>
#include <QFont>
#include <QDir>
#include <QFileInfo>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
    , currentColorScheme("light")
{
    setupFormats();
}

void MarkdownHighlighter::setupFormats()
{
    highlightingRules.clear();
    HighlightingRule rule;
    
    // Determine colors based on scheme
    QColor headerColor, textColor, codeColor, linkColor, listColor, quoteColor, latexColor;
    QColor codeBg, wikiLinkColor, brokenLinkColor;
    
    if (currentColorScheme == "dark") {
        headerColor = QColor(100, 149, 237); // Cornflower blue
        textColor = QColor(220, 220, 220);
        codeColor = QColor(206, 145, 120);
        linkColor = QColor(42, 161, 255);
        listColor = QColor(152, 195, 121);
        quoteColor = QColor(128, 128, 128);
        latexColor = QColor(229, 192, 123);
        codeBg = QColor(45, 45, 45);
        wikiLinkColor = QColor(156, 220, 254);
        brokenLinkColor = QColor(240, 113, 120);
    } else if (currentColorScheme == "solarized-light") {
        headerColor = QColor(38, 139, 210); // blue
        textColor = QColor(101, 123, 131);
        codeColor = QColor(220, 50, 47); // red
        linkColor = QColor(38, 139, 210);
        listColor = QColor(133, 153, 0); // green
        quoteColor = QColor(147, 161, 161);
        latexColor = QColor(203, 75, 22); // orange
        codeBg = QColor(238, 232, 213);
        wikiLinkColor = QColor(42, 161, 152); // cyan
        brokenLinkColor = QColor(211, 54, 130); // magenta
    } else if (currentColorScheme == "solarized-dark") {
        headerColor = QColor(38, 139, 210); // blue
        textColor = QColor(131, 148, 150);
        codeColor = QColor(220, 50, 47); // red
        linkColor = QColor(38, 139, 210);
        listColor = QColor(133, 153, 0); // green
        quoteColor = QColor(88, 110, 117);
        latexColor = QColor(203, 75, 22); // orange
        codeBg = QColor(7, 54, 66);
        wikiLinkColor = QColor(42, 161, 152); // cyan
        brokenLinkColor = QColor(211, 54, 130); // magenta
    } else { // light
        headerColor = QColor(0, 0, 139);
        textColor = QColor(0, 0, 0);
        codeColor = QColor(139, 0, 0);
        linkColor = QColor(0, 0, 255);
        listColor = QColor(0, 100, 0);
        quoteColor = QColor(128, 128, 128);
        latexColor = QColor(139, 69, 19);
        codeBg = QColor(240, 240, 240);
        wikiLinkColor = QColor(0, 128, 128);
        brokenLinkColor = QColor(178, 34, 34);
    }
    
    // Headers (H1-H6)
    h1Format.setForeground(headerColor);
    h1Format.setFontWeight(QFont::Bold);
    h1Format.setFontPointSize(18);
    rule.pattern = QRegularExpression("^#{1}\\s+.*$");
    rule.format = h1Format;
    highlightingRules.append(rule);
    
    h2Format.setForeground(headerColor);
    h2Format.setFontWeight(QFont::Bold);
    h2Format.setFontPointSize(16);
    rule.pattern = QRegularExpression("^#{2}\\s+.*$");
    rule.format = h2Format;
    highlightingRules.append(rule);
    
    h3Format.setForeground(headerColor);
    h3Format.setFontWeight(QFont::Bold);
    h3Format.setFontPointSize(14);
    rule.pattern = QRegularExpression("^#{3}\\s+.*$");
    rule.format = h3Format;
    highlightingRules.append(rule);
    
    h4Format.setForeground(headerColor);
    h4Format.setFontWeight(QFont::Bold);
    h4Format.setFontPointSize(12);
    rule.pattern = QRegularExpression("^#{4}\\s+.*$");
    rule.format = h4Format;
    highlightingRules.append(rule);
    
    h5Format.setForeground(headerColor);
    h5Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^#{5}\\s+.*$");
    rule.format = h5Format;
    highlightingRules.append(rule);
    
    h6Format.setForeground(headerColor);
    h6Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^#{6}\\s+.*$");
    rule.format = h6Format;
    highlightingRules.append(rule);
    
    // Bold text **text** or __text__
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setForeground(textColor);
    rule.pattern = QRegularExpression("(\\*\\*|__)(.*?)(\\*\\*|__)");
    rule.format = boldFormat;
    highlightingRules.append(rule);
    
    // Italic text *text* or _text_
    italicFormat.setFontItalic(true);
    italicFormat.setForeground(textColor);
    rule.pattern = QRegularExpression("(\\*|_)(.*?)(\\*|_)");
    rule.format = italicFormat;
    highlightingRules.append(rule);
    
    // Inline code `code`
    inlineCodeFormat.setForeground(codeColor);
    inlineCodeFormat.setBackground(codeBg);
    inlineCodeFormat.setFontFamilies({QString("Monospace")});
    rule.pattern = QRegularExpression("`[^`]+`");
    rule.format = inlineCodeFormat;
    highlightingRules.append(rule);
    
    // Code blocks ```
    codeFormat.setForeground(codeColor);
    codeFormat.setBackground(codeBg);
    codeFormat.setFontFamilies({QString("Monospace")});
    rule.pattern = QRegularExpression("^```.*$");
    rule.format = codeFormat;
    highlightingRules.append(rule);
    
    // Links [text](url)
    linkFormat.setForeground(linkColor);
    linkFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    rule.format = linkFormat;
    highlightingRules.append(rule);
    
    // URLs
    urlFormat.setForeground(linkColor);
    urlFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression("https?://[^\\s]+");
    rule.format = urlFormat;
    highlightingRules.append(rule);
    
    // Unordered lists
    listFormat.setForeground(listColor);
    listFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\s*[-*+]\\s+");
    rule.format = listFormat;
    highlightingRules.append(rule);
    
    // Ordered lists
    rule.pattern = QRegularExpression("^\\s*\\d+\\.\\s+");
    rule.format = listFormat;
    highlightingRules.append(rule);
    
    // Blockquotes
    blockquoteFormat.setForeground(quoteColor);
    blockquoteFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("^>+\\s+.*$");
    rule.format = blockquoteFormat;
    highlightingRules.append(rule);
    
    // Horizontal rules
    horizontalRuleFormat.setForeground(quoteColor);
    horizontalRuleFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^([-*_]\\s*){3,}$");
    rule.format = horizontalRuleFormat;
    highlightingRules.append(rule);
    
    // Wiki links - will be handled separately in highlightBlock for validation
    wikiLinkFormat.setForeground(wikiLinkColor);
    wikiLinkFormat.setFontWeight(QFont::Bold);
    wikiLinkFormat.setFontUnderline(true);
    
    brokenWikiLinkFormat.setForeground(brokenLinkColor);
    brokenWikiLinkFormat.setFontWeight(QFont::Bold);
    brokenWikiLinkFormat.setFontUnderline(true);
    
    // Inclusion links - use distinct styling
    QColor inclusionColor = currentColorScheme == "dark" ? QColor(129, 250, 183) : QColor(34, 139, 34);
    inclusionLinkFormat.setForeground(inclusionColor);
    inclusionLinkFormat.setFontWeight(QFont::Bold);
    inclusionLinkFormat.setFontUnderline(true);
    if (currentColorScheme == "dark") {
        inclusionLinkFormat.setBackground(QColor(30, 60, 40));
    } else {
        inclusionLinkFormat.setBackground(QColor(230, 255, 230));
    }
    
    brokenInclusionLinkFormat.setForeground(brokenLinkColor);
    brokenInclusionLinkFormat.setFontWeight(QFont::Bold);
    brokenInclusionLinkFormat.setFontUnderline(true);
    if (currentColorScheme == "dark") {
        brokenInclusionLinkFormat.setBackground(QColor(60, 30, 30));
    } else {
        brokenInclusionLinkFormat.setBackground(QColor(255, 230, 230));
    }
    
    // LaTeX formulas - inline $...$
    inlineLatexFormat.setForeground(latexColor);
    if (currentColorScheme == "dark") {
        inlineLatexFormat.setBackground(QColor(60, 50, 60));
    } else if (currentColorScheme.startsWith("solarized")) {
        inlineLatexFormat.setBackground(QColor(238, 232, 213));
    } else {
        inlineLatexFormat.setBackground(QColor(250, 240, 250));
    }
    
    // LaTeX formulas - block $$...$$
    blockLatexFormat.setForeground(latexColor);
    if (currentColorScheme == "dark") {
        blockLatexFormat.setBackground(QColor(60, 50, 60));
    } else if (currentColorScheme.startsWith("solarized")) {
        blockLatexFormat.setBackground(QColor(238, 232, 213));
    } else {
        blockLatexFormat.setBackground(QColor(245, 235, 245));
    }
    blockLatexFormat.setFontWeight(QFont::Bold);
}

void MarkdownHighlighter::setColorScheme(const QString &scheme)
{
    currentColorScheme = scheme;
    setupFormats();
    rehighlight();
}

void MarkdownHighlighter::updateColorScheme()
{
    rehighlight();
}

void MarkdownHighlighter::setRootPath(const QString &path)
{
    rootPath = path;
    rehighlight();
}

void MarkdownHighlighter::highlightBlock(const QString &text)
{
    // Apply all highlighting rules
    for (const HighlightingRule &rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
    
    // Handle wiki links separately for validation
    // First handle inclusion links [[!target]] or [[!target|display]]
    QRegularExpression inclusionPattern("\\[\\[!([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator inclusionIterator = inclusionPattern.globalMatch(text);
    
    while (inclusionIterator.hasNext()) {
        QRegularExpressionMatch match = inclusionIterator.next();
        QString linkTarget = match.captured(1);
        
        bool exists = checkWikiLinkExists(linkTarget);
        
        if (exists) {
            setFormat(match.capturedStart(), match.capturedLength(), inclusionLinkFormat);
        } else {
            setFormat(match.capturedStart(), match.capturedLength(), brokenInclusionLinkFormat);
        }
    }
    
    // Then handle regular wiki links [[target]] or [[target|display]]
    QRegularExpression wikiLinkPattern("\\[\\[([^\\]|!]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator wikiMatchIterator = wikiLinkPattern.globalMatch(text);
    
    while (wikiMatchIterator.hasNext()) {
        QRegularExpressionMatch match = wikiMatchIterator.next();
        QString linkTarget = match.captured(1);
        
        bool exists = checkWikiLinkExists(linkTarget);
        
        if (exists) {
            setFormat(match.capturedStart(), match.capturedLength(), wikiLinkFormat);
        } else {
            setFormat(match.capturedStart(), match.capturedLength(), brokenWikiLinkFormat);
        }
    }
    
    // Handle LaTeX formulas
    // Block formulas $$...$$
    QRegularExpression blockLatexPattern("\\$\\$[^$]+\\$\\$");
    QRegularExpressionMatchIterator blockLatexIterator = blockLatexPattern.globalMatch(text);
    
    while (blockLatexIterator.hasNext()) {
        QRegularExpressionMatch match = blockLatexIterator.next();
        setFormat(match.capturedStart(), match.capturedLength(), blockLatexFormat);
    }
    
    // Inline formulas $...$
    QRegularExpression inlineLatexPattern("\\$[^$\\n]+\\$");
    QRegularExpressionMatchIterator inlineLatexIterator = inlineLatexPattern.globalMatch(text);
    
    while (inlineLatexIterator.hasNext()) {
        QRegularExpressionMatch match = inlineLatexIterator.next();
        // Make sure it's not part of a block formula
        int start = match.capturedStart();
        if (start > 0 && text[start - 1] == '$') continue;
        if (start + match.capturedLength() < text.length() && text[start + match.capturedLength()] == '$') continue;
        
        setFormat(match.capturedStart(), match.capturedLength(), inlineLatexFormat);
    }
}

bool MarkdownHighlighter::checkWikiLinkExists(const QString &linkText) const
{
    if (rootPath.isEmpty()) {
        return false;
    }
    
    QString cleanLink = linkText.trimmed();
    
    QDir rootDir(rootPath);
    
    QStringList possibleFiles;
    possibleFiles << cleanLink + ".md"
                  << cleanLink + ".markdown"
                  << cleanLink;
    
    for (const QString &fileName : possibleFiles) {
        QString fullPath = rootDir.filePath(fileName);
        if (QFileInfo::exists(fullPath)) {
            return true;
        }
    }
    
    QStringList filters;
    filters << "*.md" << "*.markdown";
    
    QFileInfoList files = rootDir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo &fileInfo : files) {
        QString baseName = fileInfo.completeBaseName();
        if (baseName.compare(cleanLink, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }
    
    return false;
}
