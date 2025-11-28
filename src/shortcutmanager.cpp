#include "shortcutmanager.h"
#include <QSettings>

ShortcutManager* ShortcutManager::m_instance = nullptr;

ShortcutManager* ShortcutManager::instance()
{
    if (!m_instance) {
        m_instance = new ShortcutManager();
    }
    return m_instance;
}

ShortcutManager::ShortcutManager(QObject *parent)
    : QObject(parent)
{
    initializeDefaults();
    loadSettings();
}

void ShortcutManager::initializeDefaults()
{
    // Navigation actions
    m_shortcuts[MoveToStartOfLine] = QKeySequence(Qt::Key_Home);
    m_actionNames[MoveToStartOfLine] = "MoveToStartOfLine";
    m_actionDescriptions[MoveToStartOfLine] = tr("Move to Start of Line");
    m_actionCategories[MoveToStartOfLine] = tr("Navigation");
    
    m_shortcuts[MoveToEndOfLine] = QKeySequence(Qt::Key_End);
    m_actionNames[MoveToEndOfLine] = "MoveToEndOfLine";
    m_actionDescriptions[MoveToEndOfLine] = tr("Move to End of Line");
    m_actionCategories[MoveToEndOfLine] = tr("Navigation");
    
    m_shortcuts[MoveToStartOfDocument] = QKeySequence(Qt::CTRL | Qt::Key_Home);
    m_actionNames[MoveToStartOfDocument] = "MoveToStartOfDocument";
    m_actionDescriptions[MoveToStartOfDocument] = tr("Move to Start of Document");
    m_actionCategories[MoveToStartOfDocument] = tr("Navigation");
    
    m_shortcuts[MoveToEndOfDocument] = QKeySequence(Qt::CTRL | Qt::Key_End);
    m_actionNames[MoveToEndOfDocument] = "MoveToEndOfDocument";
    m_actionDescriptions[MoveToEndOfDocument] = tr("Move to End of Document");
    m_actionCategories[MoveToEndOfDocument] = tr("Navigation");
    
    m_shortcuts[MoveWordLeft] = QKeySequence(Qt::CTRL | Qt::Key_Left);
    m_actionNames[MoveWordLeft] = "MoveWordLeft";
    m_actionDescriptions[MoveWordLeft] = tr("Move One Word Left");
    m_actionCategories[MoveWordLeft] = tr("Navigation");
    
    m_shortcuts[MoveWordRight] = QKeySequence(Qt::CTRL | Qt::Key_Right);
    m_actionNames[MoveWordRight] = "MoveWordRight";
    m_actionDescriptions[MoveWordRight] = tr("Move One Word Right");
    m_actionCategories[MoveWordRight] = tr("Navigation");
    
    m_shortcuts[MoveToPreviousParagraph] = QKeySequence(Qt::CTRL | Qt::Key_Up);
    m_actionNames[MoveToPreviousParagraph] = "MoveToPreviousParagraph";
    m_actionDescriptions[MoveToPreviousParagraph] = tr("Move to Previous Paragraph");
    m_actionCategories[MoveToPreviousParagraph] = tr("Navigation");
    
    m_shortcuts[MoveToNextParagraph] = QKeySequence(Qt::CTRL | Qt::Key_Down);
    m_actionNames[MoveToNextParagraph] = "MoveToNextParagraph";
    m_actionDescriptions[MoveToNextParagraph] = tr("Move to Next Paragraph");
    m_actionCategories[MoveToNextParagraph] = tr("Navigation");
    
    m_shortcuts[MoveUpOnePage] = QKeySequence(Qt::Key_PageUp);
    m_actionNames[MoveUpOnePage] = "MoveUpOnePage";
    m_actionDescriptions[MoveUpOnePage] = tr("Move Up One Page");
    m_actionCategories[MoveUpOnePage] = tr("Navigation");
    
    m_shortcuts[MoveDownOnePage] = QKeySequence(Qt::Key_PageDown);
    m_actionNames[MoveDownOnePage] = "MoveDownOnePage";
    m_actionDescriptions[MoveDownOnePage] = tr("Move Down One Page");
    m_actionCategories[MoveDownOnePage] = tr("Navigation");
    
    // Selection + Navigation
    m_shortcuts[SelectToStartOfLine] = QKeySequence(Qt::SHIFT | Qt::Key_Home);
    m_actionNames[SelectToStartOfLine] = "SelectToStartOfLine";
    m_actionDescriptions[SelectToStartOfLine] = tr("Select to Start of Line");
    m_actionCategories[SelectToStartOfLine] = tr("Selection");
    
    m_shortcuts[SelectToEndOfLine] = QKeySequence(Qt::SHIFT | Qt::Key_End);
    m_actionNames[SelectToEndOfLine] = "SelectToEndOfLine";
    m_actionDescriptions[SelectToEndOfLine] = tr("Select to End of Line");
    m_actionCategories[SelectToEndOfLine] = tr("Selection");
    
    m_shortcuts[SelectToStartOfDocument] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Home);
    m_actionNames[SelectToStartOfDocument] = "SelectToStartOfDocument";
    m_actionDescriptions[SelectToStartOfDocument] = tr("Select to Start of Document");
    m_actionCategories[SelectToStartOfDocument] = tr("Selection");
    
    m_shortcuts[SelectToEndOfDocument] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_End);
    m_actionNames[SelectToEndOfDocument] = "SelectToEndOfDocument";
    m_actionDescriptions[SelectToEndOfDocument] = tr("Select to End of Document");
    m_actionCategories[SelectToEndOfDocument] = tr("Selection");
    
    m_shortcuts[SelectWordLeft] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Left);
    m_actionNames[SelectWordLeft] = "SelectWordLeft";
    m_actionDescriptions[SelectWordLeft] = tr("Select Word Left");
    m_actionCategories[SelectWordLeft] = tr("Selection");
    
    m_shortcuts[SelectWordRight] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Right);
    m_actionNames[SelectWordRight] = "SelectWordRight";
    m_actionDescriptions[SelectWordRight] = tr("Select Word Right");
    m_actionCategories[SelectWordRight] = tr("Selection");
    
    m_shortcuts[SelectToPreviousParagraph] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Up);
    m_actionNames[SelectToPreviousParagraph] = "SelectToPreviousParagraph";
    m_actionDescriptions[SelectToPreviousParagraph] = tr("Select to Previous Paragraph");
    m_actionCategories[SelectToPreviousParagraph] = tr("Selection");
    
    m_shortcuts[SelectToNextParagraph] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Down);
    m_actionNames[SelectToNextParagraph] = "SelectToNextParagraph";
    m_actionDescriptions[SelectToNextParagraph] = tr("Select to Next Paragraph");
    m_actionCategories[SelectToNextParagraph] = tr("Selection");
    
    m_shortcuts[SelectUpOnePage] = QKeySequence(Qt::SHIFT | Qt::Key_PageUp);
    m_actionNames[SelectUpOnePage] = "SelectUpOnePage";
    m_actionDescriptions[SelectUpOnePage] = tr("Select Up One Page");
    m_actionCategories[SelectUpOnePage] = tr("Selection");
    
    m_shortcuts[SelectDownOnePage] = QKeySequence(Qt::SHIFT | Qt::Key_PageDown);
    m_actionNames[SelectDownOnePage] = "SelectDownOnePage";
    m_actionDescriptions[SelectDownOnePage] = tr("Select Down One Page");
    m_actionCategories[SelectDownOnePage] = tr("Selection");
    
    // Editing
    m_shortcuts[DeleteWordLeft] = QKeySequence(Qt::CTRL | Qt::Key_Backspace);
    m_actionNames[DeleteWordLeft] = "DeleteWordLeft";
    m_actionDescriptions[DeleteWordLeft] = tr("Delete Word Left");
    m_actionCategories[DeleteWordLeft] = tr("Editing");
    
    m_shortcuts[DeleteWordRight] = QKeySequence(Qt::CTRL | Qt::Key_Delete);
    m_actionNames[DeleteWordRight] = "DeleteWordRight";
    m_actionDescriptions[DeleteWordRight] = tr("Delete Word Right");
    m_actionCategories[DeleteWordRight] = tr("Editing");
    
    m_shortcuts[DeleteToStartOfLine] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Backspace);
    m_actionNames[DeleteToStartOfLine] = "DeleteToStartOfLine";
    m_actionDescriptions[DeleteToStartOfLine] = tr("Delete to Start of Line");
    m_actionCategories[DeleteToStartOfLine] = tr("Editing");
    
    m_shortcuts[DeleteToEndOfLine] = QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_Delete);
    m_actionNames[DeleteToEndOfLine] = "DeleteToEndOfLine";
    m_actionDescriptions[DeleteToEndOfLine] = tr("Delete to End of Line");
    m_actionCategories[DeleteToEndOfLine] = tr("Editing");
}

