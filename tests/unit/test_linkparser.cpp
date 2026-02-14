#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QSignalSpy>
#include "linkparser.h"

class TestLinkParser : public QObject {
  Q_OBJECT

private slots:
  void initTestCase();
  void cleanupTestCase();
  void init();
  void cleanup();

  // Link parsing tests
  void testParseLinks_Simple();
  void testParseLinks_WithPipe();
  void testParseLinks_Inclusion();
  void testParseLinks_InclusionWithPipe();
  void testParseLinks_Multiple();
  void testParseLinks_NoLinks();
  void testParseLinks_EmptyText();
  void testParseLinks_PositionAndLength();
  
  // Markdown link parsing tests
  void testParseLinks_MarkdownSimple();
  void testParseLinks_MarkdownRelative();
  void testParseLinks_MarkdownSkipURLs();
  void testParseLinks_MarkdownAndWiki();

  // Link extraction from file tests
  void testExtractLinksFromFile_ValidFile();
  void testExtractLinksFromFile_NonExistent();
  void testExtractLinksFromFile_EmptyFile();
  void testExtractLinksFromFile_Duplicates();

  // Link index building tests
  void testBuildLinkIndex_SingleFile();
  void testBuildLinkIndex_MultipleFiles();
  void testBuildLinkIndex_DepthLimit();
  void testBuildLinkIndex_EmptyDirectory();
  void testBuildLinkIndex_Signals();

  // Backlinks tests
  void testGetBacklinks_Simple();
  void testGetBacklinks_WithPath();
  void testGetBacklinks_CaseInsensitive();
  void testGetBacklinks_MultipleBacklinks();
  void testGetBacklinks_NoBacklinks();
  void testGetBacklinks_MarkdownLinks();

  // Link resolution tests
  void testResolveLinkTarget_SameDirectory();
  void testResolveLinkTarget_Subdirectory();
  void testResolveLinkTarget_ParentDirectory();
  void testResolveLinkTarget_DepthLimit();
  void testResolveLinkTarget_WithExtension();
  void testResolveLinkTarget_NotFound();

  // Home directory boundary tests
  void testHomeDirectoryBoundary();

private:
  QTemporaryDir *tempDir;
  LinkParser *linkParser;

  void createFile(const QString &relativePath, const QString &content = "test");
  void createDirectory(const QString &relativePath);
  QString getFilePath(const QString &relativePath);
};

void TestLinkParser::initTestCase() {
}

void TestLinkParser::cleanupTestCase() {
}

void TestLinkParser::init() {
  tempDir = new QTemporaryDir();
  QVERIFY(tempDir->isValid());
  linkParser = new LinkParser();
  linkParser->setEnforceHomeBoundary(false);
}

void TestLinkParser::cleanup() {
  delete linkParser;
  linkParser = nullptr;
  delete tempDir;
  tempDir = nullptr;
}

void TestLinkParser::createFile(const QString &relativePath,
                                 const QString &content) {
  QString fullPath = tempDir->filePath(relativePath);
  QFileInfo fileInfo(fullPath);
  QDir dir = fileInfo.dir();
  if (!dir.exists()) {
    dir.mkpath(".");
  }

  QFile file(fullPath);
  QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
  QTextStream out(&file);
  out << content;
  file.close();
}

void TestLinkParser::createDirectory(const QString &relativePath) {
  QDir dir(tempDir->path());
  QVERIFY(dir.mkpath(relativePath));
}

QString TestLinkParser::getFilePath(const QString &relativePath) {
  return tempDir->filePath(relativePath);
}

// Link parsing tests

void TestLinkParser::testParseLinks_Simple() {
  QString text = "This is a [[link]] in text.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("link"));
  QCOMPARE(links[0].displayText, QString("link"));
  QVERIFY(!links[0].isInclusion);
}

void TestLinkParser::testParseLinks_WithPipe() {
  QString text = "This is a [[target|display text]] in text.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("target"));
  QCOMPARE(links[0].displayText, QString("display text"));
  QVERIFY(!links[0].isInclusion);
}

void TestLinkParser::testParseLinks_Inclusion() {
  QString text = "This is an [[!included]] file.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("included"));
  QCOMPARE(links[0].displayText, QString("included"));
  QVERIFY(links[0].isInclusion);
}

void TestLinkParser::testParseLinks_InclusionWithPipe() {
  QString text = "This is [[!target|custom display]] text.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("target"));
  QCOMPARE(links[0].displayText, QString("custom display"));
  QVERIFY(links[0].isInclusion);
}

void TestLinkParser::testParseLinks_Multiple() {
  QString text = "Link to [[first]] and [[second|display]] and [[!third]].";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 3);
  QCOMPARE(links[0].targetFile, QString("first"));
  QCOMPARE(links[1].targetFile, QString("second"));
  QCOMPARE(links[1].displayText, QString("display"));
  QCOMPARE(links[2].targetFile, QString("third"));
  QVERIFY(links[2].isInclusion);
}

