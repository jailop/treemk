#include <QtTest/QtTest>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include "regexpatterns.h"

class TestRegexPatterns : public QObject {
    Q_OBJECT
    
private slots:
    void testWikiLink_SimpleTarget();
    void testWikiLink_WithSpaces();
    void testWikiLink_WithForwardSlashes();
    void testWikiLink_WithBackslashes();
    void testWikiLink_WithPipeDisplay();
    void testWikiLink_WithInclusion();
    void testWikiLink_WithInclusionAndDisplay();
    void testWikiLink_WindowsFullPath();
    void testWikiLink_RelativePath();
    void testWikiLink_ComplexPaths();
    void testWikiLink_InvalidCases();
    
    void testMarkdownLink_Simple();
    void testMarkdownLink_WithSpaces();
    void testMarkdownLink_WithPaths();
    void testMarkdownLink_WithURL();
    void testMarkdownLink_WindowsPath();
    void testMarkdownLink_WithImage();
    
    void testListItem_Unordered();
    void testListItem_Ordered();
    void testListItem_WithIndentation();
    void testListItem_EdgeCases();
    
    void testTaskItem_Unchecked();
    void testTaskItem_Checked();
    void testTaskItem_InProgress();
    void testTaskItem_WithIndentation();
    
    void testHeaders_AllLevels();
    void testCodeFence_WithLanguage();
    void testLatex_Inline();
    void testLatex_Block();
    
    void testURL_ValidURLs();
    void testInclusionPattern_Basic();
    void testInclusionPattern_WithDisplay();
    
    void testListItemNoPlusSign();
    void testUnorderedList_AllMarkers();
    void testOrderedList_Numbers();
    void testListMarkerOnly();
    
    void testTaskItemShort();
    void testTaskCheckbox_AllStates();
    void testTaskCheckboxChecked();
    
    void testHeaderPrefix_AllLevels();
    void testHeaderH1_Specific();
    void testHeaderAny_MultipleLines();
    
    void testInlineCode_Basic();
    void testCodeBlockFence();
    void testQuote_SingleAndMultiple();
    void testHorizontalRule_AllStyles();
    
    void testStrikethrough_Content();
    void testStrikethroughMarker();
    
    void testNumberPattern_Integers();
    void testNumberPattern_Floats();
    void testNumberC_WithSuffixes();
    void testNumberJava_WithSuffixes();
    
    void testFunctionCall_Basic();
    void testFunctionCallJS_DollarSign();
    
    void testStringPattern_Single();
    void testStringPattern_Double();
    void testStringPatternBacktick();
    
    void testMultilineComment();
    void testWordBoundary();
    
    void testNewlineBetweenText();
    void testFilenameInvalidChars();
    void testImageFilename();
    void testWhitespaceMultiple();
    void testNonWordChars();
    void testLeadingTrailingDash();
    
    void testHTMLHeading_AllLevels();
    void testHTMLAnchor_Basic();
    void testHTMLCode_WithAttributes();
    void testLatexDisplayEquation();
    void testLatexInlineEquation();
    void testMD4CWikilink_Basic();
    void testMD4CWikilinkInclusion();
    void testTocHeaderTable();
    void testTocHeaderContents();
    void testTocHtmlComment();
    void testTocDetectHeader();
    void testTocDetectComment();
};

// Wiki Link Tests

void TestRegexPatterns::testWikiLink_SimpleTarget() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[simple]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(""));  // No inclusion marker
    QCOMPARE(match.captured(2), QString("simple"));  // Target
    QCOMPARE(match.captured(4), QString(""));  // No display
    
    match = pattern.match("[[document.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("document.md"));
}

void TestRegexPatterns::testWikiLink_WithSpaces() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[My Document.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("My Document.md"));
    
    match = pattern.match("[[Document with multiple spaces.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("Document with multiple spaces.md"));
}

