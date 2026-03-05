#include "wordpredictor.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QRegularExpression>

#include "regexpatterns.h"

WordPredictor::WordPredictor() {}

void WordPredictor::clear() {
    m_wordFrequency.clear();
    m_bigramFrequency.clear();
}

void WordPredictor::updateFromText(const QString& text) {
    m_wordFrequency.clear();
    m_bigramFrequency.clear();

    QRegularExpression wordRegex(RegexPatterns::WORD_BOUNDARY);
    QRegularExpressionMatchIterator it = wordRegex.globalMatch(text);

    QStringList words;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString word = match.captured(0).toLower();
        words.append(word);
        m_wordFrequency[word]++;
    }

    for (int i = 0; i < words.size() - 1; i++) {
        QPair<QString, QString> bigram(words[i], words[i + 1]);
        m_bigramFrequency[bigram]++;
    }
}

void WordPredictor::updateFromDirectory(const QString& dirPath,
                                        const QString& currentFilePath) {
    QDir dir(dirPath);
    if (!dir.exists()) {
        return;
    }

    QStringList filters;
    filters << "*.md" << "*.markdown";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);

    QRegularExpression wordRegex(RegexPatterns::WORD_BOUNDARY);

    for (const QFileInfo& file : files) {
        if (file.absoluteFilePath() == currentFilePath) {
            continue;
        }

        QFile f(file.absoluteFilePath());
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        QString content = QString::fromUtf8(f.readAll());
        f.close();

        QRegularExpressionMatchIterator it = wordRegex.globalMatch(content);
        QStringList words;

        while (it.hasNext()) {
            QRegularExpressionMatch match = it.next();
            QString word = match.captured(0).toLower();
            words.append(word);
            m_wordFrequency[word]++;
        }

        for (int i = 0; i < words.size() - 1; i++) {
            QPair<QString, QString> bigram(words[i], words[i + 1]);
            m_bigramFrequency[bigram]++;
        }
    }
}

QString WordPredictor::predictUnigram(const QString& prefix) const {
    if (prefix.length() < 2) {
        return QString();
    }

    QString bestMatch;
    int maxFrequency = 0;

    for (auto it = m_wordFrequency.constBegin();
         it != m_wordFrequency.constEnd(); ++it) {
        const QString& word = it.key();
        if (word.startsWith(prefix, Qt::CaseInsensitive) &&
            word.length() > prefix.length()) {
            if (it.value() > maxFrequency) {
                maxFrequency = it.value();
                bestMatch = word;
            }
        }
    }

    return bestMatch;
}

QString WordPredictor::predictBigram(const QString& previousWord,
                                     const QString& prefix) const {
    if (previousWord.isEmpty() || prefix.length() < 1) {
        return QString();
    }

    QString prevLower = previousWord.toLower();
    QString prefixLower = prefix.toLower();

    QString bestMatch;
    int maxFrequency = 0;

    for (auto it = m_bigramFrequency.constBegin();
         it != m_bigramFrequency.constEnd(); ++it) {
        const QPair<QString, QString>& bigram = it.key();

        if (bigram.first == prevLower &&
            bigram.second.startsWith(prefixLower) &&
            bigram.second.length() > prefixLower.length()) {
            if (it.value() > maxFrequency) {
                maxFrequency = it.value();
                bestMatch = bigram.second;
            }
        }
    }

    return bestMatch;
}

QString WordPredictor::predict(const QString& prefix,
                               const QString& previousWord) const {
    if (prefix.length() < 1) {
        return QString();
    }

    QString bigramPrediction = predictBigram(previousWord, prefix);
    QString unigramPrediction = predictUnigram(prefix);

    if (!bigramPrediction.isEmpty()) {
        QString prevLower = previousWord.toLower();
        QPair<QString, QString> bigram(prevLower, bigramPrediction.toLower());
        int bigramFreq = m_bigramFrequency.value(bigram, 0);
        int unigramFreq = m_wordFrequency.value(unigramPrediction.toLower(), 0);

        if (bigramFreq * 2 >= unigramFreq || unigramPrediction.isEmpty()) {
            return bigramPrediction;
        }
    }

    return unigramPrediction;
}
