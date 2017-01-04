#ifndef CLIENT_H
#define CLIENT_H

#include <QtNetwork>
#include <QObject>
#include <QString>
#include <QTcpSocket>

class Client: public QObject{

    Q_OBJECT

public:

    Client(QObject* parent = 0);
    ~Client();
    void setHost(QString hostAddress, quint16 hostPort);
    void start();

    QTcpSocket clientSocket;
    quint16 clientPort = 8889;
    QString clientAddress = "locahost";
    QHostAddress host;
    //QString clientAddress = "192.168.1.235"; // BBB
    //QString clientAddress = "192.168.1.124"; // BBBak
    //QString clientAddress = "192.168.2.232"; // BBB
    //QString clientAddress = "192.168.2.241"; // DebianVB
    //QString clientAddress = "192.168.2.222"; // Windows

    bool connected = false;
    QByteArray datagram;

public slots:

    void startTransfer();
    void connectionEstablished();

private:


signals:

    void clientConnected();

};

#endif // CLIENT_H
