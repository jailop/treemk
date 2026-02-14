#include "managers/windowmanager.h"

#include <QApplication>
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
    MainWindow* window = new MainWindow();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->setStartupArguments(folder, file);
    window->initializeSettings();

    m_windows.append(window);

    connect(window, &QObject::destroyed, this,
            &WindowManager::onWindowDestroyed);

    window->show();

    // Apply cascade offset for new windows
    if (m_windows.count() > 1) {
        QPoint offset(30 * m_windowOffset, 30 * m_windowOffset);
        window->move(window->pos() + offset);

        // Increment and wrap offset to keep windows on screen
        m_windowOffset = (m_windowOffset + 1) % 10;
    } else {
        m_windowOffset = 1;  // Start offsetting from second window
    }

    return window;
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
