#ifndef _APPINIT_H
#define _APPINIT_H

/**
 * This file defines functions intended to initialize the application
 * components. These functions are called during the application
 * startup. When the app is not user-configured, the app assumes
 * values based on the system environment and common used values.
 */

#include <QSettings>

/**
 * Initializes the AI providers based on the environment variables and 
 * the application settings. When not configured, the app assumes common
 * used values until the user set up the proper configuration.
 */
void setAIProviders(QSettings& settings);

/**
 * Initializes the theme manager based on the application settings. When not
 * configured, the app assumes a default theme until the user set up the
 * proper configuration.
 */
void setThemeManager(QSettings& settings);

#endif // _APPINIT_H
