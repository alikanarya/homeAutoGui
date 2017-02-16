#include "mainwindow.h"
#include "begindialog.h"
#include <QApplication>


int main(int argc, char *argv[]){

    QApplication a(argc, argv);

    begindialog *bd = new begindialog(NULL);
    bd->exec();

    //qDebug() << clientAddress;

    if (bd->state) {
        MainWindow w;
        w.show();
        a.exec();
    } else {
        a.exit();
    }

    //return a.exec();
    return 0;
}
