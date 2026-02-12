#include "defs.h"
#include "settingsdialog.h"
#include "shortcutsdialog.h"
#include "systempromptsdialog.h"
#include "logic/systemprompts.h"
#include "logic/ollamaprovider.h"
#include "logic/openaiprovider.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QScrollArea>

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  setWindowTitle(tr("Preferences"));
  setMinimumWidth(600);
  setMinimumHeight(500);
  setupUI();
  loadSettings();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setupUI() {
  QVBoxLayout *mainLayout = new QVBoxLayout(this);

   tabWidget = new QTabWidget(this);
   setupMainTab();
   setupEditorTab();
   setupAITab();

  mainLayout->addWidget(tabWidget);

  // Dialog buttons
   QDialogButtonBox *buttonBox = new QDialogButtonBox(
       QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
       this);

    connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
      saveSettings();
      emit settingsChanged();
      accept();
    });
   connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  mainLayout->addWidget(buttonBox);
}

void SettingsDialog::setupEditorTab() {
  QWidget *editorTab = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(editorTab);

  // Font settings group
  QGroupBox *fontGroup = new QGroupBox(tr("Font Settings"));
  QFormLayout *fontLayout = new QFormLayout(fontGroup);

  fontComboBox = new QFontComboBox();
  fontComboBox->setFontFilters(QFontComboBox::AllFonts);
  fontLayout->addRow(tr("Font:"), fontComboBox);

  fontSizeSpinBox = new QSpinBox();
  fontSizeSpinBox->setRange(8, 72);
  fontSizeSpinBox->setSuffix(tr(" pt"));
  fontLayout->addRow(tr("Font Size:"), fontSizeSpinBox);

  layout->addWidget(fontGroup);

  // Editor behavior group
  QGroupBox *behaviorGroup = new QGroupBox(tr("Editor Behavior"));
  QFormLayout *behaviorLayout = new QFormLayout(behaviorGroup);

  tabWidthSpinBox = new QSpinBox();
  tabWidthSpinBox->setRange(1, 16);
  tabWidthSpinBox->setSuffix(tr(" spaces"));
  behaviorLayout->addRow(tr("Tab Width:"), tabWidthSpinBox);

  wordWrapCheckBox = new QCheckBox(tr("Enable word wrap"));
  behaviorLayout->addRow(wordWrapCheckBox);

  showLineNumbersCheckBox = new QCheckBox(tr("Show line numbers"));
  behaviorLayout->addRow(showLineNumbersCheckBox);

  highlightCurrentLineCheckBox = new QCheckBox(tr("Highlight current line"));
  behaviorLayout->addRow(highlightCurrentLineCheckBox);

  enableCodeSyntaxCheckBox =
      new QCheckBox(tr("Enable syntax highlighting in code blocks"));
  behaviorLayout->addRow(enableCodeSyntaxCheckBox);

  autoIndentCheckBox = new QCheckBox(tr("Auto-indent new lines"));
  behaviorLayout->addRow(autoIndentCheckBox);

  autoCloseBracketsCheckBox =
      new QCheckBox(tr("Auto-close brackets and quotes"));
  behaviorLayout->addRow(autoCloseBracketsCheckBox);

   enableWordPredictionCheckBox =
       new QCheckBox(tr("Enable word prediction (Tab to accept)"));
   enableWordPredictionCheckBox->setToolTip(
       tr("Predict words based on document frequency and patterns"));
   behaviorLayout->addRow(enableWordPredictionCheckBox);

   lineBreakCheckBox = new QCheckBox(tr("Enable line breaking"));
   behaviorLayout->addRow(lineBreakCheckBox);

   lineBreakColumnsSpinBox = new QSpinBox();
   lineBreakColumnsSpinBox->setRange(40, 200);
   lineBreakColumnsSpinBox->setValue(80);
   lineBreakColumnsSpinBox->setSuffix(tr(" columns"));
   behaviorLayout->addRow(tr("Line break columns:"), lineBreakColumnsSpinBox);

   connect(lineBreakCheckBox, &QCheckBox::toggled, lineBreakColumnsSpinBox, &QSpinBox::setEnabled);

   layout->addWidget(behaviorGroup);
  layout->addStretch();

  tabWidget->addTab(editorTab, tr("Editor"));
}

