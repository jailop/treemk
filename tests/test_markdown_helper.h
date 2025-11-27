#ifndef TEST_MARKDOWN_HELPER_H
#define TEST_MARKDOWN_HELPER_H

#include <QString>
#include <md4c-html.h>

/**
 * Helper class to convert markdown to HTML using md4c
 * This mirrors the logic in MarkdownPreview::convertMarkdownToHtml
 * for testing purposes
 */
class MarkdownTestHelper {
public:
  static QString convertToHtml(const QString &markdown);
  static QString processLatexFormulas(const QString &html);
  static QString processWikiLinks(const QString &html);

private:
  struct OutputBuffer {
    QString html;
  };

  static void processOutput(const char *data, unsigned int size,
                            void *userdata);
};

#endif // TEST_MARKDOWN_HELPER_H
