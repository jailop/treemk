#ifndef WINDOWMANAGER_H
#define WINDOWMANAGER_H

#include <QList>
#include <QObject>
#include <QString>

class MainWindow;

/**
 * @brief Singleton managing all MainWindow instances
 */
class WindowManager : public QObject {
    Q_OBJECT
    
public:
    static WindowManager* instance();
    
    /**
     * @brief Create new window with optional folder/file
     * @param folder Folder path to open (empty = no folder)
     * @param file File path to open (empty = no file)
     * @return Pointer to created window
     */
    MainWindow* createWindow(const QString &folder = QString(), 
                            const QString &file = QString());
    
    QList<MainWindow*> windows() const { return m_windows; }
    bool hasOpenWindows() const { return !m_windows.isEmpty(); }
    
private:
    WindowManager();
    static WindowManager* s_instance;
    QList<MainWindow*> m_windows;
    
    void onWindowDestroyed(QObject *obj);
};

#endif // WINDOWMANAGER_H
