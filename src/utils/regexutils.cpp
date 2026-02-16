#include "regexutils.h"

#include "regexpatterns.h"

namespace RegexUtils {

QVector<WikiLinkInfo> parseWikiLinks(const QString& text) {
    QVector<WikiLinkInfo> links;
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    QRegularExpressionMatchIterator it = pattern.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        bool isInclusion = !match.captured(1).isEmpty();
        QString target = match.captured(2).trimmed();
        QString display = match.captured(4).trimmed();

        if (display.isEmpty()) {
            display = target;
        }

        links.append(WikiLinkInfo(target, display, isInclusion,
                                  match.capturedStart(),
                                  match.capturedLength()));
    }

    return links;
}

WikiLinkInfo parseWikiLink(const QString& text) {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    QRegularExpressionMatch match = pattern.match(text);

    if (match.hasMatch()) {
        bool isInclusion = !match.captured(1).isEmpty();
        QString target = match.captured(2).trimmed();
        QString display = match.captured(4).trimmed();

        if (display.isEmpty()) {
            display = target;
        }

        return WikiLinkInfo(target, display, isInclusion, match.capturedStart(),
                            match.capturedLength());
    }

    return WikiLinkInfo("", "", false, -1, 0);
}

bool isWikiLink(const QString& text) {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    return pattern.match(text).hasMatch();
}

QVector<MarkdownLinkInfo> parseMarkdownLinks(const QString& text) {
    QVector<MarkdownLinkInfo> links;
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK_WITH_IMAGE);
    QRegularExpressionMatchIterator it = pattern.globalMatch(text);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        bool isImage = !match.captured(1).isEmpty();
        QString linkText = match.captured(2).trimmed();
        QString url = match.captured(3).trimmed();

        links.append(MarkdownLinkInfo(linkText, url, isImage,
                                      match.capturedStart(),
                                      match.capturedLength()));
    }

    return links;
}

MarkdownLinkInfo parseMarkdownLink(const QString& text) {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK_WITH_IMAGE);
    QRegularExpressionMatch match = pattern.match(text);

    if (match.hasMatch()) {
        bool isImage = !match.captured(1).isEmpty();
        QString linkText = match.captured(2).trimmed();
        QString url = match.captured(3).trimmed();

        return MarkdownLinkInfo(linkText, url, isImage, match.capturedStart(),
                                match.capturedLength());
    }

    return MarkdownLinkInfo("", "", false, -1, 0);
}

bool isMarkdownLink(const QString& text) {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK);
    return pattern.match(text).hasMatch();
}

bool isListItem(const QString& line) {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM);
    return pattern.match(line).hasMatch();
}

bool isOrderedListItem(const QString& line) {
    QRegularExpression pattern(RegexPatterns::ORDERED_LIST);
    return pattern.match(line).hasMatch();
}

bool isUnorderedListItem(const QString& line) {
    QRegularExpression pattern(RegexPatterns::UNORDERED_LIST);
    return pattern.match(line).hasMatch();
}

ListItemInfo parseListItem(const QString& line) {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM);
    QRegularExpressionMatch match = pattern.match(line);

    if (match.hasMatch()) {
        QString indent = match.captured(1);
        QString marker = match.captured(2);
        QString content = line.mid(match.capturedEnd()).trimmed();
        bool ordered = marker.contains(QRegularExpression("^[0-9]+\\.$"));

        return ListItemInfo(indent, marker, content, ordered);
    }

    return ListItemInfo("", "", "", false);
}

bool isTaskItem(const QString& line) {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM);
    return pattern.match(line).hasMatch();
}

TaskItemInfo parseTaskItem(const QString& line) {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM);
    QRegularExpressionMatch match = pattern.match(line);

    if (match.hasMatch()) {
        QString indent = match.captured(1);
        QString marker = match.captured(2);
        QString state = match.captured(3);
        QString content = line.mid(match.capturedEnd()).trimmed();

        return TaskItemInfo(indent, marker, state, content);
    }

    return TaskItemInfo("", "", "", "");
}

QString getTaskState(const QString& line) {
    QRegularExpression pattern(RegexPatterns::TASK_CHECKBOX);
    QRegularExpressionMatch match = pattern.match(line);

    if (match.hasMatch()) {
        return match.captured(1);
    }

    return "";
}

bool isTaskChecked(const QString& line) {
    QString state = getTaskState(line);
    return state == "x" || state == "X" || state == ".";
}

bool isHeader(const QString& line) {
    QRegularExpression pattern(RegexPatterns::HEADER);
    return pattern.match(line).hasMatch();
}

int getHeaderLevel(const QString& line) {
    QRegularExpression pattern(RegexPatterns::HEADER);
    QRegularExpressionMatch match = pattern.match(line);

    if (match.hasMatch()) {
        return match.captured(1).length();
    }

    return 0;
}

QString getHeaderText(const QString& line) {
    QRegularExpression pattern(RegexPatterns::HEADER);
    QRegularExpressionMatch match = pattern.match(line);

    if (match.hasMatch()) {
        return match.captured(2).trimmed();
    }

    return "";
}

bool matchesPattern(const QString& text, const QString& pattern) {
    QRegularExpression regex(pattern);
    return regex.match(text).hasMatch();
}

QRegularExpressionMatch getFirstMatch(const QString& text,
                                      const QString& pattern) {
    QRegularExpression regex(pattern);
    return regex.match(text);
}

QVector<QRegularExpressionMatch> getAllMatches(const QString& text,
                                               const QString& pattern) {
    QVector<QRegularExpressionMatch> matches;
    QRegularExpression regex(pattern);
    QRegularExpressionMatchIterator it = regex.globalMatch(text);

    while (it.hasNext()) {
        matches.append(it.next());
    }

    return matches;
}

}  // namespace RegexUtils