void TestRegexPatterns::testWikiLink_WithForwardSlashes() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[folder/document.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("folder/document.md"));
    
    match = pattern.match("[[../relative/path.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("../relative/path.md"));
    
    match = pattern.match("[[folder/subfolder/file with spaces.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("folder/subfolder/file with spaces.md"));
}

void TestRegexPatterns::testWikiLink_WithBackslashes() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    // Single backslash (Windows path separator)
    QRegularExpressionMatch match = pattern.match("[[folder\\document.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("folder\\document.md"));
    
    match = pattern.match("[[folder\\subfolder\\file.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("folder\\subfolder\\file.md"));
}

void TestRegexPatterns::testWikiLink_WithPipeDisplay() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[target|Display Text]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("target"));
    QCOMPARE(match.captured(4), QString("Display Text"));
    
    match = pattern.match("[[path/to/file.md|Custom Label]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("path/to/file.md"));
    QCOMPARE(match.captured(4), QString("Custom Label"));
}

void TestRegexPatterns::testWikiLink_WithInclusion() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[!included.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("!"));  // Inclusion marker
    QCOMPARE(match.captured(2), QString("included.md"));
    
    match = pattern.match("[[!path/to/include.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("!"));
    QCOMPARE(match.captured(2), QString("path/to/include.md"));
}

void TestRegexPatterns::testWikiLink_WithInclusionAndDisplay() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[!file.md|Custom Display]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("!"));
    QCOMPARE(match.captured(2), QString("file.md"));
    QCOMPARE(match.captured(4), QString("Custom Display"));
}

void TestRegexPatterns::testWikiLink_WindowsFullPath() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[C:\\Users\\Documents\\file.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("C:\\Users\\Documents\\file.md"));
    
    match = pattern.match("[[D:\\My Projects\\Project 1\\notes.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("D:\\My Projects\\Project 1\\notes.md"));
}

void TestRegexPatterns::testWikiLink_RelativePath() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    QRegularExpressionMatch match = pattern.match("[[./local.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("./local.md"));
    
    match = pattern.match("[[../../parent/sibling.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("../../parent/sibling.md"));
}

void TestRegexPatterns::testWikiLink_ComplexPaths() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    // Mix of special characters
    QRegularExpressionMatch match = pattern.match("[[folder-name/sub_folder/file.name.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("folder-name/sub_folder/file.name.md"));
    
    // Dots in filename
    match = pattern.match("[[file.v1.2.3.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("file.v1.2.3.md"));
}

void TestRegexPatterns::testWikiLink_InvalidCases() {
    QRegularExpression pattern(RegexPatterns::WIKI_LINK);
    
    // Missing closing brackets
    QVERIFY(!pattern.match("[[incomplete").hasMatch());
    
    // Only one bracket
    QVERIFY(!pattern.match("[single]").hasMatch());
    
    // Empty target
    QVERIFY(!pattern.match("[[]]").hasMatch());
    
    // Only pipe
    QVERIFY(!pattern.match("[[|display]]").hasMatch());
}

// Markdown Link Tests

void TestRegexPatterns::testMarkdownLink_Simple() {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK);
    
    QRegularExpressionMatch match = pattern.match("[text](url)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("text"));
    QCOMPARE(match.captured(2), QString("url"));
}

void TestRegexPatterns::testMarkdownLink_WithSpaces() {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK);
    
    QRegularExpressionMatch match = pattern.match("[Link Text](file name.md)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("Link Text"));
    QCOMPARE(match.captured(2), QString("file name.md"));
}

void TestRegexPatterns::testMarkdownLink_WithPaths() {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK);
    
    QRegularExpressionMatch match = pattern.match("[Link](folder/file.md)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("folder/file.md"));
    
    match = pattern.match("[Link](../relative/path.md)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("../relative/path.md"));
}

void TestRegexPatterns::testMarkdownLink_WithURL() {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK);
    
    QRegularExpressionMatch match = pattern.match("[Link](https://example.com)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("https://example.com"));
    
    match = pattern.match("[Link](http://example.com/path?query=value)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("http://example.com/path?query=value"));
}

void TestRegexPatterns::testMarkdownLink_WindowsPath() {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK);
    
    QRegularExpressionMatch match = pattern.match("[Link](C:\\Users\\file.md)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("C:\\Users\\file.md"));
}

void TestRegexPatterns::testMarkdownLink_WithImage() {
    QRegularExpression pattern(RegexPatterns::MARKDOWN_LINK_WITH_IMAGE);
    
    QRegularExpressionMatch match = pattern.match("![Alt Text](image.png)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("!"));  // Image marker
    QCOMPARE(match.captured(2), QString("Alt Text"));
    QCOMPARE(match.captured(3), QString("image.png"));
    
    match = pattern.match("[Link Text](file.md)");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(""));  // No image marker
}

// List Item Tests

void TestRegexPatterns::testListItem_Unordered() {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM);
    
    QRegularExpressionMatch match = pattern.match("- Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("-"));
    
    match = pattern.match("* Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("*"));
    
    match = pattern.match("+ Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("+"));
}

void TestRegexPatterns::testListItem_Ordered() {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM);
    
    QRegularExpressionMatch match = pattern.match("1. Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("1."));
    
    match = pattern.match("42. Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("42."));
}

void TestRegexPatterns::testListItem_WithIndentation() {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM);
    
    QRegularExpressionMatch match = pattern.match("  - Indented");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("  "));  // Whitespace
    QCOMPARE(match.captured(2), QString("-"));
    
    match = pattern.match("    * Double indent");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("    "));
}

void TestRegexPatterns::testListItem_EdgeCases() {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM);
    
    // Must have space after marker
    QVERIFY(!pattern.match("-Item").hasMatch());
    
    // Tab indentation
    QRegularExpressionMatch match = pattern.match("\t- Item");
    QVERIFY(match.hasMatch());
}

// Task Item Tests

void TestRegexPatterns::testTaskItem_Unchecked() {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM);
    
    QRegularExpressionMatch match = pattern.match("- [ ] Task");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("-"));
    QCOMPARE(match.captured(3), QString(" "));
}

void TestRegexPatterns::testTaskItem_Checked() {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM);
    
    QRegularExpressionMatch match = pattern.match("- [x] Completed");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(3), QString("x"));
    
    match = pattern.match("* [X] Done");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(3), QString("X"));
}

