#include "defs.h"
#include "settingsdialog.h"
#include "systempromptsdialog.h"
#include "logic/openaiprovider.h"
#include "logic/ollamaprovider.h"
#include "logic/systemprompts.h"
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QScrollArea>
#include <QWidget>
#include <QDialog>
#include <QSettings>
#include <QSpinBox>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>

const int timeOutDefault = 60;
const QString ollamaDefaultUrl = "http://localhost:11434";
const QString openAiDefaultUrl = "https://api.openai.com/v1";

void SettingsDialog::detectAIProviders() {
  QString ollamaHost = qEnvironmentVariable("OLLAMA_HOST");
  if (!ollamaHost.isEmpty()) {
    ollamaEndpointLineEdit->setText(ollamaHost);
  } else {
    QSettings settings;
    QString endpoint = settings.value("ai/ollama/endpoint",
            ollamaDefaultUrl).toString();
    ollamaEndpointLineEdit->setText(endpoint);
  }
  
  QString openaiBase = qEnvironmentVariable("OPENAI_API_BASE");
  if (!openaiBase.isEmpty()) {
    openaiEndpointLineEdit->setText(openaiBase);
  } else {
    QSettings settings;
    QString endpoint = settings.value("ai/openai/endpoint",
            openAiDefaultUrl).toString();
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
  
  QString endpoint = settings.value("ai/ollama/endpoint",
          "http://localhost:11434").toString();
  ollamaEndpointLineEdit->setText(endpoint);
  
  QString model = settings.value("ai/ollama/model", "llama3.2").toString();
  int modelIndex = ollamaModelComboBox->findText(model);
  if (modelIndex >= 0) {
    ollamaModelComboBox->setCurrentIndex(modelIndex);
  } else {
    ollamaModelComboBox->setEditText(model);
  }
  
  ollamaTimeoutSpinBox->setValue(settings.value("ai/ollama/timeout",
              timeOutDefault).toInt());
  
  QString openaiEndpoint = settings.value("ai/openai/endpoint",
          "https://api.openai.com/v1").toString();
  openaiEndpointLineEdit->setText(openaiEndpoint);
  
  QString openaiApiKey = settings.value("ai/openai/apikey", "").toString();
  openaiApiKeyLineEdit->setText(openaiApiKey);
  
  QString openaiModel = settings.value("ai/openai/model",
          "gpt-4o-mini").toString();
  int openaiModelIndex = openaiModelComboBox->findText(openaiModel);
  if (openaiModelIndex >= 0) {
    openaiModelComboBox->setCurrentIndex(openaiModelIndex);
  } else {
    openaiModelComboBox->setEditText(openaiModel);
  }
  
  openaiTimeoutSpinBox->setValue(settings.value("ai/openai/timeout",
              timeOutDefault).toInt());
  
  onAIEnabledChanged(aiEnabledCheckBox->isChecked() 
          ? Qt::Checked : Qt::Unchecked);
  detectAIProviders();
}

void SettingsDialog::saveAISettings() {
  QSettings settings;
  settings.setValue("ai/enabled", aiEnabledCheckBox->isChecked());
  settings.setValue("ai/provider", aiProviderComboBox->currentData().toString());
  
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
  connectionStatusLabel->setStyleSheet("color: blue;");
  
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
    connectionStatusLabel->setText(tr("Connected"));
    connectionStatusLabel->setStyleSheet("color: green;");
  } else {
    connectionStatusLabel->setText(tr("%1").arg(error));
    connectionStatusLabel->setStyleSheet("color: red;");
  }
  
  testConnectionButton->setEnabled(true);
  testConnectionButton->setText(tr("Test Connection"));
}

