#include "searchengine.h"
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QRegularExpression>

SearchEngine::SearchEngine() 
    : m_contextSize(100), m_maxResultsPerFile(10) {
}

SearchEngine::~SearchEngine() {
}

void SearchEngine::setContextSize(int size) {
  m_contextSize = qMax(10, size); // Minimum 10 characters
}

int SearchEngine::contextSize() const {
  return m_contextSize;
}

void SearchEngine::setMaxResultsPerFile(int max) {
  m_maxResultsPerFile = max;
}

int SearchEngine::maxResultsPerFile() const {
  return m_maxResultsPerFile;
}

QList<SearchEngine::SearchResult> SearchEngine::searchInFile(
    const QString &filePath, 
    const QString &searchTerm,
    bool caseSensitive) {
  
  QList<SearchResult> results;
  
  if (searchTerm.trimmed().isEmpty()) {
    return results;
  }
  
  QFile file(filePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return results;
  }
  
  QTextStream in(&file);
  in.setEncoding(QStringConverter::Utf8);
  QString content = in.readAll();
  file.close();
  
  QFileInfo fileInfo(filePath);
  QString fileName = fileInfo.fileName();
  QString title = extractTitle(content);
  
  // Search for all occurrences
  Qt::CaseSensitivity cs = caseSensitive ? Qt::CaseSensitive : Qt::CaseInsensitive;
  int pos = 0;
  int count = 0;
  
  while ((pos = content.indexOf(searchTerm, pos, cs)) != -1) {
    SearchResult result;
    result.filePath = filePath;
    result.fileName = fileName;
    result.title = title;
    result.position = pos;
    result.lineNumber = findLineNumber(content, pos);
    result.context = extractContext(content, pos, searchTerm);
    result.matchedText = content.mid(pos, searchTerm.length());
    
    results.append(result);
    
    pos += searchTerm.length();
    count++;
    
    // Limit results per file if configured
    if (m_maxResultsPerFile > 0 && count >= m_maxResultsPerFile) {
      break;
    }
  }
  
  return results;
}

QList<SearchEngine::SearchResult> SearchEngine::searchInFiles(
    const QStringList &filePaths,
    const QString &searchTerm,
    bool caseSensitive) {
  
  QList<SearchResult> allResults;
  
  for (const QString &filePath : filePaths) {
    QList<SearchResult> fileResults = searchInFile(filePath, searchTerm, caseSensitive);
    allResults.append(fileResults);
  }
  
  return allResults;
}

QString SearchEngine::extractContext(const QString &content, int position, const QString &searchTerm) {
  int start = qMax(0, position - m_contextSize);
  int end = qMin(content.length(), position + searchTerm.length() + m_contextSize);
  
  QString context = content.mid(start, end - start);
  
  // Clean up context
  context = sanitizeContext(context);
  
  // Add ellipsis if we're not at the beginning/end
  if (start > 0) {
    context = "..." + context;
  }
  if (end < content.length()) {
    context = context + "...";
  }
  
  return context;
}

int SearchEngine::findLineNumber(const QString &content, int position) {
  int lineNumber = 1;
  for (int i = 0; i < position && i < content.length(); ++i) {
    if (content[i] == '\n') {
      lineNumber++;
    }
  }
  return lineNumber;
}

QString SearchEngine::extractTitle(const QString &content) {
  // Look for first H1 heading (# Title)
  QRegularExpression h1Regex("^#\\s+(.+)$", QRegularExpression::MultilineOption);
  QRegularExpressionMatch match = h1Regex.match(content);
  
  if (match.hasMatch()) {
    return match.captured(1).trimmed();
  }
  
  // Look for any heading
  QRegularExpression headingRegex("^#{1,6}\\s+(.+)$", QRegularExpression::MultilineOption);
  match = headingRegex.match(content);
  
  if (match.hasMatch()) {
    return match.captured(1).trimmed();
  }
  
  // No heading found, return first line
  int newlinePos = content.indexOf('\n');
  if (newlinePos > 0) {
    QString firstLine = content.left(newlinePos).trimmed();
    if (!firstLine.isEmpty() && firstLine.length() < 100) {
      return firstLine;
    }
  }
  
  return QString();
}

QString SearchEngine::sanitizeContext(const QString &context) {
  QString clean = context;
  
  // Replace multiple whitespace with single space
  clean = clean.simplified();
  
  // Remove markdown formatting characters for cleaner display
  // (but keep the text readable)
  clean.replace(QRegularExpression("^#+\\s+"), ""); // Remove heading markers
  clean.replace(QRegularExpression("\\*\\*(.+?)\\*\\*"), "\\1"); // Remove bold
  clean.replace(QRegularExpression("\\*(.+?)\\*"), "\\1"); // Remove italic
  clean.replace(QRegularExpression("`(.+?)`"), "\\1"); // Remove code marks
  clean.replace(QRegularExpression("\\[([^]]+)\\]\\([^)]+\\)"), "\\1"); // Links
  
  return clean;
}
