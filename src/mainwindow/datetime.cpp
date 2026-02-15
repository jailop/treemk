#include <QDateTime>
#include <QSettings>
#include <QTextCursor>

#include "defs.h"
#include "mainwindow.h"
#include "markdowneditor.h"
#include "tabeditor.h"

void MainWindow::insertDate() {
    // Use the last selected date format
    QSettings settings(APP_LABEL, APP_LABEL);
    QString lastFormat =
        settings.value("editor/lastDateFormat", "yyyy-MM-dd").toString();
    insertDateWithFormat(lastFormat);
}

void MainWindow::insertTime() {
    // Use the last selected time format
    QSettings settings(APP_LABEL, APP_LABEL);
    QString lastFormat =
        settings.value("editor/lastTimeFormat", "HH:mm").toString();
    insertTimeWithFormat(lastFormat);
}

void MainWindow::insertDateWithFormat(const QString& format) {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    // Save the selected format
    QSettings settings(APP_LABEL, APP_LABEL);
    settings.setValue("editor/lastDateFormat", format);

    // Format current date
    QString date = QDate::currentDate().toString(format);

    // Insert at cursor
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(date);
    tab->editor()->setTextCursor(cursor);
    tab->editor()->setFocus();
}

void MainWindow::insertTimeWithFormat(const QString& format) {
    TabEditor* tab = currentTabEditor();
    if (!tab) return;

    // Save the selected format
    QSettings settings(APP_LABEL, APP_LABEL);
    settings.setValue("editor/lastTimeFormat", format);

    // Format current time
    QString time = QTime::currentTime().toString(format);

    // Insert at cursor
    QTextCursor cursor = tab->editor()->textCursor();
    cursor.insertText(time);
    tab->editor()->setTextCursor(cursor);
    tab->editor()->setFocus();
}
