#include "mainwindow.h"
#include "begindialog.h"
#include <QApplication>

bool appState = false;
begindialog *bd;

int main(int argc, char *argv[]){

    QApplication a(argc, argv);

    bd = new begindialog(NULL);
    bd->exec();
    //qDebug() << clientAddress;

    if (appState) {
        MainWindow w;
        w.show();
        a.exec();
    } else {
        a.exit();
    }

    return 0;
    //return a.exec();
}
