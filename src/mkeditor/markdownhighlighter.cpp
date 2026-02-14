#include "markdownhighlighter.h"
#include "colorpalette.h"

#include <QColor>
#include <QDir>
#include <QFileInfo>
#include <QFont>

MarkdownHighlighter::MarkdownHighlighter(QTextDocument* parent)
    : QSyntaxHighlighter(parent),
      currentColorScheme("light"),
      codeSyntaxEnabled(false),
      currentCursorLine(-1) {
    setupFormats();
}

void MarkdownHighlighter::setupFormats() {
    highlightingRules.clear();
    HighlightingRule rule;

    QColor headerColor, textColor, codeColor, linkColor, listColor, quoteColor,
        latexColor, strikethroughColor;
    QColor codeBg, wikiLinkColor, brokenLinkColor;

    if (currentColorScheme == "dark") {
        const auto& colors = ColorPalette::getDarkTheme();
        headerColor = colors.header;
        textColor = colors.text;
        codeColor = colors.code;
        linkColor = colors.link;
        listColor = colors.listMarker;
        quoteColor = colors.blockquote;
        latexColor = colors.latex;
        codeBg = colors.backgroundCode;
        wikiLinkColor = colors.wikiLink;
        brokenLinkColor = colors.brokenLink;
        strikethroughColor = colors.strikethrough;
    } else if (currentColorScheme == "solarized-light") {
        headerColor = QColor(38, 139, 210);
        textColor = QColor(101, 123, 131);
        codeColor = QColor(220, 50, 47);
        linkColor = QColor(38, 139, 210);
        listColor = QColor(133, 153, 0);
        quoteColor = QColor(147, 161, 161);
        latexColor = QColor(203, 75, 22);
        codeBg = QColor(238, 232, 213);
        wikiLinkColor = QColor(42, 161, 152);
        brokenLinkColor = QColor(211, 54, 130);
        strikethroughColor = QColor(147, 161, 161);
    } else if (currentColorScheme == "solarized-dark") {
        headerColor = QColor(38, 139, 210);
        textColor = QColor(131, 148, 150);
        codeColor = QColor(220, 50, 47);
        linkColor = QColor(38, 139, 210);
        listColor = QColor(133, 153, 0);
        quoteColor = QColor(88, 110, 117);
        latexColor = QColor(203, 75, 22);
        codeBg = QColor(7, 54, 66);
        wikiLinkColor = QColor(42, 161, 152);
        brokenLinkColor = QColor(211, 54, 130);
        strikethroughColor = QColor(88, 110, 117);
    } else {
        const auto& colors = ColorPalette::getLightTheme();
        headerColor = colors.header;
        textColor = colors.text;
        codeColor = colors.code;
        linkColor = colors.link;
        listColor = colors.listMarker;
        quoteColor = colors.blockquote;
        latexColor = colors.latex;
        codeBg = colors.backgroundCode;
        wikiLinkColor = colors.wikiLink;
        brokenLinkColor = colors.brokenLink;
        strikethroughColor = colors.strikethrough;
    }

    strikethroughFormat.setFontStrikeOut(true);
    strikethroughFormat.setForeground(strikethroughColor);

    /*
    taskPendingFormat.setForeground(QColor(255, 0, 0)); // red
    taskDoneFormat.setForeground(QColor(0, 255, 0)); // green
    taskPartialFormat.setForeground(QColor(255, 255, 0)); // yellow
    */

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

    QColor inclusionColor;
    QColor inclusionBg;
    QColor brokenInclusionBg;
    if (currentColorScheme == "dark") {
        const auto& colors = ColorPalette::getDarkTheme();
        inclusionColor = colors.inclusion;
        inclusionBg = colors.backgroundInclusion;
        brokenInclusionBg = colors.backgroundInclusionBroken;
    } else {
        const auto& colors = ColorPalette::getLightTheme();
        inclusionColor = colors.inclusion;
        inclusionBg = colors.backgroundInclusion;
        brokenInclusionBg = colors.backgroundInclusionBroken;
    }
    
    inclusionLinkFormat.setForeground(inclusionColor);
    inclusionLinkFormat.setFontWeight(QFont::Bold);
    inclusionLinkFormat.setFontUnderline(true);
    inclusionLinkFormat.setBackground(inclusionBg);

    brokenInclusionLinkFormat.setForeground(brokenLinkColor);
    brokenInclusionLinkFormat.setFontWeight(QFont::Bold);
    brokenInclusionLinkFormat.setFontUnderline(true);
    brokenInclusionLinkFormat.setBackground(brokenInclusionBg);

    inlineLatexFormat.setForeground(latexColor);
    if (currentColorScheme == "dark") {
        const auto& colors = ColorPalette::getDarkTheme();
        inlineLatexFormat.setBackground(colors.backgroundLatexInline);
    } else if (currentColorScheme.startsWith("solarized")) {
        inlineLatexFormat.setBackground(QColor(238, 232, 213));
    } else {
        const auto& colors = ColorPalette::getLightTheme();
        inlineLatexFormat.setBackground(colors.backgroundLatexInline);
    }

    blockLatexFormat.setForeground(latexColor);
    if (currentColorScheme == "dark") {
        const auto& colors = ColorPalette::getDarkTheme();
        blockLatexFormat.setBackground(colors.backgroundLatexBlock);
    } else if (currentColorScheme.startsWith("solarized")) {
        blockLatexFormat.setBackground(QColor(238, 232, 213));
    } else {
        const auto& colors = ColorPalette::getLightTheme();
        blockLatexFormat.setBackground(colors.backgroundLatexBlock);
    }
    blockLatexFormat.setFontWeight(QFont::Bold);

    QColor keywordColor, stringColor, commentColor, numberColor, functionColor,
        typeColor;

    if (currentColorScheme == "dark") {
        const auto& colors = ColorPalette::getDarkTheme();
        keywordColor = colors.syntaxKeyword;
        stringColor = colors.syntaxString;
        commentColor = colors.syntaxComment;
        numberColor = colors.syntaxNumber;
        functionColor = colors.syntaxFunction;
        typeColor = colors.syntaxType;
    } else if (currentColorScheme == "solarized-light") {
        keywordColor = QColor(133, 153, 0);
        stringColor = QColor(42, 161, 152);
        commentColor = QColor(147, 161, 161);
        numberColor = QColor(203, 75, 22);
        functionColor = QColor(38, 139, 210);
        typeColor = QColor(181, 137, 0);
    } else if (currentColorScheme == "solarized-dark") {
        keywordColor = QColor(133, 153, 0);
        stringColor = QColor(42, 161, 152);
        commentColor = QColor(88, 110, 117);
        numberColor = QColor(203, 75, 22);
        functionColor = QColor(38, 139, 210);
        typeColor = QColor(181, 137, 0);
    } else {
        const auto& colors = ColorPalette::getLightTheme();
        keywordColor = colors.syntaxKeyword;
        stringColor = colors.syntaxString;
        commentColor = colors.syntaxComment;
        numberColor = colors.syntaxNumber;
        functionColor = colors.syntaxFunction;
        typeColor = colors.syntaxType;
    }

    codeKeywordFormat.setForeground(keywordColor);
    codeKeywordFormat.setFontWeight(QFont::Bold);

    codeStringFormat.setForeground(stringColor);

    codeCommentFormat.setForeground(commentColor);
    codeCommentFormat.setFontItalic(true);

    codeNumberFormat.setForeground(numberColor);

    codeFunctionFormat.setForeground(functionColor);

    codeTypeFormat.setForeground(typeColor);
    codeTypeFormat.setFontWeight(QFont::Bold);
}

