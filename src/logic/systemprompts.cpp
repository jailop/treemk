#include "logic/systemprompts.h"

#include <QSettings>
#include <QUuid>
#include <algorithm>

SystemPrompts* SystemPrompts::m_instance = nullptr;

SystemPrompts::SystemPrompts() {
    initializeDefaults();
    loadFromSettings();
}

SystemPrompts* SystemPrompts::instance() {
    if (!m_instance) {
        m_instance = new SystemPrompts();
    }
    return m_instance;
}

void SystemPrompts::initializeDefaults() {
    prompts.clear();

    int order = 0;

    SystemPrompt p1;
    p1.id = "rephrase";
    p1.name = "Rephrase";
    p1.prompt = "Rephrase the following text while maintaining its meaning:";
    p1.enabled = true;
    p1.isCustom = false;
    p1.order = order++;
    prompts.append(p1);

    SystemPrompt p2;
    p2.id = "grammar";
    p2.name = "Fix Grammar & Spelling";
    p2.prompt = "Fix any grammar and spelling errors in the following text:";
    p2.enabled = true;
    p2.isCustom = false;
    p2.order = order++;
    prompts.append(p2);

    SystemPrompt p3;
    p3.id = "shorter";
    p3.name = "Make Shorter";
    p3.prompt =
        "Make the following text more concise while keeping key information:";
    p3.enabled = true;
    p3.isCustom = false;
    p3.order = order++;
    prompts.append(p3);

    SystemPrompt p4;
    p4.id = "longer";
    p4.name = "Make Longer";
    p4.prompt = "Expand the following text with more details and examples:";
    p4.enabled = true;
    p4.isCustom = false;
    p4.order = order++;
    prompts.append(p4);

    SystemPrompt p5;
    p5.id = "simplify";
    p5.name = "Simplify Language";
    p5.prompt =
        "Simplify the language in the following text for easier understanding:";
    p5.enabled = true;
    p5.isCustom = false;
    p5.order = order++;
    prompts.append(p5);

    SystemPrompt p6;
    p6.id = "professional";
    p6.name = "Professional Tone";
    p6.prompt = "Rewrite the following text in a professional, formal tone:";
    p6.enabled = true;
    p6.isCustom = false;
    p6.order = order++;
    prompts.append(p6);

    SystemPrompt p7;
    p7.id = "casual";
    p7.name = "Casual Tone";
    p7.prompt = "Rewrite the following text in a casual, friendly tone:";
    p7.enabled = true;
    p7.isCustom = false;
    p7.order = order++;
    prompts.append(p7);

    SystemPrompt p8;
    p8.id = "summarize";
    p8.name = "Summarize";
    p8.prompt = "Provide a concise summary of the following text:";
    p8.enabled = true;
    p8.isCustom = false;
    p8.order = order++;
    prompts.append(p8);

    SystemPrompt p9;
    p9.id = "translate_es";
    p9.name = "Translate to Spanish";
    p9.prompt = "Translate the following text to Spanish:";
    p9.enabled = true;
    p9.isCustom = false;
    p9.order = order++;
    prompts.append(p9);

    SystemPrompt p10;
    p10.id = "add_examples";
    p10.name = "Add Examples";
    p10.prompt =
        "Add relevant examples to illustrate the points in the following text:";
    p10.enabled = true;
    p10.isCustom = false;
    p10.order = order++;
    prompts.append(p10);
}

QList<SystemPrompt> SystemPrompts::getAllPrompts() const { return prompts; }

QList<SystemPrompt> SystemPrompts::getEnabledPrompts() const {
    QList<SystemPrompt> enabled;
    for (const SystemPrompt& p : prompts) {
        if (p.enabled) {
            enabled.append(p);
        }
    }
    return enabled;
}

SystemPrompt SystemPrompts::getPromptById(const QString& id) const {
    for (const SystemPrompt& p : prompts) {
        if (p.id == id) {
            return p;
        }
    }
    return SystemPrompt();
}

