#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

#include "colorpalette.h"
#include "defs.h"
#include "logic/ollamaprovider.h"
#include "logic/openaiprovider.h"
#include "logic/systemprompts.h"
#include "settingsdialog.h"
#include "systempromptsdialog.h"

const int timeOutDefault = 60;
const QString ollamaDefaultUrl = "http://localhost:11434";
const QString openAiDefaultUrl = "https://api.openai.com/v1";

static QString getStatusColor(const QString& type, bool isDark) {
    const auto& colors = isDark ? ColorPalette::getDarkTheme() : 
        ColorPalette::getLightTheme();
    if (type == "info")
        return ColorPalette::toHexString(colors.statusInfo);
    if (type == "success")
        return ColorPalette::toHexString(colors.statusSuccess);
    if (type == "error")
        return ColorPalette::toHexString(colors.statusError);
    if (type == "warning")
        return ColorPalette::toHexString(colors.statusWarning);
    return ColorPalette::toHexString(colors.textSecondary);
}

void SettingsDialog::detectAIProviders() {
    QString ollamaHost = qEnvironmentVariable("OLLAMA_HOST");
    if (!ollamaHost.isEmpty()) {
        ollamaEndpointLineEdit->setText(ollamaHost);
    } else {
        QSettings settings;
        QString endpoint =
            settings.value("ai/ollama/endpoint", ollamaDefaultUrl).toString();
        ollamaEndpointLineEdit->setText(endpoint);
    }

    QString openaiBase = qEnvironmentVariable("OPENAI_API_BASE");
    if (!openaiBase.isEmpty()) {
        openaiEndpointLineEdit->setText(openaiBase);
    } else {
        QSettings settings;
        QString endpoint =
            settings.value("ai/openai/endpoint", openAiDefaultUrl).toString();
        openaiEndpointLineEdit->setText(endpoint);
    }

    QString openaiKey = qEnvironmentVariable("OPENAI_API_KEY");
    if (!openaiKey.isEmpty()) {
        openaiApiKeyLineEdit->setPlaceholderText(
            "Using OPENAI_API_KEY from environment");
    }
}

void SettingsDialog::loadAISettings() {
    QSettings settings;
    aiEnabledCheckBox->setChecked(settings.value("ai/enabled", true).toBool());
    QString provider = settings.value("ai/provider", "ollama").toString();
    int providerIndex = aiProviderComboBox->findData(provider);
    if (providerIndex >= 0) {
        aiProviderComboBox->setCurrentIndex(providerIndex);
    }

    QString endpoint =
        settings.value("ai/ollama/endpoint", ollamaDefaultUrl)
            .toString();
    ollamaEndpointLineEdit->setText(endpoint);

    QString model = settings.value("ai/ollama/model", "llama3.2").toString();
    int modelIndex = ollamaModelComboBox->findText(model);
    if (modelIndex >= 0) {
        ollamaModelComboBox->setCurrentIndex(modelIndex);
    } else {
        ollamaModelComboBox->setEditText(model);
    }

    ollamaTimeoutSpinBox->setValue(
        settings.value("ai/ollama/timeout", timeOutDefault).toInt());

    QString openaiEndpoint =
        settings.value("ai/openai/endpoint", openAiDefaultUrl)
            .toString();
    openaiEndpointLineEdit->setText(openaiEndpoint);

    QString openaiApiKey = settings.value("ai/openai/apikey", "").toString();
    openaiApiKeyLineEdit->setText(openaiApiKey);

    QString openaiModel =
        settings.value("ai/openai/model", "gpt-4o-mini").toString();
    int openaiModelIndex = openaiModelComboBox->findText(openaiModel);
    if (openaiModelIndex >= 0) {
        openaiModelComboBox->setCurrentIndex(openaiModelIndex);
    } else {
        openaiModelComboBox->setEditText(openaiModel);
    }

    openaiTimeoutSpinBox->setValue(
        settings.value("ai/openai/timeout", timeOutDefault).toInt());

    onAIEnabledChanged(aiEnabledCheckBox->isChecked() ? Qt::Checked
                                                      : Qt::Unchecked);
    detectAIProviders();
}

