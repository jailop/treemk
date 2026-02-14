#include "logic/openaiprovider.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>

OpenAIProvider::OpenAIProvider()
    : QObject(nullptr),
      networkManager(new QNetworkAccessManager(this)),
      modelName("gpt-4o-mini"),
      endpoint("https://api.openai.com/v1"),
      apiKey(""),
      timeoutSeconds(60) {}

OpenAIProvider::~OpenAIProvider() {}

void OpenAIProvider::setModel(const QString& model) { modelName = model; }

void OpenAIProvider::setEndpoint(const QString& url) { endpoint = url; }

void OpenAIProvider::setApiKey(const QString& key) { apiKey = key; }

void OpenAIProvider::setTimeout(int seconds) { timeoutSeconds = seconds; }

QString OpenAIProvider::buildSystemPrompt() const {
    return "You are a helpful writing assistant. "
           "Respond with only the modified text. "
           "Do not include explanations or commentary unless specifically "
           "requested.";
}

void OpenAIProvider::process(const QString& prompt, const QString& content,
                             std::function<void(const QString&)> onSuccess,
                             std::function<void(const QString&)> onError) {
    if (apiKey.isEmpty()) {
        onError(
            "API key not configured. Please set your OpenAI API key in "
            "settings.");
        return;
    }

    QJsonObject request;
    request["model"] = modelName;

    // Build messages array
    QJsonArray messages;

    // System message
    QJsonObject systemMsg;
    systemMsg["role"] = "system";
    systemMsg["content"] = buildSystemPrompt();
    messages.append(systemMsg);

    // User message
    QJsonObject userMsg;
    userMsg["role"] = "user";
    QString userContent = prompt;
    if (!content.isEmpty()) {
        userContent += "\n\nContent:\n" + content;
    }
    userMsg["content"] = userContent;
    messages.append(userMsg);

    request["messages"] = messages;
    request["temperature"] = 0.7;

    QNetworkRequest netReq(QUrl(endpoint + "/chat/completions"));
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    netReq.setRawHeader("Authorization",
                        QString("Bearer %1").arg(apiKey).toUtf8());

    QNetworkReply* reply =
        networkManager->post(netReq, QJsonDocument(request).toJson());

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(timeoutSeconds * 1000);

    connect(timer, &QTimer::timeout, this, [reply, onError]() {
        reply->abort();
        onError("Request timed out. The API might be unavailable.");
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
        timer->stop();
        timer->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QJsonObject responseObj = response.object();

            // Extract result from OpenAI response format
            QJsonArray choices = responseObj["choices"].toArray();
            if (!choices.isEmpty()) {
                QJsonObject firstChoice = choices[0].toObject();
                QJsonObject message = firstChoice["message"].toObject();
                QString result = message["content"].toString().trimmed();

                if (!result.isEmpty()) {
                    onSuccess(result);
                } else {
                    onError("AI response was empty");
                }
            } else {
                QString errorMsg =
                    responseObj["error"].toObject()["message"].toString();
                if (errorMsg.isEmpty()) {
                    errorMsg = "Invalid response format from API";
                }
                onError(errorMsg);
            }
        } else {
            QString errorMsg = reply->errorString();
            if (reply->error() == QNetworkReply::OperationCanceledError) {
                errorMsg = "Request timed out";
            } else if (reply->error() ==
                       QNetworkReply::AuthenticationRequiredError) {
                errorMsg = "Invalid API key";
            }

            // Try to extract error message from response body
            QJsonDocument errorDoc = QJsonDocument::fromJson(reply->readAll());
            if (!errorDoc.isNull()) {
                QString apiError =
                    errorDoc.object()["error"].toObject()["message"].toString();
                if (!apiError.isEmpty()) {
                    errorMsg = apiError;
                }
            }

            onError(errorMsg);
        }
        reply->deleteLater();
    });

    timer->start();
}

bool OpenAIProvider::isAvailable() const {
    if (apiKey.isEmpty()) {
        return false;
    }
    QString error;
    return const_cast<OpenAIProvider*>(this)->testConnection(error);
}