void MarkdownHighlighter::setColorScheme(const QString& scheme) {
    currentColorScheme = scheme;
    setupFormats();
    rehighlight();
}

void MarkdownHighlighter::setCodeSyntaxEnabled(bool enabled) {
    codeSyntaxEnabled = enabled;
    rehighlight();
}

void MarkdownHighlighter::setCurrentCursorLine(int lineNumber) {
    if (currentCursorLine != lineNumber) {
        int oldLine = currentCursorLine;
        currentCursorLine = lineNumber;

        // Only rehighlight the affected lines, not the entire document
        QTextDocument* doc = document();
        if (doc) {
            // Rehighlight the old cursor line
            if (oldLine >= 0 && oldLine < doc->blockCount()) {
                QTextBlock oldBlock = doc->findBlockByNumber(oldLine);
                if (oldBlock.isValid()) {
                    rehighlightBlock(oldBlock);
                }
            }

            // Rehighlight the new cursor line
            if (lineNumber >= 0 && lineNumber < doc->blockCount()) {
                QTextBlock newBlock = doc->findBlockByNumber(lineNumber);
                if (newBlock.isValid()) {
                    rehighlightBlock(newBlock);
                }
            }
        }
    }
}

QColor MarkdownHighlighter::getSubtleColor() const {
    if (currentColorScheme == "dark" || currentColorScheme == "solarized-dark") {
        return ColorPalette::getDarkTheme().subtleMarkup;
    } else {
        return ColorPalette::getLightTheme().subtleMarkup;
    }
}