void SettingsDialog::saveAISettings() {
    QSettings settings;
    settings.setValue("ai/enabled", aiEnabledCheckBox->isChecked());
    settings.setValue("ai/provider",
                      aiProviderComboBox->currentData().toString());

    settings.setValue("ai/ollama/endpoint", ollamaEndpointLineEdit->text());
    settings.setValue("ai/ollama/model", ollamaModelComboBox->currentText());
    settings.setValue("ai/ollama/timeout", ollamaTimeoutSpinBox->value());

    settings.setValue("ai/openai/endpoint", openaiEndpointLineEdit->text());
    settings.setValue("ai/openai/apikey", openaiApiKeyLineEdit->text());
    settings.setValue("ai/openai/model", openaiModelComboBox->currentText());
    settings.setValue("ai/openai/timeout", openaiTimeoutSpinBox->value());
}

void SettingsDialog::onTestOpenAIConnection() {
    testConnectionButton->setEnabled(false);
    testConnectionButton->setText(tr("Testing..."));
    connectionStatusLabel->setText(tr("Connecting..."));
    bool isDark = palette().color(QPalette::Window).lightness() < 128;
    connectionStatusLabel->setStyleSheet(QString("color: %1;")
            .arg(getStatusColor("info", isDark)));

    OpenAIProvider provider;
    provider.setEndpoint(openaiEndpointLineEdit->text());

    QString apiKey = openaiApiKeyLineEdit->text();
    if (apiKey.isEmpty()) {
        apiKey = qEnvironmentVariable("OPENAI_API_KEY");
    }
    provider.setApiKey(apiKey);

    QString error;
    bool success = provider.testConnection(error);

    if (success) {
        connectionStatusLabel->setText(tr("✓ Connected"));
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("success", isDark)));
    } else {
        connectionStatusLabel->setText(error);
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("error", isDark)));
    }

    testConnectionButton->setEnabled(true);
    testConnectionButton->setText(tr("Test Connection"));
}

void SettingsDialog::onRefreshOpenAIModels() {
    refreshOpenAIModelsButton->setEnabled(false);
    refreshOpenAIModelsButton->setText(tr("Loading..."));
    connectionStatusLabel->setText(tr("Fetching models..."));
    bool isDark = palette().color(QPalette::Window).lightness() < 128;
    connectionStatusLabel->setStyleSheet(QString("color: %1;")
            .arg(getStatusColor("info", isDark)));

    OpenAIProvider provider;
    provider.setEndpoint(openaiEndpointLineEdit->text());

    QString apiKey = openaiApiKeyLineEdit->text();
    if (apiKey.isEmpty()) {
        apiKey = qEnvironmentVariable("OPENAI_API_KEY");
    }
    provider.setApiKey(apiKey);

    QString error;
    QStringList models = provider.listModels(error);

    if (models.isEmpty()) {
        models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo" << "gpt-3.5-turbo";
        if (error.isEmpty()) {
            error = "No models returned from API";
        }
    }

    if (!error.isEmpty()) {
        connectionStatusLabel->setText(tr("Warning: %1").arg(error));
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("warning", isDark)));
    } else {
        connectionStatusLabel->setText(tr("✓ %1 models loaded")
                .arg(models.size()));
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("success", isDark)));
    }

    QString currentModel = openaiModelComboBox->currentText();

    openaiModelComboBox->clear();
    for (const QString& model : models) {
        openaiModelComboBox->addItem(model);
    }

    if (!currentModel.isEmpty()) {
        int index = openaiModelComboBox->findText(currentModel);
        if (index >= 0) {
            openaiModelComboBox->setCurrentIndex(index);
        } else {
            openaiModelComboBox->setEditText(currentModel);
        }
    }

    refreshOpenAIModelsButton->setEnabled(true);
    refreshOpenAIModelsButton->setText(tr("Refresh"));
}

void SettingsDialog::onProviderChanged(int index) {
    QString provider = aiProviderComboBox->itemData(index).toString();
    bool enabled = aiEnabledCheckBox->isChecked();

    if (provider == "ollama") {
        ollamaGroup->setVisible(true);
        openaiGroup->setVisible(false);
        refreshModelsButton->setEnabled(enabled);
        refreshOpenAIModelsButton->setEnabled(false);
    } else if (provider == "openai") {
        ollamaGroup->setVisible(false);
        openaiGroup->setVisible(true);
        refreshModelsButton->setEnabled(false);
        refreshOpenAIModelsButton->setEnabled(enabled);
    }
}

void SettingsDialog::onAIEnabledChanged(int state) {
    bool enabled = (state == Qt::Checked);
    aiProviderComboBox->setEnabled(enabled);
    ollamaGroup->setEnabled(enabled);
    openaiGroup->setEnabled(enabled);
    testConnectionButton->setEnabled(enabled);
    managePromptsButton->setEnabled(enabled);

    // Show/hide provider-specific groups
    onProviderChanged(aiProviderComboBox->currentIndex());
}