void SettingsDialog::setupMainTab() {
  QWidget *mainTab = new QWidget();
  QVBoxLayout *mainLayout = new QVBoxLayout(mainTab);

  // Use scroll area for long content
  QScrollArea *scrollArea = new QScrollArea();
  scrollArea->setWidgetResizable(true);
  QWidget *contentWidget = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(contentWidget);

  // Theme group
  QGroupBox *themeGroup = new QGroupBox(tr("Theme"));
  QFormLayout *themeLayout = new QFormLayout(themeGroup);

  themeComboBox = new QComboBox();
  themeComboBox->addItem(tr("System Default"), "system");
  themeComboBox->addItem(tr("Light"), "light");
  themeComboBox->addItem(tr("Dark"), "dark");
  themeLayout->addRow(tr("Theme:"), themeComboBox);

  QLabel *themeNote = new QLabel(tr("Note: Application restart may be required "
                                    "for theme changes to fully apply."));
  themeNote->setWordWrap(true);
  themeNote->setStyleSheet("color: gray; font-size: 9pt;");
  themeLayout->addRow(themeNote);

  layout->addWidget(themeGroup);

  // Preview appearance group
  QGroupBox *previewGroup = new QGroupBox(tr("Preview"));
  QFormLayout *previewLayout = new QFormLayout(previewGroup);

  previewRefreshRateSpinBox = new QSpinBox();
  previewRefreshRateSpinBox->setRange(100, 2000);
  previewRefreshRateSpinBox->setSuffix(tr(" ms"));
  previewLayout->addRow(tr("Refresh Rate:"), previewRefreshRateSpinBox);

  previewFontSizeSpinBox = new QSpinBox();
  previewFontSizeSpinBox->setRange(8, 72);
  previewFontSizeSpinBox->setSuffix(tr(" pt"));
  previewLayout->addRow(tr("Font Size:"), previewFontSizeSpinBox);

  layout->addWidget(previewGroup);

  // Custom CSS group
  QGroupBox *cssGroup = new QGroupBox(tr("Custom Styling"));
  QVBoxLayout *cssLayout = new QVBoxLayout(cssGroup);

  QLabel *cssLabel = new QLabel(tr("Custom CSS:"));
  cssLayout->addWidget(cssLabel);

  customCSSLineEdit = new QLineEdit();
  cssLayout->addWidget(customCSSLineEdit);

  QLabel *cssBrowseLabel = new QLabel(tr("You can load a custom CSS file to style the preview."));
  cssBrowseLabel->setWordWrap(true);
  cssBrowseLabel->setStyleSheet("color: gray; font-size: 9pt;");
  cssLayout->addWidget(cssBrowseLabel);

  browseCSSButton = new QPushButton(tr("Browse..."));
  cssLayout->addWidget(browseCSSButton);
  connect(browseCSSButton, &QPushButton::clicked, this,
          &SettingsDialog::onBrowseCustomCSS);

  layout->addWidget(cssGroup);

  // General settings group
  QGroupBox *generalGroup = new QGroupBox(tr("General"));
  QFormLayout *generalLayout = new QFormLayout(generalGroup);

  autoSaveEnabledCheck = new QCheckBox(tr("Enable auto-save"));
  generalLayout->addRow(autoSaveEnabledCheck);

  autoSaveIntervalSpinBox = new QSpinBox();
  autoSaveIntervalSpinBox->setRange(1, 60);
  autoSaveIntervalSpinBox->setSuffix(tr(" min"));
  generalLayout->addRow(tr("Auto-save interval:"), autoSaveIntervalSpinBox);

  defaultFolderLineEdit = new QLineEdit();
  generalLayout->addRow(tr("Default folder:"), defaultFolderLineEdit);

  browseFolderButton = new QPushButton(tr("Browse..."));
  generalLayout->addRow(browseFolderButton);
  connect(browseFolderButton, &QPushButton::clicked, this,
          &SettingsDialog::onBrowseDefaultFolder);

  confirmDeleteCheckBox = new QCheckBox(tr("Confirm before deleting files"));
  generalLayout->addRow(confirmDeleteCheckBox);

  openLastFolderCheckBox = new QCheckBox(tr("Open last folder on startup"));
  generalLayout->addRow(openLastFolderCheckBox);

  restoreSessionCheckBox = new QCheckBox(tr("Restore session on startup"));
  generalLayout->addRow(restoreSessionCheckBox);

  layout->addWidget(generalGroup);

  // Workspace settings group
  QGroupBox *workspaceGroup = new QGroupBox(tr("Workspace"));
  QFormLayout *workspaceLayout = new QFormLayout(workspaceGroup);

  mainFileNameLineEdit = new QLineEdit();
  mainFileNameLineEdit->setPlaceholderText(tr("main.md"));
  workspaceLayout->addRow(tr("Main file name:"), mainFileNameLineEdit);

  QLabel *mainFileNote = new QLabel(tr("File to automatically open when opening a folder. "
                                       "Common alternatives: index.md, README.md"));
  mainFileNote->setWordWrap(true);
  mainFileNote->setStyleSheet("color: gray; font-size: 9pt;");
  workspaceLayout->addRow(mainFileNote);

  layout->addWidget(workspaceGroup);

  // Shortcuts group
  QGroupBox *shortcutsGroup = new QGroupBox(tr("Keyboard Shortcuts"));
  QVBoxLayout *shortcutsLayout = new QVBoxLayout(shortcutsGroup);

  QLabel *shortcutsLabel = new QLabel(tr("Configure keyboard shortcuts for editor and application actions."));
  shortcutsLabel->setWordWrap(true);
  shortcutsLayout->addWidget(shortcutsLabel);

  configureShortcutsButton = new QPushButton(tr("Configure Shortcuts..."));
  shortcutsLayout->addWidget(configureShortcutsButton);
  connect(configureShortcutsButton, &QPushButton::clicked, this,
          &SettingsDialog::onConfigureShortcuts);

  layout->addWidget(shortcutsGroup);

  layout->addStretch();

  scrollArea->setWidget(contentWidget);
  mainLayout->addWidget(scrollArea);

  tabWidget->addTab(mainTab, tr("General"));
}

