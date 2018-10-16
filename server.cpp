#include "server.h"
#include "client.h"

#include <iostream>
#include <unistd.h>

using namespace std;

extern QString MSG_SERVER_INIT;
extern int dInpSize;
extern int dInpNum;
extern char dInpArr[];
extern int dOutSize;
extern char dOutReadArr[];
extern int aInpArr[];

Server::Server(QObject* parent): QObject(parent){

    connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    server.listen(QHostAddress::Any, serverPort);
    cout << MSG_SERVER_INIT.toUtf8().constData() << endl;
}

Server::~Server(){

    server.close();
}

void Server::acceptConnection(){

    //cout <<"acceptConnection" << endl;    //DBG
    client = server.nextPendingConnection();
    connect(client, SIGNAL(readyRead()), this, SLOT(startRead()));
}

void Server::startRead(){

    datagram.clear();
    while (client->bytesAvailable())
        datagram.append(client->readAll());
    //qDebug() << datagram.data();// << endl;      //DBG

    for (int i = 0; i < dInpNum; i++) {
        dInpArr[i] = datagram.data()[i];
    }

    /*
    for (int i = 0; i < dOutSize; i++) {
        dOutReadArr[i] = datagram.data()[i + dInpSize];
    }
    */

    int pos = dInpNum;//+dOutSize;
    char ch = datagram.at(pos);
    //cout << ch << endl;      //DBG

    int j = 0, x = 0;
    while (ch != 'Z') {
        //x = 0;
        if (ch == 'A') {
            char temp[16];
            j = -1;
            do {
                pos++;
                ch = datagram.at(pos);
                //cout << " " << ch;
                if (ch == 'Z') break;
                if (ch == 'A') break;
                j++;
                temp[j] = ch;

            } while ( ch != 'Z' );

            temp [j+1] = '\0';
            aInpArr[x] = atoi(temp);
            //cout << " " << aInpArr[i];
            x++;

        }
    }
    //cout << endl;

    emit this->readFinished();
    //client->close();
}