QStringList OpenAIProvider::listModels(QString& error) {
    QStringList models;

    if (apiKey.isEmpty()) {
        error = "API key not configured";
        // Return default models as fallback
        models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo" << "gpt-3.5-turbo";
        return models;
    }

    QNetworkRequest request(QUrl(endpoint + "/models"));
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(apiKey).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = networkManager->get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(10000);  // Increased timeout to 10 seconds

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start();
    loop.exec();

    if (timer.isActive()) {
        timer.stop();

        if (reply->error() == QNetworkReply::NoError) {
            QByteArray responseData = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(responseData);

            if (doc.isNull() || !doc.isObject()) {
                error = "Invalid JSON response from server";
                models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo"
                       << "gpt-3.5-turbo";
                reply->deleteLater();
                return models;
            }

            QJsonObject obj = doc.object();
            QJsonArray modelArray = obj["data"].toArray();

            if (modelArray.isEmpty()) {
                error = "No models returned from API";
                models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo"
                       << "gpt-3.5-turbo";
                reply->deleteLater();
                return models;
            }

            // Extract all model IDs without filtering
            for (const QJsonValue& val : modelArray) {
                QJsonObject modelObj = val.toObject();
                QString id = modelObj["id"].toString();
                if (!id.isEmpty()) {
                    models.append(id);
                }
            }

            // If no models found, add defaults
            if (models.isEmpty()) {
                models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo" << "gpt-4"
                       << "gpt-3.5-turbo";
            } else {
                // Sort models alphabetically
                models.sort();
            }

        } else {
            error = QString("Network error: %1 (HTTP %2)")
                        .arg(reply->errorString())
                        .arg(reply
                                 ->attribute(
                                     QNetworkRequest::HttpStatusCodeAttribute)
                                 .toInt());

            // Try to get more specific error from response body
            QByteArray responseData = reply->readAll();
            QJsonDocument errorDoc = QJsonDocument::fromJson(responseData);
            if (!errorDoc.isNull() && errorDoc.isObject()) {
                QJsonObject errorObj = errorDoc.object()["error"].toObject();
                QString apiError = errorObj["message"].toString();
                if (!apiError.isEmpty()) {
                    error = apiError;
                }
            }

            // Return default models as fallback
            models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo"
                   << "gpt-3.5-turbo";
        }
    } else {
        reply->abort();
        error = "Connection timeout";
        // Return default models as fallback
        models << "gpt-4o" << "gpt-4o-mini" << "gpt-4-turbo" << "gpt-3.5-turbo";
    }

    reply->deleteLater();
    return models;
}

bool OpenAIProvider::testConnection(QString& error) {
    if (apiKey.isEmpty()) {
        error = "API key not configured";
        return false;
    }

    // Test with a simple models list request
    QNetworkRequest request(QUrl(endpoint + "/models"));
    request.setRawHeader("Authorization",
                         QString("Bearer %1").arg(apiKey).toUtf8());
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    QNetworkReply* reply = networkManager->get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(10000);  // Increased timeout

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start();
    loop.exec();

    bool success = false;

    if (timer.isActive()) {
        timer.stop();
        success = (reply->error() == QNetworkReply::NoError);
        if (!success) {
            error = QString("Network error: %1 (HTTP %2)")
                        .arg(reply->errorString())
                        .arg(reply
                                 ->attribute(
                                     QNetworkRequest::HttpStatusCodeAttribute)
                                 .toInt());

            // Try to get more specific error from response body
            QByteArray responseData = reply->readAll();
            QJsonDocument errorDoc = QJsonDocument::fromJson(responseData);
            if (!errorDoc.isNull() && errorDoc.isObject()) {
                QJsonObject errorObj = errorDoc.object()["error"].toObject();
                QString apiError = errorObj["message"].toString();
                if (!apiError.isEmpty()) {
                    error = apiError;
                }
            }
        }
    } else {
        reply->abort();
        error = "Connection timeout";
    }

    reply->deleteLater();
    return success;
}
