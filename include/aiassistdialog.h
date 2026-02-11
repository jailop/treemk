#ifndef AIASSISTDIALOG_H
#define AIASSISTDIALOG_H

#include <QDialog>
#include <QStack>
#include <QDateTime>

namespace Ui {
class AIAssistDialog;
}

class AIAssistDialog : public QDialog {
  Q_OBJECT

public:
  explicit AIAssistDialog(QWidget *parent = nullptr, 
                          const QString &selectedText = QString(),
                          int cursorPos = 0,
                          int selectionStart = 0,
                          int selectionEnd = 0);
  ~AIAssistDialog();

signals:
  void insertText(int position, const QString &text);
  void replaceText(int start, int end, const QString &text);

private slots:
  void processWithAI();
  void applyToEditor();
  void copyToClipboard();
  void navigatePrevious();
  void navigateNext();
  void onSystemPromptChanged(int index);

private:
  struct HistoryEntry {
    QString prompt;
    QString content;
    QDateTime timestamp;
  };

  void setupUI();
  void loadSystemPrompts();
  void updateHistoryButtons();
  void updateApplyButtonText();
  void setProcessingState(bool processing);
  void saveCurrentState();
  void restoreHistoryEntry(int index);

  Ui::AIAssistDialog *ui;
  
  QStack<HistoryEntry> historyStack;
  int historyIndex;
  
  bool wasTextSelected;
  int cursorPosition;
  int selectionStart;
  int selectionEnd;
};

#endif // AIASSISTDIALOG_H
