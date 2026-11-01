#include "openwithdialog.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QLineEdit>
#include <QListWidget>
#include <QMimeDatabase>
#include <QMimeType>
#include <QProcess>
#include <QSet>
#include <QTextStream>
#include <QVBoxLayout>

#include <algorithm>

namespace {

constexpr int ExecRole = Qt::UserRole;
constexpr int MimeTypesRole = Qt::UserRole + 1;

struct AppEntry {
    QString name;
    QString exec;
    QStringList mimeTypes;
};

#ifdef Q_OS_MACOS

QList<AppEntry> availableApplications() {
    QList<AppEntry> apps;
    const QStringList dirs = {
        "/Applications",
        "/System/Applications",
        "/System/Applications/Utilities",
        QDir::homePath() + "/Applications",
    };
    QSet<QString> seen;
    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        const QFileInfoList entries =
            dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QFileInfo& fi : entries) {
            if (fi.suffix().compare("app", Qt::CaseInsensitive) != 0) {
                continue;
            }
            const QString name = fi.completeBaseName();
            if (seen.contains(name)) {
                continue;
            }
            seen.insert(name);
            apps.append({name, fi.absoluteFilePath(), {}});
        }
    }
    return apps;
}

void launchApplication(const QString& exec, const QString& filePath) {
    QProcess::startDetached("/usr/bin/open",
                            QStringList() << "-a" << exec << filePath);
}

#else

QList<AppEntry> availableApplications() {
    QList<AppEntry> apps;
    QSet<QString> seen;

    QStringList dirs;
    const QString dataHome = qEnvironmentVariable(
        "XDG_DATA_HOME", QDir::homePath() + "/.local/share");
    dirs << dataHome + "/applications";
    const QStringList dataDirs =
        qEnvironmentVariable("XDG_DATA_DIRS", "/usr/local/share:/usr/share")
            .split(':', Qt::SkipEmptyParts);
    for (const QString& d : dataDirs) {
        dirs << d + "/applications";
    }

    for (const QString& dirPath : dirs) {
        QDir dir(dirPath);
        const QFileInfoList entries =
            dir.entryInfoList(QStringList() << "*.desktop", QDir::Files);
        for (const QFileInfo& fi : entries) {
            QFile file(fi.filePath());
            if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                continue;
            }
            QString name;
            QString exec;
            QStringList mimeTypes;
            bool hidden = false;
            bool inEntry = false;
            QTextStream ts(&file);
            while (!ts.atEnd()) {
                const QString line = ts.readLine().trimmed();
                if (line == "[Desktop Entry]") {
                    inEntry = true;
                    continue;
                }
                if (!inEntry) {
                    continue;
                }
                if (line.startsWith("Name=") && name.isEmpty()) {
                    name = line.mid(5);
                } else if (line.startsWith("Exec=")) {
                    exec = line.mid(5);
                } else if (line.startsWith("MimeType=")) {
                    mimeTypes +=
                        line.mid(9).split(';', Qt::SkipEmptyParts);
                } else if (line == "NoDisplay=true" || line == "Hidden=true") {
                    hidden = true;
                }
            }
            if (name.isEmpty() || exec.isEmpty() || hidden) {
                continue;
            }
            if (seen.contains(name)) {
                continue;
            }
            seen.insert(name);
            apps.append({name, exec, mimeTypes});
        }
    }

    std::sort(apps.begin(), apps.end(),
              [](const AppEntry& a, const AppEntry& b) {
                  return a.name.localeAwareCompare(b.name) < 0;
              });
    return apps;
}

void launchApplication(const QString& exec, const QString& filePath) {
    QStringList tokens = QProcess::splitCommand(exec);
    if (tokens.isEmpty()) {
        return;
    }
    const QString program = tokens.takeFirst();
    QStringList args;
    bool hasFileToken = false;
    for (const QString& token : tokens) {
        if (token == "%f" || token == "%F" || token == "%u" || token == "%U") {
            args << filePath;
            hasFileToken = true;
        } else if (!token.startsWith('%')) {
            args << token;
        }
    }
    if (!hasFileToken) {
        args << filePath;
    }
    QProcess::startDetached(program, args);
}