void SettingsDialog::setupAITab() {
    QWidget* aiTab = new QWidget();
    QVBoxLayout* mainLayout = new QVBoxLayout(aiTab);

    aiEnabledCheckBox = new QCheckBox(tr("Enable AI Assistance"));
    connect(aiEnabledCheckBox, &QCheckBox::stateChanged, this,
            &SettingsDialog::onAIEnabledChanged);
    mainLayout->addWidget(aiEnabledCheckBox);

    QGroupBox* providerGroup = new QGroupBox(tr("Provider Configuration"));
    QFormLayout* providerLayout = new QFormLayout();

    aiProviderComboBox = new QComboBox();
    aiProviderComboBox->addItem("Ollama", "ollama");
    aiProviderComboBox->addItem("OpenAI", "openai");
    connect(aiProviderComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged), this,
            &SettingsDialog::onProviderChanged);
    providerLayout->addRow(tr("Provider:"), aiProviderComboBox);

    providerGroup->setLayout(providerLayout);
    mainLayout->addWidget(providerGroup);

    ollamaGroup = new QGroupBox(tr("Ollama"));
    QFormLayout* ollamaLayout = new QFormLayout();

    ollamaEndpointLineEdit = new QLineEdit();
    ollamaEndpointLineEdit->setPlaceholderText(ollamaDefaultUrl);
    ollamaLayout->addRow(tr("Endpoint:"), ollamaEndpointLineEdit);

    QHBoxLayout* ollamaModelLayout = new QHBoxLayout();
    ollamaModelComboBox = new QComboBox();
    ollamaModelComboBox->setEditable(true);
    ollamaModelComboBox->addItem("llama3.2");
    ollamaModelLayout->addWidget(ollamaModelComboBox, 1);

    refreshModelsButton = new QPushButton(tr("Refresh"));
    connect(refreshModelsButton, &QPushButton::clicked, this,
            &SettingsDialog::onRefreshOllamaModels);
    ollamaModelLayout->addWidget(refreshModelsButton);

    ollamaLayout->addRow(tr("Model:"), ollamaModelLayout);

    ollamaTimeoutSpinBox = new QSpinBox();
    ollamaTimeoutSpinBox->setRange(10, 300);
    ollamaTimeoutSpinBox->setValue(60);
    ollamaTimeoutSpinBox->setSuffix(" s");
    ollamaLayout->addRow(tr("Timeout:"), ollamaTimeoutSpinBox);

    ollamaGroup->setLayout(ollamaLayout);
    mainLayout->addWidget(ollamaGroup);

    openaiGroup = new QGroupBox(tr("OpenAI"));
    QFormLayout* openaiLayout = new QFormLayout();

    openaiEndpointLineEdit = new QLineEdit();
    openaiEndpointLineEdit->setPlaceholderText(openAiDefaultUrl);
    openaiLayout->addRow(tr("Endpoint:"), openaiEndpointLineEdit);

    openaiApiKeyLineEdit = new QLineEdit();
    openaiApiKeyLineEdit->setEchoMode(QLineEdit::Password);
    openaiApiKeyLineEdit->setPlaceholderText("sk-...");
    openaiLayout->addRow(tr("API Key:"), openaiApiKeyLineEdit);

    QHBoxLayout* openaiModelLayout = new QHBoxLayout();
    openaiModelComboBox = new QComboBox();
    openaiModelComboBox->setEditable(true);
    openaiModelComboBox->addItem("gpt-4o-mini");
    openaiModelComboBox->addItem("gpt-4o");
    openaiModelComboBox->addItem("gpt-4-turbo");
    openaiModelComboBox->addItem("gpt-3.5-turbo");
    openaiModelLayout->addWidget(openaiModelComboBox, 1);

    refreshOpenAIModelsButton = new QPushButton(tr("Refresh"));
    connect(refreshOpenAIModelsButton, &QPushButton::clicked, this,
            &SettingsDialog::onRefreshOpenAIModels);
    openaiModelLayout->addWidget(refreshOpenAIModelsButton);

    openaiLayout->addRow(tr("Model:"), openaiModelLayout);

    openaiTimeoutSpinBox = new QSpinBox();
    openaiTimeoutSpinBox->setRange(10, 300);
    openaiTimeoutSpinBox->setValue(60);
    openaiTimeoutSpinBox->setSuffix(" s");
    openaiLayout->addRow(tr("Timeout:"), openaiTimeoutSpinBox);

    openaiGroup->setLayout(openaiLayout);
    mainLayout->addWidget(openaiGroup);

    QHBoxLayout* testLayout = new QHBoxLayout();
    connectionStatusLabel = new QLabel(tr("Not tested"));
    testLayout->addWidget(connectionStatusLabel, 1);

    testConnectionButton = new QPushButton(tr("Test Connection"));
    connect(testConnectionButton, &QPushButton::clicked, this, [this]() {
        QString provider = aiProviderComboBox->currentData().toString();
        if (provider == "ollama") {
            onTestOllamaConnection();
        } else if (provider == "openai") {
            onTestOpenAIConnection();
        }
    });
    testLayout->addWidget(testConnectionButton);

    mainLayout->addLayout(testLayout);

    QGroupBox* promptsGroup = new QGroupBox(tr("System Prompts"));
    QHBoxLayout* promptsLayout = new QHBoxLayout();

    promptsInfoLabel = new QLabel();
    promptsLayout->addWidget(promptsInfoLabel, 1);

    managePromptsButton = new QPushButton(tr("Manage..."));
    connect(managePromptsButton, &QPushButton::clicked, this,
            &SettingsDialog::onManageSystemPrompts);
    promptsLayout->addWidget(managePromptsButton);

    promptsGroup->setLayout(promptsLayout);
    mainLayout->addWidget(promptsGroup);

    mainLayout->addStretch();

    tabWidget->addTab(aiTab, tr("AI Providers"));

    detectAIProviders();
    updatePromptsInfo();
}

