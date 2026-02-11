#include "logic/aiprovider.h"

AIProviderManager* AIProviderManager::m_instance = nullptr;

AIProviderManager::AIProviderManager() {
}

AIProviderManager::~AIProviderManager() {
  qDeleteAll(providers);
}

AIProviderManager* AIProviderManager::instance() {
  if (!m_instance) {
    m_instance = new AIProviderManager();
  }
  return m_instance;
}

void AIProviderManager::registerProvider(const QString &id, AIProvider* provider) {
  if (provider) {
    providers[id] = provider;
    if (activeProviderId.isEmpty()) {
      activeProviderId = id;
    }
  }
}

void AIProviderManager::setActiveProvider(const QString &id) {
  if (providers.contains(id)) {
    activeProviderId = id;
  }
}

AIProvider* AIProviderManager::activeProvider() const {
  if (activeProviderId.isEmpty() || !providers.contains(activeProviderId)) {
    return nullptr;
  }
  return providers[activeProviderId];
}

QStringList AIProviderManager::availableProviders() const {
  return providers.keys();
}

void AIProviderManager::process(const QString &prompt,
                                 const QString &content,
                                 std::function<void(const QString&)> onSuccess,
                                 std::function<void(const QString&)> onError) {
  AIProvider* provider = activeProvider();
  if (!provider) {
    onError("No AI provider configured. Please configure a provider in Settings.");
    return;
  }
  
  if (!provider->isAvailable()) {
    onError("AI provider '" + provider->name() + "' is not available.");
    return;
  }
  
  provider->process(prompt, content, onSuccess, onError);
}
