#include "markdownhighlighter.h"
#include <QColor>
#include <QFont>
#include <QDir>
#include <QFileInfo>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule rule;
    
    // Headers (H1-H6)
    h1Format.setForeground(QColor(0, 0, 139));
    h1Format.setFontWeight(QFont::Bold);
    h1Format.setFontPointSize(18);
    rule.pattern = QRegularExpression("^#{1}\\s+.*$");
    rule.format = h1Format;
    highlightingRules.append(rule);
    
    h2Format.setForeground(QColor(0, 0, 139));
    h2Format.setFontWeight(QFont::Bold);
    h2Format.setFontPointSize(16);
    rule.pattern = QRegularExpression("^#{2}\\s+.*$");
    rule.format = h2Format;
    highlightingRules.append(rule);
    
    h3Format.setForeground(QColor(0, 0, 139));
    h3Format.setFontWeight(QFont::Bold);
    h3Format.setFontPointSize(14);
    rule.pattern = QRegularExpression("^#{3}\\s+.*$");
    rule.format = h3Format;
    highlightingRules.append(rule);
    
    h4Format.setForeground(QColor(0, 0, 139));
    h4Format.setFontWeight(QFont::Bold);
    h4Format.setFontPointSize(12);
    rule.pattern = QRegularExpression("^#{4}\\s+.*$");
    rule.format = h4Format;
    highlightingRules.append(rule);
    
    h5Format.setForeground(QColor(0, 0, 139));
    h5Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^#{5}\\s+.*$");
    rule.format = h5Format;
    highlightingRules.append(rule);
    
    h6Format.setForeground(QColor(0, 0, 139));
    h6Format.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^#{6}\\s+.*$");
    rule.format = h6Format;
    highlightingRules.append(rule);
    
    // Bold text **text** or __text__
    boldFormat.setFontWeight(QFont::Bold);
    boldFormat.setForeground(QColor(0, 0, 0));
    rule.pattern = QRegularExpression("(\\*\\*|__)(.*?)(\\*\\*|__)");
    rule.format = boldFormat;
    highlightingRules.append(rule);
    
    // Italic text *text* or _text_
    italicFormat.setFontItalic(true);
    italicFormat.setForeground(QColor(0, 0, 0));
    rule.pattern = QRegularExpression("(\\*|_)(.*?)(\\*|_)");
    rule.format = italicFormat;
    highlightingRules.append(rule);
    
    // Inline code `code`
    inlineCodeFormat.setForeground(QColor(139, 0, 0));
    inlineCodeFormat.setBackground(QColor(240, 240, 240));
    inlineCodeFormat.setFontFamilies({QString("Monospace")});
    rule.pattern = QRegularExpression("`[^`]+`");
    rule.format = inlineCodeFormat;
    highlightingRules.append(rule);
    
    // Code blocks ```
    codeFormat.setForeground(QColor(139, 0, 0));
    codeFormat.setBackground(QColor(240, 240, 240));
    codeFormat.setFontFamilies({QString("Monospace")});
    rule.pattern = QRegularExpression("^```.*$");
    rule.format = codeFormat;
    highlightingRules.append(rule);
    
    // Links [text](url)
    linkFormat.setForeground(QColor(0, 0, 255));
    linkFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
    rule.format = linkFormat;
    highlightingRules.append(rule);
    
    // URLs
    urlFormat.setForeground(QColor(0, 0, 255));
    urlFormat.setFontUnderline(true);
    rule.pattern = QRegularExpression("https?://[^\\s]+");
    rule.format = urlFormat;
    highlightingRules.append(rule);
    
    // Unordered lists
    listFormat.setForeground(QColor(0, 100, 0));
    listFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^\\s*[-*+]\\s+");
    rule.format = listFormat;
    highlightingRules.append(rule);
    
    // Ordered lists
    rule.pattern = QRegularExpression("^\\s*\\d+\\.\\s+");
    rule.format = listFormat;
    highlightingRules.append(rule);
    
    // Blockquotes
    blockquoteFormat.setForeground(QColor(100, 100, 100));
    blockquoteFormat.setFontItalic(true);
    rule.pattern = QRegularExpression("^>+\\s+.*$");
    rule.format = blockquoteFormat;
    highlightingRules.append(rule);
    
    // Horizontal rules
    horizontalRuleFormat.setForeground(QColor(150, 150, 150));
    horizontalRuleFormat.setFontWeight(QFont::Bold);
    rule.pattern = QRegularExpression("^([-*_]\\s*){3,}$");
    rule.format = horizontalRuleFormat;
    highlightingRules.append(rule);
    
    // Wiki links - will be handled separately in highlightBlock for validation
    wikiLinkFormat.setForeground(QColor(0, 128, 0));
    wikiLinkFormat.setFontWeight(QFont::Bold);
    wikiLinkFormat.setFontUnderline(true);
    
    brokenWikiLinkFormat.setForeground(QColor(255, 0, 0));
    brokenWikiLinkFormat.setFontWeight(QFont::Bold);
    brokenWikiLinkFormat.setFontUnderline(true);
    
    // LaTeX formulas - inline $...$
    inlineLatexFormat.setForeground(QColor(128, 0, 128));
    inlineLatexFormat.setBackground(QColor(250, 240, 250));
    
    // LaTeX formulas - block $$...$$
    blockLatexFormat.setForeground(QColor(100, 0, 100));
    blockLatexFormat.setBackground(QColor(245, 235, 245));
    blockLatexFormat.setFontWeight(QFont::Bold);
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
    QRegularExpression wikiLinkPattern("\\[\\[([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
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
