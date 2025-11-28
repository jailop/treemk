#include "test_markdown_helper.h"
#include <QString>
#include <QtTest>

class TestMarkdownConversion : public QObject {
  Q_OBJECT

private slots:
  void testHeaders();
  void testEmphasis();
  void testLists();
  void testLinks();
  void testImages();
  void testCodeBlocks();
  void testInlineCode();
  void testBlockquotes();
  void testHorizontalRules();

  void testTables();
  void testStrikethrough();
  void testTaskLists();
  void testAutolinks();

  void testInlineLatex();
  void testBlockLatex();
  void testMixedLatex();

  void testSimpleWikiLinks();
  void testWikiLinksWithPipe();

  void testEmptyContent();
  void testSpecialCharacters();
  void testNestedFormatting();
};

// Basic Markdown Elements Tests

void TestMarkdownConversion::testHeaders() {
  QString markdown = "# Header 1\n## Header 2\n### Header 3\n#### Header "
                     "4\n##### Header 5\n###### Header 6";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<h1>Header 1</h1>"));
  QVERIFY(html.contains("<h2>Header 2</h2>"));
  QVERIFY(html.contains("<h3>Header 3</h3>"));
  QVERIFY(html.contains("<h4>Header 4</h4>"));
  QVERIFY(html.contains("<h5>Header 5</h5>"));
  QVERIFY(html.contains("<h6>Header 6</h6>"));
}

void TestMarkdownConversion::testEmphasis() {
  QString markdown = "**bold text** and *italic text* and ***bold italic***";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<strong>bold text</strong>"));
  QVERIFY(html.contains("<em>italic text</em>"));
  QVERIFY(html.contains("<strong><em>bold italic</em></strong>") ||
          html.contains("<em><strong>bold italic</strong></em>"));
}

void TestMarkdownConversion::testLists() {
  QString markdown = "Unordered:\n\n"
                     "- Item 1\n"
                     "- Item 2\n"
                     "- Item 3\n"
                     "\n"
                     "Ordered:\n\n"
                     "1. First\n"
                     "2. Second\n"
                     "3. Third";

  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<ul>"));
  QVERIFY(html.contains("</ul>"));
  QVERIFY(html.contains("<ol>"));
  QVERIFY(html.contains("</ol>"));
  QVERIFY(html.contains("<li>Item 1</li>"));
  QVERIFY(html.contains("<li>First</li>"));
}

void TestMarkdownConversion::testLinks() {
  QString markdown = "[Link text](https://example.com)";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<a href=\"https://example.com\">Link text</a>"));
}

void TestMarkdownConversion::testImages() {
  QString markdown = "![Alt text](image.png)";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<img src=\"image.png\" alt=\"Alt text\""));
}

void TestMarkdownConversion::testCodeBlocks() {
  QString markdown = "```cpp\n"
                     "int main() {\n"
                     "    return 0;\n"
                     "}\n"
                     "```";

  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<pre><code"));
  QVERIFY(html.contains("class=\"language-cpp\""));
  QVERIFY(html.contains("int main()"));
}

void TestMarkdownConversion::testInlineCode() {
  QString markdown = "This is `inline code` in text.";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<code>inline code</code>"));
}

void TestMarkdownConversion::testBlockquotes() {
  QString markdown = "> This is a blockquote";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<blockquote>"));
  QVERIFY(html.contains("This is a blockquote"));
  QVERIFY(html.contains("</blockquote>"));
}

void TestMarkdownConversion::testHorizontalRules() {
  QString markdown = "Text above\n\n---\n\nText below";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<hr"));
}

// Extended Features Tests

void TestMarkdownConversion::testTables() {
  QString markdown = "| Header 1 | Header 2 |\n"
                     "|----------|----------|\n"
                     "| Cell 1   | Cell 2   |\n"
                     "| Cell 3   | Cell 4   |";

  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<table>"));
  QVERIFY(html.contains("<thead>"));
  QVERIFY(html.contains("<tbody>"));
  QVERIFY(html.contains("<th>Header 1</th>"));
  QVERIFY(html.contains("<td>Cell 1</td>"));
}

