#ifndef WORDPREDICTOR_H
#define WORDPREDICTOR_H

#include <QMap>
#include <QPair>
#include <QString>

class WordPredictor {
public:
    WordPredictor();
    
    void updateFromText(const QString& text);
    void updateFromDirectory(const QString& dirPath, const QString& currentFilePath);
    void clear();
    
    QString predict(const QString& prefix, const QString& previousWord = QString()) const;
    
private:
    QString predictUnigram(const QString& prefix) const;
    QString predictBigram(const QString& previousWord, const QString& prefix) const;
    
    QMap<QString, int> m_wordFrequency;
    QMap<QPair<QString, QString>, int> m_bigramFrequency;
};

#endif // WORDPREDICTOR_H