void TestRegexPatterns::testTaskItem_InProgress() {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM);
    
    QRegularExpressionMatch match = pattern.match("- [.] In Progress");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(3), QString("."));
}

void TestRegexPatterns::testTaskItem_WithIndentation() {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM);
    
    QRegularExpressionMatch match = pattern.match("  - [ ] Indented task");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("  "));
    QCOMPARE(match.captured(2), QString("-"));
    QCOMPARE(match.captured(3), QString(" "));
}

// Header Tests

void TestRegexPatterns::testHeaders_AllLevels() {
    QRegularExpression pattern(RegexPatterns::HEADER);
    
    for (int level = 1; level <= 6; ++level) {
        QString header = QString("#").repeated(level) + " Header " + QString::number(level);
        QRegularExpressionMatch match = pattern.match(header);
        QVERIFY(match.hasMatch());
        QCOMPARE(match.captured(1), QString("#").repeated(level));
    }
    
    // More than 6 hashes should not match
    QVERIFY(!pattern.match("####### Too Many").hasMatch());
}

// Code Fence Tests

void TestRegexPatterns::testCodeFence_WithLanguage() {
    QRegularExpression pattern(RegexPatterns::CODE_FENCE);
    
    QRegularExpressionMatch match = pattern.match("```cpp");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("cpp"));
    
    match = pattern.match("```python");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("python"));
    
    match = pattern.match("```");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(""));
}

// LaTeX Tests

void TestRegexPatterns::testLatex_Inline() {
    QRegularExpression pattern(RegexPatterns::LATEX_INLINE);
    
    QRegularExpressionMatch match = pattern.match("$E = mc^2$");
    QVERIFY(match.hasMatch());
    
    match = pattern.match("$\\alpha + \\beta$");
    QVERIFY(match.hasMatch());
    
    // Should not match across lines
    QVERIFY(!pattern.match("$start\nend$").hasMatch());
}

void TestRegexPatterns::testLatex_Block() {
    QRegularExpression pattern(RegexPatterns::LATEX_BLOCK);
    
    QRegularExpressionMatch match = pattern.match("$$E = mc^2$$");
    QVERIFY(match.hasMatch());
    
    match = pattern.match("$$\\int_{0}^{\\infty} e^{-x} dx$$");
    QVERIFY(match.hasMatch());
}

// URL Tests

void TestRegexPatterns::testURL_ValidURLs() {
    QRegularExpression pattern(RegexPatterns::URL);
    
    QRegularExpressionMatch match = pattern.match("https://example.com");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(0), QString("https://example.com"));
    
    match = pattern.match("http://example.com/path");
    QVERIFY(match.hasMatch());
    
    match = pattern.match("https://example.com/path?query=value");
    QVERIFY(match.hasMatch());
    
    // Should not match ftp or other protocols
    QVERIFY(!pattern.match("ftp://example.com").hasMatch());
}

// Inclusion Pattern Tests

