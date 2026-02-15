#ifndef OPENAIPROVIDER_H
#define OPENAIPROVIDER_H

#include "logic/aiprovider.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

const QString openAILabel = "OpenAI";

class OpenAIProvider : public QObject, public AIProvider {
  Q_OBJECT

public:
  OpenAIProvider();
  ~OpenAIProvider() override;
  
  void process(const QString &prompt, 
               const QString &content,
               std::function<void(const QString&)> onSuccess,
               std::function<void(const QString&)> onError) override;
  
  bool isAvailable() const override;
  QString name() const override { return openAILabel; }
  
  void setModel(const QString &model);
  void setEndpoint(const QString &url);
  void setApiKey(const QString &key);
  void setTimeout(int seconds);
  
  QString getModel() const { return modelName; }
  QString getEndpoint() const { return endpoint; }
  QString getApiKey() const { return apiKey; }
  int getTimeout() const { return timeoutSeconds; }
  
  QStringList listModels(QString &error);
  bool testConnection(QString &error);
  
private:
  QNetworkAccessManager *networkManager;
  QString modelName;
  QString endpoint;
  QString apiKey;
  int timeoutSeconds;
};

#endif
