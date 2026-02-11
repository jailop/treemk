#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "logic/mainfilelocator.h"

class TestMainFileLocator : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Test cases
    void testFindMainFile_ExactMatch();
    void testFindMainFile_FallbackIndex();
    void testFindMainFile_FallbackReadme();
    void testFindMainFile_CustomName();
    void testFindMainFile_NotFound();
    void testFindMainFile_EmptyFolder();
    void testFindMainFile_InvalidPath();
    void testFindMainFile_CaseInsensitive();
    void testGetFallbackNames();
    
private:
    QTemporaryDir *tempDir;
    
    void createFile(const QString &fileName, const QString &content = "test");
};

void TestMainFileLocator::initTestCase() {
    // Called once before all tests
}

void TestMainFileLocator::cleanupTestCase() {
    // Called once after all tests
}

void TestMainFileLocator::init() {
    // Called before each test
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestMainFileLocator::cleanup() {
    // Called after each test
    delete tempDir;
    tempDir = nullptr;
}

void TestMainFileLocator::createFile(const QString &fileName, const QString &content) {
    QFile file(tempDir->filePath(fileName));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << content;
    file.close();
}

void TestMainFileLocator::testFindMainFile_ExactMatch() {
    // Create main.md
    createFile("main.md");
    
    QString result = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    
    QVERIFY(!result.isEmpty());
    QVERIFY(result.endsWith("main.md"));
    QVERIFY(QFileInfo(result).exists());
}

void TestMainFileLocator::testFindMainFile_FallbackIndex() {
    // Create only index.md (no main.md)
    createFile("index.md");
    
    QString result = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    
    QVERIFY(!result.isEmpty());
    QVERIFY(result.endsWith("index.md"));
}

void TestMainFileLocator::testFindMainFile_FallbackReadme() {
    // Create only README.md (no main.md or index.md)
    createFile("README.md");
    
    QString result = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    
    QVERIFY(!result.isEmpty());
    QVERIFY(result.endsWith("README.md"));
}

void TestMainFileLocator::testFindMainFile_CustomName() {
    // Create custom file
    createFile("intro.md");
    
    QString result = MainFileLocator::findMainFile(tempDir->path(), "intro.md");
    
    QVERIFY(!result.isEmpty());
    QVERIFY(result.endsWith("intro.md"));
}

void TestMainFileLocator::testFindMainFile_NotFound() {
    // Create some other file
    createFile("other.md");
    
    QString result = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    
    QVERIFY(result.isEmpty());
}

void TestMainFileLocator::testFindMainFile_EmptyFolder() {
    // Empty folder (no files)
    QString result = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    
    QVERIFY(result.isEmpty());
}

void TestMainFileLocator::testFindMainFile_InvalidPath() {
    QString result = MainFileLocator::findMainFile("/nonexistent/path", "main.md");
    
    QVERIFY(result.isEmpty());
}

void TestMainFileLocator::testFindMainFile_CaseInsensitive() {
    // Create MAIN.MD (uppercase)
    createFile("MAIN.MD");
    
    // Search for main.md (lowercase)
    QString result = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    
    // Should find it (case-insensitive)
    QVERIFY(!result.isEmpty());
    QVERIFY(result.endsWith("MAIN.MD"));
}

void TestMainFileLocator::testGetFallbackNames() {
    QStringList fallbacks = MainFileLocator::getFallbackNames();
    
    QVERIFY(!fallbacks.isEmpty());
    QVERIFY(fallbacks.contains("index.md"));
    QVERIFY(fallbacks.contains("README.md"));
    QCOMPARE(fallbacks.size(), 2);
}

QTEST_MAIN(TestMainFileLocator)
#include "test_mainfilelocator.moc"