void TestLinkParser::testParseLinks_NoLinks() {
  QString text = "This text has no wiki links at all.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 0);
}

void TestLinkParser::testParseLinks_EmptyText() {
  QString text = "";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 0);
}

void TestLinkParser::testParseLinks_PositionAndLength() {
  QString text = "Start [[link]] end";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].startPos, 6);
  QCOMPARE(links[0].length, 8);
}

// Link extraction from file tests

void TestLinkParser::testExtractLinksFromFile_ValidFile() {
  createFile("test.md", "This has [[link1]] and [[link2]].");

  QVector<QString> links = linkParser->extractLinksFromFile(getFilePath("test.md"));

  QCOMPARE(links.size(), 2);
  QVERIFY(links.contains("link1"));
  QVERIFY(links.contains("link2"));
}

void TestLinkParser::testExtractLinksFromFile_NonExistent() {
  QVector<QString> links = linkParser->extractLinksFromFile(getFilePath("nonexistent.md"));

  QCOMPARE(links.size(), 0);
}

void TestLinkParser::testExtractLinksFromFile_EmptyFile() {
  createFile("empty.md", "");

  QVector<QString> links = linkParser->extractLinksFromFile(getFilePath("empty.md"));

  QCOMPARE(links.size(), 0);
}

void TestLinkParser::testExtractLinksFromFile_Duplicates() {
  createFile("test.md", "[[link1]] and [[link1]] again.");

  QVector<QString> links = linkParser->extractLinksFromFile(getFilePath("test.md"));

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0], QString("link1"));
}

// Link index building tests

void TestLinkParser::testBuildLinkIndex_SingleFile() {
  createFile("note.md", "This links to [[other]].");
  createFile("other.md", "Target file.");  // Create target file so it gets scanned

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("other.md"));
  QCOMPARE(backlinks.size(), 1);
  QVERIFY(backlinks.contains(getFilePath("note.md")));
}

void TestLinkParser::testBuildLinkIndex_MultipleFiles() {
  createFile("note1.md", "Links to [[target]].");
  createFile("note2.md", "Also links to [[target]].");
  createFile("target.md", "Target file.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("target.md"));
  QCOMPARE(backlinks.size(), 2);
  QVERIFY(backlinks.contains(getFilePath("note1.md")));
  QVERIFY(backlinks.contains(getFilePath("note2.md")));
}

void TestLinkParser::testBuildLinkIndex_DepthLimit() {
  createDirectory("level1");
  createDirectory("level1/level2");
  createDirectory("level1/level2/level3");

  createFile("root.md", "Root file.");
  createFile("level1/file1.md", "Level 1 file.");
  createFile("level1/level2/file2.md", "Level 2 file.");
  createFile("level1/level2/level3/file3.md", "Level 3 file.");

  linkParser->buildLinkIndex(tempDir->path(), 1);

  QVector<QString> backlinks1 = linkParser->getBacklinks(getFilePath("level1/file1.md"));
  QVector<QString> backlinks3 = linkParser->getBacklinks(getFilePath("level1/level2/level3/file3.md"));

  QCOMPARE(backlinks1.size(), 0);
  QCOMPARE(backlinks3.size(), 0);
}

void TestLinkParser::testBuildLinkIndex_EmptyDirectory() {
  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("nonexistent.md"));
  QCOMPARE(backlinks.size(), 0);
}

void TestLinkParser::testBuildLinkIndex_Signals() {
  createFile("note.md", "Test file.");

  QSignalSpy startedSpy(linkParser, &LinkParser::indexBuildStarted);
  QSignalSpy completedSpy(linkParser, &LinkParser::indexBuildCompleted);

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QCOMPARE(startedSpy.count(), 1);
  QCOMPARE(completedSpy.count(), 1);
}

// Backlinks tests

