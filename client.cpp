#include "client.h"

#include <iostream>
#include <unistd.h>

using namespace std;

extern QString MSG_CLIENT_CONN;
extern QString clientAddress;

Client::Client(QObject* parent): QObject(parent){

    connect(&clientSocket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
}

Client::~Client(){

    clientSocket.close();
}

void Client::setHost(QString hostAddress, quint16 hostPort){

    clientAddress = hostAddress;
    clientPort = hostPort;
    host.setAddress(clientAddress);

}

void Client::start(){

    clientSocket.connectToHost(host, clientPort);
    //clientSocket.waitForConnected(5000);
}

void Client::startTransfer(){

    if (clientSocket.state() == QAbstractSocket::ConnectedState){
//        clientSocket.write(MSG_HI.toLatin1().constData(), 13);
        clientSocket.write(datagram);
    }
}

void Client::connectionEstablished(){

    connected = true;
    //emit clientConnected();
    //cout << MSG_CLIENT_CONN.toUtf8().constData() << endl;
}
