#ifndef OLLAMAPROVIDER_H
#define OLLAMAPROVIDER_H

#include "logic/aiprovider.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QString>

const QString ollamaLabel = "Ollama";

class OllamaProvider : public QObject, public AIProvider {
  Q_OBJECT

public:
  OllamaProvider();
  ~OllamaProvider() override;
  
  void process(const QString &prompt, 
               const QString &content,
               std::function<void(const QString&)> onSuccess,
               std::function<void(const QString&)> onError) override;
  
  bool isAvailable() const override;
  QString name() const override { return ollamaLabel; }
  
  void setModel(const QString &model);
  void setEndpoint(const QString &url);
  void setTimeout(int seconds);
  
  QString getModel() const { return modelName; }
  QString getEndpoint() const { return endpoint; }
  int getTimeout() const { return timeoutSeconds; }
  
  QStringList listModels(QString &error);
  bool testConnection(QString &error);
  
private:
  QNetworkAccessManager *networkManager;
  QString modelName;
  QString endpoint;
  int timeoutSeconds;
};

#endif
