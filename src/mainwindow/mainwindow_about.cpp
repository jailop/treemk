#include "mainwindow.h"
#include <QMessageBox>

void MainWindow::about() {
  QMessageBox::about(
      this, tr("About TreeMk"),
      tr("<h2>TreeMk</h2>"
         "<p>Version 0.5.0</p>"
         "<p>A wiki-markdown text editor, "
         "designed for organizing and managing interconnected notes.</p>"
         "<p>© 2025-2026 - <mailto:jaime.lopez@datainquiry.dev>Jaime Lopez</a></p>"));
}