void SystemPrompts::setPromptEnabled(const QString& id, bool enabled) {
    for (SystemPrompt& p : prompts) {
        if (p.id == id) {
            p.enabled = enabled;
            break;
        }
    }
}

QString SystemPrompts::generateId() const {
    return "custom_" + QUuid::createUuid().toString(QUuid::WithoutBraces);
}

void SystemPrompts::addCustomPrompt(const QString& name,
                                    const QString& prompt) {
    SystemPrompt p;
    p.id = generateId();
    p.name = name;
    p.prompt = prompt;
    p.enabled = true;
    p.isCustom = true;
    p.order = prompts.size();
    prompts.append(p);
}

void SystemPrompts::removeCustomPrompt(const QString& id) {
    for (int i = 0; i < prompts.size(); ++i) {
        if (prompts[i].id == id && prompts[i].isCustom) {
            prompts.removeAt(i);
            break;
        }
    }
}

void SystemPrompts::updateCustomPrompt(const QString& id, const QString& name,
                                       const QString& prompt) {
    for (SystemPrompt& p : prompts) {
        if (p.id == id && p.isCustom) {
            p.name = name;
            p.prompt = prompt;
            break;
        }
    }
}

void SystemPrompts::reorderPrompts(const QList<SystemPrompt>& newOrder) {
    prompts = newOrder;
    for (int i = 0; i < prompts.size(); ++i) {
        prompts[i].order = i;
    }
}

void SystemPrompts::loadFromSettings() {
    QSettings settings;

    prompts.erase(std::remove_if(prompts.begin(), prompts.end(),
                                  [](const SystemPrompt& p) { return p.isCustom; }),
                  prompts.end());

    for (SystemPrompt& p : prompts) {
        QString key = QString("ai/prompts/%1/enabled").arg(p.id);
        p.enabled = settings.value(key, true).toBool();

        QString orderKey = QString("ai/prompts/%1/order").arg(p.id);
        if (settings.contains(orderKey)) {
            p.order = settings.value(orderKey, p.order).toInt();
        }
    }

    int customCount = settings.beginReadArray("ai/custom_prompts");
    QList<SystemPrompt> customPrompts;
    for (int i = 0; i < customCount; ++i) {
        settings.setArrayIndex(i);

        SystemPrompt p;
        p.id = settings.value("id").toString();
        p.name = settings.value("name").toString();
        p.prompt = settings.value("prompt").toString();
        p.enabled = settings.value("enabled", true).toBool();
        p.isCustom = true;
        p.order = settings.value("order", prompts.size() + i).toInt();

        if (!p.id.isEmpty() && !p.name.isEmpty() && !p.prompt.isEmpty()) {
            customPrompts.append(p);
        }
    }
    settings.endArray();

    prompts.append(customPrompts);

    std::sort(prompts.begin(), prompts.end(),
              [](const SystemPrompt& a, const SystemPrompt& b) {
                  return a.order < b.order;
              });
}

void SystemPrompts::saveToSettings() {
    QSettings settings;

    // Save built-in prompts (enabled state and order)
    for (const SystemPrompt& p : prompts) {
        if (!p.isCustom) {
            QString enabledKey = QString("ai/prompts/%1/enabled").arg(p.id);
            settings.setValue(enabledKey, p.enabled);

            QString orderKey = QString("ai/prompts/%1/order").arg(p.id);
            settings.setValue(orderKey, p.order);
        }
    }

    // Save custom prompts
    QList<SystemPrompt> customPrompts;
    for (const SystemPrompt& p : prompts) {
        if (p.isCustom) {
            customPrompts.append(p);
        }
    }

    settings.beginWriteArray("ai/custom_prompts", customPrompts.size());
    for (int i = 0; i < customPrompts.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("id", customPrompts[i].id);
        settings.setValue("name", customPrompts[i].name);
        settings.setValue("prompt", customPrompts[i].prompt);
        settings.setValue("enabled", customPrompts[i].enabled);
        settings.setValue("order", customPrompts[i].order);
    }
    settings.endArray();
}
