#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class QTabWidget;
class QLineEdit;
class QLabel;
class QFontComboBox;
class QGroupBox;

class SettingsDialog : public QDialog {
  Q_OBJECT

public:
  explicit SettingsDialog(QWidget *parent = nullptr);
  ~SettingsDialog();

  int getAutoSaveInterval() const;
  bool getAutoSaveEnabled() const;
  QString getDefaultTheme() const;
  QString getAppTheme() const;
  QString getMainFileName() const;

signals:
  void settingsChanged();

private slots:
  void saveSettings();
  void applySettings();
  void onBrowseDefaultFolder();
  void onBrowseCustomCSS();
  void onConfigureShortcuts();
  void onManageSystemPrompts();
  void onRefreshOllamaModels();
  void onTestOllamaConnection();
  void onRefreshOpenAIModels();
  void onTestOpenAIConnection();
  void onAIEnabledChanged(int state);
  void onProviderChanged(int index);

private:
  void setupUI();
  void setupEditorTab();
   void setupMainTab();
  void loadSettings();
  void setupAITab();
  void loadAISettings();
  void saveAISettings();
  void detectAIProviders();
  void updatePromptsInfo();

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

  // Workspace settings

   // Appearance settings
  QLineEdit *mainFileNameLineEdit;
   QComboBox *themeComboBox;
  // AI settings
  QCheckBox *aiEnabledCheckBox;
  QComboBox *aiProviderComboBox;
  // Ollama settings
  QGroupBox *ollamaGroup;
  QLineEdit *ollamaEndpointLineEdit;
  QComboBox *ollamaModelComboBox;
  QSpinBox *ollamaTimeoutSpinBox;
  QPushButton *refreshModelsButton;
  // OpenAI settings
  QGroupBox *openaiGroup;
  QLineEdit *openaiEndpointLineEdit;
  QLineEdit *openaiApiKeyLineEdit;
  QComboBox *openaiModelComboBox;
  QSpinBox *openaiTimeoutSpinBox;
  QPushButton *refreshOpenAIModelsButton;
  // Common AI widgets
  QPushButton *testConnectionButton;
  QLabel *connectionStatusLabel;
  QLabel *promptsInfoLabel;
  QPushButton *managePromptsButton;

  // Appearance settings
  QPushButton *saveButton;
  QPushButton *cancelButton;
  QPushButton *configureShortcutsButton;
};

#endif // SETTINGSDIALOG_H
