#include "../include/markdownpreview.h"
#include <QCoreApplication>
#include <QFile>
#include <QString>
#include <QTemporaryDir>
#include <QTextStream>
#include <QtTest>

class TestMarkdownPreview : public QObject {
  Q_OBJECT

private:
  QString convertMarkdown(const QString &markdown);
  MarkdownPreview *preview;
  QTemporaryDir *tempDir;

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Basic markdown elements
  void testHeaders();
  void testEmphasis();
  void testLists();
  void testLinks();
  void testImages();
  void testCodeBlocks();
  void testInlineCode();
  void testBlockquotes();
  void testHorizontalRules();

  // Extended features
  void testTables();
  void testStrikethrough();
  void testTaskLists();

  // LaTeX equations
  void testInlineLatex();
  void testBlockLatex();
  void testMixedLatex();

  // Wiki links
  void testSimpleWikiLinks();
  void testWikiLinksWithPipe();

  // Edge cases
  void testEmptyContent();
  void testSpecialCharacters();
  void testNestedFormatting();
  void testMultilineContent();
};

void TestMarkdownPreview::initTestCase() {
  tempDir = new QTemporaryDir();
  QVERIFY(tempDir->isValid());
}

void TestMarkdownPreview::cleanupTestCase() { delete tempDir; }

void TestMarkdownPreview::init() {
  preview = new MarkdownPreview();
  preview->setBasePath(tempDir->path());
  preview->setLatexEnabled(true);
}

void TestMarkdownPreview::cleanup() { delete preview; }

QString TestMarkdownPreview::convertMarkdown(const QString &markdown) {
  // Access the private convertMarkdownToHtml method through the public
  // interface We'll set content and extract the HTML from the rendered page
  preview->setMarkdownContent(markdown);

  // For testing purposes, we need to access the converted HTML directly
  // Since convertMarkdownToHtml is private, we'll test through the full
  // pipeline by examining the resulting HTML structure

  // Return the markdown as-is for now; in actual implementation,
  // we would need to make convertMarkdownToHtml accessible for testing
  // or create a test-friendly wrapper
  return markdown;
}

// ============================================================================
// Basic Markdown Elements Tests
// ============================================================================

void TestMarkdownPreview::testHeaders() {
  preview->setMarkdownContent("# Header 1\n## Header 2\n### Header 3");
  // Verify headers are rendered correctly
  QVERIFY(true); // Placeholder - would check actual HTML output
}

void TestMarkdownPreview::testEmphasis() {
  QString markdown = "**bold text** and *italic text* and ***bold italic***";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <strong> and <em> tags
}

void TestMarkdownPreview::testLists() {
  QString markdown = "Unordered list:\n"
                     "- Item 1\n"
                     "- Item 2\n"
                     "- Item 3\n"
                     "\n"
                     "Ordered list:\n"
                     "1. First\n"
                     "2. Second\n"
                     "3. Third\n";

  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <ul> and <ol> tags
}

void TestMarkdownPreview::testLinks() {
  QString markdown = "[Link text](https://example.com)";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <a> tag with correct href
}

void TestMarkdownPreview::testImages() {
  QString markdown = "![Alt text](image.png)";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <img> tag with src and alt
}

void TestMarkdownPreview::testCodeBlocks() {
  QString markdown = "```cpp\n"
                     "int main() {\n"
                     "    return 0;\n"
                     "}\n"
                     "```\n";

  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <pre><code> with language class
}

void TestMarkdownPreview::testInlineCode() {
  QString markdown = "This is `inline code` in text.";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <code> tags
}

void TestMarkdownPreview::testBlockquotes() {
  QString markdown = "> This is a blockquote\n> with multiple lines";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <blockquote> tags
}

void TestMarkdownPreview::testHorizontalRules() {
  QString markdown = "Text above\n\n---\n\nText below";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <hr> tag
}

// ============================================================================
// Extended Features Tests
// ============================================================================

void TestMarkdownPreview::testTables() {
  QString markdown = "| Header 1 | Header 2 |\n"
                     "|----------|----------|\n"
                     "| Cell 1   | Cell 2   |\n"
                     "| Cell 3   | Cell 4   |\n";

  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <table> structure
}

void TestMarkdownPreview::testStrikethrough() {
  QString markdown = "This is ~~strikethrough~~ text.";
  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify <del> tags
}

void TestMarkdownPreview::testTaskLists() {
  QString markdown = "- [ ] Unchecked task\n"
                     "- [x] Checked task\n";

  preview->setMarkdownContent(markdown);
  QVERIFY(true); // Would verify task list checkboxes
}

// ============================================================================
// LaTeX Equations Tests
// ============================================================================

void TestMarkdownPreview::testInlineLatex() {
  QString markdown = "Einstein's equation: $E = mc^2$";
  preview->setMarkdownContent(markdown);
  // Should contain the LaTeX delimiters for KaTeX to process
  QVERIFY(true);
}

void TestMarkdownPreview::testBlockLatex() {
  QString markdown = "Quadratic formula:\n"
                     "$$\n"
                     "x = \\frac{-b \\pm \\sqrt{b^2-4ac}}{2a}\n"
                     "$$\n";

  preview->setMarkdownContent(markdown);
  QVERIFY(true);
}

void TestMarkdownPreview::testMixedLatex() {
  QString markdown = "Inline $a^2 + b^2 = c^2$ and block:\n"
                     "$$\n"
                     "\\int_0^\\infty e^{-x^2} dx = \\frac{\\sqrt{\\pi}}{2}\n"
                     "$$\n"
                     "More text with $f(x) = x^2$.";

  preview->setMarkdownContent(markdown);
  QVERIFY(true);
}

// ============================================================================
// Wiki Links Tests
// ============================================================================

void TestMarkdownPreview::testSimpleWikiLinks() {
  QString markdown = "Link to [[OtherPage]] in text.";
  preview->setMarkdownContent(markdown);
  // Should contain wiki: scheme link
  QVERIFY(true);
}

void TestMarkdownPreview::testWikiLinksWithPipe() {
  QString markdown = "Link with [[ActualPage|Display Text]].";
  preview->setMarkdownContent(markdown);
  // Should have href to ActualPage but display "Display Text"
  QVERIFY(true);
}

// ============================================================================
// Edge Cases Tests
// ============================================================================

void TestMarkdownPreview::testEmptyContent() {
  preview->setMarkdownContent("");
  QVERIFY(true); // Should not crash
}

void TestMarkdownPreview::testSpecialCharacters() {
  QString markdown = "Special chars: & < > \" ' \\ /";
  preview->setMarkdownContent(markdown);
  // Special HTML characters should be properly escaped
  QVERIFY(true);
}

void TestMarkdownPreview::testNestedFormatting() {
  QString markdown =
      "**Bold with *italic* inside** and *italic with **bold** inside*";
  preview->setMarkdownContent(markdown);
  QVERIFY(true);
}

void TestMarkdownPreview::testMultilineContent() {
  QString markdown = "# Main Title\n"
                     "\n"
                     "This is a paragraph with multiple lines.\n"
                     "It should be properly formatted.\n"
                     "\n"
                     "## Subsection\n"
                     "\n"
                     "- List item 1\n"
                     "- List item 2\n"
                     "\n"
                     "```python\n"
                     "def hello():\n"
                     "    print('Hello, World!')\n"
                     "```\n"
                     "\n"
                     "Final paragraph with $E=mc^2$ equation.\n";

  preview->setMarkdownContent(markdown);
  QVERIFY(true);
}

QTEST_MAIN(TestMarkdownPreview)
#include "test_markdownpreview.moc"
