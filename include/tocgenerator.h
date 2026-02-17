#ifndef TOCGENERATOR_H
#define TOCGENERATOR_H

#include <QObject>
#include <QString>
#include <QStringList>

class TocGenerator : public QObject {
    Q_OBJECT

public:
    struct TocEntry {
        int level;
        QString text;
        QString anchor;
    };

    static QString generateToc(const QString& markdownText);
    static QString removeToc(const QString& markdownText);
    static bool hasToc(const QString& markdownText);
    
private:
    static QList<TocEntry> extractHeaders(const QString& markdownText);
    static QString generateAnchor(const QString& headerText);
    static int findTocPosition(const QString& markdownText);
    static QString tocToMarkdown(const QList<TocEntry>& entries);
};

#endif
