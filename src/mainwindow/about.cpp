#include <QMessageBox>

#include "defs.h"
#include "helpdialog.h"
#include "mainwindow.h"

void MainWindow::about() {
    // Create the about message. Include the APP_LABEL and APP_VERSION
    // constants from defs.h
    QString aboutText = QString("<h2>%1</h2>"
                                "<p>Version %2</p>"
                                "<p>A wiki-markdown text editor, "
                                "designed for organizing and managing interconnected notes.</p>"
                                "<p>© 2025-2026 - <mailto:jailop.lopez@datainquiry.dev>"
                                "Jaime Lopez</a></p>")
                            .arg(APP_LABEL)
                            .arg(APP_VERSION);
    QMessageBox::about(
        this, tr("About TreeMk"),
        tr(aboutText.toStdString().c_str()));
}

void MainWindow::showUserGuide() {
    HelpDialog* helpDialog = new HelpDialog(this);
    helpDialog->setAttribute(Qt::WA_DeleteOnClose);
    helpDialog->show();
}
