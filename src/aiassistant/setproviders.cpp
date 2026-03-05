#include "appinit.h"
#include "logic/aiprovider.h"
#include "logic/ollamaprovider.h"
#include "logic/openaiprovider.h"
#include "logic/systemprompts.h"

namespace {

/**
 * Initializes the Ollama provider with the application settings
 * and environment variables, if they are set. The provider is then
 * registerd.
 */
void setOllamaProvider(QSettings& settings) {
    QString ollamaHost = qEnvironmentVariable("OLLAMA_HOST");
    if (ollamaHost.isEmpty()) {
        ollamaHost =
            settings.value("ai/ollama/endpoint", "http://localhost:11434")
                .toString();
    }
    OllamaProvider* ollamaProvider = new OllamaProvider();
    ollamaProvider->setEndpoint(ollamaHost);
    QString ollamaModel =
        settings.value("ai/ollama/model", "llama3.2").toString();
    ollamaProvider->setModel(ollamaModel);
    int ollamaTimeout = settings.value("ai/ollama/timeout", 60).toInt();
    ollamaProvider->setTimeout(ollamaTimeout);
    AIProviderManager::instance()->registerProvider("ollama", ollamaProvider);
}

/**
 * Initializes the OpenAI provider with the application settings
 * and environment variables, if they are set. The provider is then
 * registerd.
 */
void setOpenAIProvider(QSettings& settings) {
    QString openaiBase = qEnvironmentVariable("OPENAI_API_BASE");
    if (openaiBase.isEmpty()) {
        openaiBase =
            settings.value("ai/openai/endpoint", "https://api.openai.com/v1")
                .toString();
    }
    QString openaiKey = qEnvironmentVariable("OPENAI_API_KEY");
    if (openaiKey.isEmpty()) {
        openaiKey = settings.value("ai/openai/apikey", "").toString();
    }
    OpenAIProvider* openaiProvider = new OpenAIProvider();
    openaiProvider->setEndpoint(openaiBase);
    openaiProvider->setApiKey(openaiKey);
    QString openaiModel =
        settings.value("ai/openai/model", "gpt-4o-mini").toString();
    openaiProvider->setModel(openaiModel);
    int openaiTimeout = settings.value("ai/openai/timeout", 60).toInt();
    openaiProvider->setTimeout(openaiTimeout);
    AIProviderManager::instance()->registerProvider("openai", openaiProvider);
}

} // anonymous namespace

void setAIProviders(QSettings& settings) {
    setOllamaProvider(settings);
    setOpenAIProvider(settings);
    QString activeProvider = settings.value("ai/provider", "ollama").toString();
    AIProviderManager::instance()->setActiveProvider(activeProvider);
    SystemPrompts::instance()->loadFromSettings();
}

