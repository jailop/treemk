#include "managers/windowmanager.h"
#include "mainwindow.h"
#include <QWidget>

WindowManager* WindowManager::s_instance = nullptr;

WindowManager* WindowManager::instance() {
    if (!s_instance) {
        s_instance = new WindowManager();
    }
    return s_instance;
}

WindowManager::WindowManager() : QObject(nullptr) {
}

MainWindow* WindowManager::createWindow(const QString &folder, const QString &file) {
    MainWindow* window = new MainWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setStartupArguments(folder, file);
    window->initializeSettings();
    
    m_windows.append(window);
    
    connect(window, &QObject::destroyed, this, &WindowManager::onWindowDestroyed);
    
    window->show();
    return window;
}

void WindowManager::onWindowDestroyed(QObject *obj) {
    MainWindow *window = qobject_cast<MainWindow*>(obj);
    if (window) {
        m_windows.removeAll(window);
    }
}
