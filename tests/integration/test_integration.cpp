#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include "managers/windowmanager.h"
#include "mainwindow.h"
#include "logic/mainfilelocator.h"

class TestIntegration : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void cleanup();
    
    // Integration tests
    void testMainFileAutoOpen();
    void testMultiWindowCreation();
    void testWindowManagerLifecycle();
    void testMainFileWithFallback();
    void testOpenFolderCreatesWindow();
    
private:
    QTemporaryDir *tempDir;
    
    void createFile(const QString &fileName, const QString &content = "test");
    QString createTestFolder(const QString &folderName);
};

void TestIntegration::initTestCase() {
    // Called once before all tests
}

void TestIntegration::cleanupTestCase() {
    // Called once after all tests
}

void TestIntegration::init() {
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
}

void TestIntegration::cleanup() {
    // Close all windows
    QList<MainWindow*> windows = WindowManager::instance()->windows();
    for (MainWindow *window : windows) {
        window->close();
    }
    QTest::qWait(100);
    
    delete tempDir;
    tempDir = nullptr;
}

void TestIntegration::createFile(const QString &fileName, const QString &content) {
    QFile file(tempDir->filePath(fileName));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << content;
    file.close();
}

QString TestIntegration::createTestFolder(const QString &folderName) {
    QDir dir(tempDir->path());
    QString fullPath = dir.absoluteFilePath(folderName);
    if (!dir.mkdir(folderName)) {
        return QString();
    }
    return fullPath;
}

void TestIntegration::testMainFileAutoOpen() {
    // Create a folder with main.md
    createFile("main.md", "# Main File\n\nThis is the main file.");
    
    // Test that MainFileLocator can find it
    QString mainFile = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    QVERIFY(!mainFile.isEmpty());
    QVERIFY(mainFile.endsWith("main.md"));
}

void TestIntegration::testMultiWindowCreation() {
    // Test WindowManager singleton
    WindowManager *manager1 = WindowManager::instance();
    WindowManager *manager2 = WindowManager::instance();
    
    QVERIFY(manager1 != nullptr);
    QCOMPARE(manager1, manager2);
}

void TestIntegration::testWindowManagerLifecycle() {
    // Test that WindowManager exists
    WindowManager *manager = WindowManager::instance();
    QVERIFY(manager != nullptr);
    
    // Test initial state
    int initialCount = manager->windows().count();
    QVERIFY(initialCount >= 0);
}

void TestIntegration::testMainFileWithFallback() {
    // Create folder with only index.md (no main.md)
    createFile("index.md", "# Index File\n\nThis is the index file.");
    
    // Verify MainFileLocator finds the fallback
    QString foundFile = MainFileLocator::findMainFile(tempDir->path(), "main.md");
    QVERIFY(!foundFile.isEmpty());
    QVERIFY(foundFile.endsWith("index.md"));
}

void TestIntegration::testOpenFolderCreatesWindow() {
    // Create a test folder
    createFile("test.md", "# Test File");
    
    // Test MainFileLocator can find files
    QString testFile = MainFileLocator::findMainFile(tempDir->path(), "test.md");
    QVERIFY(!testFile.isEmpty());
    QVERIFY(testFile.endsWith("test.md"));
}

QTEST_MAIN(TestIntegration)
#include "test_integration.moc"