void SettingsDialog::onRefreshOpenAIModels() {
  refreshOpenAIModelsButton->setEnabled(false);
  refreshOpenAIModelsButton->setText(tr("Loading..."));
  connectionStatusLabel->setText(tr("Fetching models..."));
  connectionStatusLabel->setStyleSheet("color: blue;");
  
  OpenAIProvider provider;
  provider.setEndpoint(openaiEndpointLineEdit->text());
  
  // Get API key from field or environment
  QString apiKey = openaiApiKeyLineEdit->text();
  if (apiKey.isEmpty()) {
    apiKey = qEnvironmentVariable("OPENAI_API_KEY");
  }
  provider.setApiKey(apiKey);
  
  QString error;
  QStringList models = provider.listModels(error);
  
  // Ensure we always have at least default models
  if (models.isEmpty()) {
    models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo" << "gpt-3.5-turbo";
    if (error.isEmpty()) {
      error = "No models returned from API";
    }
  }
  
  if (!error.isEmpty()) {
    connectionStatusLabel->setText(tr("Warning: %1 (showing defaults)").arg(error));
    connectionStatusLabel->setStyleSheet("color: orange;");
  } else {
    connectionStatusLabel->setText(tr("✓ %1 models loaded").arg(models.size()));
    connectionStatusLabel->setStyleSheet("color: green;");
  }
  
  // Save current selection
  QString currentModel = openaiModelComboBox->currentText();
  
  // Update combo box
  openaiModelComboBox->clear();
  for (const QString &model : models) {
    openaiModelComboBox->addItem(model);
  }
  
  // Restore previous selection if it exists in new list
  if (!currentModel.isEmpty()) {
    int index = openaiModelComboBox->findText(currentModel);
    if (index >= 0) {
      openaiModelComboBox->setCurrentIndex(index);
    } else {
      // If previous model not in list, set it as editable text
      openaiModelComboBox->setEditText(currentModel);
    }
  }
  
  refreshOpenAIModelsButton->setEnabled(true);
  refreshOpenAIModelsButton->setText(tr("Refresh Models"));
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
  QWidget *aiTab = new QWidget();
  QScrollArea *scrollArea = new QScrollArea();
  QWidget *scrollWidget = new QWidget();
  QVBoxLayout *mainLayout = new QVBoxLayout(scrollWidget);

  // AI Feature Enable/Disable
  QGroupBox *featureGroup = new QGroupBox(tr("AI Assistance Feature"));
  QVBoxLayout *featureLayout = new QVBoxLayout();
  
  aiEnabledCheckBox = new QCheckBox(tr("Enable AI Assistance"));
  connect(aiEnabledCheckBox, &QCheckBox::checkStateChanged,
          this, &SettingsDialog::onAIEnabledChanged);
  featureLayout->addWidget(aiEnabledCheckBox);
  
  featureGroup->setLayout(featureLayout);
  mainLayout->addWidget(featureGroup);

  // Provider Selection
  QGroupBox *providerGroup = new QGroupBox(tr("AI Provider"));
  QFormLayout *providerLayout = new QFormLayout();
  
  aiProviderComboBox = new QComboBox();
  aiProviderComboBox->addItem("Ollama", "ollama");
  aiProviderComboBox->addItem("OpenAI", "openai");
  connect(aiProviderComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &SettingsDialog::onProviderChanged);
  providerLayout->addRow(tr("Active Provider:"), aiProviderComboBox);
  
  providerGroup->setLayout(providerLayout);
  mainLayout->addWidget(providerGroup);

  // Ollama Settings
  ollamaGroup = new QGroupBox(tr("Ollama Settings"));
  QFormLayout *ollamaLayout = new QFormLayout();
  
  ollamaEndpointLineEdit = new QLineEdit();
  ollamaEndpointLineEdit->setPlaceholderText("http://localhost:11434");
  ollamaLayout->addRow(tr("Endpoint:"), ollamaEndpointLineEdit);
  
  QLabel *endpointHint = new QLabel(tr("(auto-detected from OLLAMA_HOST)"));
  QFont hintFont = endpointHint->font();
  hintFont.setItalic(true);
  endpointHint->setFont(hintFont);
  endpointHint->setStyleSheet("color: gray;");
  ollamaLayout->addRow("", endpointHint);
  
  QHBoxLayout *modelLayout = new QHBoxLayout();
  ollamaModelComboBox = new QComboBox();
  ollamaModelComboBox->setEditable(true);
  ollamaModelComboBox->addItem("llama3.2");
  modelLayout->addWidget(ollamaModelComboBox, 1);
  
  refreshModelsButton = new QPushButton(tr("Refresh Models"));
  connect(refreshModelsButton, &QPushButton::clicked,
          this, &SettingsDialog::onRefreshOllamaModels);
  modelLayout->addWidget(refreshModelsButton);
  
  ollamaLayout->addRow(tr("Model:"), modelLayout);
  
  ollamaTimeoutSpinBox = new QSpinBox();
  ollamaTimeoutSpinBox->setRange(10, 300);
  ollamaTimeoutSpinBox->setValue(60);
  ollamaTimeoutSpinBox->setSuffix(" seconds");
  ollamaLayout->addRow(tr("Timeout:"), ollamaTimeoutSpinBox);
  
  ollamaGroup->setLayout(ollamaLayout);
  mainLayout->addWidget(ollamaGroup);

  // OpenAI Settings
  openaiGroup = new QGroupBox(tr("OpenAI Settings"));
  QFormLayout *openaiLayout = new QFormLayout();
  
  openaiEndpointLineEdit = new QLineEdit();
  openaiEndpointLineEdit->setPlaceholderText("https://api.openai.com/v1");
  openaiLayout->addRow(tr("Endpoint:"), openaiEndpointLineEdit);
  
  QLabel *openaiEndpointHint = new QLabel(tr("(auto-detected from OPENAI_API_BASE or use default)"));
  QFont openaiHintFont = openaiEndpointHint->font();
  openaiHintFont.setItalic(true);
  openaiEndpointHint->setFont(openaiHintFont);
  openaiEndpointHint->setStyleSheet("color: gray;");
  openaiLayout->addRow("", openaiEndpointHint);
  
  openaiApiKeyLineEdit = new QLineEdit();
  openaiApiKeyLineEdit->setEchoMode(QLineEdit::Password);
  openaiApiKeyLineEdit->setPlaceholderText("sk-...");
  openaiLayout->addRow(tr("API Key:"), openaiApiKeyLineEdit);
  
  QLabel *apiKeyHint = new QLabel(tr("(auto-detected from OPENAI_API_KEY environment variable)"));
  QFont apiKeyHintFont = apiKeyHint->font();
  apiKeyHintFont.setItalic(true);
  apiKeyHint->setFont(apiKeyHintFont);
  apiKeyHint->setStyleSheet("color: gray;");
  openaiLayout->addRow("", apiKeyHint);
  
  QHBoxLayout *openaiModelLayout = new QHBoxLayout();
  openaiModelComboBox = new QComboBox();
  openaiModelComboBox->setEditable(true);
  openaiModelComboBox->addItem("gpt-4o-mini");
  openaiModelComboBox->addItem("gpt-4o");
  openaiModelComboBox->addItem("gpt-4-turbo");
  openaiModelComboBox->addItem("gpt-3.5-turbo");
  openaiModelLayout->addWidget(openaiModelComboBox, 1);
  
  refreshOpenAIModelsButton = new QPushButton(tr("Refresh Models"));
  connect(refreshOpenAIModelsButton, &QPushButton::clicked,
          this, &SettingsDialog::onRefreshOpenAIModels);
  openaiModelLayout->addWidget(refreshOpenAIModelsButton);
  
  openaiLayout->addRow(tr("Model:"), openaiModelLayout);
  
  openaiTimeoutSpinBox = new QSpinBox();
  openaiTimeoutSpinBox->setRange(10, 300);
  openaiTimeoutSpinBox->setValue(60);
  openaiTimeoutSpinBox->setSuffix(" seconds");
  openaiLayout->addRow(tr("Timeout:"), openaiTimeoutSpinBox);
  
  openaiGroup->setLayout(openaiLayout);
  mainLayout->addWidget(openaiGroup);
  
  // Connection Testing (shared for all providers)
  QGroupBox *testGroup = new QGroupBox(tr("Connection Test"));
  QFormLayout *testLayout = new QFormLayout();
  
  connectionStatusLabel = new QLabel(tr("Status: Not tested"));
  testLayout->addRow(tr("Status:"), connectionStatusLabel);
  
  testConnectionButton = new QPushButton(tr("Test Connection"));
  connect(testConnectionButton, &QPushButton::clicked, this, [this]() {
    QString provider = aiProviderComboBox->currentData().toString();
    if (provider == "ollama") {
      onTestOllamaConnection();
    } else if (provider == "openai") {
      onTestOpenAIConnection();
    }
  });
  testLayout->addRow("", testConnectionButton);
  
  testGroup->setLayout(testLayout);
  mainLayout->addWidget(testGroup);

  // System Prompts
  QGroupBox *promptsGroup = new QGroupBox(tr("System Prompts"));
  QVBoxLayout *promptsLayout = new QVBoxLayout();
  
  promptsInfoLabel = new QLabel();
  promptsLayout->addWidget(promptsInfoLabel);
  
  managePromptsButton = new QPushButton(tr("Manage System Prompts..."));
  connect(managePromptsButton, &QPushButton::clicked,
          this, &SettingsDialog::onManageSystemPrompts);
  promptsLayout->addWidget(managePromptsButton);
  
  promptsGroup->setLayout(promptsLayout);
  mainLayout->addWidget(promptsGroup);

  mainLayout->addStretch();

  scrollWidget->setLayout(mainLayout);
  scrollArea->setWidget(scrollWidget);
  scrollArea->setWidgetResizable(true);
  
  QVBoxLayout *tabLayout = new QVBoxLayout(aiTab);
  tabLayout->addWidget(scrollArea);
  tabLayout->setContentsMargins(0, 0, 0, 0);

  tabWidget->addTab(aiTab, tr("AI Providers"));
  
  // Initialize
  detectAIProviders();
  updatePromptsInfo();
}

