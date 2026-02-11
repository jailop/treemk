#include <QtTest/QtTest>
#include <QTemporaryDir>
#include <QFile>
#include <QTextStream>
#include <QSignalSpy>
#include "managers/workspacemanager.h"

class TestWorkspaceManager : public QObject {
    Q_OBJECT
    
private slots:
    void init();
    void cleanup();
    
    void testOpenFolder_WithMainFile();
    void testOpenFolder_WithoutMainFile();
    void testOpenFolder_InvalidPath();
    void testOpenFolder_CustomMainFile();
    void testOpenFolder_AutoOpenDisabled();
    void testSignals_FolderOpened();
    void testSignals_MainFileFound();
    void testSignals_NoMainFileFound();
    
private:
    QTemporaryDir *tempDir;
    WorkspaceManager *manager;
    
    void createFile(const QString &fileName, const QString &content = "test");
};

void TestWorkspaceManager::init() {
    tempDir = new QTemporaryDir();
    QVERIFY(tempDir->isValid());
    manager = new WorkspaceManager();
}

void TestWorkspaceManager::cleanup() {
    delete manager;
    delete tempDir;
    tempDir = nullptr;
}

void TestWorkspaceManager::createFile(const QString &fileName, const QString &content) {
    QFile file(tempDir->filePath(fileName));
    QVERIFY(file.open(QIODevice::WriteOnly | QIODevice::Text));
    QTextStream out(&file);
    out << content;
    file.close();
}

void TestWorkspaceManager::testOpenFolder_WithMainFile() {
    createFile("main.md");
    
    bool result = manager->openFolder(tempDir->path(), true, "main.md");
    
    QVERIFY(result);
    QCOMPARE(manager->getCurrentFolder(), tempDir->path());
    QVERIFY(!manager->getMainFile().isEmpty());
    QVERIFY(manager->getMainFile().endsWith("main.md"));
}

void TestWorkspaceManager::testOpenFolder_WithoutMainFile() {
    createFile("other.md");
    
    bool result = manager->openFolder(tempDir->path(), true, "main.md");
    
    QVERIFY(result);
    QCOMPARE(manager->getCurrentFolder(), tempDir->path());
    QVERIFY(manager->getMainFile().isEmpty());
}

void TestWorkspaceManager::testOpenFolder_InvalidPath() {
    bool result = manager->openFolder("/nonexistent/path", true, "main.md");
    
    QVERIFY(!result);
    QVERIFY(manager->getCurrentFolder().isEmpty());
    QVERIFY(manager->getMainFile().isEmpty());
}

void TestWorkspaceManager::testOpenFolder_CustomMainFile() {
    createFile("intro.md");
    
    bool result = manager->openFolder(tempDir->path(), true, "intro.md");
    
    QVERIFY(result);
    QVERIFY(!manager->getMainFile().isEmpty());
    QVERIFY(manager->getMainFile().endsWith("intro.md"));
}

void TestWorkspaceManager::testOpenFolder_AutoOpenDisabled() {
    createFile("main.md");
    
    bool result = manager->openFolder(tempDir->path(), false, "main.md");
    
    QVERIFY(result);
    QCOMPARE(manager->getCurrentFolder(), tempDir->path());
    QVERIFY(manager->getMainFile().isEmpty());
}

void TestWorkspaceManager::testSignals_FolderOpened() {
    QSignalSpy spy(manager, &WorkspaceManager::folderOpened);
    
    manager->openFolder(tempDir->path(), false);
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QCOMPARE(arguments.at(0).toString(), tempDir->path());
}

void TestWorkspaceManager::testSignals_MainFileFound() {
    createFile("main.md");
    QSignalSpy spy(manager, &WorkspaceManager::mainFileFound);
    
    manager->openFolder(tempDir->path(), true, "main.md");
    
    QCOMPARE(spy.count(), 1);
    QList<QVariant> arguments = spy.takeFirst();
    QVERIFY(arguments.at(0).toString().endsWith("main.md"));
}

void TestWorkspaceManager::testSignals_NoMainFileFound() {
    createFile("other.md");
    QSignalSpy spy(manager, &WorkspaceManager::noMainFileFound);
    
    manager->openFolder(tempDir->path(), true, "main.md");
    
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN(TestWorkspaceManager)
#include "test_workspacemanager.moc"
