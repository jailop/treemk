#include <QtTest/QtTest>
#include "linkparser.h"
#include "regexutils.h"

class TestInternalLinks : public QObject {
    Q_OBJECT

private slots:
    void testSlugGeneration();
    void testSlugGeneration_WithSpaces();
    void testSlugGeneration_WithPunctuation();
    void testSlugGeneration_WithNumbers();
    void testSlugGeneration_WithUnderscores();
    void testSlugGeneration_WithUnicode();
    void testSlugGeneration_EmptyString();
    
    void testLinkTargetParsing_PureAnchor();
    void testLinkTargetParsing_FileWithAnchor();
    void testLinkTargetParsing_FileOnly();
    void testLinkTargetParsing_AnchorNormalization();
    void testLinkTargetParsing_EmptyString();
    void testLinkTargetParsing_ComplexPath();
};

void TestInternalLinks::testSlugGeneration() {
    QCOMPARE(RegexUtils::generateSlug("Hello World"), QString("hello-world"));
    QCOMPARE(RegexUtils::generateSlug("Test Heading"), QString("test-heading"));
}

void TestInternalLinks::testSlugGeneration_WithSpaces() {
    QCOMPARE(RegexUtils::generateSlug("Multiple   Spaces"), QString("multiple-spaces"));
    QCOMPARE(RegexUtils::generateSlug("  Leading Trailing  "), QString("leading-trailing"));
}

void TestInternalLinks::testSlugGeneration_WithPunctuation() {
    QCOMPARE(RegexUtils::generateSlug("Section 2.1"), QString("section-21"));
    QCOMPARE(RegexUtils::generateSlug("Hello, World!"), QString("hello-world"));
    QCOMPARE(RegexUtils::generateSlug("What's this?"), QString("whats-this"));
    QCOMPARE(RegexUtils::generateSlug("C++ Programming"), QString("c-programming"));
}

void TestInternalLinks::testSlugGeneration_WithNumbers() {
    QCOMPARE(RegexUtils::generateSlug("2. Introduction"), QString("2-introduction"));
    QCOMPARE(RegexUtils::generateSlug("Chapter 42"), QString("chapter-42"));
    QCOMPARE(RegexUtils::generateSlug("123"), QString("123"));
}

void TestInternalLinks::testSlugGeneration_WithUnderscores() {
    QCOMPARE(RegexUtils::generateSlug("test_function"), QString("test_function"));
    QCOMPARE(RegexUtils::generateSlug("my_var_name"), QString("my_var_name"));
    QCOMPARE(RegexUtils::generateSlug("_private"), QString("_private"));
}

void TestInternalLinks::testSlugGeneration_WithUnicode() {
    QCOMPARE(RegexUtils::generateSlug("Über Cool"), QString("über-cool"));
    QCOMPARE(RegexUtils::generateSlug("Café"), QString("café"));
}

void TestInternalLinks::testSlugGeneration_EmptyString() {
    QCOMPARE(RegexUtils::generateSlug(""), QString(""));
    QCOMPARE(RegexUtils::generateSlug("   "), QString(""));
}

void TestInternalLinks::testLinkTargetParsing_PureAnchor() {
    LinkTarget result = LinkParser::parseLinkTarget("#heading");
    
    QVERIFY(result.isInternalOnly);
    QCOMPARE(result.filePath, QString());
    QCOMPARE(result.anchor, QString("heading"));
}

void TestInternalLinks::testLinkTargetParsing_FileWithAnchor() {
    LinkTarget result = LinkParser::parseLinkTarget("file.md#section");
    
    QVERIFY(!result.isInternalOnly);
    QCOMPARE(result.filePath, QString("file.md"));
    QCOMPARE(result.anchor, QString("section"));
}

void TestInternalLinks::testLinkTargetParsing_FileOnly() {
    LinkTarget result = LinkParser::parseLinkTarget("file.md");
    
    QVERIFY(!result.isInternalOnly);
    QCOMPARE(result.filePath, QString("file.md"));
    QVERIFY(result.anchor.isEmpty());
}

void TestInternalLinks::testLinkTargetParsing_AnchorNormalization() {
    LinkTarget result1 = LinkParser::parseLinkTarget("#Some Heading");
    QCOMPARE(result1.anchor, QString("some-heading"));
    
    LinkTarget result2 = LinkParser::parseLinkTarget("file.md#Another Section");
    QCOMPARE(result2.anchor, QString("another-section"));
    
    LinkTarget result3 = LinkParser::parseLinkTarget("#test_function");
    QCOMPARE(result3.anchor, QString("test_function"));
}

void TestInternalLinks::testLinkTargetParsing_EmptyString() {
    LinkTarget result = LinkParser::parseLinkTarget("");
    
    QVERIFY(!result.isInternalOnly);
    QVERIFY(result.filePath.isEmpty());
    QVERIFY(result.anchor.isEmpty());
}

void TestInternalLinks::testLinkTargetParsing_ComplexPath() {
    LinkTarget result = LinkParser::parseLinkTarget("../docs/file.md#introduction");
    
    QVERIFY(!result.isInternalOnly);
    QCOMPARE(result.filePath, QString("../docs/file.md"));
    QCOMPARE(result.anchor, QString("introduction"));
}

QTEST_MAIN(TestInternalLinks)
#include "test_internal_links.moc"
