#ifndef SYSTEMPROMPTS_H
#define SYSTEMPROMPTS_H

#include <QString>
#include <QList>

struct SystemPrompt {
  QString id;
  QString name;
  QString prompt;
  bool enabled;
  bool isCustom;
  int order;
  
  SystemPrompt() : enabled(true), isCustom(false), order(0) {}
};

class SystemPrompts {
public:
  static SystemPrompts* instance();
  
  QList<SystemPrompt> getAllPrompts() const;
  QList<SystemPrompt> getEnabledPrompts() const;
  SystemPrompt getPromptById(const QString &id) const;
  
  void setPromptEnabled(const QString &id, bool enabled);
  void addCustomPrompt(const QString &name, const QString &prompt);
  void removeCustomPrompt(const QString &id);
  void updateCustomPrompt(const QString &id, const QString &name, const QString &prompt);
  void reorderPrompts(const QList<SystemPrompt> &newOrder);
  
  void loadFromSettings();
  void saveToSettings();
  
private:
  SystemPrompts();
  void initializeDefaults();
  QString generateId() const;
  
  QList<SystemPrompt> prompts;
  static SystemPrompts* m_instance;
};

#endif // SYSTEMPROMPTS_H
