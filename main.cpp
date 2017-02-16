#include "mainwindow.h"
#include "begindialog.h"
#include <QApplication>


int main(int argc, char *argv[]){

    QApplication a(argc, argv);



    begindialog *bd = new begindialog(NULL);



    bd->exec();



    MainWindow w;
    w.show();

    return a.exec();
}
