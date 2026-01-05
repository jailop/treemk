#include "settingsdialog.h"
#include "defs.h"
#include "shortcutsdialog.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFontComboBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>
#include <QVBoxLayout>

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
  setupAppearanceTab();
  setupEditorTab();
  setupPreviewTab();
  setupGeneralTab();
  setupWikiLinksTab();
  setupShortcutsTab();

  mainLayout->addWidget(tabWidget);

  // Dialog buttons
  QDialogButtonBox *buttonBox = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Apply,
      this);

  connect(buttonBox, &QDialogButtonBox::accepted, this, [this]() {
    saveSettings();
    emit settingsChanged();
    accept();
  });
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
  connect(buttonBox->button(QDialogButtonBox::Apply), &QPushButton::clicked,
          this, &SettingsDialog::applySettings);
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

  layout->addWidget(behaviorGroup);
  layout->addStretch();

  tabWidget->addTab(editorTab, tr("Editor"));
}

void SettingsDialog::setupPreviewTab() {
  QWidget *previewTab = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(previewTab);

  // Preview appearance group
  QGroupBox *appearanceGroup = new QGroupBox(tr("Appearance"));
  QFormLayout *appearanceLayout = new QFormLayout(appearanceGroup);

  themeComboBox = new QComboBox();
  themeComboBox->addItem(tr("Light"), "light");
  themeComboBox->addItem(tr("Dark"), "dark");
  themeComboBox->addItem(tr("Sepia"), "sepia");
  appearanceLayout->addRow(tr("Theme:"), themeComboBox);

  previewFontSizeSpinBox = new QSpinBox();
  previewFontSizeSpinBox->setRange(8, 72);
  previewFontSizeSpinBox->setSuffix(tr(" pt"));
  appearanceLayout->addRow(tr("Font Size:"), previewFontSizeSpinBox);

  layout->addWidget(appearanceGroup);

  // Custom CSS group
  QGroupBox *cssGroup = new QGroupBox(tr("Custom Styling"));
  QVBoxLayout *cssLayout = new QVBoxLayout(cssGroup);

  QLabel *cssLabel =
      new QLabel(tr("Custom CSS file (leave empty for default):"));
  cssLayout->addWidget(cssLabel);

  QHBoxLayout *cssFileLayout = new QHBoxLayout();
  customCSSLineEdit = new QLineEdit();
  customCSSLineEdit->setPlaceholderText(tr("Path to custom CSS file..."));
  browseCSSButton = new QPushButton(tr("Browse..."));
  connect(browseCSSButton, &QPushButton::clicked, this,
          &SettingsDialog::onBrowseCustomCSS);

  cssFileLayout->addWidget(customCSSLineEdit);
  cssFileLayout->addWidget(browseCSSButton);
  cssLayout->addLayout(cssFileLayout);

  layout->addWidget(cssGroup);

  // Performance group
  QGroupBox *performanceGroup = new QGroupBox(tr("Performance"));
  QFormLayout *performanceLayout = new QFormLayout(performanceGroup);

  previewRefreshRateSpinBox = new QSpinBox();
  previewRefreshRateSpinBox->setRange(100, 5000);
  previewRefreshRateSpinBox->setSingleStep(100);
  previewRefreshRateSpinBox->setSuffix(tr(" ms"));
  performanceLayout->addRow(tr("Refresh Rate:"), previewRefreshRateSpinBox);

  layout->addWidget(performanceGroup);
  layout->addStretch();

  tabWidget->addTab(previewTab, tr("Preview"));
}

void SettingsDialog::setupGeneralTab() {
  QWidget *generalTab = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(generalTab);

  // Auto-save group
  QGroupBox *autoSaveGroup = new QGroupBox(tr("Auto-Save"));
  QVBoxLayout *autoSaveLayout = new QVBoxLayout(autoSaveGroup);

  autoSaveEnabledCheck = new QCheckBox(tr("Enable auto-save"));
  autoSaveLayout->addWidget(autoSaveEnabledCheck);

  QFormLayout *autoSaveFormLayout = new QFormLayout();
  autoSaveIntervalSpinBox = new QSpinBox();
  autoSaveIntervalSpinBox->setRange(10, 600);
  autoSaveIntervalSpinBox->setSuffix(tr(" seconds"));
  autoSaveFormLayout->addRow(tr("Interval:"), autoSaveIntervalSpinBox);
  autoSaveLayout->addLayout(autoSaveFormLayout);

  layout->addWidget(autoSaveGroup);

  // File locations group
  QGroupBox *locationsGroup = new QGroupBox(tr("File Locations"));
  QVBoxLayout *locationsLayout = new QVBoxLayout(locationsGroup);

  QLabel *folderLabel = new QLabel(tr("Default folder for new files:"));
  locationsLayout->addWidget(folderLabel);

  QHBoxLayout *folderLayout = new QHBoxLayout();
  defaultFolderLineEdit = new QLineEdit();
  defaultFolderLineEdit->setPlaceholderText(
      tr("Leave empty to use last opened folder"));
  browseFolderButton = new QPushButton(tr("Browse..."));
  connect(browseFolderButton, &QPushButton::clicked, this,
          &SettingsDialog::onBrowseDefaultFolder);

  folderLayout->addWidget(defaultFolderLineEdit);
  folderLayout->addWidget(browseFolderButton);
  locationsLayout->addLayout(folderLayout);

  layout->addWidget(locationsGroup);

  // Behavior group
  QGroupBox *behaviorGroup = new QGroupBox(tr("Behavior"));
  QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);

  openLastFolderCheckBox = new QCheckBox(tr("Open last folder on startup"));
  behaviorLayout->addWidget(openLastFolderCheckBox);

  confirmDeleteCheckBox = new QCheckBox(tr("Confirm before deleting files"));
  behaviorLayout->addWidget(confirmDeleteCheckBox);

  restoreSessionCheckBox = new QCheckBox(tr("Restore open files on startup"));
  behaviorLayout->addWidget(restoreSessionCheckBox);

  layout->addWidget(behaviorGroup);
  layout->addStretch();

  // Connect auto-save checkbox to enable/disable interval spinbox
  connect(autoSaveEnabledCheck, &QCheckBox::toggled, autoSaveIntervalSpinBox,
          &QSpinBox::setEnabled);

  tabWidget->addTab(generalTab, tr("General"));
}

