#include "managers/windowmanager.h"

#include <QApplication>
#include <QCoreApplication>
#include <QProcess>
#include <QScreen>
#include <QWidget>

#include "mainwindow.h"

WindowManager* WindowManager::s_instance = nullptr;

WindowManager* WindowManager::instance() {
    if (!s_instance) {
        s_instance = new WindowManager();
    }
    return s_instance;
}

WindowManager::WindowManager() : QObject(nullptr), m_windowOffset(0) {}

MainWindow* WindowManager::createWindow(const QString& folder,
                                        const QString& file) {
    // Safety check: prevent recursive spawning
    // If TREEMK_NEW_WINDOW env var is set, we're being launched as a new window
    // Don't spawn another one
    if (qEnvironmentVariableIsSet("TREEMK_NEW_WINDOW")) {
        qWarning() << "Already in new window mode, not spawning another";
        return nullptr;
    }
    
    // Launch as a separate process to avoid QtWebEngine multi-window issues
    // Each process gets its own Chromium renderer, avoiding zygote fork problems
    
    QStringList arguments;
    if (!folder.isEmpty()) {
        arguments << folder;
    }
    if (!file.isEmpty()) {
        arguments << file;
    }
    
    QString program = QCoreApplication::applicationFilePath();
    
    // Set environment variable to prevent recursive spawning
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("TREEMK_NEW_WINDOW", "1");
    
    QProcess process;
    process.setProcessEnvironment(env);
    bool started = process.startDetached(program, arguments);
    
    if (!started) {
        qWarning() << "Failed to start new window process";
    }
    
    // Return nullptr since we're not managing the window in this process
    return nullptr;
}

void WindowManager::onWindowDestroyed(QObject* obj) {
    MainWindow* window = qobject_cast<MainWindow*>(obj);
    if (window) {
        m_windows.removeAll(window);
        // Reset offset when all windows are closed
        if (m_windows.isEmpty()) {
            m_windowOffset = 0;
        }
    }
}