void SettingsDialog::updatePromptsInfo() {
  QList<SystemPrompt> enabled = SystemPrompts::instance()->getEnabledPrompts();
  QList<SystemPrompt> all = SystemPrompts::instance()->getAllPrompts();
  
  int customCount = 0;
  for (const SystemPrompt &p : all) {
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
  
  OllamaProvider provider;
  provider.setEndpoint(ollamaEndpointLineEdit->text());
  
  QString error;
  QStringList models = provider.listModels(error);
  
  if (!error.isEmpty()) {
    connectionStatusLabel->setText(tr("Error: %1").arg(error));
    connectionStatusLabel->setStyleSheet("color: red;");
  } else {
    ollamaModelComboBox->clear();
    for (const QString &model : models) {
      ollamaModelComboBox->addItem(model);
    }
    connectionStatusLabel->setText(tr("✓ %1 models found").arg(models.size()));
    connectionStatusLabel->setStyleSheet("color: green;");
  }
  
  refreshModelsButton->setEnabled(true);
  refreshModelsButton->setText(tr("Refresh Models"));
}

void SettingsDialog::onTestOllamaConnection() {
  testConnectionButton->setEnabled(false);
  testConnectionButton->setText(tr("Testing..."));
  connectionStatusLabel->setText(tr("Connecting..."));
  connectionStatusLabel->setStyleSheet("color: blue;");
  
  OllamaProvider provider;
  provider.setEndpoint(ollamaEndpointLineEdit->text());
  
  QString error;
  bool success = provider.testConnection(error);
  
  if (success) {
    connectionStatusLabel->setText(tr("✓ Connected"));
    connectionStatusLabel->setStyleSheet("color: green;");
  } else {
    connectionStatusLabel->setText(tr("✗ %1").arg(error));
    connectionStatusLabel->setStyleSheet("color: red;");
  }
  
  testConnectionButton->setEnabled(true);
  testConnectionButton->setText(tr("Test Connection"));
}