void MarkdownHighlighter::updateColorScheme() {
    setupFormats();
    rehighlight();
}

void MarkdownHighlighter::setRootPath(const QString& path) {
    rootPath = path;
    rehighlight();
}

void MarkdownHighlighter::highlightBlock(const QString& text) {
    // Check if we're starting or ending a code block
    int previousState = previousBlockState();
    bool inCodeBlock = (previousState == InCodeBlock);

    // Check if this is the current cursor line
    bool isCurrentLine = (currentBlock().blockNumber() == currentCursorLine);
    QColor subtleColor = getSubtleColor();

    // Check if this line contains a code fence
    QRegularExpression codeFencePattern("^```(\\w+)?");
    QRegularExpressionMatch fenceMatch = codeFencePattern.match(text);

    if (fenceMatch.hasMatch()) {
        // Toggle code block state
        if (!inCodeBlock) {
            // Starting a code block - extract language
            currentCodeLanguage = fenceMatch.captured(1).toLower();
        } else {
            // Ending a code block
            currentCodeLanguage.clear();
        }
        inCodeBlock = !inCodeBlock;
        setCurrentBlockState(inCodeBlock ? InCodeBlock : Normal);
        // Apply code format to the fence line
        setFormat(0, text.length(), codeFormat);
        // Make backticks subtle if not on current line
        if (!isCurrentLine) {
            QTextCharFormat subtleFormat;
            subtleFormat.setForeground(subtleColor);
            setFormat(0, 3, subtleFormat);  // The ``` part
        }
        return;
    }

    // Set the block state
    setCurrentBlockState(inCodeBlock ? InCodeBlock : Normal);

    // If we're inside a code block, apply code formatting with syntax
    // highlighting
    if (inCodeBlock) {
        setFormat(0, text.length(), codeFormat);
        if (codeSyntaxEnabled) {
            highlightCodeLine(text, currentCodeLanguage);
        }
        return;
    }

    // Apply all highlighting rules (only when not in code block)
    for (const HighlightingRule& rule : std::as_const(highlightingRules)) {
        QRegularExpressionMatchIterator matchIterator =
            rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      rule.format);
        }
    }

    // Make markdown control characters subtle when not on current line
    if (!isCurrentLine) {
        QTextCharFormat subtleFormat;
        subtleFormat.setForeground(subtleColor);

        // Headers: # ## ### etc.
        QRegularExpression headerPattern("^(#{1,6})\\s");
        QRegularExpressionMatch headerMatch = headerPattern.match(text);
        if (headerMatch.hasMatch()) {
            setFormat(0, headerMatch.capturedLength(1), subtleFormat);
        }

        // Bold: **text** or __text__
        QRegularExpression boldPattern("(\\*\\*|__)(?=\\S)(.+?)(?<=\\S)\\1");
        QRegularExpressionMatchIterator boldIt = boldPattern.globalMatch(text);
        while (boldIt.hasNext()) {
            QRegularExpressionMatch match = boldIt.next();
            // Make the ** or __ subtle
            setFormat(match.capturedStart(), 2, subtleFormat);
            setFormat(match.capturedEnd() - 2, 2, subtleFormat);
        }

        // Italic: *text* or _text_
        QRegularExpression italicPattern(
            "(?<!\\*)\\*(?=\\S)(.+?)(?<=\\S)\\*(?!\\*)|(?<!_)_(?=\\S)(.+?)(?<="
            "\\S)_(?!_)");
        QRegularExpressionMatchIterator italicIt =
            italicPattern.globalMatch(text);
        while (italicIt.hasNext()) {
            QRegularExpressionMatch match = italicIt.next();
            setFormat(match.capturedStart(), 1, subtleFormat);
            setFormat(match.capturedEnd() - 1, 1, subtleFormat);
        }

        // Strikethrough: ~~text~~
        QRegularExpression strikePattern("~~");
        QRegularExpressionMatchIterator strikeIt =
            strikePattern.globalMatch(text);
        while (strikeIt.hasNext()) {
            QRegularExpressionMatch match = strikeIt.next();
            setFormat(match.capturedStart(), 2, subtleFormat);
        }

        // List markers: -, *, +
        QRegularExpression listPattern("^(\\s*)([-*+])\\s");
        QRegularExpressionMatch listMatch = listPattern.match(text);
        if (listMatch.hasMatch()) {
            int markerPos = listMatch.capturedStart(2);
            setFormat(markerPos, 1, subtleFormat);
        }

        // Numbered list: 1. 2. etc.
        QRegularExpression numListPattern("^(\\s*)(\\d+\\.)\\s");
        QRegularExpressionMatch numListMatch = numListPattern.match(text);
        if (numListMatch.hasMatch()) {
            int markerPos = numListMatch.capturedStart(2);
            setFormat(markerPos, numListMatch.capturedLength(2), subtleFormat);
        }

        // Blockquote: >
        QRegularExpression quotePattern("^(\\s*>+)\\s");
        QRegularExpressionMatch quoteMatch = quotePattern.match(text);
        if (quoteMatch.hasMatch()) {
            setFormat(quoteMatch.capturedStart(1), quoteMatch.capturedLength(1),
                      subtleFormat);
        }

        // Inline code: `code`
        QRegularExpression inlineCodePattern("`[^`]+`");
        QRegularExpressionMatchIterator inlineCodeIt =
            inlineCodePattern.globalMatch(text);
        while (inlineCodeIt.hasNext()) {
            QRegularExpressionMatch match = inlineCodeIt.next();
            setFormat(match.capturedStart(), 1, subtleFormat);    // Opening `
            setFormat(match.capturedEnd() - 1, 1, subtleFormat);  // Closing `
        }

        // Links: [text](url)
        QRegularExpression linkPattern("\\[([^\\]]+)\\]\\(([^\\)]+)\\)");
        QRegularExpressionMatchIterator linkIt = linkPattern.globalMatch(text);
        while (linkIt.hasNext()) {
            QRegularExpressionMatch match = linkIt.next();
            setFormat(match.capturedStart(), 1, subtleFormat);  // [
            setFormat(match.capturedStart(1) + match.capturedLength(1), 2,
                      subtleFormat);                              // ](
            setFormat(match.capturedEnd() - 1, 1, subtleFormat);  // )
        }

        // Wiki links: [[link]]
        QRegularExpression wikiPattern(
            "\\[\\[(!)?([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
        QRegularExpressionMatchIterator wikiIt = wikiPattern.globalMatch(text);
        while (wikiIt.hasNext()) {
            QRegularExpressionMatch match = wikiIt.next();
            setFormat(match.capturedStart(), 2, subtleFormat);  // [[
            if (match.capturedLength(3) > 0) {
                setFormat(match.capturedStart(3), 1, subtleFormat);  // |
            }
            setFormat(match.capturedEnd() - 2, 2, subtleFormat);  // ]]
        }
    }

    // Highlight strikethrough ~~text~~
    QRegularExpression strikethroughRegex("~~(.*?)~~");
    QRegularExpressionMatchIterator it = strikethroughRegex.globalMatch(text);
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        int start = match.capturedStart(1);
        int length = match.capturedLength(1);
        setFormat(start, length, strikethroughFormat);
    }

    // Highlight tasks - [ ], - [X], - [.]
    /*
    QRegularExpression taskRegex("[-*+] \\[([ xX.]?)\\]");
    QRegularExpressionMatchIterator taskIt = taskRegex.globalMatch(text);
    while (taskIt.hasNext()) {
      QRegularExpressionMatch match = taskIt.next();
      QString state = match.captured(1);
      QTextCharFormat format;
      if (state == "X") {
        format = taskDoneFormat;
      } else if (state == ".") {
        format = taskPartialFormat;
      } else {
        format = taskPendingFormat;
      }
      setFormat(match.capturedStart(), match.capturedLength(), format);
    }
    */

    // Handle wiki links separately for validation
    // First handle inclusion links [[!target]] or [[!target|display]]
    QRegularExpression inclusionPattern(
        "\\[\\[!([^\\]|]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator inclusionIterator =
        inclusionPattern.globalMatch(text);

    while (inclusionIterator.hasNext()) {
        QRegularExpressionMatch match = inclusionIterator.next();
        QString linkTarget = match.captured(1);

        bool exists = checkWikiLinkExists(linkTarget);

        if (exists) {
            setFormat(match.capturedStart(), match.capturedLength(),
                      inclusionLinkFormat);
        } else {
            setFormat(match.capturedStart(), match.capturedLength(),
                      brokenInclusionLinkFormat);
        }
    }

    // Then handle regular wiki links [[target]] or [[target|display]]
    QRegularExpression wikiLinkPattern(
        "\\[\\[([^\\]|!]+)(\\|([^\\]]+))?\\]\\]");
    QRegularExpressionMatchIterator wikiMatchIterator =
        wikiLinkPattern.globalMatch(text);

    while (wikiMatchIterator.hasNext()) {
        QRegularExpressionMatch match = wikiMatchIterator.next();
        QString linkTarget = match.captured(1);

        bool exists = checkWikiLinkExists(linkTarget);

        if (exists) {
            setFormat(match.capturedStart(), match.capturedLength(),
                      wikiLinkFormat);
        } else {
            setFormat(match.capturedStart(), match.capturedLength(),
                      brokenWikiLinkFormat);
        }
    }

    // Handle LaTeX formulas
    // Block formulas $$...$$
    QRegularExpression blockLatexPattern("\\$\\$[^$]+\\$\\$");
    QRegularExpressionMatchIterator blockLatexIterator =
        blockLatexPattern.globalMatch(text);

    while (blockLatexIterator.hasNext()) {
        QRegularExpressionMatch match = blockLatexIterator.next();
        setFormat(match.capturedStart(), match.capturedLength(),
                  blockLatexFormat);
    }

    // Inline formulas $...$
    QRegularExpression inlineLatexPattern("\\$[^$\\n]+\\$");
    QRegularExpressionMatchIterator inlineLatexIterator =
        inlineLatexPattern.globalMatch(text);

    while (inlineLatexIterator.hasNext()) {
        QRegularExpressionMatch match = inlineLatexIterator.next();
        // Make sure it's not part of a block formula
        int start = match.capturedStart();
        if (start > 0 && text[start - 1] == '$') continue;
        if (start + match.capturedLength() < text.length() &&
            text[start + match.capturedLength()] == '$')
            continue;

        setFormat(match.capturedStart(), match.capturedLength(),
                  inlineLatexFormat);
    }
}

