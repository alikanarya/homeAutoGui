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
    void zoneQuery(int zoneNumber);

    QSettings *settings;                // settings: to read/write ini file
    ThreadProgressDialog *progress;

    QFont fontBold;

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
    void allZonesTable();
    void zoneTable();
    void summaryResult();

    void on_pushButton_clicked();
    void on_saveAllZonesButton_clicked();
    void on_zoneButton_clicked();
    void on_zoneSaveButton_clicked();
    void on_timeUpButton_clicked();
    void on_timeDownButton_clicked();
    void on_report2DBButton_clicked();

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