void TestRegexPatterns::testInclusionPattern_Basic() {
    QRegularExpression pattern(RegexPatterns::INCLUSION_PATTERN);
    
    QRegularExpressionMatch match = pattern.match("[[!include.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("include.md"));
    
    match = pattern.match("[[!folder/file.md]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("folder/file.md"));
}

void TestRegexPatterns::testInclusionPattern_WithDisplay() {
    QRegularExpression pattern(RegexPatterns::INCLUSION_PATTERN);
    
    QRegularExpressionMatch match = pattern.match("[[!file.md|Display]]");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("file.md"));
    QCOMPARE(match.captured(3), QString("Display"));
}

// List Item Variations Tests

void TestRegexPatterns::testListItemNoPlusSign() {
    QRegularExpression pattern(RegexPatterns::LIST_ITEM_NO_PLUS);
    
    QRegularExpressionMatch match = pattern.match("- Item");
    QVERIFY(match.hasMatch());
    
    match = pattern.match("* Item");
    QVERIFY(match.hasMatch());
    
    match = pattern.match("1. Item");
    QVERIFY(match.hasMatch());
    
    // Note: This pattern has \s not \s+, so single space
    QVERIFY(!pattern.match("-Item").hasMatch());
}

void TestRegexPatterns::testUnorderedList_AllMarkers() {
    QRegularExpression pattern(RegexPatterns::UNORDERED_LIST);
    
    QRegularExpressionMatch match = pattern.match("- Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("-"));
    
    match = pattern.match("* Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("*"));
    
    match = pattern.match("+ Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("+"));
    
    // Should not match ordered lists
    QVERIFY(!pattern.match("1. Item").hasMatch());
}

void TestRegexPatterns::testOrderedList_Numbers() {
    QRegularExpression pattern(RegexPatterns::ORDERED_LIST);
    
    QRegularExpressionMatch match = pattern.match("1. Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("1."));
    
    match = pattern.match("99. Item");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("99."));
    
    match = pattern.match("  42. Indented");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("  "));
    QCOMPARE(match.captured(2), QString("42."));
}

void TestRegexPatterns::testListMarkerOnly() {
    QRegularExpression pattern(RegexPatterns::LIST_MARKER_ONLY);
    
    // Empty list markers (no content)
    QVERIFY(pattern.match("- ").hasMatch());
    QVERIFY(pattern.match("*  ").hasMatch());
    QVERIFY(pattern.match("  + ").hasMatch());
    
    // Should not match with content
    QVERIFY(!pattern.match("- Item").hasMatch());
}

// Task Item Variations Tests

void TestRegexPatterns::testTaskItemShort() {
    QRegularExpression pattern(RegexPatterns::TASK_ITEM_SHORT);
    
    QRegularExpressionMatch match = pattern.match("- [ ] Task");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("-"));
    QCOMPARE(match.captured(2), QString(" "));
    
    match = pattern.match("* [x] Done");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("*"));
    QCOMPARE(match.captured(2), QString("x"));
}

void TestRegexPatterns::testTaskCheckbox_AllStates() {
    QRegularExpression pattern(RegexPatterns::TASK_CHECKBOX);
    
    QRegularExpressionMatch match = pattern.match("- [ ] Unchecked");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(" "));
    
    match = pattern.match("- [x] Checked");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("x"));
    
    match = pattern.match("* [X] Checked uppercase");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("X"));
    
    match = pattern.match("+ [.] In progress");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("."));
}

void TestRegexPatterns::testTaskCheckboxChecked() {
    QRegularExpression pattern(RegexPatterns::TASK_CHECKBOX_CHECKED);
    
    // This pattern is for checked/in-progress only (no space)
    QRegularExpressionMatch match = pattern.match("- [x] Task");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("x"));
    
    match = pattern.match("* [X] Task");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("X"));
    
    match = pattern.match("+ [.] Task");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("."));
}

// Header Variations Tests

void TestRegexPatterns::testHeaderPrefix_AllLevels() {
    QRegularExpression pattern(RegexPatterns::HEADER_PREFIX);
    
    for (int level = 1; level <= 6; ++level) {
        QString header = QString("#").repeated(level) + " Header";
        QRegularExpressionMatch match = pattern.match(header);
        QVERIFY(match.hasMatch());
        QCOMPARE(match.captured(1), QString("#").repeated(level));
    }
}

void TestRegexPatterns::testHeaderH1_Specific() {
    QRegularExpression pattern(RegexPatterns::HEADER_H1);
    
    QRegularExpressionMatch match = pattern.match("# Main Title");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("Main Title"));
    
    // Should not match other levels
    QVERIFY(!pattern.match("## Level 2").hasMatch());
}