void SettingsDialog::updatePromptsInfo() {
    QList<SystemPrompt> enabled =
        SystemPrompts::instance()->getEnabledPrompts();
    QList<SystemPrompt> all = SystemPrompts::instance()->getAllPrompts();

    int customCount = 0;
    for (const SystemPrompt& p : all) {
        if (p.isCustom) customCount++;
    }

    promptsInfoLabel->setText(QString("%1 of %2 prompts enabled (%3 custom)")
                                  .arg(enabled.size())
                                  .arg(all.size())
                                  .arg(customCount));
}

void SettingsDialog::onManageSystemPrompts() {
    SystemPromptsDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        updatePromptsInfo();
    }
}

void SettingsDialog::onRefreshOllamaModels() {
    refreshModelsButton->setEnabled(false);
    refreshModelsButton->setText(tr("Loading..."));
    bool isDark = palette().color(QPalette::Window).lightness() < 128;
    connectionStatusLabel->setText(tr("Fetching models..."));
    connectionStatusLabel->setStyleSheet(QString("color: %1;")
            .arg(getStatusColor("info", isDark)));

    OllamaProvider provider;
    provider.setEndpoint(ollamaEndpointLineEdit->text());

    QString error;
    QStringList models = provider.listModels(error);

    if (!error.isEmpty()) {
        connectionStatusLabel->setText(error);
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("error", isDark)));
    } else {
        ollamaModelComboBox->clear();
        for (const QString& model : models) {
            ollamaModelComboBox->addItem(model);
        }
        connectionStatusLabel->setText(tr("✓ %1 models found")
                .arg(models.size()));
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("success", isDark)));
    }

    refreshModelsButton->setEnabled(true);
    refreshModelsButton->setText(tr("Refresh"));
}

void SettingsDialog::onTestOllamaConnection() {
    testConnectionButton->setEnabled(false);
    testConnectionButton->setText(tr("Testing..."));
    connectionStatusLabel->setText(tr("Connecting..."));
    bool isDark = palette().color(QPalette::Window).lightness() < 128;
    connectionStatusLabel->setStyleSheet(QString("color: %1;")
            .arg(getStatusColor("info", isDark)));

    OllamaProvider provider;
    provider.setEndpoint(ollamaEndpointLineEdit->text());

    QString error;
    bool success = provider.testConnection(error);

    if (success) {
        connectionStatusLabel->setText(tr("✓ Connected"));
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("success", isDark)));
    } else {
        connectionStatusLabel->setText(error);
        connectionStatusLabel->setStyleSheet(QString("color: %1;")
                .arg(getStatusColor("error", isDark)));
    }

    testConnectionButton->setEnabled(true);
    testConnectionButton->setText(tr("Test Connection"));
}
