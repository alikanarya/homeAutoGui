#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QtSql>
#include <QtSql/QSql>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlDriver>
#include <QDebug>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

private:

    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;

private slots:

    void ConnectedToServer();
    void NotConnectedToServer();

    void displayInputs();
    void transferData();


signals:

    void connectTo();
    void sendData();
};

#endif // MAINWINDOW_H
