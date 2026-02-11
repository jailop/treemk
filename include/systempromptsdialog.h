#ifndef SYSTEMPROMPTSDIALOG_H
#define SYSTEMPROMPTSDIALOG_H

#include <QDialog>

namespace Ui {
class SystemPromptsDialog;
}

struct SystemPrompt;

class SystemPromptsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SystemPromptsDialog(QWidget *parent = nullptr);
  ~SystemPromptsDialog();

private slots:
  void addPrompt();
  void editPrompt();
  void deletePrompt();
  void movePromptUp();
  void movePromptDown();
  void resetToDefaults();
  void onPromptSelectionChanged();
  void onPromptToggled();
  void saveChanges();

private:
  void loadPrompts();
  void updatePreview();
  void updateButtonStates();
  bool validatePrompt(const QString &name, const QString &prompt);

  Ui::SystemPromptsDialog *ui;
  QList<SystemPrompt> currentPrompts;
  int selectedRow;
};

#endif // SYSTEMPROMPTSDIALOG_H
