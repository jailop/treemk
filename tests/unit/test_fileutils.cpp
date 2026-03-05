#include "fileutils.h"

#include <QTest>

class TestFileUtils : public QObject {
    Q_OBJECT

private slots:
    void testIsImageFile();
    void testIsMarkdownFile();
};

void TestFileUtils::testIsImageFile() {
    QVERIFY(FileUtils::isImageFile("image.png"));
    QVERIFY(FileUtils::isImageFile("photo.jpg"));
    QVERIFY(FileUtils::isImageFile("picture.JPEG"));
    QVERIFY(FileUtils::isImageFile("icon.gif"));
    QVERIFY(FileUtils::isImageFile("bitmap.bmp"));
    QVERIFY(FileUtils::isImageFile("vector.svg"));
    QVERIFY(FileUtils::isImageFile("modern.webp"));
    QVERIFY(FileUtils::isImageFile("/path/to/image.PNG"));
    
    QVERIFY(!FileUtils::isImageFile("document.md"));
    QVERIFY(!FileUtils::isImageFile("file.txt"));
    QVERIFY(!FileUtils::isImageFile("data.json"));
    QVERIFY(!FileUtils::isImageFile("noextension"));
}

void TestFileUtils::testIsMarkdownFile() {
    QVERIFY(FileUtils::isMarkdownFile("note.md"));
    QVERIFY(FileUtils::isMarkdownFile("README.MD"));
    QVERIFY(FileUtils::isMarkdownFile("doc.markdown"));
    QVERIFY(FileUtils::isMarkdownFile("file.txt"));
    QVERIFY(FileUtils::isMarkdownFile("/path/to/document.md"));
    
    QVERIFY(!FileUtils::isMarkdownFile("image.png"));
    QVERIFY(!FileUtils::isMarkdownFile("data.json"));
    QVERIFY(!FileUtils::isMarkdownFile("style.css"));
    QVERIFY(!FileUtils::isMarkdownFile("noextension"));
}

QTEST_MAIN(TestFileUtils)
#include "test_fileutils.moc"
