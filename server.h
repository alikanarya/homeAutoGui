#ifndef SERVER_H
#define SERVER_H

#include <QtNetwork>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Server: public QObject{

    Q_OBJECT

public:

    Server(QObject * parent = 0);
    ~Server();

    int serverPort = 8888;
    QByteArray datagram;

public slots:

    void acceptConnection();
    void startRead();

private:

    QTcpServer server;
    QTcpSocket* client;

signals:

    void readFinished();
};

#endif // SERVER_H