void TestRegexPatterns::testHeaderAny_MultipleLines() {
    QRegularExpression pattern(RegexPatterns::HEADER_ANY);
    
    // Test each line separately since pattern is anchored with ^ and $
    QVERIFY(pattern.match("# Header 1").hasMatch());
    QVERIFY(pattern.match("## Header 2").hasMatch());
    QVERIFY(pattern.match("### Header 3").hasMatch());
}

// Inline Code and Block Tests

void TestRegexPatterns::testInlineCode_Basic() {
    QRegularExpression pattern(RegexPatterns::INLINE_CODE);
    
    QRegularExpressionMatch match = pattern.match("`code`");
    QVERIFY(match.hasMatch());
    
    match = pattern.match("`const x = 42;`");
    QVERIFY(match.hasMatch());
    
    // Should not match empty
    QVERIFY(!pattern.match("``").hasMatch());
}

void TestRegexPatterns::testCodeBlockFence() {
    QRegularExpression pattern(RegexPatterns::CODE_BLOCK_FENCE);
    
    QVERIFY(pattern.match("```").hasMatch());
    QVERIFY(pattern.match("```cpp").hasMatch());
    QVERIFY(pattern.match("```python").hasMatch());
    QVERIFY(pattern.match("```javascript extra stuff").hasMatch());
}

void TestRegexPatterns::testQuote_SingleAndMultiple() {
    QRegularExpression pattern(RegexPatterns::QUOTE);
    
    QRegularExpressionMatch match = pattern.match("> Quote");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(">"));
    
    match = pattern.match(">> Nested quote");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(">>"));
    
    match = pattern.match("  >> Indented nested");
    QVERIFY(match.hasMatch());
}

void TestRegexPatterns::testHorizontalRule_AllStyles() {
    QRegularExpression pattern(RegexPatterns::HORIZONTAL_RULE);
    
    QVERIFY(pattern.match("---").hasMatch());
    QVERIFY(pattern.match("***").hasMatch());
    QVERIFY(pattern.match("___").hasMatch());
    
    // With spaces
    QVERIFY(pattern.match("- - -").hasMatch());
    QVERIFY(pattern.match("* * *").hasMatch());
    QVERIFY(pattern.match("_ _ _").hasMatch());
    
    // More than 3
    QVERIFY(pattern.match("----").hasMatch());
    QVERIFY(pattern.match("*****").hasMatch());
}

// Strikethrough Tests

void TestRegexPatterns::testStrikethrough_Content() {
    QRegularExpression pattern(RegexPatterns::STRIKETHROUGH);
    
    QRegularExpressionMatch match = pattern.match("~~deleted~~");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("deleted"));
    
    match = pattern.match("~~multiple words deleted~~");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("multiple words deleted"));
}

void TestRegexPatterns::testStrikethroughMarker() {
    QRegularExpression pattern(RegexPatterns::STRIKETHROUGH_MARKER);
    
    QVERIFY(pattern.match("~~").hasMatch());
    
    // Find multiple occurrences
    QString text = "~~start~~ middle ~~end~~";
    QRegularExpressionMatchIterator it = pattern.globalMatch(text);
    int count = 0;
    while (it.hasNext()) {
        it.next();
        count++;
    }
    QCOMPARE(count, 4);  // 4 markers total
}

// Number Pattern Tests

void TestRegexPatterns::testNumberPattern_Integers() {
    QRegularExpression pattern(RegexPatterns::NUMBER);
    
    QVERIFY(pattern.match("42").hasMatch());
    QVERIFY(pattern.match("0").hasMatch());
    QVERIFY(pattern.match("999").hasMatch());
}

void TestRegexPatterns::testNumberPattern_Floats() {
    QRegularExpression pattern(RegexPatterns::NUMBER);
    
    QVERIFY(pattern.match("3.14").hasMatch());
    QVERIFY(pattern.match("0.5").hasMatch());
    QVERIFY(pattern.match("99.99").hasMatch());
}

void TestRegexPatterns::testNumberC_WithSuffixes() {
    QRegularExpression pattern(RegexPatterns::NUMBER_C);
    
    QVERIFY(pattern.match("42").hasMatch());
    QVERIFY(pattern.match("42L").hasMatch());
    QVERIFY(pattern.match("42l").hasMatch());
    QVERIFY(pattern.match("3.14f").hasMatch());
    QVERIFY(pattern.match("3.14F").hasMatch());
    QVERIFY(pattern.match("42U").hasMatch());
    QVERIFY(pattern.match("42u").hasMatch());
}

