#ifndef AIPROVIDER_H
#define AIPROVIDER_H

#include <QString>
#include <QMap>
#include <functional>

class AIProvider {
public:
  virtual ~AIProvider() = default;
  
  virtual void process(const QString &prompt, 
                       const QString &content,
                       std::function<void(const QString&)> onSuccess,
                       std::function<void(const QString&)> onError) = 0;
  
  virtual bool isAvailable() const = 0;
  virtual QString name() const = 0;
};

class AIProviderManager {
public:
  static AIProviderManager* instance();
  
  void registerProvider(const QString &id, AIProvider* provider);
  void setActiveProvider(const QString &id);
  AIProvider* activeProvider() const;
  
  QStringList availableProviders() const;
  
  void process(const QString &prompt,
               const QString &content,
               std::function<void(const QString&)> onSuccess,
               std::function<void(const QString&)> onError);

private:
  AIProviderManager();
  ~AIProviderManager();
  
  QMap<QString, AIProvider*> providers;
  QString activeProviderId;
  
  static AIProviderManager* m_instance;
};

#endif // AIPROVIDER_H
