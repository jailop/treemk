#include "test_markdown_helper.h"
#include <QRegularExpression>

void MarkdownTestHelper::processOutput(const char *data, unsigned int size,
                                       void *userdata) {
  OutputBuffer *buffer = static_cast<OutputBuffer *>(userdata);
  buffer->html.append(QString::fromUtf8(data, size));
}

QString MarkdownTestHelper::convertToHtml(const QString &markdown) {
  OutputBuffer buffer;
  QByteArray utf8Data = markdown.toUtf8();

  // Configure parser flags (same as MarkdownPreview)
  unsigned parserFlags = MD_FLAG_TABLES | MD_FLAG_STRIKETHROUGH |
                         MD_FLAG_TASKLISTS | MD_FLAG_LATEXMATHSPANS |
                         MD_FLAG_WIKILINKS | MD_FLAG_PERMISSIVEURLAUTOLINKS |
                         MD_FLAG_PERMISSIVEEMAILAUTOLINKS |
                         MD_FLAG_PERMISSIVEWWWAUTOLINKS;

  unsigned rendererFlags = 0;

  int result = md_html(utf8Data.constData(), utf8Data.size(), processOutput,
                       &buffer, parserFlags, rendererFlags);

  if (result != 0) {
    return "<p style=\"color: red;\">Error parsing markdown</p>";
  }

  return buffer.html;
}

QString MarkdownTestHelper::processLatexFormulas(const QString &html) {
  QString result = html;

  // Convert display (block) equations to $$...$$
  result.replace(
      QRegularExpression("<x-equation type=\"display\">([^<]*)</x-equation>"),
      "$$\\1$$");

  // Convert inline equations to $...$
  result.replace(QRegularExpression("<x-equation>([^<]*)</x-equation>"),
                 "$\\1$");

  return result;
}

QString MarkdownTestHelper::processWikiLinks(const QString &html) {
  QString result = html;

  // Convert md4c wiki links to custom format
  result.replace(
      QRegularExpression(
          "<x-wikilink data-target=\"([^\"]+)\">([^<]+)</x-wikilink>"),
      "<a href=\"wiki:\\1\" class=\"wiki-link\">\\2</a>");

  return result;
}
