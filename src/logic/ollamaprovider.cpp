#include "logic/ollamaprovider.h"

#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTimer>
#include <QUrl>

OllamaProvider::OllamaProvider()
    : QObject(nullptr),
      networkManager(new QNetworkAccessManager(this)),
      modelName("llama3.2"),
      endpoint("http://localhost:11434"),
      timeoutSeconds(60) {}

OllamaProvider::~OllamaProvider() {}

void OllamaProvider::setModel(const QString& model) { modelName = model; }

void OllamaProvider::setEndpoint(const QString& url) { endpoint = url; }

void OllamaProvider::setTimeout(int seconds) { timeoutSeconds = seconds; }

void OllamaProvider::process(const QString& prompt, const QString& content,
                             std::function<void(const QString&)> onSuccess,
                             std::function<void(const QString&)> onError) {
    QJsonObject request;
    request["model"] = modelName;
    request["stream"] = false;

    QString systemPrompt = buildSystemPrompt();
    QString userPrompt = prompt;
    if (!content.isEmpty()) {
        userPrompt += "\n\nContent:\n" + content;
    }

    QString fullPrompt = systemPrompt + "\n\n" + userPrompt;
    request["prompt"] = fullPrompt;

    QNetworkRequest netReq(QUrl(endpoint + "/api/generate"));
    netReq.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* reply =
        networkManager->post(netReq, QJsonDocument(request).toJson());

    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    timer->setInterval(timeoutSeconds * 1000);

    connect(timer, &QTimer::timeout, this, [reply, onError]() {
        reply->abort();
        onError(
            "Request timed out. The model might be too slow or unavailable.");
    });

    connect(reply, &QNetworkReply::finished, this, [=]() {
        timer->stop();
        timer->deleteLater();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
            QString rawResponse = response["response"].toString();
            QString result = extractResultFromMarkdown(rawResponse);
            onSuccess(result);
        } else {
            QString errorMsg = reply->errorString();
            if (reply->error() == QNetworkReply::OperationCanceledError) {
                errorMsg = "Request timed out";
            }
            onError(errorMsg);
        }
        reply->deleteLater();
    });

    timer->start();
}

bool OllamaProvider::isAvailable() const {
    QString error;
    return const_cast<OllamaProvider*>(this)->testConnection(error);
}

QStringList OllamaProvider::listModels(QString& error) {
    QStringList models;

    QNetworkRequest request(QUrl(endpoint + "/api/tags"));
    QNetworkReply* reply = networkManager->get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(5000);

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start();
    loop.exec();

    if (timer.isActive()) {
        timer.stop();

        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
            QJsonArray modelArray = doc.object()["models"].toArray();

            for (const QJsonValue& val : modelArray) {
                QString name = val.toObject()["name"].toString();
                if (!name.isEmpty()) {
                    models.append(name);
                }
            }
        } else {
            error = reply->errorString();
        }
    } else {
        reply->abort();
        error = "Connection timeout";
    }

    reply->deleteLater();
    return models;
}

bool OllamaProvider::testConnection(QString& error) {
    QNetworkRequest request(QUrl(endpoint + "/api/tags"));
    QNetworkReply* reply = networkManager->get(request);

    QEventLoop loop;
    QTimer timer;
    timer.setSingleShot(true);
    timer.setInterval(5000);

    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    timer.start();
    loop.exec();

    bool success = false;

    if (timer.isActive()) {
        timer.stop();
        success = (reply->error() == QNetworkReply::NoError);
        if (!success) {
            error = reply->errorString();
        }
    } else {
        reply->abort();
        error = "Connection timeout";
    }

    reply->deleteLater();
    return success;
}