void SettingsDialog::loadSettings() {
  QSettings settings(APP_LABEL, APP_LABEL);

  // Editor settings
  QString fontFamily = settings.value("editor/font", "Sans Serif").toString();
  fontComboBox->setCurrentFont(QFont(fontFamily));
  fontSizeSpinBox->setValue(settings.value("editor/fontSize", 11).toInt());
  tabWidthSpinBox->setValue(settings.value("editor/tabWidth", 4).toInt());
  wordWrapCheckBox->setChecked(
      settings.value("editor/wordWrap", true).toBool());
  showLineNumbersCheckBox->setChecked(
      settings.value("editor/showLineNumbers", true).toBool());
  highlightCurrentLineCheckBox->setChecked(
      settings.value("editor/highlightCurrentLine", true).toBool());
  enableCodeSyntaxCheckBox->setChecked(
      settings.value("editor/enableCodeSyntax", false).toBool());
  autoIndentCheckBox->setChecked(
      settings.value("editor/autoIndent", true).toBool());
  autoCloseBracketsCheckBox->setChecked(
      settings.value("editor/autoCloseBrackets", true).toBool());
   enableWordPredictionCheckBox->setChecked(
       settings.value("editor/enableWordPrediction", true).toBool());
   lineBreakCheckBox->setChecked(
       settings.value("editor/lineBreakEnabled", false).toBool());
   lineBreakColumnsSpinBox->setValue(
       settings.value("editor/lineBreakColumns", 80).toInt());
   lineBreakColumnsSpinBox->setEnabled(lineBreakCheckBox->isChecked());

    // Preview settings
   previewRefreshRateSpinBox->setValue(
      settings.value("preview/refreshRate", 500).toInt());
  previewFontSizeSpinBox->setValue(
      settings.value("preview/fontSize", 14).toInt());
  customCSSLineEdit->setText(
      settings.value("preview/customCSS", "").toString());

  // General settings
  bool autoSaveEnabled = settings.value("autoSaveEnabled", true).toBool();
  autoSaveEnabledCheck->setChecked(autoSaveEnabled);
  autoSaveIntervalSpinBox->setEnabled(autoSaveEnabled);
  autoSaveIntervalSpinBox->setValue(
      settings.value("autoSaveInterval", 60).toInt());
  defaultFolderLineEdit->setText(
      settings.value("general/defaultFolder", "").toString());
  confirmDeleteCheckBox->setChecked(
      settings.value("general/confirmDelete", true).toBool());
  openLastFolderCheckBox->setChecked(
      settings.value("general/openLastFolder", true).toBool());
  restoreSessionCheckBox->setChecked(
      settings.value("general/restoreSession", true).toBool());

  // Workspace settings
  mainFileNameLineEdit->setText(
      settings.value("workspace/mainFileName", "main.md").toString());

   // Appearance settings
   QString theme = settings.value("appearance/appTheme", "system").toString();
   int themeIndex = themeComboBox->findData(theme);
   if (themeIndex >= 0)
     themeComboBox->setCurrentIndex(themeIndex);

   // AI settings
   loadAISettings();
}

