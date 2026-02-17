#include "tocgenerator.h"
#include "regexpatterns.h"
#include <QRegularExpression>
#include <QTextStream>
#include <QCoreApplication>

QString TocGenerator::generateToc(const QString& markdownText) {
    QString content = removeToc(markdownText);
    QList<TocEntry> entries = extractHeaders(content);
    
    if (entries.isEmpty()) {
        return content;
    }
    
    QString toc = tocToMarkdown(entries);
    int insertPos = findTocPosition(content);
    
    content.insert(insertPos, toc);
    
    return content;
}

QString TocGenerator::removeToc(const QString& markdownText) {
    QString result = markdownText;
    
    QRegularExpression tocPattern1(
        RegexPatterns::TOC_HEADER_TABLE,
        QRegularExpression::MultilineOption
    );
    result.replace(tocPattern1, "");
    
    QRegularExpression tocPattern2(
        RegexPatterns::TOC_HEADER_CONTENTS,
        QRegularExpression::MultilineOption
    );
    result.replace(tocPattern2, "");
    
    QRegularExpression tocPattern3(
        RegexPatterns::TOC_HTML_COMMENT,
        QRegularExpression::DotMatchesEverythingOption
    );
    result.replace(tocPattern3, "");
    
    return result.trimmed() + "\n";
}

bool TocGenerator::hasToc(const QString& markdownText) {
    QRegularExpression tocPattern1(
        RegexPatterns::TOC_DETECT_HEADER,
        QRegularExpression::MultilineOption
    );
    
    if (tocPattern1.match(markdownText).hasMatch()) {
        return true;
    }
    
    QRegularExpression tocPattern2(RegexPatterns::TOC_DETECT_COMMENT);
    return tocPattern2.match(markdownText).hasMatch();
}

QList<TocGenerator::TocEntry> TocGenerator::extractHeaders(const QString& markdownText) {
    QList<TocEntry> entries;
    QStringList lines = markdownText.split('\n');
    
    bool firstLine = true;
    bool skipFirstH1 = false;
    
    for (const QString& line : lines) {
        if (firstLine && !line.trimmed().isEmpty()) {
            if (line.trimmed().startsWith("# ")) {
                skipFirstH1 = true;
            }
            firstLine = false;
        }
        
        QRegularExpression headerPattern(RegexPatterns::HEADER_ANY);
        QRegularExpressionMatch match = headerPattern.match(line);
        
        if (match.hasMatch()) {
            QString hashes = match.captured(1);
            QString text = match.captured(2).trimmed();
            int level = hashes.length();
            
            if (skipFirstH1 && level == 1) {
                skipFirstH1 = false;
                continue;
            }
            
            if (!text.isEmpty() && level >= 1 && level <= 6) {
                TocEntry entry;
                entry.level = level;
                entry.text = text;
                entry.anchor = generateAnchor(text);
                entries.append(entry);
            }
        }
    }
    
    return entries;
}

QString TocGenerator::generateAnchor(const QString& headerText) {
    QString anchor = headerText.toLower();
    
    anchor.replace(QRegularExpression("[^a-z0-9\\s-]"), "");
    anchor.replace(QRegularExpression("\\s+"), "-");
    anchor.replace(QRegularExpression("-+"), "-");
    anchor = anchor.trimmed();
    
    if (anchor.startsWith('-')) {
        anchor = anchor.mid(1);
    }
    if (anchor.endsWith('-')) {
        anchor.chop(1);
    }
    
    return anchor;
}

int TocGenerator::findTocPosition(const QString& markdownText) {
    QStringList lines = markdownText.split('\n');
    
    if (lines.isEmpty()) {
        return 0;
    }
    
    QString firstLine = lines.first().trimmed();
    if (firstLine.startsWith("# ")) {
        if (lines.size() > 1) {
            int pos = lines.first().length() + 1;
            
            while (pos < markdownText.length() && 
                   markdownText[pos] == '\n') {
                pos++;
            }
            
            return pos;
        }
        return markdownText.length();
    }
    
    return 0;
}

QString TocGenerator::tocToMarkdown(const QList<TocEntry>& entries) {
    if (entries.isEmpty()) {
        return QString();
    }
    
    QString toc = QString("## %1\n\n").arg(QCoreApplication::translate("TocGenerator", "Table of Contents"));
    
    int minLevel = 6;
    for (const TocEntry& entry : entries) {
        if (entry.level < minLevel) {
            minLevel = entry.level;
        }
    }
    
    for (const TocEntry& entry : entries) {
        int indent = (entry.level - minLevel) * 2;
        QString spaces = QString(indent, ' ');
        toc += QString("%1- [%2](#%3)\n")
                   .arg(spaces)
                   .arg(entry.text)
                   .arg(entry.anchor);
    }
    
    toc += "\n";
    
    return toc;
}
