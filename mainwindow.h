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
    bool readSettings();
    void connectToDB();

    QSettings *settings;                // settings: to read/write ini file
    QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");

private:

    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;

private slots:

    void ConnectedToServer();
    void NotConnectedToServer();

    void displayInputs();
    void transferData();


    void on_pushButton_clicked();

signals:

    void connectTo();
    void sendData();
};

#endif // MAINWINDOW_H