bool MarkdownHighlighter::checkWikiLinkExists(const QString& linkText) const {
    if (rootPath.isEmpty()) {
        return false;
    }

    QString cleanLink = linkText.trimmed();

    QDir rootDir(rootPath);

    QStringList possibleFiles;
    possibleFiles << cleanLink + ".md" << cleanLink + ".markdown" << cleanLink;

    for (const QString& fileName : possibleFiles) {
        QString fullPath = rootDir.filePath(fileName);
        if (QFileInfo::exists(fullPath)) {
            return true;
        }
    }

    QStringList filters;
    filters << "*.md" << "*.markdown";

    QFileInfoList files =
        rootDir.entryInfoList(filters, QDir::Files, QDir::Name);
    for (const QFileInfo& fileInfo : files) {
        QString baseName = fileInfo.completeBaseName();
        if (baseName.compare(cleanLink, Qt::CaseInsensitive) == 0) {
            return true;
        }
    }

    return false;
}

void MarkdownHighlighter::highlightCodeLine(const QString& text,
                                            const QString& language) {
    if (language.isEmpty() || text.trimmed().isEmpty()) {
        return;
    }

    // Python syntax highlighting
    if (language == "python" || language == "py") {
        // Python keywords
        QStringList keywords = {
            "\\bdef\\b",    "\\bclass\\b",   "\\bif\\b",      "\\belif\\b",
            "\\belse\\b",   "\\bfor\\b",     "\\bwhile\\b",   "\\bin\\b",
            "\\breturn\\b", "\\bimport\\b",  "\\bfrom\\b",    "\\bas\\b",
            "\\btry\\b",    "\\bexcept\\b",  "\\bfinally\\b", "\\bwith\\b",
            "\\braise\\b",  "\\bassert\\b",  "\\bbreak\\b",   "\\bcontinue\\b",
            "\\bpass\\b",   "\\byield\\b",   "\\blambda\\b",  "\\band\\b",
            "\\bor\\b",     "\\bnot\\b",     "\\bis\\b",      "\\bNone\\b",
            "\\bTrue\\b",   "\\bFalse\\b",   "\\basync\\b",   "\\bawait\\b",
            "\\bglobal\\b", "\\bnonlocal\\b"};

        for (const QString& pattern : keywords) {
            QRegularExpression re(pattern);
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          codeKeywordFormat);
            }
        }

        // Python strings (single and double quotes, including triple quotes)
        QRegularExpression stringPattern(
            "('''[^']*'''|\"\"\"[^\"]*\"\"\"|'[^']*'|\"[^\"]*\")");
        QRegularExpressionMatchIterator stringIt =
            stringPattern.globalMatch(text);
        while (stringIt.hasNext()) {
            QRegularExpressionMatch match = stringIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeStringFormat);
        }

        // Python comments
        int commentStart = text.indexOf('#');
        if (commentStart >= 0) {
            setFormat(commentStart, text.length() - commentStart,
                      codeCommentFormat);
        }

        // Numbers
        QRegularExpression numberPattern("\\b\\d+(\\.\\d+)?\\b");
        QRegularExpressionMatchIterator numIt = numberPattern.globalMatch(text);
        while (numIt.hasNext()) {
            QRegularExpressionMatch match = numIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeNumberFormat);
        }

        // Function calls
        QRegularExpression funcPattern(
            "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?=\\()");
        QRegularExpressionMatchIterator funcIt = funcPattern.globalMatch(text);
        while (funcIt.hasNext()) {
            QRegularExpressionMatch match = funcIt.next();
            setFormat(match.capturedStart(1), match.capturedLength(1),
                      codeFunctionFormat);
        }
    }
    // JavaScript/TypeScript syntax highlighting
    else if (language == "javascript" || language == "js" ||
             language == "typescript" || language == "ts") {
        QStringList keywords = {
            "\\bvar\\b",      "\\blet\\b",       "\\bconst\\b",
            "\\bfunction\\b", "\\bclass\\b",     "\\bif\\b",
            "\\belse\\b",     "\\bfor\\b",       "\\bwhile\\b",
            "\\bdo\\b",       "\\breturn\\b",    "\\bimport\\b",
            "\\bexport\\b",   "\\btry\\b",       "\\bcatch\\b",
            "\\bfinally\\b",  "\\bthrow\\b",     "\\bnew\\b",
            "\\bthis\\b",     "\\bsuper\\b",     "\\bbreak\\b",
            "\\bcontinue\\b", "\\bswitch\\b",    "\\bcase\\b",
            "\\bdefault\\b",  "\\btrue\\b",      "\\bfalse\\b",
            "\\bnull\\b",     "\\bundefined\\b", "\\basync\\b",
            "\\bawait\\b",    "\\bof\\b",        "\\bin\\b",
            "\\btypeof\\b",   "\\binstanceof\\b"};

        for (const QString& pattern : keywords) {
            QRegularExpression re(pattern);
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          codeKeywordFormat);
            }
        }

        // Strings
        QRegularExpression stringPattern("(`[^`]*`|'[^']*'|\"[^\"]*\")");
        QRegularExpressionMatchIterator stringIt =
            stringPattern.globalMatch(text);
        while (stringIt.hasNext()) {
            QRegularExpressionMatch match = stringIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeStringFormat);
        }

        // Single-line comments
        int commentStart = text.indexOf("//");
        if (commentStart >= 0) {
            setFormat(commentStart, text.length() - commentStart,
                      codeCommentFormat);
        }

        // Multi-line comments (simplified)
        QRegularExpression multiCommentPattern("/\\*.*\\*/");
        QRegularExpressionMatchIterator multiIt =
            multiCommentPattern.globalMatch(text);
        while (multiIt.hasNext()) {
            QRegularExpressionMatch match = multiIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeCommentFormat);
        }

        // Numbers
        QRegularExpression numberPattern("\\b\\d+(\\.\\d+)?\\b");
        QRegularExpressionMatchIterator numIt = numberPattern.globalMatch(text);
        while (numIt.hasNext()) {
            QRegularExpressionMatch match = numIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeNumberFormat);
        }

        // Function calls
        QRegularExpression funcPattern(
            "\\b([a-zA-Z_$][a-zA-Z0-9_$]*)\\s*(?=\\()");
        QRegularExpressionMatchIterator funcIt = funcPattern.globalMatch(text);
        while (funcIt.hasNext()) {
            QRegularExpressionMatch match = funcIt.next();
            setFormat(match.capturedStart(1), match.capturedLength(1),
                      codeFunctionFormat);
        }
    }
    // C/C++ syntax highlighting
    else if (language == "c" || language == "cpp" || language == "c++" ||
             language == "cc") {
        QStringList keywords = {
            "\\bif\\b",       "\\belse\\b",     "\\bfor\\b",
            "\\bwhile\\b",    "\\bdo\\b",       "\\breturn\\b",
            "\\bbreak\\b",    "\\bcontinue\\b", "\\bswitch\\b",
            "\\bcase\\b",     "\\bdefault\\b",  "\\btrue\\b",
            "\\bfalse\\b",    "\\bnullptr\\b",  "\\bNULL\\b",
            "\\bconst\\b",    "\\bstatic\\b",   "\\bextern\\b",
            "\\bvolatile\\b", "\\bregister\\b", "\\bauto\\b",
            "\\btypedef\\b",  "\\bstruct\\b",   "\\bunion\\b",
            "\\benum\\b",     "\\bclass\\b",    "\\bnamespace\\b",
            "\\busing\\b",    "\\btemplate\\b", "\\btypename\\b",
            "\\bpublic\\b",   "\\bprivate\\b",  "\\bprotected\\b",
            "\\bvirtual\\b",  "\\binline\\b",   "\\btry\\b",
            "\\bcatch\\b",    "\\bthrow\\b",    "\\bnew\\b",
            "\\bdelete\\b"};

        QStringList types = {"\\bint\\b",    "\\bchar\\b",     "\\bfloat\\b",
                             "\\bdouble\\b", "\\bvoid\\b",     "\\blong\\b",
                             "\\bshort\\b",  "\\bunsigned\\b", "\\bsigned\\b",
                             "\\bbool\\b",   "\\bsize_t\\b",   "\\bwchar_t\\b"};

        for (const QString& pattern : keywords) {
            QRegularExpression re(pattern);
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          codeKeywordFormat);
            }
        }

        for (const QString& pattern : types) {
            QRegularExpression re(pattern);
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          codeTypeFormat);
            }
        }

        // Strings
        QRegularExpression stringPattern("(\"[^\"]*\"|'[^']*')");
        QRegularExpressionMatchIterator stringIt =
            stringPattern.globalMatch(text);
        while (stringIt.hasNext()) {
            QRegularExpressionMatch match = stringIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeStringFormat);
        }

        // Single-line comments
        int commentStart = text.indexOf("//");
        if (commentStart >= 0) {
            setFormat(commentStart, text.length() - commentStart,
                      codeCommentFormat);
        }

        // Preprocessor directives
        if (text.trimmed().startsWith("#")) {
            setFormat(0, text.length(), codeTypeFormat);
        }

        // Numbers
        QRegularExpression numberPattern("\\b\\d+(\\.\\d+)?[fFuUlL]*\\b");
        QRegularExpressionMatchIterator numIt = numberPattern.globalMatch(text);
        while (numIt.hasNext()) {
            QRegularExpressionMatch match = numIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeNumberFormat);
        }

        // Function calls
        QRegularExpression funcPattern(
            "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?=\\()");
        QRegularExpressionMatchIterator funcIt = funcPattern.globalMatch(text);
        while (funcIt.hasNext()) {
            QRegularExpressionMatch match = funcIt.next();
            setFormat(match.capturedStart(1), match.capturedLength(1),
                      codeFunctionFormat);
        }
    }
    // Java syntax highlighting
    else if (language == "java") {
        QStringList keywords = {
            "\\bif\\b",       "\\belse\\b",         "\\bfor\\b",
            "\\bwhile\\b",    "\\bdo\\b",           "\\breturn\\b",
            "\\bbreak\\b",    "\\bcontinue\\b",     "\\bswitch\\b",
            "\\bcase\\b",     "\\bdefault\\b",      "\\btrue\\b",
            "\\bfalse\\b",    "\\bnull\\b",         "\\bpublic\\b",
            "\\bprivate\\b",  "\\bprotected\\b",    "\\bstatic\\b",
            "\\bfinal\\b",    "\\bclass\\b",        "\\binterface\\b",
            "\\bextends\\b",  "\\bimplements\\b",   "\\bnew\\b",
            "\\bthis\\b",     "\\bsuper\\b",        "\\bpackage\\b",
            "\\bimport\\b",   "\\btry\\b",          "\\bcatch\\b",
            "\\bfinally\\b",  "\\bthrow\\b",        "\\bthrows\\b",
            "\\babstract\\b", "\\bsynchronized\\b", "\\bvolatile\\b",
            "\\btransient\\b"};

        QStringList types = {"\\bint\\b",    "\\bchar\\b",    "\\bfloat\\b",
                             "\\bdouble\\b", "\\bvoid\\b",    "\\blong\\b",
                             "\\bshort\\b",  "\\bbyte\\b",    "\\bboolean\\b",
                             "\\bString\\b", "\\bInteger\\b", "\\bBoolean\\b",
                             "\\bDouble\\b"};

        for (const QString& pattern : keywords) {
            QRegularExpression re(pattern);
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          codeKeywordFormat);
            }
        }

        for (const QString& pattern : types) {
            QRegularExpression re(pattern);
            QRegularExpressionMatchIterator it = re.globalMatch(text);
            while (it.hasNext()) {
                QRegularExpressionMatch match = it.next();
                setFormat(match.capturedStart(), match.capturedLength(),
                          codeTypeFormat);
            }
        }

        // Strings
        QRegularExpression stringPattern("(\"[^\"]*\"|'[^']*')");
        QRegularExpressionMatchIterator stringIt =
            stringPattern.globalMatch(text);
        while (stringIt.hasNext()) {
            QRegularExpressionMatch match = stringIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeStringFormat);
        }

        // Single-line comments
        int commentStart = text.indexOf("//");
        if (commentStart >= 0) {
            setFormat(commentStart, text.length() - commentStart,
                      codeCommentFormat);
        }

        // Numbers
        QRegularExpression numberPattern("\\b\\d+(\\.\\d+)?[fFdDlL]*\\b");
        QRegularExpressionMatchIterator numIt = numberPattern.globalMatch(text);
        while (numIt.hasNext()) {
            QRegularExpressionMatch match = numIt.next();
            setFormat(match.capturedStart(), match.capturedLength(),
                      codeNumberFormat);
        }

        // Function/method calls
        QRegularExpression funcPattern(
            "\\b([a-zA-Z_][a-zA-Z0-9_]*)\\s*(?=\\()");
        QRegularExpressionMatchIterator funcIt = funcPattern.globalMatch(text);
        while (funcIt.hasNext()) {
            QRegularExpressionMatch match = funcIt.next();
            setFormat(match.capturedStart(1), match.capturedLength(1),
                      codeFunctionFormat);
        }
    }
    // Add more languages as needed
}
