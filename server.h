#ifndef SERVER_H
#define SERVER_H

#include <QtNetwork>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class Server: public QObject{

    Q_OBJECT

public:

    QTcpServer server;
    QTcpSocket* client;

    Server(QObject * parent = 0);
    ~Server();

    int serverPort;
    QByteArray datagram;

    void init();
    void frameMainIO();
    void frameRemote1IO();

public slots:

    void acceptConnection();
    void startRead();

private:


signals:

    void readFinished();
    void mainIO();
    void remote1IO();
    void dataValid();
    void dataInValid();
};

#endif // SERVER_H