#endif

QStringList mimeCandidatesForFile(const QString& filePath) {
    QMimeDatabase db;
    const QMimeType mime = db.mimeTypeForFile(filePath);
    if (!mime.isValid()) {
        return {};
    }
    QStringList candidates;
    candidates << mime.name();
    candidates << mime.aliases();
    for (const QString& parent : mime.parentMimeTypes()) {
        candidates << parent;
    }
    candidates.removeDuplicates();
    return candidates;
}

bool supportsMime(const QStringList& appMimes, const QStringList& fileMimes) {
    for (const QString& declared : appMimes) {
        if (declared.endsWith("/*")) {
            const QString prefix = declared.left(declared.indexOf('/') + 1);
            for (const QString& candidate : fileMimes) {
                if (candidate.startsWith(prefix)) {
                    return true;
                }
            }
        } else if (fileMimes.contains(declared)) {
            return true;
        }
    }
    return false;
}

}  // namespace

OpenWithDialog::OpenWithDialog(const QString& filePath, QWidget* parent)
    : QDialog(parent), filePath(filePath) {
    setWindowTitle(tr("Open With..."));
    resize(420, 460);

#ifndef Q_OS_MACOS
    fileMimeTypes = mimeCandidatesForFile(filePath);
#endif

    searchEdit = new QLineEdit(this);
    searchEdit->setPlaceholderText(tr("Filter applications"));

    appList = new QListWidget(this);

    showAllCheck = new QCheckBox(tr("Show all applications"), this);
#ifndef Q_OS_MACOS
    showAllCheck->setChecked(fileMimeTypes.isEmpty());
#else
    showAllCheck->hide();
#endif

    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Open |
                                         QDialogButtonBox::Cancel, this);

    auto* layout = new QVBoxLayout(this);
    layout->addWidget(searchEdit);
    layout->addWidget(appList);
    layout->addWidget(showAllCheck);
    layout->addWidget(buttons);

    populate();
    applyFilter();

    connect(buttons, &QDialogButtonBox::accepted, this,
            &OpenWithDialog::launchCurrent);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(appList, &QListWidget::itemDoubleClicked, this,
            &OpenWithDialog::launchCurrent);
    connect(searchEdit, &QLineEdit::textChanged, this,
            &OpenWithDialog::applyFilter);
    connect(showAllCheck, &QCheckBox::toggled, this,
            &OpenWithDialog::applyFilter);
}

void OpenWithDialog::populate() {
    appList->clear();
    const QList<AppEntry> apps = availableApplications();
    for (const AppEntry& app : apps) {
        auto* item = new QListWidgetItem(app.name, appList);
        item->setData(ExecRole, app.exec);
        item->setData(MimeTypesRole, app.mimeTypes);
    }
}

void OpenWithDialog::applyFilter() {
    const QString text = searchEdit->text();
    const bool showAll = showAllCheck->isChecked() || fileMimeTypes.isEmpty();
    int firstVisible = -1;

    for (int i = 0; i < appList->count(); ++i) {
        QListWidgetItem* item = appList->item(i);
        const bool nameMatches =
            text.isEmpty() ||
            item->text().contains(text, Qt::CaseInsensitive);
        const bool mimeMatches =
            showAll ||
            supportsMime(item->data(MimeTypesRole).toStringList(),
                         fileMimeTypes);
        item->setHidden(!nameMatches || !mimeMatches);
        if (!item->isHidden() && firstVisible < 0) {
            firstVisible = i;
        }
    }

    appList->setCurrentRow(firstVisible);
}

void OpenWithDialog::launchCurrent() {
    QListWidgetItem* item = appList->currentItem();
    if (!item || item->isHidden()) {
        return;
    }
    launchApplication(item->data(ExecRole).toString(), filePath);
    accept();
}
