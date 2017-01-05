#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QDebug>
#include <QProgressDialog>

namespace Ui {
class MainWindow;
}

class ThreadProgressDialog;

class MainWindow : public QMainWindow{

    Q_OBJECT

public:

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool readSettings();

    QSettings *settings;                // settings: to read/write ini file
    ThreadProgressDialog *progress;

private:

    Ui::MainWindow *ui;
    QUdpSocket *udpSocket;

private slots:

    void ConnectedToServer();
    void NotConnectedToServer();
    void connectedToDB();
    void unconnectedToDB();

    void displayInputs();
    void transferData();


    void on_pushButton_clicked();

signals:

    void connectTo();
    void sendData();
};




class ThreadProgressDialog : public QProgressDialog {

    Q_OBJECT

public:

    explicit ThreadProgressDialog(QWidget* parent = 0) : QProgressDialog(parent), m_timer(new QTimer(this)) {

        setRange(0, 0);
        setCancelButton(0);
//        setWindowFlags(this->windowFlags() & ~Qt::WindowCloseButtonHint);
//        setWindowFlags(Qt:: Dialog | Qt:: FramelessWindowHint | Qt:: WindowTitleHint | Qt:: CustomizeWindowHint);
        setWindowFlags(Qt:: Dialog | Qt:: WindowTitleHint | Qt:: CustomizeWindowHint);
        setWindowModality(Qt::WindowModal);
        m_timer->setInterval(500);
        connect(m_timer, SIGNAL(timeout()), this, SLOT(advance()));
    }

public slots:

    void threadStarted() {
        m_timer->start();
        show();
    }

    void threadFinished() {
        m_timer->stop();
        hide();
    }

private:

    QTimer *m_timer;

private slots:

    void advance() {
        setValue(value() + 1);
    }

};

#endif // MAINWINDOW_H
