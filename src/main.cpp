#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("MkEd");
    app.setOrganizationName("MkEd");
    app.setApplicationVersion("0.1.0");
    
    MainWindow window;
    window.show();
    
    return app.exec();
}