void SettingsDialog::setupWikiLinksTab() {
  QWidget *wikiTab = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(wikiTab);

  // Link format group
  QGroupBox *formatGroup = new QGroupBox(tr("Link Format"));
  QFormLayout *formatLayout = new QFormLayout(formatGroup);

  wikiLinkFormatComboBox = new QComboBox();
  wikiLinkFormatComboBox->addItem("[[Note]]", "double-bracket");
  wikiLinkFormatComboBox->addItem("[[Note|Display]]", "with-display");
  wikiLinkFormatComboBox->addItem("[Display](note.md)", "markdown");
  formatLayout->addRow(tr("Default Format:"), wikiLinkFormatComboBox);

  layout->addWidget(formatGroup);

  // Link behavior group
  QGroupBox *behaviorGroup = new QGroupBox(tr("Link Behavior"));
  QVBoxLayout *behaviorLayout = new QVBoxLayout(behaviorGroup);

  relativeLinkPathsCheckBox = new QCheckBox(tr("Use relative paths for links"));
  relativeLinkPathsCheckBox->setToolTip(
      tr("Convert absolute paths to relative when inserting links"));
  behaviorLayout->addWidget(relativeLinkPathsCheckBox);

  autoCompleteLinksCheckBox =
      new QCheckBox(tr("Enable auto-completion for wiki links"));
  autoCompleteLinksCheckBox->setToolTip(tr("Show suggestions when typing [["));
  behaviorLayout->addWidget(autoCompleteLinksCheckBox);

  showBacklinksCheckBox = new QCheckBox(tr("Show backlinks panel"));
  showBacklinksCheckBox->setToolTip(
      tr("Display files that link to the current note"));
  behaviorLayout->addWidget(showBacklinksCheckBox);

  layout->addWidget(behaviorGroup);
  layout->addStretch();

  tabWidget->addTab(wikiTab, tr("Wiki Links"));
}