void TestLinkParser::testGetBacklinks_Simple() {
  createFile("source.md", "Links to [[target]].");
  createFile("target.md", "Target file.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("target.md"));
  QCOMPARE(backlinks.size(), 1);
  QVERIFY(backlinks.contains(getFilePath("source.md")));
}

void TestLinkParser::testGetBacklinks_WithPath() {
  createDirectory("subfolder");
  createFile("source.md", "Links to [[subfolder/target]].");
  createFile("subfolder/target.md", "Target in subfolder.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("subfolder/target.md"));
  QCOMPARE(backlinks.size(), 1);
  QVERIFY(backlinks.contains(getFilePath("source.md")));
}

void TestLinkParser::testGetBacklinks_CaseInsensitive() {
  createFile("source.md", "Links to [[TARGET]].");
  createFile("target.md", "Target file.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("target.md"));
  QCOMPARE(backlinks.size(), 1);
  QVERIFY(backlinks.contains(getFilePath("source.md")));
}

void TestLinkParser::testGetBacklinks_MultipleBacklinks() {
  createFile("source1.md", "Links to [[target]].");
  createFile("source2.md", "Also links to [[target]].");
  createFile("source3.md", "And another link to [[target]].");
  createFile("target.md", "Popular target.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("target.md"));
  QCOMPARE(backlinks.size(), 3);
}

void TestLinkParser::testGetBacklinks_NoBacklinks() {
  createFile("isolated.md", "No links here.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("isolated.md"));
  QCOMPARE(backlinks.size(), 0);
}

// Link resolution tests

void TestLinkParser::testResolveLinkTarget_SameDirectory() {
  createFile("current.md", "Current file.");
  createFile("target.md", "Target file.");

  QString resolved = linkParser->resolveLinkTarget("target", getFilePath("current.md"), 2);

  QCOMPARE(resolved, getFilePath("target.md"));
}

void TestLinkParser::testResolveLinkTarget_Subdirectory() {
  createDirectory("subfolder");
  createFile("current.md", "Current file.");
  createFile("subfolder/target.md", "Target in subfolder.");

  QString resolved = linkParser->resolveLinkTarget("subfolder/target", getFilePath("current.md"), 2);

  QCOMPARE(resolved, getFilePath("subfolder/target.md"));
}

void TestLinkParser::testResolveLinkTarget_ParentDirectory() {
  createDirectory("subfolder");
  createFile("target.md", "Target in parent.");
  createFile("subfolder/current.md", "Current file.");

  QString resolved = linkParser->resolveLinkTarget("target", getFilePath("subfolder/current.md"), 2);

  QCOMPARE(resolved, getFilePath("target.md"));
}

void TestLinkParser::testResolveLinkTarget_DepthLimit() {
  createDirectory("level1");
  createDirectory("level1/level2");
  createDirectory("level1/level2/level3");

  createFile("target.md", "Target at root.");
  createFile("level1/level2/level3/current.md", "Deep file.");

  QString resolved = linkParser->resolveLinkTarget("target", getFilePath("level1/level2/level3/current.md"), 1);

  QVERIFY(resolved.isEmpty());
}

void TestLinkParser::testResolveLinkTarget_WithExtension() {
  createFile("current.md", "Current file.");
  createFile("target.md", "Target file.");

  QString resolved = linkParser->resolveLinkTarget("target.md", getFilePath("current.md"), 2);

  QCOMPARE(resolved, getFilePath("target.md"));
}

void TestLinkParser::testResolveLinkTarget_NotFound() {
  createFile("current.md", "Current file.");

  QString resolved = linkParser->resolveLinkTarget("nonexistent", getFilePath("current.md"), 2);

  QVERIFY(resolved.isEmpty());
}

// Home directory boundary tests

void TestLinkParser::testHomeDirectoryBoundary() {
  createFile("note.md", "Test file.");

  linkParser->buildLinkIndex(tempDir->path(), 100);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("note.md"));
  QCOMPARE(backlinks.size(), 0);
}

// Markdown link parsing tests

void TestLinkParser::testParseLinks_MarkdownSimple() {
  QString text = "This is a [link](target.md) in text.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("target.md"));
  QCOMPARE(links[0].displayText, QString("link"));
  QVERIFY(!links[0].isInclusion);
}

void TestLinkParser::testParseLinks_MarkdownRelative() {
  QString text = "Link to [project](./projects/datainquiry.md) here.";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("./projects/datainquiry.md"));
  QCOMPARE(links[0].displayText, QString("project"));
}

void TestLinkParser::testParseLinks_MarkdownSkipURLs() {
  QString text = "Link to [Google](https://google.com) and [local](file.md).";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 1);
  QCOMPARE(links[0].targetFile, QString("file.md"));
  QCOMPARE(links[0].displayText, QString("local"));
}

void TestLinkParser::testParseLinks_MarkdownAndWiki() {
  QString text = "Wiki [[note]] and markdown [link](other.md).";
  QVector<WikiLink> links = linkParser->parseLinks(text);

  QCOMPARE(links.size(), 2);
  QCOMPARE(links[0].targetFile, QString("note"));
  QCOMPARE(links[1].targetFile, QString("other.md"));
}

void TestLinkParser::testGetBacklinks_MarkdownLinks() {
  createFile("source.md", "This links to [target](target.md).");
  createFile("target.md", "Target file.");

  linkParser->buildLinkIndex(tempDir->path(), 2);

  QVector<QString> backlinks = linkParser->getBacklinks(getFilePath("target.md"));
  QCOMPARE(backlinks.size(), 1);
  QVERIFY(backlinks.contains(getFilePath("source.md")));
}

QTEST_MAIN(TestLinkParser)
#include "test_linkparser.moc"
