#include "server.h"
#include "client.h"

#include <iostream>
#include <unistd.h>

using namespace std;

extern QString MSG_SERVER_INIT;
extern int dInpSize;
extern int dInpSizeUsed;
extern char dInpArr[];
extern int dOutSize;
extern const int dOutSizeUsed;
extern char dOutReadArr[];
extern int aInpArr[];
extern const int aInpSizeUsed;
extern int dataBufferSizeMax;
extern int dataBufferSizeMin;

Server::Server(QObject* parent): QObject(parent){
}

void Server::init(){
    connect(&server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));
    server.listen(QHostAddress::Any, serverPort);
    cout << MSG_SERVER_INIT.toUtf8().constData() << serverPort << endl;
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
    //while (client->waitForReadyRead(300)) {
    //cout << client->bytesAvailable() << "--";
    while (client->bytesAvailable() > 0) {
        datagram.append(client->readAll());
        client->flush();
    }
    //}

    bool validData = !datagram.isEmpty() && datagram.size() >= dataBufferSizeMin && datagram.size() <= dataBufferSizeMax && datagram.at(datagram.size()-1) == 'Z';

    // DI (&& DO) buffer check
    if (validData) {
        for (int k=0; k<dInpSizeUsed; k++) {   // +dOutSizeUsed
            if ( !QChar(datagram.at(k)).isDigit() ) {
                validData = false;
                cout << datagram.at(k);
            }
        }
    }

    if (validData) {

        emit dataValid();

        for (int i = 0; i < dInpSizeUsed; i++) {
            dInpArr[i] = datagram.data()[i];
        }

        //for (int i = 0; i < dOutSize; i++) { dOutReadArr[i] = datagram.data()[i + dInpSize]; }

        int pos = dInpSizeUsed;         //+dOutSizeUsed;
        char ch = datagram.at(pos);
        //cout << ch << endl;       //DBG
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
                if ( x < aInpSizeUsed ) {
                    aInpArr[x] = atoi(temp);
                    //cout << " " << aInpArr[x];
                } else
                    break;
                x++;
            }
        }
        //cout << endl;
        emit this->readFinished();
    } else {
        if (!datagram.isEmpty()) {
            cout << datagram.data() << endl;      //DBG
            emit dataInValid();
        }
    }

    //client->close();
}

/* old fn
void Server::startRead(){
    datagram.clear();
    while (client->bytesAvailable())
        datagram.append(client->readAll());
    //qDebug() << datagram.data();// << endl;      //DBG

    for (int i = 0; i < dInpSizeUsed; i++) { dInpArr[i] = datagram.data()[i]; }

    //for (int i = 0; i < dOutSize; i++) { dOutReadArr[i] = datagram.data()[i + dInpSize]; }

    int pos = dInpSizeUsed;//+dOutSize;
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
*/