void SettingsDialog::setupAppearanceTab() {
  QWidget *appearanceTab = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(appearanceTab);

  // Application theme group
  QGroupBox *appThemeGroup = new QGroupBox(tr("Application Theme"));
  QFormLayout *appThemeLayout = new QFormLayout(appThemeGroup);

  appThemeComboBox = new QComboBox();
  appThemeComboBox->addItem(tr("System Default"), "system");
  appThemeComboBox->addItem(tr("Light"), "light");
  appThemeComboBox->addItem(tr("Dark"), "dark");
  appThemeLayout->addRow(tr("Theme:"), appThemeComboBox);

  QLabel *themeNote = new QLabel(tr("Note: Application restart may be required "
                                    "for theme changes to fully apply."));
  themeNote->setWordWrap(true);
  themeNote->setStyleSheet("color: gray; font-size: 9pt;");
  appThemeLayout->addRow(themeNote);

  layout->addWidget(appThemeGroup);

  // Editor color scheme group
  QGroupBox *editorSchemeGroup = new QGroupBox(tr("Editor Color Scheme"));
  QFormLayout *editorSchemeLayout = new QFormLayout(editorSchemeGroup);

  editorColorSchemeComboBox = new QComboBox();
  editorColorSchemeComboBox->addItem(tr("Auto (Follow App Theme)"), "auto");
  editorColorSchemeComboBox->addItem(tr("Light"), "light");
  editorColorSchemeComboBox->addItem(tr("Dark"), "dark");
  editorColorSchemeComboBox->addItem(tr("Solarized Light"), "solarized-light");
  editorColorSchemeComboBox->addItem(tr("Solarized Dark"), "solarized-dark");
  editorSchemeLayout->addRow(tr("Color Scheme:"), editorColorSchemeComboBox);

  layout->addWidget(editorSchemeGroup);

  // Preview Color Scheme
  QGroupBox *previewSchemeGroup = new QGroupBox(tr("Preview Color Scheme"));
  QFormLayout *previewSchemeLayout = new QFormLayout(previewSchemeGroup);

  previewColorSchemeComboBox = new QComboBox();
  previewColorSchemeComboBox->addItem(tr("Auto (Follow App Theme)"), "auto");
  previewColorSchemeComboBox->addItem(tr("Light"), "light");
  previewColorSchemeComboBox->addItem(tr("Dark"), "dark");
  previewSchemeLayout->addRow(tr("Color Scheme:"), previewColorSchemeComboBox);

  layout->addWidget(previewSchemeGroup);
  layout->addStretch();

  tabWidget->addTab(appearanceTab, tr("Appearance"));
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

  // Preview settings
  QString theme = settings.value("previewTheme", "light").toString();
  int themeIndex = themeComboBox->findData(theme);
  if (themeIndex >= 0)
    themeComboBox->setCurrentIndex(themeIndex);

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

  // Wiki links settings
  QString linkFormat =
      settings.value("wikiLinks/format", "double-bracket").toString();
  int linkFormatIndex = wikiLinkFormatComboBox->findData(linkFormat);
  if (linkFormatIndex >= 0)
    wikiLinkFormatComboBox->setCurrentIndex(linkFormatIndex);

  relativeLinkPathsCheckBox->setChecked(
      settings.value("wikiLinks/relativePaths", true).toBool());
  autoCompleteLinksCheckBox->setChecked(
      settings.value("wikiLinks/autoComplete", true).toBool());
  showBacklinksCheckBox->setChecked(
      settings.value("wikiLinks/showBacklinks", true).toBool());

  // Appearance settings
  QString appTheme = settings.value("appearance/appTheme", "system").toString();
  int appThemeIndex = appThemeComboBox->findData(appTheme);
  if (appThemeIndex >= 0)
    appThemeComboBox->setCurrentIndex(appThemeIndex);

  QString editorScheme =
      settings.value("appearance/editorColorScheme", "auto").toString();
  int editorSchemeIndex = editorColorSchemeComboBox->findData(editorScheme);
  if (editorSchemeIndex >= 0)
    editorColorSchemeComboBox->setCurrentIndex(editorSchemeIndex);

  QString previewScheme =
      settings.value("appearance/previewColorScheme", "auto").toString();
  int previewSchemeIndex = previewColorSchemeComboBox->findData(previewScheme);
  if (previewSchemeIndex >= 0)
    previewColorSchemeComboBox->setCurrentIndex(previewSchemeIndex);
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

  // Wiki links settings
  settings.setValue("wikiLinks/format",
                    wikiLinkFormatComboBox->currentData().toString());
  settings.setValue("wikiLinks/relativePaths",
                    relativeLinkPathsCheckBox->isChecked());
  settings.setValue("wikiLinks/autoComplete",
                    autoCompleteLinksCheckBox->isChecked());
  settings.setValue("wikiLinks/showBacklinks",
                    showBacklinksCheckBox->isChecked());

  // Appearance settings
  settings.setValue("appearance/appTheme",
                    appThemeComboBox->currentData().toString());
  settings.setValue("appearance/editorColorScheme",
                    editorColorSchemeComboBox->currentData().toString());
  settings.setValue("appearance/previewColorScheme",
                    previewColorSchemeComboBox->currentData().toString());
}

void SettingsDialog::applySettings() {
  saveSettings();
  emit settingsChanged();
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
  return appThemeComboBox->currentData().toString();
}

QString SettingsDialog::getEditorColorScheme() const {
  return editorColorSchemeComboBox->currentData().toString();
}

void SettingsDialog::setupShortcutsTab() {
  QWidget *shortcutsTab = new QWidget();
  QVBoxLayout *layout = new QVBoxLayout(shortcutsTab);

  QLabel *infoLabel = new QLabel(tr("Configure keyboard shortcuts for editor "
                                    "navigation and text manipulation."),
                                 shortcutsTab);
  infoLabel->setWordWrap(true);
  layout->addWidget(infoLabel);

  QPushButton *configureButton =
      new QPushButton(tr("Configure Shortcuts..."), shortcutsTab);
  configureButton->setMinimumHeight(40);
  connect(configureButton, &QPushButton::clicked, this,
          &SettingsDialog::onConfigureShortcuts);

  QHBoxLayout *buttonLayout = new QHBoxLayout();
  buttonLayout->addStretch();
  buttonLayout->addWidget(configureButton);
  buttonLayout->addStretch();

  layout->addLayout(buttonLayout);
  layout->addStretch();

  tabWidget->addTab(shortcutsTab, tr("Shortcuts"));
}

void SettingsDialog::onConfigureShortcuts() {
  ShortcutsDialog *dialog = new ShortcutsDialog(this);
  dialog->exec();
  delete dialog;
}
