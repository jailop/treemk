#ifndef REGEXUTILS_H
#define REGEXUTILS_H

#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QVector>

namespace RegexUtils {

/**
 * Structure to hold parsed wiki link information
 *
 * This structure represents a wiki link, which can be in the format
 * [[target|display]] or [[target]]. It contains the target page, the
 * display text, whether it's an inclusion link, and the position and
 * length of the link in the original text.
 *
 * Example:
 *
 * [[PageName|Display Text]] will have:
 *
 * - target: "PageName"
 * - display: "Display Text"
 * - isInclusion: false
 * - position: (position of the link in the text)
 * - length: (length of the entire link)
 */
struct WikiLinkInfo {
    QString target;
    QString display;
    bool isInclusion;
    int position;
    int length;

    WikiLinkInfo(const QString& t, const QString& d, bool incl, int pos,
                 int len)
        : target(t),
          display(d),
          isInclusion(incl),
          position(pos),
          length(len) {}
};

/**
 * Structure to hold parsed markdown link information
 *
 * This structure represents a markdown link, which can be in the format
 * [text](url) or ![alt text](image.png). It contains the link text,
 * the URL or path, whether it's an image, and the position and length
 * of the link in the original text.
 *
 * Example:
 *
 * [Click here](https://example.com) will have:
 *
 * - text: "Click here"
 * - url: "https://example.com"
 * - isImage: false
 * - position: (position of the link in the text)
 * - length: (length of the entire link)
 */
struct MarkdownLinkInfo {
    QString text;
    QString url;
    bool isImage;
    int position;
    int length;

    MarkdownLinkInfo(const QString& txt, const QString& u, bool img, int pos,
                     int len)
        : text(txt), url(u), isImage(img), position(pos), length(len) {}
};

/**
 * Structure to hold list item information
 *
 * This structure represents a markdown list item, which can be ordered
 * or unordered. It contains the indentation, the marker (bullet or number),
 * the content after the marker, and whether it's an ordered list.
 *
 * Examples:
 *
 * "  - Item text" will have:
 * - indent: "  "
 * - marker: "-"
 * - content: "Item text"
 * - isOrdered: false
 *
 * "1. First item" will have:
 * - indent: ""
 * - marker: "1."
 * - content: "First item"
 * - isOrdered: true
 */
struct ListItemInfo {
    QString indent;
    QString marker;
    QString content;
    bool isOrdered;

    ListItemInfo(const QString& ind, const QString& mark, const QString& cont,
                 bool ordered)
        : indent(ind), marker(mark), content(cont), isOrdered(ordered) {}
};

/**
 * Structure to hold task item information
 *
 * This structure represents a markdown task list item, which includes
 * a checkbox. It contains the indentation, the bullet marker, the state
 * of the checkbox, and the content after the checkbox.
 *
 * Examples:
 *
 * "- [ ] Unchecked task" will have:
 * - indent: ""
 * - marker: "-"
 * - state: " "
 * - content: "Unchecked task"
 *
 * "  * [x] Completed task" will have:
 * - indent: "  "
 * - marker: "*"
 * - state: "x"
 * - content: "Completed task"
 *
 * Note: State can be " " (unchecked), "x"/"X" (checked), or "." (in progress)
 */
struct TaskItemInfo {
    QString indent;
    QString marker;
    QString state;
    QString content;

    TaskItemInfo(const QString& ind, const QString& mark, const QString& st,
                 const QString& cont)
        : indent(ind), marker(mark), state(st), content(cont) {}
};

/**
 * Wiki link parsing functions
 *
 * These functions parse wiki-style links in the format [[target]] or
 * [[target|display]] or [[!target]] for inclusions.
 */
QVector<WikiLinkInfo> parseWikiLinks(const QString& text);
WikiLinkInfo parseWikiLink(const QString& text);
bool isWikiLink(const QString& text);

/**
 * Markdown link parsing functions
 *
 * These functions parse markdown-style links in the format [text](url)
 * or ![alt](image.png) for images.
 */
QVector<MarkdownLinkInfo> parseMarkdownLinks(const QString& text);
MarkdownLinkInfo parseMarkdownLink(const QString& text);
bool isMarkdownLink(const QString& text);

/**
 * List item functions
 *
 * These functions detect and parse markdown list items, both ordered
 * (1. 2. 3.) and unordered (- * +).
 */
bool isListItem(const QString& line);
bool isOrderedListItem(const QString& line);
bool isUnorderedListItem(const QString& line);
ListItemInfo parseListItem(const QString& line);

/**
 * Task item functions
 *
 * These functions detect and parse markdown task list items in the
 * format "- [ ] task" or "- [x] completed task".
 */
bool isTaskItem(const QString& line);
TaskItemInfo parseTaskItem(const QString& line);
QString getTaskState(const QString& line);
bool isTaskChecked(const QString& line);

/**
 * Header functions
 *
 * These functions detect and parse markdown headers (ATX-style) in
 * the format "# Header" through "###### Header".
 */
bool isHeader(const QString& line);
int getHeaderLevel(const QString& line);
QString getHeaderText(const QString& line);

/**
 * Utility functions
 *
 * General-purpose regex matching utilities.
 */
bool matchesPattern(const QString& text, const QString& pattern);
QRegularExpressionMatch getFirstMatch(const QString& text,
                                      const QString& pattern);
QVector<QRegularExpressionMatch> getAllMatches(const QString& text,
                                               const QString& pattern);

}  // namespace RegexUtils

#endif  // REGEXUTILS_H
