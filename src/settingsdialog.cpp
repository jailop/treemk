#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QSpinBox>
#include <QCheckBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>
#include <QSettings>

SettingsDialog::SettingsDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Settings"));
    setMinimumWidth(400);
    
    setupUI();
    loadSettings();
}

SettingsDialog::~SettingsDialog()
{
}

void SettingsDialog::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // Editor settings
    QGroupBox *editorGroup = new QGroupBox(tr("Editor Settings"), this);
    QFormLayout *editorLayout = new QFormLayout(editorGroup);
    
    autoSaveEnabledCheck = new QCheckBox(tr("Enable auto-save"), this);
    editorLayout->addRow(autoSaveEnabledCheck);
    
    autoSaveIntervalSpinBox = new QSpinBox(this);
    autoSaveIntervalSpinBox->setMinimum(10);
    autoSaveIntervalSpinBox->setMaximum(600);
    autoSaveIntervalSpinBox->setSuffix(tr(" seconds"));
    autoSaveIntervalSpinBox->setValue(60);
    editorLayout->addRow(tr("Auto-save interval:"), autoSaveIntervalSpinBox);
    
    mainLayout->addWidget(editorGroup);
    
    // Preview settings
    QGroupBox *previewGroup = new QGroupBox(tr("Preview Settings"), this);
    QFormLayout *previewLayout = new QFormLayout(previewGroup);
    
    themeComboBox = new QComboBox(this);
    themeComboBox->addItem(tr("Light"), "light");
    themeComboBox->addItem(tr("Dark"), "dark");
    themeComboBox->addItem(tr("Sepia"), "sepia");
    previewLayout->addRow(tr("Default theme:"), themeComboBox);
    
    mainLayout->addWidget(previewGroup);
    
    // Buttons
    mainLayout->addStretch();
    
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    saveButton = new QPushButton(tr("Save"), this);
    cancelButton = new QPushButton(tr("Cancel"), this);
    
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(saveButton, &QPushButton::clicked, this, &SettingsDialog::saveSettings);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    
    connect(autoSaveEnabledCheck, &QCheckBox::toggled, 
            autoSaveIntervalSpinBox, &QSpinBox::setEnabled);
}

void SettingsDialog::loadSettings()
{
    QSettings settings("MkEd", "MkEd");
    
    bool autoSaveEnabled = settings.value("autoSaveEnabled", true).toBool();
    autoSaveEnabledCheck->setChecked(autoSaveEnabled);
    autoSaveIntervalSpinBox->setEnabled(autoSaveEnabled);
    
    int autoSaveInterval = settings.value("autoSaveInterval", 60).toInt();
    autoSaveIntervalSpinBox->setValue(autoSaveInterval);
    
    QString theme = settings.value("previewTheme", "light").toString();
    int index = themeComboBox->findData(theme);
    if (index >= 0) {
        themeComboBox->setCurrentIndex(index);
    }
}

void SettingsDialog::saveSettings()
{
    QSettings settings("MkEd", "MkEd");
    
    settings.setValue("autoSaveEnabled", autoSaveEnabledCheck->isChecked());
    settings.setValue("autoSaveInterval", autoSaveIntervalSpinBox->value());
    settings.setValue("previewTheme", themeComboBox->currentData().toString());
    
    accept();
}

int SettingsDialog::getAutoSaveInterval() const
{
    return autoSaveIntervalSpinBox->value();
}

bool SettingsDialog::getAutoSaveEnabled() const
{
    return autoSaveEnabledCheck->isChecked();
}

QString SettingsDialog::getDefaultTheme() const
{
    return themeComboBox->currentData().toString();
}