void TestMarkdownConversion::testStrikethrough() {
  QString markdown = "This is ~~strikethrough~~ text.";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(html.contains("<del>strikethrough</del>"));
}

void TestMarkdownConversion::testTaskLists() {
  QString markdown = "- [ ] Unchecked task\n"
                     "- [x] Checked task";

  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // md4c outputs task lists with checkboxes
  QVERIFY(html.contains("type=\"checkbox\""));
  QVERIFY(html.contains("Unchecked task"));
  QVERIFY(html.contains("Checked task"));
}

void TestMarkdownConversion::testAutolinks() {
  QString markdown = "Visit https://example.com for more info.";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  QVERIFY(
      html.contains("<a href=\"https://example.com\">https://example.com</a>"));
}

// LaTeX Equations Tests

void TestMarkdownConversion::testInlineLatex() {
  QString markdown = "Einstein's equation: $E = mc^2$";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // md4c outputs <x-equation> tags
  QVERIFY(html.contains("<x-equation>E = mc^2</x-equation>"));

  // After processing
  QString processed = MarkdownTestHelper::processLatexFormulas(html);
  QVERIFY(processed.contains("$E = mc^2$"));
}

void TestMarkdownConversion::testBlockLatex() {
  QString markdown = "Quadratic formula:\n\n"
                     "$$\n"
                     "x = \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}\n"
                     "$$";

  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // Should contain display equation
  QVERIFY(html.contains("<x-equation type=\"display\">"));

  // After processing
  QString processed = MarkdownTestHelper::processLatexFormulas(html);
  QVERIFY(processed.contains("$$") && processed.contains("\\frac"));
}

void TestMarkdownConversion::testMixedLatex() {
  QString markdown = "Inline $a^2 + b^2 = c^2$ and block:\n\n"
                     "$$\n"
                     "\\int_0^\\infty e^{-x^2} dx\n"
                     "$$\n\n"
                     "More text with $f(x) = x^2$.";

  QString html = MarkdownTestHelper::convertToHtml(markdown);
  QString processed = MarkdownTestHelper::processLatexFormulas(html);

  // Should have both inline and display math
  int dollarCount = processed.count('$');
  QVERIFY(dollarCount >= 6); // At least 2 inline (2 each) + 1 display (2)
}

// Wiki Links Tests

void TestMarkdownConversion::testSimpleWikiLinks() {
  QString markdown = "Link to [[OtherPage]] in text.";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // md4c outputs <x-wikilink>
  QVERIFY(html.contains("<x-wikilink"));
  QVERIFY(html.contains("OtherPage"));

  // After processing
  QString processed = MarkdownTestHelper::processWikiLinks(html);
  QVERIFY(processed.contains("wiki:OtherPage"));
  QVERIFY(processed.contains("class=\"wiki-link\""));
}

void TestMarkdownConversion::testWikiLinksWithPipe() {
  QString markdown = "Link with [[ActualPage|Display Text]].";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // After processing
  QString processed = MarkdownTestHelper::processWikiLinks(html);
  QVERIFY(processed.contains("wiki:ActualPage"));
  QVERIFY(processed.contains("Display Text"));
}

// Edge Cases Tests

void TestMarkdownConversion::testEmptyContent() {
  QString markdown = "";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // Should return empty or minimal HTML, not error
  QVERIFY(!html.contains("Error parsing"));
}

void TestMarkdownConversion::testSpecialCharacters() {
  QString markdown = "Special chars: & < >";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // HTML entities should be escaped
  QVERIFY(html.contains("&amp;"));
  QVERIFY(html.contains("&lt;"));
  QVERIFY(html.contains("&gt;"));
}

void TestMarkdownConversion::testNestedFormatting() {
  QString markdown = "**Bold with *italic* inside**";
  QString html = MarkdownTestHelper::convertToHtml(markdown);

  // Should handle nested formatting
  QVERIFY(html.contains("<strong>"));
  QVERIFY(html.contains("<em>"));
}

QTEST_MAIN(TestMarkdownConversion)
#include "test_markdown_conversion.moc"
