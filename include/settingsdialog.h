#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>

class QSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();
    
    int getAutoSaveInterval() const;
    bool getAutoSaveEnabled() const;
    QString getDefaultTheme() const;

private slots:
    void saveSettings();

private:
    void setupUI();
    void loadSettings();
    
    QSpinBox *autoSaveIntervalSpinBox;
    QCheckBox *autoSaveEnabledCheck;
    QComboBox *themeComboBox;
    QPushButton *saveButton;
    QPushButton *cancelButton;
};

#endif // SETTINGSDIALOG_H
