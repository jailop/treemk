#include "mainwindow.h"
#include <QMessageBox>

void MainWindow::about() {
  QMessageBox::about(
      this, tr("About TreeMk"),
      tr("<h2>TreeMk</h2>"
         "<p>Version 0.1.0</p>"
         "<p>A wiki-markdown text editor, "
         "designed for organizing and managing interconnected notes.</p>"
         "<p>Copyright © 2025 - Jaime Lopez - Data Inquiry Consulting "
         "LLC</p>"));
}