void SettingsDialog::saveSettings() {
  QSettings settings(APP_LABEL, APP_LABEL);

  // Editor settings
  settings.setValue("editor/font", fontComboBox->currentFont().family());
  settings.setValue("editor/fontSize", fontSizeSpinBox->value());
  settings.setValue("editor/tabWidth", tabWidthSpinBox->value());
  settings.setValue("editor/wordWrap", wordWrapCheckBox->isChecked());
  settings.setValue("editor/showLineNumbers",
                    showLineNumbersCheckBox->isChecked());
  settings.setValue("editor/highlightCurrentLine",
                    highlightCurrentLineCheckBox->isChecked());
  settings.setValue("editor/enableCodeSyntax",
                    enableCodeSyntaxCheckBox->isChecked());
  settings.setValue("editor/autoIndent", autoIndentCheckBox->isChecked());
  settings.setValue("editor/autoCloseBrackets",
                    autoCloseBracketsCheckBox->isChecked());
   settings.setValue("editor/enableWordPrediction",
                     enableWordPredictionCheckBox->isChecked());
   settings.setValue("editor/lineBreakEnabled", lineBreakCheckBox->isChecked());
   settings.setValue("editor/lineBreakColumns", lineBreakColumnsSpinBox->value());

   // Preview settings
  settings.setValue("previewTheme", themeComboBox->currentData().toString());
  settings.setValue("preview/refreshRate", previewRefreshRateSpinBox->value());
  settings.setValue("preview/fontSize", previewFontSizeSpinBox->value());
  settings.setValue("preview/customCSS", customCSSLineEdit->text());

  // General settings
  settings.setValue("autoSaveEnabled", autoSaveEnabledCheck->isChecked());
  settings.setValue("autoSaveInterval", autoSaveIntervalSpinBox->value());
  settings.setValue("general/defaultFolder", defaultFolderLineEdit->text());
  settings.setValue("general/confirmDelete",
                    confirmDeleteCheckBox->isChecked());
  settings.setValue("general/openLastFolder",
                    openLastFolderCheckBox->isChecked());
  settings.setValue("general/restoreSession",
                    restoreSessionCheckBox->isChecked());

  // Workspace settings
  settings.setValue("workspace/mainFileName", mainFileNameLineEdit->text());

   // Appearance settings
   QString theme = themeComboBox->currentData().toString();
   settings.setValue("appearance/appTheme", theme);
   if (theme == "system") {
     settings.setValue("appearance/editorColorScheme", "auto");
     settings.setValue("appearance/previewColorScheme", "auto");
   } else {
     settings.setValue("appearance/editorColorScheme", theme);
     settings.setValue("appearance/previewColorScheme", theme);
   }

   // AI settings
   saveAISettings();
}

void SettingsDialog::applySettings() {
  saveSettings();
  // emit settingsChanged();
}

void SettingsDialog::onBrowseDefaultFolder() {
  QString folder = QFileDialog::getExistingDirectory(
      this, tr("Select Default Folder"), defaultFolderLineEdit->text(),
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

  if (!folder.isEmpty()) {
    defaultFolderLineEdit->setText(folder);
  }
}

void SettingsDialog::onBrowseCustomCSS() {
  QString file = QFileDialog::getOpenFileName(
      this, tr("Select Custom CSS File"), customCSSLineEdit->text(),
      tr("CSS Files (*.css);;All Files (*)"));

  if (!file.isEmpty()) {
    customCSSLineEdit->setText(file);
  }
}

int SettingsDialog::getAutoSaveInterval() const {
  return autoSaveIntervalSpinBox->value();
}

bool SettingsDialog::getAutoSaveEnabled() const {
  return autoSaveEnabledCheck->isChecked();
}

QString SettingsDialog::getDefaultTheme() const {
  return themeComboBox->currentData().toString();
}

QString SettingsDialog::getAppTheme() const {
  return themeComboBox->currentData().toString();
}

QString SettingsDialog::getMainFileName() const {
  QString fileName = mainFileNameLineEdit->text().trimmed();
  return fileName.isEmpty() ? "main.md" : fileName;
}



void SettingsDialog::onConfigureShortcuts() {
  ShortcutsDialog *dialog = new ShortcutsDialog(this);
  dialog->exec();
  delete dialog;
}
