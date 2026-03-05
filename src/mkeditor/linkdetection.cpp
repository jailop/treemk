#include "markdowneditor.h"

#include <QRegularExpression>
#include <QTextBlock>
#include <QTextCursor>

#include "regexpatterns.h"
#include "regexutils.h"

QString MarkdownEditor::getLinkAtPosition(int position) const {
    QString dummy;
    return getLinkAtPosition(position, dummy);
}

QString MarkdownEditor::getLinkAtPosition(int position, QString& displayText) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<RegexUtils::WikiLinkInfo> links = RegexUtils::parseWikiLinks(line);
    for (const auto& link : links) {
        int start = link.position;
        int end = start + link.length;

        if (posInBlock >= start && posInBlock <= end) {
            displayText = link.display;
            return link.target;
        }
    }

    displayText.clear();
    return QString();
}

QString MarkdownEditor::getExternalLinkAtPosition(int position) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<RegexUtils::MarkdownLinkInfo> links = RegexUtils::parseMarkdownLinks(line);
    for (const auto& link : links) {
        int start = link.position;
        int end = start + link.length;

        if (posInBlock >= start && posInBlock <= end) {
            if (link.url.startsWith("http://") || link.url.startsWith("https://")) {
                return link.url;
            }
        }
    }

    QRegularExpression urlPattern(RegexPatterns::URL);
    QRegularExpressionMatchIterator urlIterator = urlPattern.globalMatch(line);

    while (urlIterator.hasNext()) {
        QRegularExpressionMatch match = urlIterator.next();
        int start = match.capturedStart();
        int end = start + match.capturedLength();

        if (posInBlock >= start && posInBlock < end) {
            return match.captured(1);
        }
    }

    return QString();
}

QString MarkdownEditor::getMarkdownLinkAtPosition(int position) const {
    QString dummy;
    return getMarkdownLinkAtPosition(position, dummy);
}

QString MarkdownEditor::getMarkdownLinkAtPosition(int position, QString& labelText) const {
    QTextCursor cursor(document());
    cursor.setPosition(position);

    QString line = cursor.block().text();
    int posInBlock = cursor.positionInBlock();

    QVector<RegexUtils::MarkdownLinkInfo> links = RegexUtils::parseMarkdownLinks(line);
    for (const auto& link : links) {
        int start = link.position;
        int end = start + link.length;

        if (posInBlock >= start && posInBlock <= end) {
            if (!link.url.startsWith("http://") && !link.url.startsWith("https://")) {
                labelText = link.text;
                return link.url;
            }
        }
    }

    labelText.clear();
    return QString();
}
