#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class QTabWidget;
class QLineEdit;
class QFontComboBox;

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog();

  int getAutoSaveInterval() const;
  bool getAutoSaveEnabled() const;
  QString getDefaultTheme() const;
   QString getAppTheme() const;

signals:
  void settingsChanged();

private slots:
  void saveSettings();
  void applySettings();
  void onBrowseDefaultFolder();
  void onBrowseCustomCSS();
  void onConfigureShortcuts();

private:
  void setupUI();
  void setupEditorTab();
   void setupMainTab();
  void loadSettings();

  QTabWidget *tabWidget;

  // Editor settings
  QFontComboBox *fontComboBox;
  QSpinBox *fontSizeSpinBox;
  QSpinBox *tabWidthSpinBox;
  QCheckBox *wordWrapCheckBox;
  QCheckBox *showLineNumbersCheckBox;
  QCheckBox *highlightCurrentLineCheckBox;
  QCheckBox *enableCodeSyntaxCheckBox;
  QCheckBox *autoIndentCheckBox;
  QCheckBox *autoCloseBracketsCheckBox;
  QCheckBox *enableWordPredictionCheckBox;
  QCheckBox *lineBreakCheckBox;
  QSpinBox *lineBreakColumnsSpinBox;

   // Preview settings
   QSpinBox *previewRefreshRateSpinBox;
  QSpinBox *previewFontSizeSpinBox;
  QLineEdit *customCSSLineEdit;
  QPushButton *browseCSSButton;

  // General settings
  QSpinBox *autoSaveIntervalSpinBox;
  QCheckBox *autoSaveEnabledCheck;
  QLineEdit *defaultFolderLineEdit;
  QPushButton *browseFolderButton;
  QCheckBox *confirmDeleteCheckBox;
  QCheckBox *openLastFolderCheckBox;
  QCheckBox *restoreSessionCheckBox;

   // Appearance settings
   QComboBox *themeComboBox;

   QPushButton *saveButton;
   QPushButton *cancelButton;
   QPushButton *configureShortcutsButton;
};

#endif // SETTINGSDIALOG_H