void TestRegexPatterns::testNumberJava_WithSuffixes() {
    QRegularExpression pattern(RegexPatterns::NUMBER_JAVA);
    
    QVERIFY(pattern.match("42").hasMatch());
    QVERIFY(pattern.match("42L").hasMatch());
    QVERIFY(pattern.match("42l").hasMatch());
    QVERIFY(pattern.match("3.14f").hasMatch());
    QVERIFY(pattern.match("3.14F").hasMatch());
    QVERIFY(pattern.match("3.14d").hasMatch());
    QVERIFY(pattern.match("3.14D").hasMatch());
}

// Function Call Tests

void TestRegexPatterns::testFunctionCall_Basic() {
    QRegularExpression pattern(RegexPatterns::FUNCTION_CALL);
    
    QRegularExpressionMatch match = pattern.match("function()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("function"));
    
    match = pattern.match("my_function()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("my_function"));
    
    match = pattern.match("MyClass123()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("MyClass123"));
}

void TestRegexPatterns::testFunctionCallJS_DollarSign() {
    QRegularExpression pattern(RegexPatterns::FUNCTION_CALL_JS);
    
    // JavaScript allows $ and _ in identifiers
    // Note: \b word boundary treats $ as non-word char, so $element matches "element"
    QRegularExpressionMatch match = pattern.match("jQuery()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("jQuery"));
    
    match = pattern.match("_privateFunc()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("_privateFunc"));
    
    // $ at start: matches the identifier after $
    match = pattern.match("$element()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("element"));
    
    // $ in middle/end works fine
    match = pattern.match("func$123()");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("func$123"));
}

// String Pattern Tests

void TestRegexPatterns::testStringPattern_Single() {
    QRegularExpression pattern(RegexPatterns::STRING_PATTERN);
    
    QVERIFY(pattern.match("'single'").hasMatch());
    QVERIFY(pattern.match("'hello world'").hasMatch());
}

void TestRegexPatterns::testStringPattern_Double() {
    QRegularExpression pattern(RegexPatterns::STRING_PATTERN);
    
    QVERIFY(pattern.match("\"double\"").hasMatch());
    QVERIFY(pattern.match("\"hello world\"").hasMatch());
}

void TestRegexPatterns::testStringPatternBacktick() {
    QRegularExpression pattern(RegexPatterns::STRING_PATTERN_BACKTICK);
    
    QVERIFY(pattern.match("`backtick`").hasMatch());
    QVERIFY(pattern.match("'single'").hasMatch());
    QVERIFY(pattern.match("\"double\"").hasMatch());
}

// Comment Tests

void TestRegexPatterns::testMultilineComment() {
    QRegularExpression pattern(RegexPatterns::MULTILINE_COMMENT);
    
    QVERIFY(pattern.match("/* comment */").hasMatch());
    QVERIFY(pattern.match("/* multi word comment */").hasMatch());
}

void TestRegexPatterns::testWordBoundary() {
    QRegularExpression pattern(RegexPatterns::WORD_BOUNDARY);
    
    // Matches words with 3+ characters
    QVERIFY(pattern.match("abc").hasMatch());
    QVERIFY(pattern.match("hello").hasMatch());
    QVERIFY(pattern.match("WordBoundary").hasMatch());
    
    // Should not match short words
    QVERIFY(!pattern.match("ab").hasMatch());
    QVERIFY(!pattern.match("a").hasMatch());
}

// Utility Pattern Tests

void TestRegexPatterns::testNewlineBetweenText() {
    QRegularExpression pattern(RegexPatterns::NEWLINE_BETWEEN_TEXT);
    
    QRegularExpressionMatch match = pattern.match("line1\nline2");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("1"));
    QCOMPARE(match.captured(2), QString("l"));
    
    // Should not match double newlines
    QVERIFY(!pattern.match("line1\n\nline2").hasMatch());
}

void TestRegexPatterns::testFilenameInvalidChars() {
    QRegularExpression pattern(RegexPatterns::FILENAME_INVALID_CHARS);
    
    // Check if invalid chars are detected
    QVERIFY(pattern.match("<").hasMatch());
    QVERIFY(pattern.match(">").hasMatch());
    QVERIFY(pattern.match(":").hasMatch());
    QVERIFY(pattern.match("\"").hasMatch());
    QVERIFY(pattern.match("/").hasMatch());
    QVERIFY(pattern.match("\\").hasMatch());
    QVERIFY(pattern.match("|").hasMatch());
    QVERIFY(pattern.match("?").hasMatch());
    QVERIFY(pattern.match("*").hasMatch());
    
    // Valid filename should not match
    QVERIFY(!pattern.match("valid_filename.txt").hasMatch());
}

void TestRegexPatterns::testImageFilename() {
    QRegularExpression pattern(RegexPatterns::IMAGE_FILENAME);
    
    QRegularExpressionMatch match = pattern.match("image_1.png");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("1"));
    
    match = pattern.match("image_42.png");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("42"));
    
    match = pattern.match("image_999.png");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("999"));
    
    // Should not match other formats
    QVERIFY(!pattern.match("image_1.jpg").hasMatch());
    QVERIFY(!pattern.match("photo_1.png").hasMatch());
    QVERIFY(!pattern.match("image_abc.png").hasMatch());
}

void TestRegexPatterns::testWhitespaceMultiple() {
    QRegularExpression pattern(RegexPatterns::WHITESPACE_MULTIPLE);
    
    QVERIFY(pattern.match("  ").hasMatch());
    QVERIFY(pattern.match("\t").hasMatch());
    QVERIFY(pattern.match("\n").hasMatch());
    QVERIFY(pattern.match(" \t\n ").hasMatch());
}

void TestRegexPatterns::testNonWordChars() {
    QRegularExpression pattern(RegexPatterns::NON_WORD_CHARS);
    
    QVERIFY(pattern.match("@").hasMatch());
    QVERIFY(pattern.match("#").hasMatch());
    QVERIFY(pattern.match("!").hasMatch());
    
    // Should not match word chars, dash, or underscore
    QVERIFY(!pattern.match("a").hasMatch());
    QVERIFY(!pattern.match("_").hasMatch());
    QVERIFY(!pattern.match("-").hasMatch());
}

void TestRegexPatterns::testLeadingTrailingDash() {
    QRegularExpression pattern(RegexPatterns::LEADING_TRAILING_DASH);
    
    QVERIFY(pattern.match("-start").hasMatch());
    QVERIFY(pattern.match("end-").hasMatch());
    QVERIFY(pattern.match("--multiple").hasMatch());
    QVERIFY(pattern.match("both--").hasMatch());
    
    // Should not match dashes in middle only
    QVERIFY(!pattern.match("no-dash-here").hasMatch());
}

void TestRegexPatterns::testHTMLHeading_AllLevels() {
    QRegularExpression pattern(RegexPatterns::HTML_HEADING, 
                               QRegularExpression::CaseInsensitiveOption);
    
    QRegularExpressionMatch match;
    
    match = pattern.match("<h1>Title</h1>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("h1"));
    QCOMPARE(match.captured(3), QString("Title"));
    
    match = pattern.match("<h3 class=\"heading\">Section</h3>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("h3"));
    QCOMPARE(match.captured(2), QString(" class=\"heading\""));
    QCOMPARE(match.captured(3), QString("Section"));
    
    match = pattern.match("<h6>Deep Level</h6>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("h6"));
    QCOMPARE(match.captured(3), QString("Deep Level"));
}

void TestRegexPatterns::testHTMLAnchor_Basic() {
    QRegularExpression pattern(RegexPatterns::HTML_ANCHOR);
    
    QRegularExpressionMatch match;
    
    match = pattern.match("<a href=\"page.html\">Link Text</a>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("page.html"));
    QCOMPARE(match.captured(2), QString("Link Text"));
    
    match = pattern.match("<a href=\"https://example.com\">Example</a>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("https://example.com"));
    QCOMPARE(match.captured(2), QString("Example"));
}

void TestRegexPatterns::testHTMLCode_WithAttributes() {
    QRegularExpression pattern(RegexPatterns::HTML_CODE);
    
    QRegularExpressionMatch match;
    
    match = pattern.match("<code>inline code</code>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(2), QString("inline code"));
    
    match = pattern.match("<code class=\"language-python\">print('hello')</code>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString(" class=\"language-python\""));
    QCOMPARE(match.captured(2), QString("print('hello')"));
}

void TestRegexPatterns::testLatexDisplayEquation() {
    QRegularExpression pattern(RegexPatterns::LATEX_DISPLAY_EQUATION);
    
    QRegularExpressionMatch match = pattern.match(
        "<x-equation type=\"display\">E = mc^2</x-equation>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("E = mc^2"));
}

void TestRegexPatterns::testLatexInlineEquation() {
    QRegularExpression pattern(RegexPatterns::LATEX_INLINE_EQUATION);
    
    QRegularExpressionMatch match = pattern.match("<x-equation>x^2</x-equation>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("x^2"));
}

void TestRegexPatterns::testMD4CWikilink_Basic() {
    QRegularExpression pattern(RegexPatterns::MD4C_WIKILINK);
    
    QRegularExpressionMatch match = pattern.match(
        "<x-wikilink data-target=\"PageName\">Display Text</x-wikilink>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("PageName"));
    QCOMPARE(match.captured(2), QString("Display Text"));
}

void TestRegexPatterns::testMD4CWikilinkInclusion() {
    QRegularExpression pattern(RegexPatterns::MD4C_WIKILINK_INCLUSION);
    
    QRegularExpressionMatch match = pattern.match(
        "<x-wikilink data-target=\"!IncludedFile\">Include This</x-wikilink>");
    QVERIFY(match.hasMatch());
    QCOMPARE(match.captured(1), QString("IncludedFile"));
    QCOMPARE(match.captured(2), QString("Include This"));
}

void TestRegexPatterns::testTocHeaderTable() {
    QRegularExpression re(RegexPatterns::TOC_HEADER_TABLE,
                          QRegularExpression::MultilineOption);

    QString text1 = R"(# Title

## Table of Contents

- [Section 1](#section-1)
- [Section 2](#section-2)

## Section 1
)";
    QRegularExpressionMatch match1 = re.match(text1);
    QVERIFY(match1.hasMatch());
    QVERIFY(match1.captured(0).contains("Table of Contents"));

    QString text2 = "## Contents\n";
    QVERIFY(!re.match(text2).hasMatch());
}

void TestRegexPatterns::testTocHeaderContents() {
    QRegularExpression re(RegexPatterns::TOC_HEADER_CONTENTS,
                          QRegularExpression::MultilineOption);

    QString text1 = R"(# Title

## Contents

- [Section 1](#section-1)

## Section 1
)";
    QRegularExpressionMatch match1 = re.match(text1);
    QVERIFY(match1.hasMatch());
    QVERIFY(match1.captured(0).contains("Contents"));

    QString text2 = "## Table of Contents\n";
    QVERIFY(!re.match(text2).hasMatch());
}

void TestRegexPatterns::testTocHtmlComment() {
    QRegularExpression re(RegexPatterns::TOC_HTML_COMMENT,
                          QRegularExpression::DotMatchesEverythingOption);

    QString text1 = R"(# Title

<!-- TOC -->
- [Section 1](#section-1)
- [Section 2](#section-2)
<!-- /TOC -->

## Section 1
)";
    QRegularExpressionMatch match1 = re.match(text1);
    QVERIFY(match1.hasMatch());
    QVERIFY(match1.captured(0).contains("<!-- TOC -->"));
    QVERIFY(match1.captured(0).contains("<!-- /TOC -->"));

    QString text2 = "<!-- Not a TOC -->";
    QVERIFY(!re.match(text2).hasMatch());
}

void TestRegexPatterns::testTocDetectHeader() {
    QRegularExpression re(RegexPatterns::TOC_DETECT_HEADER,
                          QRegularExpression::MultilineOption);

    QString text1 = "## Table of Contents\n";
    QVERIFY(re.match(text1).hasMatch());

    QString text2 = "## Contents\n";
    QVERIFY(re.match(text2).hasMatch());

    QString text3 = "\n## Table of Contents\n";
    QVERIFY(re.match(text3).hasMatch());

    QString text4 = "## Introduction\n";
    QVERIFY(!re.match(text4).hasMatch());

    QString text5 = "### Table of Contents\n";
    QVERIFY(!re.match(text5).hasMatch());
}

void TestRegexPatterns::testTocDetectComment() {
    QRegularExpression re(RegexPatterns::TOC_DETECT_COMMENT);

    QString text1 = "<!-- TOC -->";
    QVERIFY(re.match(text1).hasMatch());

    QString text2 = R"(# Title
<!-- TOC -->
- Content
)";
    QVERIFY(re.match(text2).hasMatch());

    QString text3 = "<!-- Not TOC -->";
    QVERIFY(!re.match(text3).hasMatch());
}

QTEST_MAIN(TestRegexPatterns)
#include "test_regexpatterns.moc"
