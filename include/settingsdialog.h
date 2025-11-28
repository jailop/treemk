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

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    
    int getAutoSaveInterval() const;
    bool getAutoSaveEnabled() const;
    QString getDefaultTheme() const;
    QString getAppTheme() const;
    QString getEditorColorScheme() const;

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
    void setupPreviewTab();
    void setupGeneralTab();
    void setupWikiLinksTab();
    void setupAppearanceTab();
    void setupShortcutsTab();
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

    // Preview settings
    QComboBox *themeComboBox;
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

    // Wiki links settings
    QComboBox *wikiLinkFormatComboBox;
    QCheckBox *relativeLinkPathsCheckBox;
    QCheckBox *autoCompleteLinksCheckBox;
    QCheckBox *showBacklinksCheckBox;
    
    // Appearance settings
    QComboBox *appThemeComboBox;
    QComboBox *editorColorSchemeComboBox;
    
    QPushButton *saveButton;
    QPushButton *cancelButton;
};

#endif // SETTINGSDIALOG_H
