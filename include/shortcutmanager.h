#ifndef SHORTCUTMANAGER_H
#define SHORTCUTMANAGER_H

#include <QObject>
#include <QKeySequence>
#include <QMap>
#include <QString>

class ShortcutManager : public QObject
{
    Q_OBJECT

public:
    enum Action {
        // Navigation
        MoveToStartOfLine,
        MoveToEndOfLine,
        MoveToStartOfDocument,
        MoveToEndOfDocument,
        MoveWordLeft,
        MoveWordRight,
        MoveToPreviousParagraph,
        MoveToNextParagraph,
        MoveUpOnePage,
        MoveDownOnePage,
        
        // Selection + Navigation
        SelectToStartOfLine,
        SelectToEndOfLine,
        SelectToStartOfDocument,
        SelectToEndOfDocument,
        SelectWordLeft,
        SelectWordRight,
        SelectToPreviousParagraph,
        SelectToNextParagraph,
        SelectUpOnePage,
        SelectDownOnePage,
        
        // Editing
        DeleteWordLeft,
        DeleteWordRight,
        DeleteToStartOfLine,
        DeleteToEndOfLine
    };
    Q_ENUM(Action)
    
    static ShortcutManager* instance();
    
    QKeySequence getShortcut(Action action) const;
    void setShortcut(Action action, const QKeySequence &sequence);
    QString getActionName(Action action) const;
    QString getActionDescription(Action action) const;
    QString getActionCategory(Action action) const;
    QList<Action> getAllActions() const;
    
    void loadSettings();
    void saveSettings();
    void resetToDefaults();

signals:
    void shortcutsChanged();

private:
    explicit ShortcutManager(QObject *parent = nullptr);
    void initializeDefaults();
    
    static ShortcutManager *m_instance;
    QMap<Action, QKeySequence> m_shortcuts;
    QMap<Action, QString> m_actionNames;
    QMap<Action, QString> m_actionDescriptions;
    QMap<Action, QString> m_actionCategories;
};

#endif // SHORTCUTMANAGER_H