QKeySequence ShortcutManager::getShortcut(Action action) const
{
    return m_shortcuts.value(action, QKeySequence());
}

void ShortcutManager::setShortcut(Action action, const QKeySequence &sequence)
{
    m_shortcuts[action] = sequence;
    emit shortcutsChanged();
}

QString ShortcutManager::getActionName(Action action) const
{
    return m_actionNames.value(action, QString());
}

QString ShortcutManager::getActionDescription(Action action) const
{
    return m_actionDescriptions.value(action, QString());
}

QString ShortcutManager::getActionCategory(Action action) const
{
    return m_actionCategories.value(action, QString());
}

QList<ShortcutManager::Action> ShortcutManager::getAllActions() const
{
    return m_shortcuts.keys();
}

void ShortcutManager::loadSettings()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");
    
    for (auto it = m_shortcuts.begin(); it != m_shortcuts.end(); ++it) {
        QString keyName = m_actionNames.value(it.key());
        if (!keyName.isEmpty()) {
            QString savedSequence = settings.value(keyName).toString();
            if (!savedSequence.isEmpty()) {
                it.value() = QKeySequence::fromString(savedSequence);
            }
        }
    }
    
    settings.endGroup();
}

void ShortcutManager::saveSettings()
{
    QSettings settings;
    settings.beginGroup("Shortcuts");
    
    for (auto it = m_shortcuts.constBegin(); it != m_shortcuts.constEnd(); ++it) {
        QString keyName = m_actionNames.value(it.key());
        if (!keyName.isEmpty()) {
            settings.setValue(keyName, it.value().toString());
        }
    }
    
    settings.endGroup();
}

void ShortcutManager::resetToDefaults()
{
    initializeDefaults();
    emit shortcutsChanged();
}
